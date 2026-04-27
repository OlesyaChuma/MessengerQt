#include "PasswordHasher.h"

#include <QCryptographicHash>
#include <QRandomGenerator>

namespace messenger {

QString PasswordHasher::generateSalt() {
    QByteArray salt(kSaltLengthBytes, Qt::Uninitialized);

    auto* rng = QRandomGenerator::system();
    for (int i = 0; i < kSaltLengthBytes; ++i) {
        salt[i] = static_cast<char>(rng->bounded(256));
    }

    return QString::fromLatin1(salt.toHex());
}

QString PasswordHasher::hashPassword(const QString& password, const QString& saltHex) {
    const QByteArray saltBytes = QByteArray::fromHex(saltHex.toLatin1());

    QCryptographicHash hasher(QCryptographicHash::Sha256);
    hasher.addData(saltBytes);
    hasher.addData(password.toUtf8());

    return QString::fromLatin1(hasher.result().toHex());
}

bool PasswordHasher::verify(const QString& password,
                            const QString& saltHex,
                            const QString& expectedHashHex) {
    const QString actual = hashPassword(password, saltHex);

    // Сравнение в постоянное время: чтобы не было side-channel по тайнингу.
    if (actual.size() != expectedHashHex.size()) {
        return false;
    }

    int diff = 0;
    for (int i = 0; i < actual.size(); ++i) {
        diff |= static_cast<int>(actual[i].unicode()) ^
                static_cast<int>(expectedHashHex[i].unicode());
    }
    return diff == 0;
}

} // namespace messenger