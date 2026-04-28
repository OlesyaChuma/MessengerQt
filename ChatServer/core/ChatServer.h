#pragma once

#include "Models.h"
#include "Config.h"

#include <QObject>
#include <QTcpServer>
#include <QHash>
#include <QList>

namespace messenger::server {

class ClientSession;
class Database;
class AuthService;

// TCP-сервер чата. Менеджер активных клиентских сессий.
// Раздаёт сообщения по правилам видимости (broadcast / получатель / отправитель).
class ChatServer : public QObject {
    Q_OBJECT
public:
    ChatServer(Database* db, AuthService* auth,
               const ListenConfig& cfg, QObject* parent = nullptr);
    ~ChatServer() override;

    bool start(QString* errorOut = nullptr);
    void stop();

    bool isUserOnline(qint64 userId) const;
    QList<qint64> onlineUserIds() const;
    int sessionsCount() const { return _sessions.size(); }

    // Действия администратора
    bool kickUser(qint64 userId, const QString& reason);
    bool banUser(qint64 userId, qint64 byAdminId, const QString& reason);
    bool unbanUser(qint64 userId);

    // Раздача push-событий — вызывается из ClientSession при обработке команд.
    void dispatchNewMessage(const Message& msg, ClientSession* origin);
    void dispatchMessageEdited(const Message& msg, ClientSession* origin);
    void dispatchMessageDeleted(const Message& msg, ClientSession* origin);

signals:
    // Нотификации для GUI / логирования
    void clientConnected(const QString& peer);
    void clientAuthenticated(qint64 userId, const QString& login);
    void clientDisconnected(qint64 userId, const QString& login);
    void newMessageDispatched(const Message& msg);
    void serverMessage(const QString& text);

private slots:
    void onNewConnection();
    void onSessionAuthenticated(qint64 userId, const QString& login);
    void onSessionDisconnected();

private:
    QList<ClientSession*> sessionsForVisibilityOf(const Message& msg) const;
    ClientSession* sessionByUserId(qint64 userId) const;

    Database*     _db = nullptr;
    AuthService*  _auth = nullptr;
    ListenConfig  _cfg;
    QTcpServer*   _tcpServer = nullptr;

    // Активные сессии, плюс отображение userId -> session для онлайн-юзеров
    QList<ClientSession*>     _sessions;
    QHash<qint64, ClientSession*> _byUser;
};

} // namespace messenger::server