#pragma once

#include "Models.h"
#include "Protocol.h"
#include "FrameCodec.h"

#include <QObject>
#include <QPointer>
#include <QTcpSocket>

namespace messenger::server {

class ChatServer;
class Database;
class AuthService;

// Обработчик одного клиентского подключения.
// Живёт в основном потоке сервера, владеет сокетом.
class ClientSession : public QObject {
    Q_OBJECT
public:
    ClientSession(QTcpSocket* socket,
                  ChatServer* server,
                  Database* db,
                  AuthService* auth,
                  QObject* parent = nullptr);
    ~ClientSession() override;

    bool isAuthenticated() const { return _userId > 0; }
    qint64 userId() const { return _userId; }
    QString login() const { return _login; }
    UserRole role() const { return _role; }
    QString peerAddress() const;

    // Отправить произвольный envelope клиенту
    void send(const Envelope& env);

    // Принудительно отключить клиента (kick).
    // Перед закрытием отправляется событие Kicked с описанием.
    void kick(const QString& reason);

    // Принудительный бан-флоу: отправить Banned + закрыть.
    // Сам бан в БД ставит вызывающий (ChatServer/Admin).
    void notifyBannedAndClose(const QString& reason);

signals:
    // Срабатывает один раз при первой успешной аутентификации
    void authenticated(qint64 userId, const QString& login);

    // Срабатывает при разрыве соединения (по любой причине)
    void disconnectedFromPeer();

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    // Диспетчеризация команд
    void handleEnvelope(const Envelope& env);

    void handleRegister(const Envelope& env);
    void handleLogin(const Envelope& env);
    void handleLogout(const Envelope& env);
    void handleSendMessage(const Envelope& env);
    void handleEditMessage(const Envelope& env);
    void handleDeleteMessage(const Envelope& env);
    void handleGetHistory(const Envelope& env);
    void handleGetUsers(const Envelope& env);

    // Отправить ошибку
    void sendError(ResultCode code, const QString& description, quint32 requestId);

    // Заполнить sender_login и attachment в Message перед отдачей наружу
    void enrichMessage(Message& m) const;

    QPointer<QTcpSocket> _socket;
    ChatServer* _server = nullptr;
    Database* _db = nullptr;
    AuthService* _auth = nullptr;

    FrameCodec _codec;

    qint64   _userId = 0;
    QString  _login;
    UserRole _role = UserRole::Regular;
};

} // namespace messenger::server