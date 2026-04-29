#include "ConsoleUi.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QJsonArray>
#include <QStringList>
#include <QMetaObject>
#include <QPointer>
#include <iostream>
#include <string>
#include <thread>

namespace messenger::client {

ConsoleUi::ConsoleUi(ChatClientCore* core,
                     const QString& host, quint16 port,
                     QObject* parent)
    : QObject(parent), _core(core), _host(host), _port(port) {
    connect(_core, &ChatClientCore::stateChanged, this, &ConsoleUi::onStateChanged);
    connect(_core, &ChatClientCore::serverError,  this, &ConsoleUi::onServerError);
    connect(_core, &ChatClientCore::requestSucceeded, this, &ConsoleUi::onRequestSucceeded);
    connect(_core, &ChatClientCore::requestFailed,    this, &ConsoleUi::onRequestFailed);

    connect(_core, &ChatClientCore::newMessageArrived, this, &ConsoleUi::onNewMessageArrived);
    connect(_core, &ChatClientCore::messageEdited,     this, &ConsoleUi::onMessageEdited);
    connect(_core, &ChatClientCore::messageDeleted,    this, &ConsoleUi::onMessageDeleted);
    connect(_core, &ChatClientCore::userOnline,        this, &ConsoleUi::onUserOnline);
    connect(_core, &ChatClientCore::userOffline,       this, &ConsoleUi::onUserOffline);
    connect(_core, &ChatClientCore::kickedByServer,    this, &ConsoleUi::onKicked);
    connect(_core, &ChatClientCore::bannedByServer,    this, &ConsoleUi::onBanned);
}

ConsoleUi::~ConsoleUi() {
    _shuttingDown.store(true);
}

void ConsoleUi::start() {
    println("=== MessengerQt console client ===");
    println(QString("Connecting to %1:%2 ...").arg(_host).arg(_port));
    _core->connectToServer(_host, _port);

    // Поток stdin — detach, чтобы не было QThread проблем при выходе.
    QPointer<ConsoleUi> guard(this);
    std::thread([guard]() {
        std::string line;
        while (std::getline(std::cin, line)) {
            if (!guard) return;
            if (guard->_shuttingDown.load()) return;
            const QString q = QString::fromLocal8Bit(line.c_str());
            QMetaObject::invokeMethod(guard.data(), "onLineRead",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, q));
        }
        // EOF
        if (guard && !guard->_shuttingDown.load()) {
            QMetaObject::invokeMethod(guard.data(), "shutdownRequested",
                                      Qt::QueuedConnection);
        }
    }).detach();
}

void ConsoleUi::println(const QString& s) const {
    std::cout << s.toLocal8Bit().constData() << std::endl;
}

void ConsoleUi::prompt() const {
    std::cout << "> " << std::flush;
}

void ConsoleUi::printHelp() const {
    println("");
    println("Commands:");
    println("  /help                                 - this help");
    println("  /register <login> <name> <password>   - register new user");
    println("  /login <login> <password>             - login");
    println("  /logout                               - logout");
    println("  /users                                - list all users");
    println("  /history                              - load common chat history");
    println("  /history <login>                      - load private history with <login>");
    println("  /to <login> <text>                    - send private message");
    println("  /edit <message_id> <new text>         - edit your message");
    println("  /delete <message_id>                  - delete your message");
    println("  /quit                                 - exit");
    println("  <text>                                - send to common chat");
    println("");
}

void ConsoleUi::onLineRead(const QString& raw) {
    if (_shuttingDown.load()) return;

    const QString line = raw.trimmed();
    if (line.isEmpty()) { prompt(); return; }

    if (line.startsWith('/')) {
        const QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        const QString cmd = parts.first().toLower();
        const QStringList args = parts.mid(1);

        if      (cmd == "/help")     printHelp();
        else if (cmd == "/register") cmdRegister(args);
        else if (cmd == "/login")    cmdLogin(args);
        else if (cmd == "/logout")   cmdLogout();
        else if (cmd == "/users")    cmdUsers();
        else if (cmd == "/history")  cmdHistory(args);
        else if (cmd == "/to")       cmdSendPrivate(args);
        else if (cmd == "/edit")     cmdEdit(args);
        else if (cmd == "/delete")   cmdDelete(args);
        else if (cmd == "/quit" || cmd == "/exit") {
            _shuttingDown.store(true);
            emit shutdownRequested();
            return;
        }
        else println(QString("Unknown command: %1. Type /help.").arg(cmd));
    } else {
        // Обычный текст — broadcast
        cmdSendBroadcast({line});
    }

    prompt();
}

// ---------- Команды ----------

void ConsoleUi::cmdRegister(const QStringList& args) {
    if (args.size() < 3) { println("Usage: /register <login> <name> <password>"); return; }
    const QString login = args[0];
    const QString name  = args[1];
    const QString pass  = args.mid(2).join(' ');
    const auto rid = _core->sendRegister(login, name, pass);
    _requestLabel.insert(rid, "register");
}

void ConsoleUi::cmdLogin(const QStringList& args) {
    if (args.size() < 2) { println("Usage: /login <login> <password>"); return; }
    const auto rid = _core->sendLogin(args[0], args.mid(1).join(' '));
    _requestLabel.insert(rid, "login");
}

void ConsoleUi::cmdLogout() {
    if (!_core->isAuthenticated()) { println("Not logged in."); return; }
    const auto rid = _core->sendLogout();
    _requestLabel.insert(rid, "logout");
}

void ConsoleUi::cmdUsers() {
    if (!_core->isAuthenticated()) { println("Login first."); return; }
    const auto rid = _core->requestUsers();
    _requestLabel.insert(rid, "users");
}

void ConsoleUi::cmdHistory(const QStringList& args) {
    if (!_core->isAuthenticated()) { println("Login first."); return; }
    qint64 peerId = 0;
    if (!args.isEmpty()) {
        const auto peerLogin = args.first();
        // Найдём id в кэше
        for (auto it = _userLoginCache.cbegin(); it != _userLoginCache.cend(); ++it) {
            if (it.value() == peerLogin) { peerId = it.key(); break; }
        }
        if (peerId == 0) {
            println("Unknown user. Run /users first.");
            return;
        }
    }
    const auto rid = _core->requestHistory(peerId);
    _requestLabel.insert(rid, peerId == 0 ? "history broadcast" : "history with " + args.first());
}

void ConsoleUi::cmdSendBroadcast(const QStringList& args) {
    if (!_core->isAuthenticated()) { println("Login first."); return; }
    const auto text = args.join(' ');
    if (text.isEmpty()) { println("Empty message."); return; }
    const auto rid = _core->sendBroadcast(text);
    _requestLabel.insert(rid, "send broadcast");
}

void ConsoleUi::cmdSendPrivate(const QStringList& args) {
    if (!_core->isAuthenticated()) { println("Login first."); return; }
    if (args.size() < 2) { println("Usage: /to <login> <text>"); return; }
    const auto receiverLogin = args[0];
    qint64 peerId = 0;
    for (auto it = _userLoginCache.cbegin(); it != _userLoginCache.cend(); ++it) {
        if (it.value() == receiverLogin) { peerId = it.key(); break; }
    }
    if (peerId == 0) {
        println("Unknown recipient. Run /users first.");
        return;
    }
    const auto text = args.mid(1).join(' ');
    const auto rid = _core->sendPrivateMessage(peerId, text);
    _requestLabel.insert(rid, "send to " + receiverLogin);
}

void ConsoleUi::cmdEdit(const QStringList& args) {
    if (!_core->isAuthenticated()) { println("Login first."); return; }
    if (args.size() < 2) { println("Usage: /edit <message_id> <new text>"); return; }
    const qint64 mid = args[0].toLongLong();
    const auto rid = _core->sendEdit(mid, args.mid(1).join(' '));
    _requestLabel.insert(rid, QString("edit #%1").arg(mid));
}

void ConsoleUi::cmdDelete(const QStringList& args) {
    if (!_core->isAuthenticated()) { println("Login first."); return; }
    if (args.isEmpty()) { println("Usage: /delete <message_id>"); return; }
    const qint64 mid = args[0].toLongLong();
    const auto rid = _core->sendDelete(mid);
    _requestLabel.insert(rid, QString("delete #%1").arg(mid));
}

// ---------- События от core ----------

void ConsoleUi::onStateChanged(ChatClientCore::State, ChatClientCore::State newS) {
    switch (newS) {
        case ChatClientCore::State::Connecting:
            println("Connecting...");
            break;
        case ChatClientCore::State::Connected:
            println("Connected. Use /register or /login. Type /help for commands.");
            prompt();
            break;
        case ChatClientCore::State::Authenticated:
            println(QString("Logged in as %1 (%2).")
                    .arg(_core->currentLogin())
                    .arg(_core->currentDisplayName()));
            // Сразу подгрузим список пользователей для удобства
            _requestLabel.insert(_core->requestUsers(), "users (auto)");
            prompt();
            break;
        case ChatClientCore::State::Disconnected:
            println("Disconnected from server.");
            if (!_shuttingDown.load()) {
                emit shutdownRequested();
            }
            break;
    }
}

void ConsoleUi::onServerError(const QString& desc) {
    println("Network error: " + desc);
}

void ConsoleUi::rememberUsers(const QJsonArray& arr) {
    _userLoginCache.clear();
    for (const auto& v : arr) {
        const auto u = userFromJson(v.toObject());
        _userLoginCache.insert(u.id, u.login);
    }
}

QString ConsoleUi::loginForId(qint64 id) const {
    return _userLoginCache.value(id, QString::number(id));
}

void ConsoleUi::renderMessage(const Message& m) const {
    const QString time = m.createdAt.toLocalTime().toString("HH:mm:ss");
    const QString sender = m.senderLogin.isEmpty()
        ? QString::number(m.senderId) : m.senderLogin;
    const QString tag = m.isBroadcast
        ? QString("[all]")
        : QString("[%1->%2]").arg(sender, loginForId(m.receiverId));
    const QString suffix = m.deletedAt.isValid() ? " (deleted)" :
                           m.editedAt.isValid()  ? " (edited)"  : "";
    println(QString("  #%1 %2 %3 %4: %5%6")
            .arg(m.id).arg(time).arg(tag, -25).arg(sender, -16).arg(m.body).arg(suffix));
}

void ConsoleUi::onRequestSucceeded(quint32 rid, const QJsonObject& payload) {
    const QString label = _requestLabel.take(rid);

    if (label == "users" || label == "users (auto)") {
        const auto arr = payload.value("users").toArray();
        rememberUsers(arr);
        if (label == "users") {
            println("Users:");
            for (const auto& v : arr) {
                const auto u = userFromJson(v.toObject());
                const auto status = u.isOnline ? "online" : "offline";
                const auto banned = u.isBanned ? " (BANNED)" : "";
                println(QString("  [%1] %2 %3 — %4%5")
                        .arg(u.id).arg(u.login, -16)
                        .arg(u.displayName, -25).arg(status).arg(banned));
            }
        }
        return;
    }

    if (label.startsWith("history")) {
        const auto arr = payload.value("messages").toArray();
        if (arr.isEmpty()) {
            println(QString("No messages in %1.").arg(label));
            return;
        }
        println(QString("--- %1 (%2 messages) ---").arg(label).arg(arr.size()));
        for (const auto& v : arr) {
            renderMessage(messageFromJson(v.toObject()));
        }
        return;
    }

    if (label == "register") {
        println("Registered successfully. Now /login.");
        return;
    }

    if (label == "login") {
        // Состояние и приветствие уже выводятся в onStateChanged
        return;
    }

    if (label == "logout") {
        // Сервер закроет соединение — disconnected обработается отдельно
        return;
    }

    if (label.startsWith("send ")) {
        if (payload.contains("message")) {
            const auto m = messageFromJson(payload.value("message").toObject());
            renderMessage(m);
        }
        return;
    }

    if (label.startsWith("edit ") || label.startsWith("delete ")) {
        println("OK: " + label);
        return;
    }
}

void ConsoleUi::onRequestFailed(quint32 rid, ResultCode code,
                                const QString& desc) {
    const QString label = _requestLabel.take(rid);
    println(QString("ERROR (%1): %2 — %3")
            .arg(label.isEmpty() ? "?" : label)
            .arg(resultCodeToString(code))
            .arg(desc));
}

void ConsoleUi::onNewMessageArrived(const Message& m) {
    println(""); // подскок строки, чтобы не сливалось с prompt
    renderMessage(m);
    prompt();
}

void ConsoleUi::onMessageEdited(qint64 id, const QString& body, const QDateTime&) {
    println(QString("\n  edited #%1: %2").arg(id).arg(body));
    prompt();
}

void ConsoleUi::onMessageDeleted(qint64 id, const QDateTime&) {
    println(QString("\n  deleted #%1").arg(id));
    prompt();
}

void ConsoleUi::onUserOnline(qint64 id) {
    println(QString("\n  *** user %1 online").arg(loginForId(id)));
    prompt();
}

void ConsoleUi::onUserOffline(qint64 id) {
    println(QString("\n  *** user %1 offline").arg(loginForId(id)));
    prompt();
}

void ConsoleUi::onKicked(const QString& reason) {
    println("\n*** Kicked by server: " + reason);
}

void ConsoleUi::onBanned(const QString& reason) {
    println("\n*** Banned by server: " + reason);
}

} // namespace messenger::client