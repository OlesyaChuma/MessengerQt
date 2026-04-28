#include "ChatServer.h"
#include "ClientSession.h"
#include "Database.h"
#include "AuthService.h"
#include "Protocol.h"

#include <QHostAddress>
#include <QTcpSocket>

namespace messenger::server {

ChatServer::ChatServer(Database* db, AuthService* auth,
                       const ListenConfig& cfg, QObject* parent)
    : QObject(parent),
      _db(db), _auth(auth), _cfg(cfg),
      _tcpServer(new QTcpServer(this)) {
    connect(_tcpServer, &QTcpServer::newConnection,
            this, &ChatServer::onNewConnection);
}

ChatServer::~ChatServer() {
    stop();
}

bool ChatServer::start(QString* errorOut) {
    QHostAddress addr(_cfg.host);
    if (_cfg.host == "0.0.0.0") addr = QHostAddress::Any;

    if (!_tcpServer->listen(addr, static_cast<quint16>(_cfg.port))) {
        if (errorOut) *errorOut = _tcpServer->errorString();
        return false;
    }
    emit serverMessage(QString("Server is listening on %1:%2")
                       .arg(_cfg.host).arg(_cfg.port));
    return true;
}

void ChatServer::stop() {
    if (_tcpServer && _tcpServer->isListening()) _tcpServer->close();

    // Закрываем все сессии
    for (auto* s : _sessions) {
        s->kick("Server is shutting down");
    }
    _sessions.clear();
    _byUser.clear();
}

bool ChatServer::isUserOnline(qint64 userId) const {
    return _byUser.contains(userId);
}

QList<qint64> ChatServer::onlineUserIds() const {
    return _byUser.keys();
}

ClientSession* ChatServer::sessionByUserId(qint64 userId) const {
    return _byUser.value(userId, nullptr);
}

bool ChatServer::kickUser(qint64 userId, const QString& reason) {
    auto* s = sessionByUserId(userId);
    if (!s) return false;
    _db->logConnectionEvent(userId, s->login(), s->peerAddress(),
                            ConnectionEvent::Kick, reason);
    s->kick(reason);
    return true;
}

bool ChatServer::banUser(qint64 userId, qint64 byAdminId, const QString& reason) {
    if (!_db->setBanned(userId, true, reason, byAdminId)) return false;

    QString login;
    QString peer;
    if (auto rec = _db->findUserById(userId)) login = rec->login;
    if (auto* s = sessionByUserId(userId)) {
        peer = s->peerAddress();
        s->notifyBannedAndClose(reason);
    }
    _db->logConnectionEvent(userId, login, peer,
                            ConnectionEvent::Ban, reason);
    return true;
}

bool ChatServer::unbanUser(qint64 userId) {
    if (!_db->setBanned(userId, false)) return false;
    QString login;
    if (auto rec = _db->findUserById(userId)) login = rec->login;
    _db->logConnectionEvent(userId, login, {}, ConnectionEvent::Unban);
    return true;
}

void ChatServer::onNewConnection() {
    while (_tcpServer->hasPendingConnections()) {
        QTcpSocket* socket = _tcpServer->nextPendingConnection();
        socket->setParent(this);

        auto* session = new ClientSession(socket, this, _db, _auth, this);
        _sessions.append(session);
        emit clientConnected(socket->peerAddress().toString());

        connect(session, &ClientSession::authenticated,
                this, [this, session](qint64 uid, const QString& login) {
                    onSessionAuthenticated(uid, login);
                    _byUser.insert(uid, session);

                    // Push: я появился онлайн — оповестить остальных
                    const auto evt = make::userOnlineEvent(uid);
                    for (auto* other : _sessions) {
                        if (other != session && other->isAuthenticated()) {
                            other->send(evt);
                        }
                    }
                });

        connect(session, &ClientSession::disconnectedFromPeer,
                this, &ChatServer::onSessionDisconnected);
    }
}

void ChatServer::onSessionAuthenticated(qint64 userId, const QString& login) {
    emit clientAuthenticated(userId, login);
    emit serverMessage(QString("User logged in: %1 (id=%2)").arg(login).arg(userId));
}

void ChatServer::onSessionDisconnected() {
    auto* session = qobject_cast<ClientSession*>(sender());
    if (!session) return;

    const qint64 uid   = session->userId();
    const QString login = session->login();

    if (uid > 0) {
        _byUser.remove(uid);
        _db->updateLastSeen(uid, QDateTime::currentDateTimeUtc());
        _db->logConnectionEvent(uid, login, session->peerAddress(),
                                ConnectionEvent::Logout);

        // Push: я ушёл оффлайн — оповестить остальных
        const auto evt = make::userOfflineEvent(uid);
        for (auto* other : _sessions) {
            if (other != session && other->isAuthenticated()) {
                other->send(evt);
            }
        }
    }
    _sessions.removeAll(session);
    session->deleteLater();

    emit clientDisconnected(uid, login);
}

QList<ClientSession*>
ChatServer::sessionsForVisibilityOf(const Message& msg) const {
    QList<ClientSession*> out;
    if (msg.isBroadcast) {
        for (auto* s : _sessions) {
            if (s->isAuthenticated()) out.append(s);
        }
        return out;
    }
    // Приватное: видят отправитель и получатель.
    if (auto* s = sessionByUserId(msg.senderId))   out.append(s);
    if (msg.receiverId > 0) {
        if (auto* s = sessionByUserId(msg.receiverId); s && !out.contains(s)) {
            out.append(s);
        }
    }
    return out;
}

void ChatServer::dispatchNewMessage(const Message& msg, ClientSession* origin) {
    const auto envelope = make::newMessageEvent(msg);
    for (auto* s : sessionsForVisibilityOf(msg)) {
        if (s == origin) continue; // отправителю уже ответили в Ok
        s->send(envelope);
    }
    emit newMessageDispatched(msg);
}

void ChatServer::dispatchMessageEdited(const Message& msg, ClientSession* origin) {
    const auto envelope = make::messageEditedEvent(msg.id, msg.body, msg.editedAt);
    for (auto* s : sessionsForVisibilityOf(msg)) {
        if (s == origin) continue;
        s->send(envelope);
    }
}

void ChatServer::dispatchMessageDeleted(const Message& msg, ClientSession* origin) {
    const auto envelope = make::messageDeletedEvent(msg.id, msg.deletedAt);
    for (auto* s : sessionsForVisibilityOf(msg)) {
        if (s == origin) continue;
        s->send(envelope);
    }
}

} // namespace messenger::server