#pragma once

#include "Database.h"
#include "Models.h"

#include <variant>

namespace messenger::server {

// Сервис аутентификации поверх Database.
// Возвращает либо UserRecord (успех), либо ResultCode (отказ).
class AuthService {
public:
    explicit AuthService(Database& db);

    using Result = std::variant<UserRecord, ResultCode>;

    // Регистрация. Создаёт обычного пользователя.
    // Не выполняет автоматический логин — это ответственность вызывающего.
    Result registerUser(const QString& login,
                        const QString& displayName,
                        const QString& password);

    // Регистрация админа (используется при первом запуске).
    Result registerAdmin(const QString& login,
                         const QString& displayName,
                         const QString& password);

    // Проверка логина и пароля.
    Result login(const QString& login, const QString& password);

    // Есть ли в системе хоть один действующий админ.
    bool hasAnyAdmin() const;

private:
    Database& _db;

    Result registerImpl(const QString& login,
                        const QString& displayName,
                        const QString& password,
                        UserRole role);
};

} // namespace messenger::server