#include "AdminConsole.h"
#include "ChatServer.h"
#include "Database.h"
#include "AuthService.h"

#include <QStringList>
#include <QMetaObject>
#include <QPointer>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>

namespace messenger::server {

AdminConsole::AdminConsole(ChatServer* server, Database* db,
                           AuthService* auth, qint64 adminId,
                           QObject* parent)
    : QObject(parent),
    _server(server), _db(db), _auth(auth), _adminId(adminId) {}

AdminConsole::~AdminConsole() {
    _shuttingDown.store(true);
    // Поток stdin-reader detached — он сам помрёт при exit() процесса.
}

void AdminConsole::start() {
    printLine("");
    printLine("=== MessengerQt admin console ===");
    printHelp();
    prompt();

    // Фоновый поток чтения stdin. Detached — живёт до конца процесса.
    QPointer<AdminConsole> guard(this);
    std::thread([guard]() {
        std::string line;
        while (std::getline(std::cin, line)) {
            // Если объект уничтожен/выключение — завершаемся
            if (!guard) return;
            if (guard->_shuttingDown.load()) return;

            const QString qline = QString::fromLocal8Bit(line.c_str());
            // Доставка в Qt-поток
            QMetaObject::invokeMethod(guard.data(), "onLineRead",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, qline));
        }
        // EOF
        if (guard && !guard->_shuttingDown.load()) {
            QMetaObject::invokeMethod(guard.data(), "onEofReached",
                                      Qt::QueuedConnection);
        }
    }).detach();
}

void AdminConsole::onLineRead(const QString& raw) {
    if (_shuttingDown.load()) return;

    const QString line = raw.trimmed();
    if (line.isEmpty()) {
        prompt();
        return;
    }

    const QStringList parts = line.split(' ', Qt::SkipEmptyParts);
    const QString cmd = parts.first();
    const QStringList args = parts.mid(1);

    if (cmd == "help" || cmd == "?") {
        printHelp();
    } else if (cmd == "users") {
        printUsers();
    } else if (cmd == "online") {
        printOnline();
    } else if (cmd == "messages") {
        const int limit = args.isEmpty() ? 20 : args.first().toInt();
        printRecentMessages(limit > 0 ? limit : 20);
    } else if (cmd == "log") {
        const int limit = args.isEmpty() ? 20 : args.first().toInt();
        printConnectionLog(limit > 0 ? limit : 20);
    } else if (cmd == "kick") {
        cmdKick(args);
    } else if (cmd == "ban") {
        cmdBan(args);
    } else if (cmd == "unban") {
        cmdUnban(args);
    } else if (cmd == "stop" || cmd == "quit" || cmd == "exit") {
        _shuttingDown.store(true);
        emit shutdownRequested();
        return;
    } else {
        printLine(QString("Unknown command: %1. Type 'help'.").arg(cmd));
    }

    prompt();
}

void AdminConsole::onEofReached() {
    if (_shuttingDown.load()) return;
    _shuttingDown.store(true);
    emit shutdownRequested();
}

void AdminConsole::printHelp() const {
    printLine("Available commands:");
    printLine("  help                   - this help");
    printLine("  users                  - list all registered users");
    printLine("  online                 - list users currently online");
    printLine("  messages [N]           - last N messages (default 20)");
    printLine("  log [N]                - last N connection log entries");
    printLine("  kick  <user_id> [...]  - disconnect user (with optional reason)");
    printLine("  ban   <user_id> [...]  - block user permanently with reason");
    printLine("  unban <user_id>        - unblock user");
    printLine("  stop                   - shutdown server");
}

void AdminConsole::printUsers() const {
    const auto users = _db->listAllUsers();
    if (users.isEmpty()) { printLine("No users registered."); return; }

    printLine(QString("%1 %2 %3 %4 %5 %6")
                  .arg("ID", -5).arg("Login", -20).arg("Name", -25)
                  .arg("Role", -7).arg("Status", -9).arg("Banned"));
    for (const auto& r : users) {
        const bool online = _server->isUserOnline(r.id);
        printLine(QString("%1 %2 %3 %4 %5 %6")
                      .arg(QString::number(r.id), -5)
                      .arg(r.login.left(20), -20)
                      .arg(r.displayName.left(25), -25)
                      .arg(r.role == UserRole::Admin ? "admin" : "user", -7)
                      .arg(online ? "online" : "offline", -9)
                      .arg(r.isBanned ? ("yes (" + r.banReason + ")") : QString("no")));
    }
}

void AdminConsole::printOnline() const {
    const auto ids = _server->onlineUserIds();
    if (ids.isEmpty()) { printLine("Nobody online."); return; }

    for (qint64 id : ids) {
        if (auto rec = _db->findUserById(id)) {
            printLine(QString("  [%1] %2 (%3)")
                          .arg(rec->id).arg(rec->login).arg(rec->displayName));
        }
    }
}

void AdminConsole::printRecentMessages(int limit) const {
    const auto msgs = _db->loadAllMessages(0, false, false, 0, limit);
    if (msgs.isEmpty()) { printLine("No messages yet."); return; }

    for (const auto& m : msgs) {
        const QString kind = m.isBroadcast
                                 ? QStringLiteral("broadcast")
                                 : QString("-> %1").arg(m.receiverId);
        const QString deleted = m.deletedAt.isValid() ? " [deleted]" : "";
        printLine(QString("  #%1 [%2] %3 %4: %5%6")
                      .arg(m.id)
                      .arg(m.createdAt.toLocalTime().toString("HH:mm:ss"))
                      .arg(m.senderLogin, -16)
                      .arg(kind, -12)
                      .arg(m.body.left(80))
                      .arg(deleted));
    }
}

void AdminConsole::printConnectionLog(int limit) const {
    const auto rows = _db->loadConnectionLog(limit);
    for (const auto& r : rows) {
        QString event;
        switch (r.event) {
        case ConnectionEvent::Login:       event = "LOGIN";       break;
        case ConnectionEvent::Logout:      event = "LOGOUT";      break;
        case ConnectionEvent::Kick:        event = "KICK";        break;
        case ConnectionEvent::Ban:         event = "BAN";         break;
        case ConnectionEvent::Unban:       event = "UNBAN";       break;
        case ConnectionEvent::FailedLogin: event = "FAILED_LOGIN";break;
        case ConnectionEvent::Register:    event = "REGISTER";    break;
        }
        printLine(QString("  [%1] %2 %3 (%4) %5")
                      .arg(r.occurredAt.toLocalTime().toString("yyyy-MM-dd HH:mm:ss"))
                      .arg(event, -13)
                      .arg(r.loginCache, -16)
                      .arg(r.ipAddress, -15)
                      .arg(r.details));
    }
}

void AdminConsole::cmdKick(const QStringList& args) {
    if (args.isEmpty()) { printLine("Usage: kick <user_id> [reason]"); return; }
    const qint64 uid = args.first().toLongLong();
    const QString reason = args.mid(1).join(' ').isEmpty()
                               ? QStringLiteral("Kicked by admin")
                               : args.mid(1).join(' ');
    if (_server->kickUser(uid, reason)) {
        printLine(QString("User %1 kicked.").arg(uid));
    } else {
        printLine(QString("User %1 is not online.").arg(uid));
    }
}

void AdminConsole::cmdBan(const QStringList& args) {
    if (args.isEmpty()) { printLine("Usage: ban <user_id> [reason]"); return; }
    const qint64 uid = args.first().toLongLong();
    const QString reason = args.mid(1).join(' ').isEmpty()
                               ? QStringLiteral("Banned by admin")
                               : args.mid(1).join(' ');
    if (_server->banUser(uid, _adminId, reason)) {
        printLine(QString("User %1 banned: %2").arg(uid).arg(reason));
    } else {
        printLine(QString("Cannot ban user %1.").arg(uid));
    }
}

void AdminConsole::cmdUnban(const QStringList& args) {
    if (args.isEmpty()) { printLine("Usage: unban <user_id>"); return; }
    const qint64 uid = args.first().toLongLong();
    if (_server->unbanUser(uid)) {
        printLine(QString("User %1 unbanned.").arg(uid));
    } else {
        printLine(QString("Cannot unban user %1.").arg(uid));
    }
}

void AdminConsole::prompt() const {
    std::cout << "admin> " << std::flush;
}

void AdminConsole::printLine(const QString& line) const {
    std::cout << line.toLocal8Bit().constData() << std::endl;
}

} // namespace messenger::server
