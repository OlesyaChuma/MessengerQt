#include "ClientSession.h"
#include "ChatServer.h"
#include "Database.h"
#include "AuthService.h"

#include <QDebug>
#include <QHostAddress>
#include <QJsonArray>

namespace messenger::server {

ClientSession::ClientSession(QTcpSocket* socket,
                             ChatServer* server,
                             Database* db,
                             AuthService* auth,
                             QObject* parent)
    : QObject(parent),
      _socket(socket),
      _server(server),
      _db(db),
      _auth(auth) {
    Q_ASSERT(socket && server && db && auth);

    connect(_socket, &QTcpSocket::readyRead,
            this, &ClientSession::onReadyRead);
    connect(_socket, &QTcpSocket::disconnected,
            this, &ClientSession::onDisconnected);
}

ClientSession::~ClientSession() = default;

QString ClientSession::peerAddress() const {
    return _socket ? _socket->peerAddress().toString() : QString();
}

void ClientSession::send(const Envelope& env) {
    if (!_socket || _socket->state() != QAbstractSocket::ConnectedState) return;
    const QByteArray frame = FrameCodec::encode(env.toBytes());
    _socket->write(frame);
}

void ClientSession::kick(const QString& reason) {
    send(make::kickedEvent(reason));
    if (_socket) {
        _socket->flush();
        _socket->disconnectFromHost();
    }
}

void ClientSession::notifyBannedAndClose(const QString& reason) {
    send(make::bannedEvent(reason));
    if (_socket) {
        _socket->flush();
        _socket->disconnectFromHost();
    }
}

void ClientSession::onReadyRead() {
    if (!_socket) return;

    if (_codec.hasError()) {
        // Поток испорчен, рвём соединение
        _socket->disconnectFromHost();
        return;
    }

    const auto frames = _codec.feed(_socket->readAll());
    for (const auto& payload : frames) {
        bool ok = false;
        const Envelope env = Envelope::fromBytes(payload, &ok);
        if (!ok) {
            sendError(ResultCode::InvalidRequest, "Malformed JSON", 0);
            continue;
        }
        handleEnvelope(env);
    }
}

void ClientSession::onDisconnected() {
    emit disconnectedFromPeer();
}

void ClientSession::handleEnvelope(const Envelope& env) {
    // Команды, доступные неавторизованному клиенту
    if (env.cmd() == CommandType::Register) { handleRegister(env); return; }
    if (env.cmd() == CommandType::Login)    { handleLogin(env);    return; }

    // Дальше — только для авторизованных
    if (!isAuthenticated()) {
        sendError(ResultCode::AuthRequired, "Login required", env.requestId());
        return;
    }

    switch (env.cmd()) {
        case CommandType::Logout:        handleLogout(env);        break;
        case CommandType::SendMessage:   handleSendMessage(env);   break;
        case CommandType::EditMessage:   handleEditMessage(env);   break;
        case CommandType::DeleteMessage: handleDeleteMessage(env); break;
        case CommandType::GetHistory:    handleGetHistory(env);    break;
        case CommandType::GetUsers:      handleGetUsers(env);      break;
        default:
            sendError(ResultCode::InvalidRequest,
                      "Unknown or unsupported command", env.requestId());
            break;
    }
}

// ---------- Команды ----------

void ClientSession::handleRegister(const Envelope& env) {
    const auto& p = env.payload();
    const auto login        = p.value("login").toString();
    const auto displayName  = p.value("display_name").toString();
    const auto password     = p.value("password").toString();

    auto result = _auth->registerUser(login, displayName, password);
    if (auto* code = std::get_if<ResultCode>(&result)) {
        const QString desc = (*code == ResultCode::LoginAlreadyTaken)
            ? "Login is already taken"
            : "Cannot register user";
        sendError(*code, desc, env.requestId());
        return;
    }

    const auto& rec = std::get<UserRecord>(result);
    QJsonObject ok = {
        {"user", userToJson(rec.toPublicUser())}
    };
    send(make::okResponse(ok, env.requestId()));
}

void ClientSession::handleLogin(const Envelope& env) {
    const auto& p = env.payload();
    const auto loginStr = p.value("login").toString();
    const auto password = p.value("password").toString();

    auto result = _auth->login(loginStr, password);
    if (auto* code = std::get_if<ResultCode>(&result)) {
        const QString desc = (*code == ResultCode::UserBanned)
            ? "You are blocked"
            : "Wrong login or password";
        sendError(*code, desc, env.requestId());

        if (*code == ResultCode::UserBanned) {
            // Дополнительно отправим явное событие Banned
            send(make::bannedEvent(desc));
            if (_socket) _socket->disconnectFromHost();
        }
        return;
    }

    const auto& rec = std::get<UserRecord>(result);
    _userId = rec.id;
    _login  = rec.login;
    _role   = rec.role;

    _db->logConnectionEvent(_userId, _login, peerAddress(),
                            ConnectionEvent::Login);

    QJsonObject ok = {
        {"user", userToJson(rec.toPublicUser())}
    };
    send(make::okResponse(ok, env.requestId()));

    emit authenticated(_userId, _login);
}

void ClientSession::handleLogout(const Envelope& env) {
    _db->logConnectionEvent(_userId, _login, peerAddress(),
                            ConnectionEvent::Logout);
    send(make::okResponse({}, env.requestId()));
    if (_socket) _socket->disconnectFromHost();
}

void ClientSession::enrichMessage(Message& m) const {
    if (m.senderLogin.isEmpty()) {
        if (auto rec = _db->findUserById(m.senderId)) {
            m.senderLogin = rec->login;
        }
    }
}

void ClientSession::handleSendMessage(const Envelope& env) {
    const auto& p = env.payload();
    const qint64 receiverId = p.value("receiver_id").toVariant().toLongLong();
    const bool   broadcast  = p.value("is_broadcast").toBool();
    const QString body      = p.value("body").toString();

    Attachment att;
    if (p.contains("attachment")) {
        att = attachmentFromJson(p.value("attachment").toObject());
        if (att.sizeBytes > kMaxAttachmentSize ||
            att.data.size() > kMaxAttachmentSize) {
            sendError(ResultCode::AttachmentTooLarge,
                      "Attachment exceeds 10 MB limit", env.requestId());
            return;
        }
    }

    if (!broadcast && receiverId <= 0) {
        sendError(ResultCode::InvalidRequest,
                  "receiver_id is required for private message",
                  env.requestId());
        return;
    }

    if (!broadcast) {
        if (!_db->findUserById(receiverId)) {
            sendError(ResultCode::UserNotFound, "Recipient not found",
                      env.requestId());
            return;
        }
    }

    const qint64 messageId = _db->saveMessage(_userId,
                                              broadcast ? 0 : receiverId,
                                              broadcast, body, att);
    if (messageId <= 0) {
        sendError(ResultCode::DatabaseError, "Cannot save message",
                  env.requestId());
        return;
    }

    auto saved = _db->findMessageById(messageId);
    if (!saved) {
        sendError(ResultCode::InternalError, "Cannot fetch saved message",
                  env.requestId());
        return;
    }
    Message msg = *saved;
    enrichMessage(msg);
    msg.attachment = att; // отдадим вложение получателям как пришло

    // Ответ автору
    QJsonObject ok = {{"message", messageToJson(msg)}};
    send(make::okResponse(ok, env.requestId()));

    // Раздача push-события
    _server->dispatchNewMessage(msg, this);
}

void ClientSession::handleEditMessage(const Envelope& env) {
    const auto& p = env.payload();
    const qint64 messageId = p.value("message_id").toVariant().toLongLong();
    const QString newBody  = p.value("body").toString();

    auto existing = _db->findMessageById(messageId);
    if (!existing) {
        sendError(ResultCode::MessageNotFound, "Message not found",
                  env.requestId());
        return;
    }
    if (existing->senderId != _userId) {
        sendError(ResultCode::NotMessageAuthor,
                  "You can only edit your own messages", env.requestId());
        return;
    }

    if (!_db->editMessageBody(messageId, _userId, newBody)) {
        sendError(ResultCode::DatabaseError, "Cannot edit message",
                  env.requestId());
        return;
    }

    auto updated = _db->findMessageById(messageId);
    if (!updated) {
        sendError(ResultCode::InternalError, "Cannot fetch edited message",
                  env.requestId());
        return;
    }

    QJsonObject ok = {{"message", messageToJson(*updated)}};
    send(make::okResponse(ok, env.requestId()));

    _server->dispatchMessageEdited(*updated, this);
}

void ClientSession::handleDeleteMessage(const Envelope& env) {
    const qint64 messageId =
        env.payload().value("message_id").toVariant().toLongLong();

    auto existing = _db->findMessageById(messageId);
    if (!existing) {
        sendError(ResultCode::MessageNotFound, "Message not found",
                  env.requestId());
        return;
    }
    if (existing->senderId != _userId) {
        sendError(ResultCode::NotMessageAuthor,
                  "You can only delete your own messages", env.requestId());
        return;
    }

    if (!_db->deleteMessage(messageId, _userId)) {
        sendError(ResultCode::DatabaseError, "Cannot delete message",
                  env.requestId());
        return;
    }

    auto updated = _db->findMessageById(messageId);
    if (!updated) {
        sendError(ResultCode::InternalError, "Cannot fetch deleted message",
                  env.requestId());
        return;
    }

    send(make::okResponse({{"message_id", messageId}}, env.requestId()));
    _server->dispatchMessageDeleted(*updated, this);
}

void ClientSession::handleGetHistory(const Envelope& env) {
    const auto& p = env.payload();
    const qint64 peerId   = p.value("peer_id").toVariant().toLongLong();
    const qint64 beforeId = p.value("before_id").toVariant().toLongLong();
    const int    limit    = qBound(1, p.value("limit").toInt(50), 200);

    auto messages = _db->loadHistory(_userId, peerId, beforeId, limit);

    QJsonArray arr;
    for (auto& m : messages) {
        if (auto rec = _db->findUserById(m.senderId)) {
            m.senderLogin = rec->login;
        }
        if (auto a = _db->loadAttachment(m.id)) {
            m.attachment = *a;
        }
        arr.append(messageToJson(m));
    }

    send(make::okResponse({
        {"messages", arr},
        {"peer_id",  peerId}
    }, env.requestId()));
}

void ClientSession::handleGetUsers(const Envelope& env) {
    const auto users = _db->listAllUsers();

    QJsonArray arr;
    for (const auto& rec : users) {
        User u = rec.toPublicUser();
        u.isOnline = _server->isUserOnline(rec.id);
        arr.append(userToJson(u));
    }
    send(make::okResponse({{"users", arr}}, env.requestId()));
}

void ClientSession::sendError(ResultCode code, const QString& description,
                              quint32 requestId) {
    send(make::errorResponse(code, description, requestId));
}

} // namespace messenger::server
