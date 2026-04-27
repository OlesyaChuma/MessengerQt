#pragma once

#include <QByteArray>
#include <QString>

namespace messenger {

// Хеширование паролей: SHA-256 с per-user случайной солью.
//
// Формат хранения в БД:
//   password_salt CHAR(32)  - 16 случайных байт в hex
//   password_hash CHAR(64)  - SHA-256(salt_bytes + utf8(password)) в hex

class PasswordHasher {
public:
    // Длина соли в байтах (хранится в hex => 32 символа).
    static constexpr int kSaltLengthBytes = 16;

    // Сгенерировать новую случайную соль и вернуть в hex (lowercase, 32 символа).
    static QString generateSalt();

    // Посчитать SHA-256(saltBytes + utf8(password)), вернуть в hex (64 символа).
    // saltHex принимается в hex (как из generateSalt / БД).
    static QString hashPassword(const QString& password, const QString& saltHex);

    // Проверка пароля: повторно хеширует и сравнивает в постоянное время.
    static bool verify(const QString& password,
                       const QString& saltHex,
                       const QString& expectedHashHex);
};

} // namespace messenger