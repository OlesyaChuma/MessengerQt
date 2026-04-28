#include "AuthService.h"
#include "PasswordHasher.h"

namespace messenger::server {

AuthService::AuthService(Database& db) : _db(db) {}

bool AuthService::hasAnyAdmin() const {
    return _db.hasAnyAdmin();
}

AuthService::Result AuthService::registerImpl(const QString& login,
                                              const QString& displayName,
                                              const QString& password,
                                              UserRole role) {
    if (login.trimmed().isEmpty() || password.isEmpty() ||
        displayName.trimmed().isEmpty()) {
        return ResultCode::InvalidRequest;
    }

    if (_db.isLoginTaken(login)) {
        return ResultCode::LoginAlreadyTaken;
    }

    const QString salt = PasswordHasher::generateSalt();
    const QString hash = PasswordHasher::hashPassword(password, salt);

    const qint64 id = _db.createUser(login, displayName, hash, salt, role);
    if (id <= 0) {
        return ResultCode::DatabaseError;
    }

    auto rec = _db.findUserById(id);
    if (!rec) return ResultCode::DatabaseError;

    _db.logConnectionEvent(id, login, {}, ConnectionEvent::Register);
    return *rec;
}

AuthService::Result AuthService::registerUser(const QString& login,
                                              const QString& displayName,
                                              const QString& password) {
    return registerImpl(login, displayName, password, UserRole::Regular);
}

AuthService::Result AuthService::registerAdmin(const QString& login,
                                               const QString& displayName,
                                               const QString& password) {
    return registerImpl(login, displayName, password, UserRole::Admin);
}

AuthService::Result AuthService::login(const QString& login_,
                                       const QString& password) {
    if (login_.trimmed().isEmpty() || password.isEmpty()) {
        return ResultCode::InvalidRequest;
    }

    auto rec = _db.findUserByLogin(login_);
    if (!rec) {
        _db.logConnectionEvent(0, login_, {}, ConnectionEvent::FailedLogin,
                               "user not found");
        return ResultCode::InvalidCredentials;
    }

    if (rec->isBanned) {
        _db.logConnectionEvent(rec->id, login_, {}, ConnectionEvent::FailedLogin,
                               "user is banned");
        return ResultCode::UserBanned;
    }

    if (!PasswordHasher::verify(password, rec->passwordSalt, rec->passwordHash)) {
        _db.logConnectionEvent(rec->id, login_, {}, ConnectionEvent::FailedLogin,
                               "wrong password");
        return ResultCode::InvalidCredentials;
    }

    _db.updateLastSeen(rec->id, QDateTime::currentDateTimeUtc());
    return *rec;
}

} // namespace messenger::server