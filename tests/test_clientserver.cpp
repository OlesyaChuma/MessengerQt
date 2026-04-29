#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QCoreApplication>
#include <QTcpServer>
#include <QHostAddress>
#include <QElapsedTimer>

#include "Database.h"
#include "AuthService.h"
#include "ChatServer.h"
#include "ChatClientCore.h"

using namespace messenger;
using namespace messenger::server;
using namespace messenger::client;

namespace {

// Найти свободный порт, чтобы тесты не конфликтовали друг с другом
quint16 findFreePort() {
    QTcpServer probe;
    probe.listen(QHostAddress::LocalHost, 0);
    const quint16 p = probe.serverPort();
    probe.close();
    return p;
}

// Подождать одно событие или истечь таймаут
bool waitForOne(QSignalSpy& spy, int ms = 5000) {
    if (!spy.isEmpty()) return true;
    return spy.wait(ms);
}

// Подождать конкретное состояние клиента (любым числом переходов)
bool waitForState(ChatClientCore& core, ChatClientCore::State target,
                  int totalMs = 5000) {
    if (core.state() == target) return true;
    QElapsedTimer t; t.start();
    while (core.state() != target) {
        if (t.elapsed() > totalMs) return false;
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    }
    return true;
}

// Ждём пока пройдут все ожидающие события Qt — для устаканивания сигналов
void drainEvents(int ms = 50) {
    QElapsedTimer t; t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
}

// Регистрация и логин одного клиента — общая последовательность
bool registerAndLogin(ChatClientCore& core,
                      const QString& login, const QString& name,
                      const QString& password,
                      quint16 port) {
    core.connectToServer("127.0.0.1", port);
    if (!waitForState(core, ChatClientCore::State::Connected)) return false;

    QSignalSpy okSpy(&core, &ChatClientCore::requestSucceeded);
    QSignalSpy errSpy(&core, &ChatClientCore::requestFailed);

    core.sendRegister(login, name, password);
    // Дождёмся либо ok, либо err
    if (okSpy.isEmpty() && errSpy.isEmpty()) {
        // ждём первый из двух
        QElapsedTimer t; t.start();
        while (okSpy.isEmpty() && errSpy.isEmpty() && t.elapsed() < 5000) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        }
    }
    if (!errSpy.isEmpty()) {
        // Регистрация может провалиться (например, логин занят) —
        // в наших тестах БД свежая, такого быть не должно.
        return false;
    }

    okSpy.clear();
    core.sendLogin(login, password);
    if (!waitForState(core, ChatClientCore::State::Authenticated)) return false;

    return true;
}

} // namespace

class TestClientServer : public QObject {
    Q_OBJECT

private:
    Database*    _db = nullptr;
    AuthService* _auth = nullptr;
    ChatServer*  _server = nullptr;
    quint16      _port = 0;

private slots:
    void init() {
        _db = new Database();
        QVERIFY(_db->openSqlite(":memory:"));
        QVERIFY(_db->initSchema());
        _auth = new AuthService(*_db);

        ListenConfig cfg;
        _port = findFreePort();
        cfg.host = "127.0.0.1";
        cfg.port = _port;

        _server = new ChatServer(_db, _auth, cfg);
        QString err;
        QVERIFY2(_server->start(&err), qPrintable(err));
    }

    void cleanup() {
        delete _server; _server = nullptr;
        delete _auth;   _auth = nullptr;
        delete _db;     _db = nullptr;
        drainEvents(100); // дать сокетам корректно умереть
    }

    void registerLoginAndBroadcast() {
        ChatClientCore alice;
        ChatClientCore bob;

        QVERIFY(registerAndLogin(alice, "alice", "Alice", "pwd1", _port));
        QVERIFY(registerAndLogin(bob,   "bob",   "Bob",   "pwd2", _port));

        QVERIFY(alice.isAuthenticated());
        QVERIFY(bob.isAuthenticated());
        QVERIFY(alice.currentUserId() > 0);
        QVERIFY(bob.currentUserId() > 0);

        // Broadcast: Alice пишет, Bob получает
        QSignalSpy bobNew(&bob, &ChatClientCore::newMessageArrived);
        alice.sendBroadcast("hello world");

        QVERIFY(waitForOne(bobNew));
        const auto args = bobNew.takeFirst();
        const Message m = args.at(0).value<Message>();
        QCOMPARE(m.body, QStringLiteral("hello world"));
        QVERIFY(m.isBroadcast);
        QCOMPARE(m.senderLogin, QStringLiteral("alice"));
    }

    void privateMessageVisibility() {
        ChatClientCore alice, bob, carol;

        QVERIFY(registerAndLogin(alice, "a", "A", "p", _port));
        QVERIFY(registerAndLogin(bob,   "b", "B", "p", _port));
        QVERIFY(registerAndLogin(carol, "c", "C", "p", _port));

        // Спим чуток, чтобы push-события UserOnline всех успели дойти
        drainEvents(100);

        const qint64 bobId = bob.currentUserId();
        QVERIFY(bobId > 0);

        QSignalSpy bobNew(&bob, &ChatClientCore::newMessageArrived);
        QSignalSpy carolNew(&carol, &ChatClientCore::newMessageArrived);
        alice.sendPrivateMessage(bobId, "secret");

        QVERIFY(waitForOne(bobNew));
        // Carol НЕ должна получить — даже если ждать дольше
        drainEvents(300);
        QCOMPARE(carolNew.size(), 0);

        const Message m = bobNew.first().at(0).value<Message>();
        QCOMPARE(m.body, QStringLiteral("secret"));
        QVERIFY(!m.isBroadcast);
    }

    void bannedUserCannotLogin() {
        // Регистрируем bob, баним напрямую через БД, пробуем логиниться
        const qint64 bobId = std::get<UserRecord>(
                                 _auth->registerUser("bob", "Bob", "pp")).id;

        QVERIFY(_db->setBanned(bobId, true, "no reason", 0));

        ChatClientCore client;
        client.connectToServer("127.0.0.1", _port);
        QVERIFY(waitForState(client, ChatClientCore::State::Connected));

        QSignalSpy failSpy(&client, &ChatClientCore::requestFailed);
        QSignalSpy banSpy(&client,  &ChatClientCore::bannedByServer);

        client.sendLogin("bob", "pp");
        QVERIFY(waitForOne(failSpy));
        const auto fa = failSpy.takeFirst();
        QCOMPARE(fa.at(1).value<ResultCode>(), ResultCode::UserBanned);

        // Дополнительно сервер шлёт Banned событие
        QVERIFY(waitForOne(banSpy));
    }

    void editAndDelete() {
        ChatClientCore alice, bob;
        QVERIFY(registerAndLogin(alice, "a", "A", "p", _port));
        QVERIFY(registerAndLogin(bob,   "b", "B", "p", _port));
        drainEvents(100);

        // Alice отправляет broadcast и ждёт ответ с message.id
        QSignalSpy aSendOk(&alice, &ChatClientCore::requestSucceeded);
        const auto rid = alice.sendBroadcast("original");
        QVERIFY(waitForOne(aSendOk));
        const auto args = aSendOk.takeFirst();
        QCOMPARE(quint32(args.at(0).toUInt()), rid);
        const auto payload = args.at(1).toJsonObject();
        const auto msg = messageFromJson(payload.value("message").toObject());
        const qint64 mid = msg.id;
        QVERIFY(mid > 0);

        QSignalSpy bobEdit(&bob, &ChatClientCore::messageEdited);
        QSignalSpy bobDel (&bob, &ChatClientCore::messageDeleted);

        // Edit
        alice.sendEdit(mid, "edited");
        QVERIFY(waitForOne(bobEdit));
        QCOMPARE(bobEdit.first().at(1).toString(), QStringLiteral("edited"));

        // Delete
        alice.sendDelete(mid);
        QVERIFY(waitForOne(bobDel));
        QCOMPARE(bobDel.first().at(0).toLongLong(), mid);
    }
};

QTEST_MAIN(TestClientServer)
#include "test_clientserver.moc"
