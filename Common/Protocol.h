#pragma once

#include "Models.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QString>

namespace messenger {

// Утилиты конверсии команд и кодов в строки и обратно (для JSON)
QString commandTypeToString(CommandType cmd);
CommandType commandTypeFromString(const QString& str);

QString resultCodeToString(ResultCode code);
ResultCode resultCodeFromString(const QString& str);

QString userRoleToString(UserRole role);
UserRole userRoleFromString(const QString& str);

// Сериализация моделей в JSON
QJsonObject userToJson(const User& user);
User userFromJson(const QJsonObject& obj);

QJsonObject messageToJson(const Message& msg);
Message messageFromJson(const QJsonObject& obj);

QJsonObject attachmentToJson(const Attachment& att);
Attachment attachmentFromJson(const QJsonObject& obj);

// Базовый конверт сообщения протокола.
// На проводе летит JSON-объект:
// {
//   "cmd":     "<CommandType>",
//   "payload": { ... },          // полезная нагрузка команды
//   "request_id": <uint>         // опциональный id для корреляции запрос-ответ
// }
class Envelope {
public:
    Envelope() = default;
    Envelope(CommandType cmd, QJsonObject payload, quint32 requestId = 0);

    CommandType cmd() const { return _cmd; }
    const QJsonObject& payload() const { return _payload; }
    quint32 requestId() const { return _requestId; }

    QJsonObject toJson() const;
    QByteArray toBytes() const;          // компактный JSON в UTF-8

    static Envelope fromJson(const QJsonObject& obj);
    static Envelope fromBytes(const QByteArray& bytes, bool* ok = nullptr);

private:
    CommandType _cmd = CommandType::Unknown;
    QJsonObject _payload;
    quint32 _requestId = 0;
};

// Удобные хелперы для типичных запросов и ответов
namespace make {
    Envelope registerRequest(const QString& login, const QString& displayName,
                             const QString& password, quint32 requestId = 0);
    Envelope loginRequest(const QString& login, const QString& password,
                          quint32 requestId = 0);
    Envelope logoutRequest(quint32 requestId = 0);

    Envelope sendMessageRequest(qint64 receiverId, bool isBroadcast,
                                const QString& body, const Attachment& attachment = {},
                                quint32 requestId = 0);
    Envelope editMessageRequest(qint64 messageId, const QString& newBody,
                                quint32 requestId = 0);
    Envelope deleteMessageRequest(qint64 messageId, quint32 requestId = 0);

    // Запрос истории сообщений: peerId == 0 для broadcast,
    // beforeId == 0 для самой свежей пачки.
    Envelope getHistoryRequest(qint64 peerId, qint64 beforeId, int limit,
                               quint32 requestId = 0);
    Envelope getUsersRequest(quint32 requestId = 0);

    // Push-события сервера
    Envelope newMessageEvent(const Message& msg);
    Envelope messageEditedEvent(qint64 messageId, const QString& newBody,
                                const QDateTime& editedAt);
    Envelope messageDeletedEvent(qint64 messageId, const QDateTime& deletedAt);
    Envelope userOnlineEvent(qint64 userId);
    Envelope userOfflineEvent(qint64 userId);
    Envelope kickedEvent(const QString& reason);
    Envelope bannedEvent(const QString& reason);

    // Стандартные ответы
    Envelope okResponse(QJsonObject data = {}, quint32 requestId = 0);
    Envelope errorResponse(ResultCode code, const QString& description,
                           quint32 requestId = 0);
}

} // namespace messenger