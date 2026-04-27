#include <QtTest/QtTest>

#include "PasswordHasher.h"

using namespace messenger;

class TestPasswordHasher : public QObject {
    Q_OBJECT
private slots:
    void saltLengthAndHex() {
        const auto salt = PasswordHasher::generateSalt();
        QCOMPARE(salt.size(), PasswordHasher::kSaltLengthBytes * 2);
        for (auto ch : salt) {
            QVERIFY((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f'));
        }
    }

    void saltsAreUnique() {
        const auto a = PasswordHasher::generateSalt();
        const auto b = PasswordHasher::generateSalt();
        QVERIFY(a != b);
    }

    void hashIsDeterministicForSameSalt() {
        const QString salt = PasswordHasher::generateSalt();
        const QString h1 = PasswordHasher::hashPassword("hunter2", salt);
        const QString h2 = PasswordHasher::hashPassword("hunter2", salt);
        QCOMPARE(h1, h2);
        QCOMPARE(h1.size(), 64); // SHA-256 hex
    }

    void differentSaltsProduceDifferentHashes() {
        const auto s1 = PasswordHasher::generateSalt();
        const auto s2 = PasswordHasher::generateSalt();
        const auto h1 = PasswordHasher::hashPassword("samepass", s1);
        const auto h2 = PasswordHasher::hashPassword("samepass", s2);
        QVERIFY(h1 != h2);
    }

    void verifyCorrectPassword() {
        const QString salt = PasswordHasher::generateSalt();
        const QString hash = PasswordHasher::hashPassword("MyP@ssw0rd", salt);
        QVERIFY(PasswordHasher::verify("MyP@ssw0rd", salt, hash));
    }

    void verifyWrongPassword() {
        const QString salt = PasswordHasher::generateSalt();
        const QString hash = PasswordHasher::hashPassword("correct", salt);
        QVERIFY(!PasswordHasher::verify("WRONG", salt, hash));
    }

    void verifyHandlesUnicode() {
        const QString salt = PasswordHasher::generateSalt();
        const QString password = QString::fromUtf8("Пароль42!");
        const QString hash = PasswordHasher::hashPassword(password, salt);
        QVERIFY(PasswordHasher::verify(password, salt, hash));
        QVERIFY(!PasswordHasher::verify(QString::fromUtf8("пароль42!"),
                                        salt, hash));
    }
};

QTEST_APPLESS_MAIN(TestPasswordHasher)
#include "test_password.moc"