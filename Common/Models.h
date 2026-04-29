#pragma once

#include <QString>
#include <QDateTime>
#include <QByteArray>
#include <QtGlobal>

namespace messenger {

// Роль пользователя
enum class UserRole {
    Regular,
    Admin
};

// Событие в журнале подключений
enum class ConnectionEvent {
    Login,
    Logout,
    Kick,
    Ban,
    Unban,
    FailedLogin,
    Register
};

// Команды протокола (запросы клиент -> сервер)
enum class CommandType {
    // Аутентификация
    Register,
    Login,
    Logout,

    // Сообщения
    SendMessage,
    EditMessage,
    DeleteMessage,
    GetHistory,

    // Пользователи
    GetUsers,

    // Серверные push-события (сервер -> клиент)
    NewMessage,
    MessageEdited,
    MessageDeleted,
    UserOnline,
    UserOffline,
    Kicked,
    Banned,

    // Общий ответ-результат
    Ok,
    Error,

    Unknown
};

// Код результата операции
enum class ResultCode {
    Ok,
    InvalidRequest,
    AuthRequired,
    InvalidCredentials,
    LoginAlreadyTaken,
    UserNotFound,
    UserBanned,
    MessageNotFound,
    NotMessageAuthor,
    AttachmentTooLarge,
    DatabaseError,
    InternalError
};

// Пользователь (видимая клиентская проекция)
struct User {
    qint64 id = 0;
    QString login;
    QString displayName;
    UserRole role = UserRole::Regular;
    bool isOnline = false;
    bool isBanned = false;
    QString banReason;
    QDateTime lastSeen;

    bool isValid() const { return id > 0 && !login.isEmpty(); }
};

// Вложение к сообщению (картинка / файл)
struct Attachment {
    qint64 id = 0;
    QString filename;
    QString mimeType;
    qint64 sizeBytes = 0;
    QByteArray data; // содержимое файла (на сервер передаётся как base64)

    bool isEmpty() const { return data.isEmpty(); }
};

// Сообщение
struct Message {
    qint64 id = 0;
    qint64 senderId = 0;
    qint64 receiverId = 0;     // 0 для broadcast
    bool isBroadcast = false;
    QString body;
    QDateTime createdAt;
    QDateTime editedAt;        // невалидный QDateTime = не редактировалось
    QDateTime deletedAt;       // невалидный QDateTime = не удалено
    QString senderLogin;       // удобство для UI, заполняется на сервере
    Attachment attachment;     // пустой если нет
};

// Лимит размера вложения (10 МБ)
constexpr qint64 kMaxAttachmentSize = 10LL * 1024 * 1024;

} // namespace messenger

#include <QMetaType>
Q_DECLARE_METATYPE(messenger::Message)
Q_DECLARE_METATYPE(messenger::User)
Q_DECLARE_METATYPE(messenger::Attachment)
Q_DECLARE_METATYPE(messenger::ResultCode)