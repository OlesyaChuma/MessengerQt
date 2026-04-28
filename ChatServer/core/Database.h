#pragma once

#include "Models.h"
#include "Config.h"

#include <QSqlDatabase>
#include <QString>
#include <QList>
#include <QDateTime>
#include <optional>

namespace messenger::server {

// Запись о пользователе для внутренних операций сервера
// (включает чувствительные поля password_hash/salt).
struct UserRecord {
    qint64    id = 0;
    QString   login;
    QString   displayName;
    QString   passwordHash;
    QString   passwordSalt;
    UserRole  role = UserRole::Regular;
    bool      isBanned = false;
    QString   banReason;
    QDateTime bannedUntil;
    QDateTime createdAt;
    QDateTime lastSeen;

    // Преобразование в публичную модель (без чувствительных полей).
    User toPublicUser() const;
};

// Слой работы с БД. Вся синхронная логика; на сервере вызывать из одного потока
// (или защищать мьютексом — в первом приближении сервер однопоточный по БД).
class Database {
public:
    enum class Driver {
        PostgreSQL,
        SQLite        // только для тестов
    };

    Database();
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Открыть соединение с PostgreSQL.
    bool openPostgres(const DatabaseConfig& cfg, QString* errorOut = nullptr);

    // Открыть SQLite (in-memory или файл) — для тестов.
    bool openSqlite(const QString& path, QString* errorOut = nullptr);

    bool isOpen() const;
    void close();

    Driver driver() const { return _driver; }

    // Создать схему. Для SQLite применяется встроенная схема,
    // для PostgreSQL — содержимое файла sql/001_init.sql.
    bool initSchema(QString* errorOut = nullptr);

    // ---------- Пользователи ----------
    bool hasAnyAdmin() const;
    bool isLoginTaken(const QString& login) const;

    // Создать пользователя. Возвращает id или -1 при ошибке.
    qint64 createUser(const QString& login,
                      const QString& displayName,
                      const QString& passwordHash,
                      const QString& passwordSalt,
                      UserRole role = UserRole::Regular);

    std::optional<UserRecord> findUserByLogin(const QString& login) const;
    std::optional<UserRecord> findUserById(qint64 id) const;

    QList<UserRecord> listAllUsers() const;

    bool updateLastSeen(qint64 userId, const QDateTime& when);
    bool setBanned(qint64 userId, bool banned,
                   const QString& reason = {},
                   qint64 byUserId = 0);

    // ---------- Сообщения ----------
    qint64 saveMessage(qint64 senderId,
                       qint64 receiverId,    // 0 для broadcast
                       bool isBroadcast,
                       const QString& body,
                       const Attachment& attachment = {});

    bool editMessageBody(qint64 messageId, qint64 byUserId,
                         const QString& newBody);
    bool deleteMessage(qint64 messageId, qint64 byUserId);

    // История между двумя пользователями (peerId == 0 для broadcast),
    // beforeId == 0 для самой свежей пачки, limit ограничивает количество.
    QList<Message> loadHistory(qint64 ownUserId, qint64 peerId,
                               qint64 beforeId, int limit) const;

    // Все сообщения для админ-окна сервера (с фильтрами).
    QList<Message> loadAllMessages(qint64 senderFilter,    // 0 = все
                                   bool onlyBroadcast,
                                   bool onlyPrivate,
                                   qint64 beforeId,
                                   int limit) const;

    std::optional<Message> findMessageById(qint64 id) const;

    // Загрузить вложение к сообщению, если есть.
    std::optional<Attachment> loadAttachment(qint64 messageId) const;

    // ---------- Журнал подключений ----------
    bool logConnectionEvent(qint64 userId,            // 0 если неизвестен
                            const QString& loginCache,
                            const QString& ipAddress,
                            ConnectionEvent event,
                            const QString& details = {});

    struct ConnectionLogRow {
        qint64    id = 0;
        qint64    userId = 0;
        QString   loginCache;
        QString   ipAddress;
        ConnectionEvent event = ConnectionEvent::Login;
        QString   details;
        QDateTime occurredAt;
    };

    QList<ConnectionLogRow> loadConnectionLog(int limit) const;

private:
    QSqlDatabase _db;
    QString      _connectionName;
    Driver       _driver = Driver::PostgreSQL;

    // Возвращает текущую дату-время в UTC для записи в БД (тип-зависимо).
    QVariant nowUtc() const;

    // Исполнение схемы: одна команда за раз.
    bool execSchema(const QString& sql, QString* errorOut);
};

} // namespace messenger::server