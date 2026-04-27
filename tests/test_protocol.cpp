#include <QtTest/QtTest>
#include <QJsonDocument>

#include "Protocol.h"

using namespace messenger;

class TestProtocol : public QObject {
    Q_OBJECT
private slots:
    void commandTypeRoundtrip() {
        for (auto cmd : { CommandType::Register, CommandType::Login,
                          CommandType::SendMessage, CommandType::NewMessage,
                          CommandType::Error, CommandType::Ok }) {
            const auto str = commandTypeToString(cmd);
            QCOMPARE(commandTypeFromString(str), cmd);
        }
    }

    void unknownCommandFallback() {
        QCOMPARE(commandTypeFromString("__nope__"), CommandType::Unknown);
    }

    void resultCodeRoundtrip() {
        for (auto code : { ResultCode::Ok, ResultCode::InvalidCredentials,
                           ResultCode::UserBanned, ResultCode::DatabaseError }) {
            const auto str = resultCodeToString(code);
            QCOMPARE(resultCodeFromString(str), code);
        }
    }

    void userJsonRoundtrip() {
        User u;
        u.id = 42;
        u.login = "alice";
        u.displayName = QString::fromUtf8("Алиса");
        u.role = UserRole::Admin;
        u.isOnline = true;
        u.isBanned = false;
        u.lastSeen = QDateTime::currentDateTimeUtc();

        const User out = userFromJson(userToJson(u));
        QCOMPARE(out.id, u.id);
        QCOMPARE(out.login, u.login);
        QCOMPARE(out.displayName, u.displayName);
        QCOMPARE(out.role, u.role);
        QCOMPARE(out.isOnline, u.isOnline);
    }

    void messageJsonRoundtripWithAttachment() {
        Message m;
        m.id = 100;
        m.senderId = 1;
        m.receiverId = 2;
        m.isBroadcast = false;
        m.body = QString::fromUtf8("Привет, мир!");
        m.createdAt = QDateTime::currentDateTimeUtc();
        m.attachment.filename = "photo.png";
        m.attachment.mimeType = "image/png";
        m.attachment.data = QByteArray("\x89PNG\r\n\x1a\n", 8);
        m.attachment.sizeBytes = m.attachment.data.size();

        const Message out = messageFromJson(messageToJson(m));
        QCOMPARE(out.id, m.id);
        QCOMPARE(out.senderId, m.senderId);
        QCOMPARE(out.body, m.body);
        QCOMPARE(out.attachment.filename, m.attachment.filename);
        QCOMPARE(out.attachment.data, m.attachment.data);
    }

    void envelopeBytesRoundtrip() {
        Envelope env(CommandType::Login, {
            {"login", "bob"},
            {"password", "secret"}
        }, 7);

        const QByteArray bytes = env.toBytes();
        QVERIFY(!bytes.isEmpty());

        bool ok = false;
        const Envelope back = Envelope::fromBytes(bytes, &ok);
        QVERIFY(ok);
        QCOMPARE(back.cmd(), CommandType::Login);
        QCOMPARE(back.requestId(), 7u);
        QCOMPARE(back.payload().value("login").toString(), QStringLiteral("bob"));
    }

    void envelopeFromGarbage() {
        bool ok = true;
        const Envelope env = Envelope::fromBytes("not a json", &ok);
        QVERIFY(!ok);
        QCOMPARE(env.cmd(), CommandType::Unknown);
    }

    void makeErrorResponse() {
        const auto env = make::errorResponse(ResultCode::UserBanned,
                                             "You are blocked", 13);
        QCOMPARE(env.cmd(), CommandType::Error);
        QCOMPARE(env.requestId(), 13u);
        QCOMPARE(env.payload().value("code").toString(), QStringLiteral("user_banned"));
    }
};

QTEST_APPLESS_MAIN(TestProtocol)
#include "test_protocol.moc"