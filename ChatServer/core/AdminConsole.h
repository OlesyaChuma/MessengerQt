#pragma once

#include <QObject>
#include <atomic>

namespace messenger::server {

class ChatServer;
class Database;
class AuthService;

// Простое консольное меню администратора.
// Чтение stdin в отдельном std::thread (detached), сигнал в Qt-поток
// через QMetaObject::invokeMethod.
class AdminConsole : public QObject {
    Q_OBJECT
public:
    AdminConsole(ChatServer* server, Database* db,
                 AuthService* auth, qint64 adminId,
                 QObject* parent = nullptr);
    ~AdminConsole() override;

    void start();   // печатает приветствие и запускает фоновое чтение stdin

signals:
    void shutdownRequested();

public slots:
    void onLineRead(const QString& line);
    void onEofReached();

private:
    void printHelp() const;
    void printUsers() const;
    void printOnline() const;
    void printRecentMessages(int limit) const;
    void printConnectionLog(int limit) const;
    void cmdKick(const QStringList& args);
    void cmdBan(const QStringList& args);
    void cmdUnban(const QStringList& args);

    void prompt() const;
    void printLine(const QString& line) const;

    ChatServer*  _server = nullptr;
    Database*    _db     = nullptr;
    AuthService* _auth   = nullptr;
    qint64       _adminId = 0;

    // Флаг "нас просили выйти" — std::thread его проверяет.
    std::atomic<bool> _shuttingDown{false};
};

} // namespace messenger::server
