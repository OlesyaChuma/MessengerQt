#include "ChatClientCore.h"

#include <QJsonObject>
#include <QJsonDocument>

namespace messenger::client {

ChatClientCore::ChatClientCore(QObject* parent)
    : QObject(parent),
      _socket(new QTcpSocket(this)) {
    connect(_socket, &QTcpSocket::connected,
            this, &ChatClientCore::onConnected);
    connect(_socket, &QTcpSocket::disconnected,
            this, &ChatClientCore::onDisconnected);
    connect(_socket, &QTcpSocket::readyRead,
            this, &ChatClientCore::onReadyRead);
    connect(_socket, &QTcpSocket::errorOccurred,
            this, &ChatClientCore::onSocketError);
}

ChatClientCore::~ChatClientCore() = default;

void ChatClientCore::setState(State s) {
    if (s == _state) return;
    State old = _state;
    _state = s;
    emit stateChanged(old, _state);
}

void ChatClientCore::connectToServer(const QString& host, quint16 port) {
    if (_state != State::Disconnected) return;
    _codec.reset();
    setState(State::Connecting);
    _socket->connectToHost(host, port);
}

void ChatClientCore::disconnectFromServer() {
    if (_socket->state() != QAbstractSocket::UnconnectedState) {
        _socket->disconnectFromHost();
    }
}

void ChatClientCore::onConnected() {
    setState(State::Connected);
}

void ChatClientCore::onDisconnected() {
    _inflight.clear();
    _currentUserId = 0;
    _currentLogin.clear();
    _currentDisplayName.clear();
    _currentRole = UserRole::Regular;
    _codec.reset();
    setState(State::Disconnected);
}

void ChatClientCore::onSocketError(QAbstractSocket::SocketError) {
    emit serverError(_socket->errorString());
}

void ChatClientCore::send(const Envelope& env) {
    if (_state == State::Disconnected || _state == State::Connecting) return;
    const QByteArray frame = FrameCodec::encode(env.toBytes());
    _socket->write(frame);
}

void ChatClientCore::onReadyRead() {
    const auto frames = _codec.feed(_socket->readAll());
    for (const auto& payload : frames) {
        bool ok = false;
        const auto env = Envelope::fromBytes(payload, &ok);
        if (!ok) {
            emit serverError("Malformed frame from server");
            continue;
        }
        handleEnvelope(env);
    }
}

void ChatClientCore::handleEnvelope(const Envelope& env) {
    switch (env.cmd()) {
        case CommandType::Ok: {
            const quint32 rid = env.requestId();
            const auto cmd = _inflight.take(rid);

            // Если это ответ на login или register — сохраняем личные данные
            if ((cmd == CommandType::Login || cmd == CommandType::Register)
                && env.payload().contains("user")) {
                const User u = userFromJson(env.payload().value("user").toObject());
                _currentUserId      = u.id;
                _currentLogin       = u.login;
                _currentDisplayName = u.displayName;
                _currentRole        = u.role;
                if (cmd == CommandType::Login) {
                    setState(State::Authenticated);
                }
                // Для Register состояние не меняем — обычно сразу делается Login
            }

            emit requestSucceeded(rid, env.payload());
            break;
        }

        case CommandType::Error: {
            const quint32 rid = env.requestId();
            _inflight.remove(rid);
            const auto code = resultCodeFromString(env.payload().value("code").toString());
            const auto desc = env.payload().value("description").toString();
            emit requestFailed(rid, code, desc);
            break;
        }

        case CommandType::NewMessage: {
            const Message m = messageFromJson(env.payload().value("message").toObject());
            emit newMessageArrived(m);
            break;
        }

        case CommandType::MessageEdited: {
            const auto& p = env.payload();
            const qint64 mid = p.value("message_id").toVariant().toLongLong();
            const QString body = p.value("body").toString();
            const auto at = QDateTime::fromString(p.value("edited_at").toString(),
                                                  Qt::ISODateWithMs).toUTC();
            emit messageEdited(mid, body, at);
            break;
        }

        case CommandType::MessageDeleted: {
            const auto& p = env.payload();
            const qint64 mid = p.value("message_id").toVariant().toLongLong();
            const auto at = QDateTime::fromString(p.value("deleted_at").toString(),
                                                  Qt::ISODateWithMs).toUTC();
            emit messageDeleted(mid, at);
            break;
        }

        case CommandType::UserOnline: {
            emit userOnline(env.payload().value("user_id").toVariant().toLongLong());
            break;
        }

        case CommandType::UserOffline: {
            emit userOffline(env.payload().value("user_id").toVariant().toLongLong());
            break;
        }

        case CommandType::Kicked: {
            emit kickedByServer(env.payload().value("reason").toString());
            break;
        }

        case CommandType::Banned: {
            emit bannedByServer(env.payload().value("reason").toString());
            break;
        }

        default:
            // Неизвестная команда от сервера — пропускаем
            break;
    }
}

// ---------- Запросы ----------

quint32 ChatClientCore::sendRegister(const QString& login,
                                     const QString& displayName,
                                     const QString& password) {
    const quint32 rid = nextRequestId();
    _inflight.insert(rid, CommandType::Register);
    send(make::registerRequest(login, displayName, password, rid));
    return rid;
}

quint32 ChatClientCore::sendLogin(const QString& login, const QString& password) {
    const quint32 rid = nextRequestId();
    _inflight.insert(rid, CommandType::Login);
    send(make::loginRequest(login, password, rid));
    return rid;
}

quint32 ChatClientCore::sendLogout() {
    const quint32 rid = nextRequestId();
    _inflight.insert(rid, CommandType::Logout);
    send(make::logoutRequest(rid));
    return rid;
}

quint32 ChatClientCore::sendBroadcast(const QString& body, const Attachment& att) {
    const quint32 rid = nextRequestId();
    _inflight.insert(rid, CommandType::SendMessage);
    send(make::sendMessageRequest(0, true, body, att, rid));
    return rid;
}

quint32 ChatClientCore::sendPrivateMessage(qint64 receiverId,
                                           const QString& body,
                                           const Attachment& att) {
    const quint32 rid = nextRequestId();
    _inflight.insert(rid, CommandType::SendMessage);
    send(make::sendMessageRequest(receiverId, false, body, att, rid));
    return rid;
}

quint32 ChatClientCore::sendEdit(qint64 messageId, const QString& newBody) {
    const quint32 rid = nextRequestId();
    _inflight.insert(rid, CommandType::EditMessage);
    send(make::editMessageRequest(messageId, newBody, rid));
    return rid;
}

quint32 ChatClientCore::sendDelete(qint64 messageId) {
    const quint32 rid = nextRequestId();
    _inflight.insert(rid, CommandType::DeleteMessage);
    send(make::deleteMessageRequest(messageId, rid));
    return rid;
}

quint32 ChatClientCore::requestHistory(qint64 peerId, qint64 beforeId, int limit) {
    const quint32 rid = nextRequestId();
    _inflight.insert(rid, CommandType::GetHistory);
    send(make::getHistoryRequest(peerId, beforeId, limit, rid));
    return rid;
}

quint32 ChatClientCore::requestUsers() {
    const quint32 rid = nextRequestId();
    _inflight.insert(rid, CommandType::GetUsers);
    send(make::getUsersRequest(rid));
    return rid;
}

} // namespace messenger::client