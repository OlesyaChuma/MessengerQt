#include "Protocol.h"

#include <QJsonArray>
#include <QHash>

namespace messenger {

namespace {

const QHash<CommandType, QString> kCommandToStr = {
    {CommandType::Register,        "register"},
    {CommandType::Login,           "login"},
    {CommandType::Logout,          "logout"},
    {CommandType::SendMessage,     "send_message"},
    {CommandType::EditMessage,     "edit_message"},
    {CommandType::DeleteMessage,   "delete_message"},
    {CommandType::GetHistory,      "get_history"},
    {CommandType::GetUsers,        "get_users"},
    {CommandType::NewMessage,      "new_message"},
    {CommandType::MessageEdited,   "message_edited"},
    {CommandType::MessageDeleted,  "message_deleted"},
    {CommandType::UserOnline,      "user_online"},
    {CommandType::UserOffline,     "user_offline"},
    {CommandType::Kicked,          "kicked"},
    {CommandType::Banned,          "banned"},
    {CommandType::Ok,              "ok"},
    {CommandType::Error,           "error"},
    {CommandType::Unknown,         "unknown"}
};

const QHash<ResultCode, QString> kResultToStr = {
    {ResultCode::Ok,                  "ok"},
    {ResultCode::InvalidRequest,      "invalid_request"},
    {ResultCode::AuthRequired,        "auth_required"},
    {ResultCode::InvalidCredentials,  "invalid_credentials"},
    {ResultCode::LoginAlreadyTaken,   "login_already_taken"},
    {ResultCode::UserNotFound,        "user_not_found"},
    {ResultCode::UserBanned,          "user_banned"},
    {ResultCode::MessageNotFound,     "message_not_found"},
    {ResultCode::NotMessageAuthor,    "not_message_author"},
    {ResultCode::AttachmentTooLarge,  "attachment_too_large"},
    {ResultCode::DatabaseError,       "database_error"},
    {ResultCode::InternalError,       "internal_error"}
};

const QHash<UserRole, QString> kRoleToStr = {
    {UserRole::Regular, "user"},
    {UserRole::Admin,   "admin"}
};

template<class K, class V>
K reverseLookup(const QHash<K, V>& map, const V& value, const K& fallback) {
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        if (it.value() == value) return it.key();
    }
    return fallback;
}

QString isoOrEmpty(const QDateTime& dt) {
    return dt.isValid() ? dt.toUTC().toString(Qt::ISODateWithMs) : QString();
}

QDateTime isoToDateTime(const QString& s) {
    if (s.isEmpty()) return {};
    return QDateTime::fromString(s, Qt::ISODateWithMs).toUTC();
}

} // namespace

// ---------- Конверсии перечислений ----------

QString commandTypeToString(CommandType cmd) {
    return kCommandToStr.value(cmd, "unknown");
}

CommandType commandTypeFromString(const QString& str) {
    return reverseLookup(kCommandToStr, str, CommandType::Unknown);
}

QString resultCodeToString(ResultCode code) {
    return kResultToStr.value(code, "internal_error");
}

ResultCode resultCodeFromString(const QString& str) {
    return reverseLookup(kResultToStr, str, ResultCode::InternalError);
}

QString userRoleToString(UserRole role) {
    return kRoleToStr.value(role, "user");
}

UserRole userRoleFromString(const QString& str) {
    return reverseLookup(kRoleToStr, str, UserRole::Regular);
}

// ---------- Сериализация моделей ----------

QJsonObject userToJson(const User& user) {
    return {
        {"id",            user.id},
        {"login",         user.login},
        {"display_name",  user.displayName},
        {"role",          userRoleToString(user.role)},
        {"is_online",     user.isOnline},
        {"is_banned",     user.isBanned},
        {"ban_reason",    user.banReason},
        {"last_seen",     isoOrEmpty(user.lastSeen)}
    };
}

User userFromJson(const QJsonObject& obj) {
    User u;
    u.id          = obj.value("id").toVariant().toLongLong();
    u.login       = obj.value("login").toString();
    u.displayName = obj.value("display_name").toString();
    u.role        = userRoleFromString(obj.value("role").toString());
    u.isOnline    = obj.value("is_online").toBool();
    u.isBanned    = obj.value("is_banned").toBool();
    u.banReason   = obj.value("ban_reason").toString();
    u.lastSeen    = isoToDateTime(obj.value("last_seen").toString());
    return u;
}

QJsonObject attachmentToJson(const Attachment& att) {
    return {
        {"id",         att.id},
        {"filename",   att.filename},
        {"mime_type",  att.mimeType},
        {"size_bytes", att.sizeBytes},
        {"data_b64",   QString::fromLatin1(att.data.toBase64())}
    };
}

Attachment attachmentFromJson(const QJsonObject& obj) {
    Attachment a;
    a.id        = obj.value("id").toVariant().toLongLong();
    a.filename  = obj.value("filename").toString();
    a.mimeType  = obj.value("mime_type").toString();
    a.sizeBytes = obj.value("size_bytes").toVariant().toLongLong();
    a.data      = QByteArray::fromBase64(obj.value("data_b64").toString().toLatin1());
    return a;
}

QJsonObject messageToJson(const Message& msg) {
    QJsonObject o = {
        {"id",            msg.id},
        {"sender_id",     msg.senderId},
        {"receiver_id",   msg.receiverId},
        {"is_broadcast",  msg.isBroadcast},
        {"body",          msg.body},
        {"created_at",    isoOrEmpty(msg.createdAt)},
        {"edited_at",     isoOrEmpty(msg.editedAt)},
        {"deleted_at",    isoOrEmpty(msg.deletedAt)},
        {"sender_login",  msg.senderLogin}
    };
    if (!msg.attachment.isEmpty()) {
        o.insert("attachment", attachmentToJson(msg.attachment));
    }
    return o;
}

Message messageFromJson(const QJsonObject& obj) {
    Message m;
    m.id          = obj.value("id").toVariant().toLongLong();
    m.senderId    = obj.value("sender_id").toVariant().toLongLong();
    m.receiverId  = obj.value("receiver_id").toVariant().toLongLong();
    m.isBroadcast = obj.value("is_broadcast").toBool();
    m.body        = obj.value("body").toString();
    m.createdAt   = isoToDateTime(obj.value("created_at").toString());
    m.editedAt    = isoToDateTime(obj.value("edited_at").toString());
    m.deletedAt   = isoToDateTime(obj.value("deleted_at").toString());
    m.senderLogin = obj.value("sender_login").toString();
    if (obj.contains("attachment")) {
        m.attachment = attachmentFromJson(obj.value("attachment").toObject());
    }
    return m;
}

// ---------- Envelope ----------

Envelope::Envelope(CommandType cmd, QJsonObject payload, quint32 requestId)
    : _cmd(cmd), _payload(std::move(payload)), _requestId(requestId) {}

QJsonObject Envelope::toJson() const {
    QJsonObject o = {
        {"cmd",     commandTypeToString(_cmd)},
        {"payload", _payload}
    };
    if (_requestId != 0) {
        o.insert("request_id", static_cast<qint64>(_requestId));
    }
    return o;
}

QByteArray Envelope::toBytes() const {
    return QJsonDocument(toJson()).toJson(QJsonDocument::Compact);
}

Envelope Envelope::fromJson(const QJsonObject& obj) {
    Envelope e;
    e._cmd       = commandTypeFromString(obj.value("cmd").toString());
    e._payload   = obj.value("payload").toObject();
    e._requestId = static_cast<quint32>(obj.value("request_id").toVariant().toUInt());
    return e;
}

Envelope Envelope::fromBytes(const QByteArray& bytes, bool* ok) {
    QJsonParseError err{};
    auto doc = QJsonDocument::fromJson(bytes, &err);
    if (ok) *ok = (err.error == QJsonParseError::NoError && doc.isObject());
    if (!doc.isObject()) return {};
    return fromJson(doc.object());
}

// ---------- make::* ----------

namespace make {

Envelope registerRequest(const QString& login, const QString& displayName,
                         const QString& password, quint32 requestId) {
    return {CommandType::Register, {
        {"login", login},
        {"display_name", displayName},
        {"password", password}
    }, requestId};
}

Envelope loginRequest(const QString& login, const QString& password, quint32 requestId) {
    return {CommandType::Login, {
        {"login", login},
        {"password", password}
    }, requestId};
}

Envelope logoutRequest(quint32 requestId) {
    return {CommandType::Logout, {}, requestId};
}

Envelope sendMessageRequest(qint64 receiverId, bool isBroadcast,
                            const QString& body, const Attachment& attachment,
                            quint32 requestId) {
    QJsonObject p = {
        {"receiver_id",  receiverId},
        {"is_broadcast", isBroadcast},
        {"body",         body}
    };
    if (!attachment.isEmpty()) {
        p.insert("attachment", attachmentToJson(attachment));
    }
    return {CommandType::SendMessage, p, requestId};
}

Envelope editMessageRequest(qint64 messageId, const QString& newBody, quint32 requestId) {
    return {CommandType::EditMessage, {
        {"message_id", messageId},
        {"body",       newBody}
    }, requestId};
}

Envelope deleteMessageRequest(qint64 messageId, quint32 requestId) {
    return {CommandType::DeleteMessage, {
        {"message_id", messageId}
    }, requestId};
}

Envelope getHistoryRequest(qint64 peerId, qint64 beforeId, int limit, quint32 requestId) {
    return {CommandType::GetHistory, {
        {"peer_id",   peerId},
        {"before_id", beforeId},
        {"limit",     limit}
    }, requestId};
}

Envelope getUsersRequest(quint32 requestId) {
    return {CommandType::GetUsers, {}, requestId};
}

Envelope newMessageEvent(const Message& msg) {
    return {CommandType::NewMessage, {{"message", messageToJson(msg)}}, 0};
}

Envelope messageEditedEvent(qint64 messageId, const QString& newBody,
                            const QDateTime& editedAt) {
    return {CommandType::MessageEdited, {
        {"message_id", messageId},
        {"body",       newBody},
        {"edited_at",  editedAt.toUTC().toString(Qt::ISODateWithMs)}
    }, 0};
}

Envelope messageDeletedEvent(qint64 messageId, const QDateTime& deletedAt) {
    return {CommandType::MessageDeleted, {
        {"message_id", messageId},
        {"deleted_at", deletedAt.toUTC().toString(Qt::ISODateWithMs)}
    }, 0};
}

Envelope userOnlineEvent(qint64 userId) {
    return {CommandType::UserOnline, {{"user_id", userId}}, 0};
}

Envelope userOfflineEvent(qint64 userId) {
    return {CommandType::UserOffline, {{"user_id", userId}}, 0};
}

Envelope kickedEvent(const QString& reason) {
    return {CommandType::Kicked, {{"reason", reason}}, 0};
}

Envelope bannedEvent(const QString& reason) {
    return {CommandType::Banned, {{"reason", reason}}, 0};
}

Envelope okResponse(QJsonObject data, quint32 requestId) {
    return {CommandType::Ok, data, requestId};
}

Envelope errorResponse(ResultCode code, const QString& description, quint32 requestId) {
    return {CommandType::Error, {
        {"code",        resultCodeToString(code)},
        {"description", description}
    }, requestId};
}

} // namespace make

} // namespace messenger