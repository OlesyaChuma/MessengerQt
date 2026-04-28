#include <QtTest/QtTest>

#include "Database.h"

using namespace messenger;
using namespace messenger::server;

class TestDatabase : public QObject {
    Q_OBJECT

private:
    Database* _db = nullptr;

private slots:
    void init() {
        _db = new Database();
        QString err;
        QVERIFY2(_db->openSqlite(":memory:", &err), qPrintable(err));
        QVERIFY2(_db->initSchema(&err), qPrintable(err));
    }

    void cleanup() {
        delete _db;
        _db = nullptr;
    }

    void emptyDatabase() {
        QVERIFY(!_db->hasAnyAdmin());
        QVERIFY(_db->listAllUsers().isEmpty());
        QVERIFY(!_db->isLoginTaken("anyone"));
    }

    void createAndFindUser() {
        const qint64 id = _db->createUser("alice", "Alice",
                                          "hash123", "saltabc",
                                          UserRole::Regular);
        QVERIFY(id > 0);
        QVERIFY(_db->isLoginTaken("alice"));

        auto found = _db->findUserByLogin("alice");
        QVERIFY(found.has_value());
        QCOMPARE(found->id, id);
        QCOMPARE(found->displayName, QStringLiteral("Alice"));
        QCOMPARE(found->passwordHash, QStringLiteral("hash123"));
        QCOMPARE(found->role, UserRole::Regular);

        auto byId = _db->findUserById(id);
        QVERIFY(byId.has_value());
        QCOMPARE(byId->login, QStringLiteral("alice"));
    }

    void duplicateLoginRejected() {
        QVERIFY(_db->createUser("bob", "Bob", "h", "s") > 0);
        const qint64 second = _db->createUser("bob", "Bob 2", "h", "s");
        QCOMPARE(second, qint64(-1));
    }

    void hasAnyAdminTracksRole() {
        _db->createUser("u1", "User",  "h", "s", UserRole::Regular);
        QVERIFY(!_db->hasAnyAdmin());

        _db->createUser("a1", "Admin", "h", "s", UserRole::Admin);
        QVERIFY(_db->hasAnyAdmin());
    }

    void banUnban() {
        const qint64 admin = _db->createUser("ad", "Admin", "h", "s",
                                             UserRole::Admin);
        const qint64 user  = _db->createUser("us", "User",  "h", "s");

        QVERIFY(_db->setBanned(user, true, "spam", admin));
        auto rec = _db->findUserById(user);
        QVERIFY(rec.has_value());
        QVERIFY(rec->isBanned);
        QCOMPARE(rec->banReason, QStringLiteral("spam"));

        QVERIFY(_db->setBanned(user, false));
        rec = _db->findUserById(user);
        QVERIFY(!rec->isBanned);
    }

    void saveAndLoadMessages() {
        const qint64 alice = _db->createUser("alice", "A", "h", "s");
        const qint64 bob   = _db->createUser("bob",   "B", "h", "s");

        const qint64 m1 = _db->saveMessage(alice, bob, false, "hi bob");
        const qint64 m2 = _db->saveMessage(bob, alice, false, "hi alice");
        const qint64 m3 = _db->saveMessage(alice, 0,   true,  "hello all");

        QVERIFY(m1 > 0);
        QVERIFY(m2 > 0);
        QVERIFY(m3 > 0);

        const auto privateHistory = _db->loadHistory(alice, bob, 0, 50);
        QCOMPARE(privateHistory.size(), 2);
        QCOMPARE(privateHistory[0].body, QStringLiteral("hi bob"));
        QCOMPARE(privateHistory[1].body, QStringLiteral("hi alice"));

        const auto broadcast = _db->loadHistory(alice, 0, 0, 50);
        QCOMPARE(broadcast.size(), 1);
        QCOMPARE(broadcast[0].body, QStringLiteral("hello all"));
        QVERIFY(broadcast[0].isBroadcast);
    }

    void editAndDeleteByOwnerOnly() {
        const qint64 alice = _db->createUser("alice", "A", "h", "s");
        const qint64 bob   = _db->createUser("bob",   "B", "h", "s");
        const qint64 m = _db->saveMessage(alice, bob, false, "original");

        // Bob не может редактировать чужое
        QVERIFY(!_db->editMessageBody(m, bob, "hacked"));
        // Alice — может
        QVERIFY(_db->editMessageBody(m, alice, "edited"));

        auto found = _db->findMessageById(m);
        QVERIFY(found.has_value());
        QCOMPARE(found->body, QStringLiteral("edited"));
        QVERIFY(found->editedAt.isValid());

        // Bob не может удалить чужое
        QVERIFY(!_db->deleteMessage(m, bob));
        // Alice — может
        QVERIFY(_db->deleteMessage(m, alice));

        // После удаления соо не возвращается в loadHistory
        QVERIFY(_db->loadHistory(alice, bob, 0, 50).isEmpty());
    }

    void historyPagination() {
        const qint64 a = _db->createUser("a", "A", "h", "s");
        const qint64 b = _db->createUser("b", "B", "h", "s");

        QList<qint64> ids;
        for (int i = 0; i < 10; ++i) {
            ids << _db->saveMessage(a, b, false, QString("m%1").arg(i));
        }

        const auto firstPage = _db->loadHistory(a, b, 0, 3);
        QCOMPARE(firstPage.size(), 3);
        // последние три (m7, m8, m9) в хронологическом порядке
        QCOMPARE(firstPage.last().body, QStringLiteral("m9"));

        const qint64 oldestInFirst = firstPage.first().id;
        const auto secondPage = _db->loadHistory(a, b, oldestInFirst, 3);
        QCOMPARE(secondPage.size(), 3);
        QVERIFY(secondPage.last().id < oldestInFirst);
    }

    void messageWithAttachment() {
        const qint64 alice = _db->createUser("alice", "A", "h", "s");
        const qint64 bob   = _db->createUser("bob",   "B", "h", "s");

        Attachment att;
        att.filename  = "pic.png";
        att.mimeType  = "image/png";
        att.data      = QByteArray("\x89PNG_BINARY", 11);
        att.sizeBytes = att.data.size();

        const qint64 m = _db->saveMessage(alice, bob, false, "see pic", att);
        QVERIFY(m > 0);

        auto loaded = _db->loadAttachment(m);
        QVERIFY(loaded.has_value());
        QCOMPARE(loaded->filename, QStringLiteral("pic.png"));
        QCOMPARE(loaded->data, att.data);
    }

    void connectionLog() {
        const qint64 u = _db->createUser("u", "U", "h", "s");
        QVERIFY(_db->logConnectionEvent(u, "u", "127.0.0.1",
                                        ConnectionEvent::Login));
        QVERIFY(_db->logConnectionEvent(u, "u", "127.0.0.1",
                                        ConnectionEvent::Logout));

        const auto rows = _db->loadConnectionLog(10);
        QCOMPARE(rows.size(), 2);
        // По умолчанию DESC по id, поэтому первое — Logout
        QCOMPARE(rows[0].event, ConnectionEvent::Logout);
        QCOMPARE(rows[1].event, ConnectionEvent::Login);
    }
};

QTEST_MAIN (TestDatabase)
#include "test_database.moc"
