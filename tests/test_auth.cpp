#include <QtTest/QtTest>

#include "AuthService.h"
#include "Database.h"

using namespace messenger;
using namespace messenger::server;

class TestAuthService : public QObject {
    Q_OBJECT

private:
    Database* _db = nullptr;
    AuthService* _auth = nullptr;

    template <class T>
    static T* getIfRecord(AuthService::Result& r) {
        return std::get_if<T>(&r);
    }

private slots:
    void init() {
        _db = new Database();
        QVERIFY(_db->openSqlite(":memory:"));
        QVERIFY(_db->initSchema());
        _auth = new AuthService(*_db);
    }

    void cleanup() {
        delete _auth;
        delete _db;
        _auth = nullptr;
        _db = nullptr;
    }

    void registerCreatesUser() {
        auto r = _auth->registerUser("alice", "Alice", "secret123");
        auto* rec = getIfRecord<UserRecord>(r);
        QVERIFY(rec != nullptr);
        QCOMPARE(rec->login, QStringLiteral("alice"));
        QCOMPARE(rec->role, UserRole::Regular);
    }

    void registerRejectsEmpty() {
        auto r = _auth->registerUser("", "X", "p");
        auto* code = std::get_if<ResultCode>(&r);
        QVERIFY(code != nullptr);
        QCOMPARE(*code, ResultCode::InvalidRequest);
    }

    void registerDuplicate() {
        QVERIFY(getIfRecord<UserRecord>(
            *new AuthService::Result(_auth->registerUser("bob", "Bob", "p"))) != nullptr);

        auto r = _auth->registerUser("bob", "Bob 2", "p");
        auto* code = std::get_if<ResultCode>(&r);
        QVERIFY(code != nullptr);
        QCOMPARE(*code, ResultCode::LoginAlreadyTaken);
    }

    void loginRequiresCorrectPassword() {
        QVERIFY(std::holds_alternative<UserRecord>(
            _auth->registerUser("eve", "Eve", "rightpass")));

        // Правильный пароль
        auto good = _auth->login("eve", "rightpass");
        QVERIFY(std::holds_alternative<UserRecord>(good));

        // Неправильный пароль
        auto bad = _auth->login("eve", "WRONG");
        auto* code = std::get_if<ResultCode>(&bad);
        QVERIFY(code != nullptr);
        QCOMPARE(*code, ResultCode::InvalidCredentials);

        // Несуществующий пользователь
        auto missing = _auth->login("ghost", "any");
        code = std::get_if<ResultCode>(&missing);
        QVERIFY(code != nullptr);
        QCOMPARE(*code, ResultCode::InvalidCredentials);
    }

    void bannedUserCannotLogin() {
        auto r = _auth->registerUser("u", "User", "pp");
        auto* rec = std::get_if<UserRecord>(&r);
        QVERIFY(rec != nullptr);

        QVERIFY(_db->setBanned(rec->id, true, "reason", 0));

        auto bad = _auth->login("u", "pp");
        auto* code = std::get_if<ResultCode>(&bad);
        QVERIFY(code != nullptr);
        QCOMPARE(*code, ResultCode::UserBanned);
    }

    void firstAdminFlow() {
        QVERIFY(!_auth->hasAnyAdmin());

        auto r = _auth->registerAdmin("root", "Root", "supersecret");
        QVERIFY(std::holds_alternative<UserRecord>(r));
        QVERIFY(_auth->hasAnyAdmin());

        auto good = _auth->login("root", "supersecret");
        auto* rec = std::get_if<UserRecord>(&good);
        QVERIFY(rec != nullptr);
        QCOMPARE(rec->role, UserRole::Admin);
    }
};

QTEST_MAIN(TestAuthService)
#include "test_auth.moc"
