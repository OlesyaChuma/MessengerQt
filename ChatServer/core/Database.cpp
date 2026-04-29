#include "Database.h"
#include "Protocol.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QFile>
#include <QTextStream>
#include <QUuid>
#include <QDebug>
#include <QtGlobal>

namespace messenger::server {

namespace {

ConnectionEvent eventFromString(const QString& s) {
    if (s == "login")        return ConnectionEvent::Login;
    if (s == "logout")       return ConnectionEvent::Logout;
    if (s == "kick")         return ConnectionEvent::Kick;
    if (s == "ban")          return ConnectionEvent::Ban;
    if (s == "unban")        return ConnectionEvent::Unban;
    if (s == "failed_login") return ConnectionEvent::FailedLogin;
    if (s == "register")     return ConnectionEvent::Register;
    return ConnectionEvent::Login;
}

QString eventToString(ConnectionEvent e) {
    switch (e) {
        case ConnectionEvent::Login:       return "login";
        case ConnectionEvent::Logout:      return "logout";
        case ConnectionEvent::Kick:        return "kick";
        case ConnectionEvent::Ban:         return "ban";
        case ConnectionEvent::Unban:       return "unban";
        case ConnectionEvent::FailedLogin: return "failed_login";
        case ConnectionEvent::Register:    return "register";
    }
    return "login";
}

// Встроенная SQLite-схема для тестов
const char* kSqliteSchema = R"SQL(
CREATE TABLE IF NOT EXISTS users (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    login           TEXT    UNIQUE NOT NULL,
    display_name    TEXT    NOT NULL,
    password_hash   TEXT    NOT NULL,
    password_salt   TEXT    NOT NULL,
    role            TEXT    NOT NULL DEFAULT 'user',
    is_banned       INTEGER NOT NULL DEFAULT 0,
    ban_reason      TEXT,
    banned_until    TEXT,
    banned_by       INTEGER,
    created_at      TEXT    NOT NULL,
    last_seen       TEXT
);

CREATE INDEX IF NOT EXISTS idx_users_login ON users(login);

CREATE TABLE IF NOT EXISTS messages (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    sender_id       INTEGER NOT NULL REFERENCES users(id),
    receiver_id     INTEGER REFERENCES users(id),
    is_broadcast    INTEGER NOT NULL,
    body            TEXT,
    created_at      TEXT    NOT NULL,
    edited_at       TEXT,
    deleted_at      TEXT
);

CREATE INDEX IF NOT EXISTS idx_messages_created ON messages(created_at DESC);
CREATE INDEX IF NOT EXISTS idx_messages_sender  ON messages(sender_id);

CREATE TABLE IF NOT EXISTS attachments (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    message_id  INTEGER NOT NULL REFERENCES messages(id) ON DELETE CASCADE,
    filename    TEXT    NOT NULL,
    mime_type   TEXT    NOT NULL,
    size_bytes  INTEGER NOT NULL,
    data        BLOB    NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_attachments_msg ON attachments(message_id);

CREATE TABLE IF NOT EXISTS connection_log (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id      INTEGER REFERENCES users(id),
    login_cache  TEXT,
    ip_address   TEXT,
    event_type   TEXT    NOT NULL,
    details      TEXT,
    occurred_at  TEXT    NOT NULL
);
)SQL";

} // namespace

User UserRecord::toPublicUser() const {
    User u;
    u.id          = id;
    u.login       = login;
    u.displayName = displayName;
    u.role        = role;
    u.isBanned    = isBanned;
    u.banReason   = banReason;
    u.lastSeen    = lastSeen;
    u.isOnline    = false; // online выставляется отдельно
    return u;
}

// ---------- Database ----------

Database::Database() {
    _connectionName = "messenger_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

Database::~Database() {
    close();
}

bool Database::openPostgres(const DatabaseConfig& cfg, QString* errorOut) {
    close();
    _driver = Driver::PostgreSQL;

    _db = QSqlDatabase::addDatabase("QPSQL", _connectionName);
    _db.setHostName(cfg.host);
    _db.setPort(cfg.port);
    _db.setDatabaseName(cfg.name);
    _db.setUserName(cfg.user);
    _db.setPassword(cfg.password);

    if (!_db.open()) {
        if (errorOut) *errorOut = _db.lastError().text();
        return false;
    }
    return true;
}

bool Database::openSqlite(const QString& path, QString* errorOut) {
    close();
    _driver = Driver::SQLite;

    _db = QSqlDatabase::addDatabase("QSQLITE", _connectionName);
    _db.setDatabaseName(path);

    if (!_db.open()) {
        if (errorOut) *errorOut = _db.lastError().text();
        return false;
    }

    // Включить foreign keys в SQLite
    QSqlQuery q(_db);
    q.exec("PRAGMA foreign_keys = ON;");
    return true;
}

bool Database::isOpen() const { return _db.isOpen(); }

void Database::close() {
    if (_db.isOpen()) _db.close();
    if (QSqlDatabase::contains(_connectionName)) {
        QSqlDatabase::removeDatabase(_connectionName);
    }
}

QVariant Database::nowUtc() const {
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
}

bool Database::execSchema(const QString& sql, QString* errorOut) {
    // SQLite/Postgres: разбиваем по `;` и выполняем по одному оператору
    QString accumulated;
    for (const auto& line : sql.split('\n')) {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith("--") || trimmed.isEmpty()) continue;
        accumulated += line + '\n';
    }

    const auto statements = accumulated.split(';', Qt::SkipEmptyParts);
    QSqlQuery q(_db);
    for (const auto& stmt : statements) {
        const QString s = stmt.trimmed();
        if (s.isEmpty()) continue;
        if (!q.exec(s)) {
            if (errorOut) {
                *errorOut = QString("SQL: %1\nError: %2")
                    .arg(s, q.lastError().text());
            }
            return false;
        }
    }
    return true;
}

bool Database::initSchema(QString* errorOut) {
    if (!isOpen()) {
        if (errorOut) *errorOut = "Database is not open";
        return false;
    }

    if (_driver == Driver::SQLite) {
        return execSchema(kSqliteSchema, errorOut);
    }

    // PostgreSQL: ищем sql/001_init.sql относительно exe
    const QStringList candidates = {
        "sql/001_init.sql",
        "../sql/001_init.sql",
        QString::fromLocal8Bit(qgetenv("MESSENGER_SQL_INIT"))
    };

    for (const auto& path : candidates) {
        if (path.isEmpty()) continue;
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        const QString sql = QTextStream(&f).readAll();
        return execSchema(sql, errorOut);
    }

    if (errorOut) *errorOut = "Cannot find sql/001_init.sql next to executable";
    return false;
}

// ---------- Пользователи ----------

bool Database::hasAnyAdmin() const {
    QSqlQuery q(_db);
    q.prepare("SELECT COUNT(*) FROM users WHERE role = 'admin' AND is_banned = FALSE");
    if (!q.exec() || !q.next()) return false;
    return q.value(0).toLongLong() > 0;
}

bool Database::isLoginTaken(const QString& login) const {
    QSqlQuery q(_db);
    q.prepare("SELECT COUNT(*) FROM users WHERE login = :login");
    q.bindValue(":login", login);
    if (!q.exec() || !q.next()) return false;
    return q.value(0).toLongLong() > 0;
}

qint64 Database::createUser(const QString& login,
                            const QString& displayName,
                            const QString& passwordHash,
                            const QString& passwordSalt,
                            UserRole role) {
    QSqlQuery q(_db);
    q.prepare(R"SQL(
        INSERT INTO users (login, display_name, password_hash, password_salt,
                           role, created_at)
        VALUES (:login, :display_name, :hash, :salt, :role, :created_at)
    )SQL");
    q.bindValue(":login",        login);
    q.bindValue(":display_name", displayName);
    q.bindValue(":hash",         passwordHash);
    q.bindValue(":salt",         passwordSalt);
    q.bindValue(":role",         userRoleToString(role));
    q.bindValue(":created_at",   nowUtc());

    if (!q.exec()) {
        qWarning() << "createUser failed:" << q.lastError().text();
        return -1;
    }

    if (_driver == Driver::SQLite) {
        return q.lastInsertId().toLongLong();
    }

    // Postgres: достаём id отдельным запросом, чтобы не зависеть от RETURNING
    QSqlQuery sel(_db);
    sel.prepare("SELECT id FROM users WHERE login = :login");
    sel.bindValue(":login", login);
    if (sel.exec() && sel.next()) return sel.value(0).toLongLong();
    return -1;
}

static UserRecord rowToUserRecord(QSqlQuery& q) {
    UserRecord r;
    r.id           = q.value("id").toLongLong();
    r.login        = q.value("login").toString();
    r.displayName  = q.value("display_name").toString();
    r.passwordHash = q.value("password_hash").toString();
    r.passwordSalt = q.value("password_salt").toString();
    r.role         = userRoleFromString(q.value("role").toString());
    r.isBanned     = q.value("is_banned").toBool();
    r.banReason    = q.value("ban_reason").toString();

    const auto bannedUntil = q.value("banned_until").toString();
    if (!bannedUntil.isEmpty()) {
        r.bannedUntil = QDateTime::fromString(bannedUntil, Qt::ISODateWithMs).toUTC();
    }
    r.createdAt = QDateTime::fromString(q.value("created_at").toString(),
                                        Qt::ISODateWithMs).toUTC();
    const auto lastSeen = q.value("last_seen").toString();
    if (!lastSeen.isEmpty()) {
        r.lastSeen = QDateTime::fromString(lastSeen, Qt::ISODateWithMs).toUTC();
    }
    return r;
}

std::optional<UserRecord> Database::findUserByLogin(const QString& login) const {
    QSqlQuery q(_db);
    q.prepare("SELECT * FROM users WHERE login = :login");
    q.bindValue(":login", login);
    if (!q.exec() || !q.next()) return std::nullopt;
    return rowToUserRecord(q);
}

std::optional<UserRecord> Database::findUserById(qint64 id) const {
    QSqlQuery q(_db);
    q.prepare("SELECT * FROM users WHERE id = :id");
    q.bindValue(":id", id);
    if (!q.exec() || !q.next()) return std::nullopt;
    return rowToUserRecord(q);
}

QList<UserRecord> Database::listAllUsers() const {
    QList<UserRecord> result;
    QSqlQuery q(_db);
    if (!q.exec("SELECT * FROM users ORDER BY login")) return result;
    while (q.next()) result.append(rowToUserRecord(q));
    return result;
}

bool Database::updateLastSeen(qint64 userId, const QDateTime& when) {
    QSqlQuery q(_db);
    q.prepare("UPDATE users SET last_seen = :ts WHERE id = :id");
    q.bindValue(":ts", when.toUTC().toString(Qt::ISODateWithMs));
    q.bindValue(":id", userId);
    return q.exec();
}

bool Database::setBanned(qint64 userId, bool banned,
                         const QString& reason, qint64 byUserId) {
    QSqlQuery q(_db);
    q.prepare(R"SQL(
        UPDATE users
        SET is_banned  = :banned,
            ban_reason = :reason,
            banned_by  = :by_user
        WHERE id = :id
    )SQL");
    q.bindValue(":banned",  QVariant::fromValue(banned));
    q.bindValue(":reason",  banned ? reason : QString());
    q.bindValue(":by_user", banned && byUserId > 0 ? QVariant(byUserId) : QVariant(QMetaType(QMetaType::LongLong)));
    q.bindValue(":id",      userId);
    return q.exec();
}

// ---------- Сообщения ----------

qint64 Database::saveMessage(qint64 senderId, qint64 receiverId,
                             bool isBroadcast, const QString& body,
                             const Attachment& attachment) {
    QSqlQuery q(_db);
    q.prepare(R"SQL(
        INSERT INTO messages (sender_id, receiver_id, is_broadcast,
                              body, created_at)
        VALUES (:sender, :receiver, :broadcast, :body, :created_at)
    )SQL");
    q.bindValue(":sender",     senderId);
    q.bindValue(":receiver",   isBroadcast ? QVariant(QMetaType(QMetaType::LongLong))
                                           : QVariant(receiverId));
    q.bindValue(":broadcast",  QVariant::fromValue(isBroadcast));
    q.bindValue(":body",       body);
    q.bindValue(":created_at", nowUtc());

    if (!q.exec()) {
        qWarning() << "saveMessage failed:" << q.lastError().text();
        return -1;
    }

    qint64 messageId = -1;
    if (_driver == Driver::SQLite) {
        messageId = q.lastInsertId().toLongLong();
    } else {
        QSqlQuery sel(_db);
        sel.prepare(
            "SELECT id FROM messages WHERE sender_id = :s "
            "ORDER BY id DESC LIMIT 1");
        sel.bindValue(":s", senderId);
        if (sel.exec() && sel.next()) messageId = sel.value(0).toLongLong();
    }

    if (messageId > 0 && !attachment.isEmpty()) {
        QSqlQuery att(_db);
        att.prepare(R"SQL(
            INSERT INTO attachments (message_id, filename, mime_type,
                                     size_bytes, data)
            VALUES (:msg, :filename, :mime, :size, :data)
        )SQL");
        att.bindValue(":msg",      messageId);
        att.bindValue(":filename", attachment.filename);
        att.bindValue(":mime",     attachment.mimeType);
        att.bindValue(":size",     attachment.sizeBytes);
        att.bindValue(":data",     attachment.data);
        if (!att.exec()) {
            qWarning() << "save attachment failed:" << att.lastError().text();
        }
    }

    return messageId;
}

bool Database::editMessageBody(qint64 messageId, qint64 byUserId,
                               const QString& newBody) {
    QSqlQuery q(_db);
    q.prepare(R"SQL(
        UPDATE messages
        SET body = :body, edited_at = :ts
        WHERE id = :id AND sender_id = :uid AND deleted_at IS NULL
    )SQL");
    q.bindValue(":body", newBody);
    q.bindValue(":ts",   nowUtc());
    q.bindValue(":id",   messageId);
    q.bindValue(":uid",  byUserId);
    return q.exec() && q.numRowsAffected() > 0;
}

bool Database::deleteMessage(qint64 messageId, qint64 byUserId) {
    QSqlQuery q(_db);
    q.prepare(R"SQL(
        UPDATE messages
        SET deleted_at = :ts
        WHERE id = :id AND sender_id = :uid AND deleted_at IS NULL
    )SQL");
    q.bindValue(":ts",  nowUtc());
    q.bindValue(":id",  messageId);
    q.bindValue(":uid", byUserId);
    return q.exec() && q.numRowsAffected() > 0;
}

static Message rowToMessage(QSqlQuery& q) {
    Message m;
    m.id          = q.value("id").toLongLong();
    m.senderId    = q.value("sender_id").toLongLong();
    if (!q.value("receiver_id").isNull()) {
        m.receiverId = q.value("receiver_id").toLongLong();
    }
    m.isBroadcast = q.value("is_broadcast").toBool();
    m.body        = q.value("body").toString();
    m.createdAt   = QDateTime::fromString(q.value("created_at").toString(),
                                          Qt::ISODateWithMs).toUTC();
    const auto edited = q.value("edited_at").toString();
    if (!edited.isEmpty()) {
        m.editedAt = QDateTime::fromString(edited, Qt::ISODateWithMs).toUTC();
    }
    const auto deleted = q.value("deleted_at").toString();
    if (!deleted.isEmpty()) {
        m.deletedAt = QDateTime::fromString(deleted, Qt::ISODateWithMs).toUTC();
    }
    return m;
}

QList<Message> Database::loadHistory(qint64 ownUserId, qint64 peerId,
                                     qint64 beforeId, int limit) const {
    QList<Message> result;
    QSqlQuery q(_db);

    QString sql = R"SQL(
        SELECT m.*, u.login AS sender_login
        FROM messages m
        JOIN users u ON u.id = m.sender_id
        WHERE m.deleted_at IS NULL AND
    )SQL";

    if (peerId == 0) {
        sql += " m.is_broadcast = TRUE ";
    } else {
        sql += R"SQL(
            ((m.sender_id = :own AND m.receiver_id = :peer)
          OR (m.sender_id = :peer AND m.receiver_id = :own))
        )SQL";
    }

    if (beforeId > 0) sql += " AND m.id < :before ";
    sql += " ORDER BY m.id DESC LIMIT :limit";

    q.prepare(sql);
    if (peerId != 0) {
        q.bindValue(":own",  ownUserId);
        q.bindValue(":peer", peerId);
    }
    if (beforeId > 0) q.bindValue(":before", beforeId);
    q.bindValue(":limit", limit);

    if (!q.exec()) return result;
    while (q.next()) {
        Message m = rowToMessage(q);
        m.senderLogin = q.value("sender_login").toString();
        result.append(m);
    }

    // Возвращаем в хронологическом порядке (сначала старые)
    std::reverse(result.begin(), result.end());
    return result;
}

QList<Message> Database::loadAllMessages(qint64 senderFilter,
                                         bool onlyBroadcast,
                                         bool onlyPrivate,
                                         qint64 beforeId,
                                         int limit) const {
    QList<Message> result;
    QSqlQuery q(_db);

    QString sql = R"SQL(
        SELECT m.*, u.login AS sender_login
        FROM messages m
        JOIN users u ON u.id = m.sender_id
        WHERE 1=1
    )SQL";

    if (senderFilter > 0)   sql += " AND m.sender_id = :sender ";
    if (onlyBroadcast)      sql += " AND m.is_broadcast = TRUE ";
    if (onlyPrivate)        sql += " AND m.is_broadcast = FALSE ";
    if (beforeId > 0)       sql += " AND m.id < :before ";

    sql += " ORDER BY m.id DESC LIMIT :limit";

    q.prepare(sql);
    if (senderFilter > 0) q.bindValue(":sender", senderFilter);
    if (beforeId > 0)     q.bindValue(":before", beforeId);
    q.bindValue(":limit", limit);

    if (!q.exec()) return result;
    while (q.next()) {
        Message m = rowToMessage(q);
        m.senderLogin = q.value("sender_login").toString();
        result.append(m);
    }
    std::reverse(result.begin(), result.end());
    return result;
}

std::optional<Message> Database::findMessageById(qint64 id) const {
    QSqlQuery q(_db);
    q.prepare(R"SQL(
        SELECT m.*, u.login AS sender_login
        FROM messages m
        JOIN users u ON u.id = m.sender_id
        WHERE m.id = :id
    )SQL");
    q.bindValue(":id", id);
    if (!q.exec() || !q.next()) return std::nullopt;
    Message m = rowToMessage(q);
    m.senderLogin = q.value("sender_login").toString();
    return m;
}

std::optional<Attachment> Database::loadAttachment(qint64 messageId) const {
    QSqlQuery q(_db);
    q.prepare(R"SQL(
        SELECT id, filename, mime_type, size_bytes, data
        FROM attachments WHERE message_id = :msg
    )SQL");
    q.bindValue(":msg", messageId);
    if (!q.exec() || !q.next()) return std::nullopt;

    Attachment a;
    a.id        = q.value("id").toLongLong();
    a.filename  = q.value("filename").toString();
    a.mimeType  = q.value("mime_type").toString();
    a.sizeBytes = q.value("size_bytes").toLongLong();
    a.data      = q.value("data").toByteArray();
    return a;
}

// ---------- Журнал подключений ----------

bool Database::logConnectionEvent(qint64 userId,
                                  const QString& loginCache,
                                  const QString& ipAddress,
                                  ConnectionEvent event,
                                  const QString& details) {
    QSqlQuery q(_db);
    q.prepare(R"SQL(
        INSERT INTO connection_log
            (user_id, login_cache, ip_address, event_type, details, occurred_at)
        VALUES (:uid, :login, :ip, :event, :details, :ts)
    )SQL");
    q.bindValue(":uid",     userId > 0 ? QVariant(userId)
                                       : QVariant(QMetaType(QMetaType::LongLong)));
    q.bindValue(":login",   loginCache);
    q.bindValue(":ip",      ipAddress);
    q.bindValue(":event",   eventToString(event));
    q.bindValue(":details", details);
    q.bindValue(":ts",      nowUtc());
    return q.exec();
}

QList<Database::ConnectionLogRow>
Database::loadConnectionLog(int limit) const {
    QList<ConnectionLogRow> result;
    QSqlQuery q(_db);
    q.prepare("SELECT * FROM connection_log ORDER BY id DESC LIMIT :lim");
    q.bindValue(":lim", limit);
    if (!q.exec()) return result;

    while (q.next()) {
        ConnectionLogRow r;
        r.id          = q.value("id").toLongLong();
        r.userId      = q.value("user_id").toLongLong();
        r.loginCache  = q.value("login_cache").toString();
        r.ipAddress   = q.value("ip_address").toString();
        r.event       = eventFromString(q.value("event_type").toString());
        r.details     = q.value("details").toString();
        r.occurredAt  = QDateTime::fromString(q.value("occurred_at").toString(),
                                              Qt::ISODateWithMs).toUTC();
        result.append(r);
    }
    return result;
}

} // namespace messenger::server
