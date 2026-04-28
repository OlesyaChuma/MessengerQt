#include "core/Config.h"
#include "core/Database.h"
#include "core/AuthService.h"
#include "core/ChatServer.h"
#include "core/AdminConsole.h"
#include "PasswordHasher.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <iostream>
#include <string>

using namespace messenger;
using namespace messenger::server;

namespace {

QString readLineFromStdin(const QString& prompt) {
    std::cout << prompt.toLocal8Bit().constData() << std::flush;
    std::string line;
    std::getline(std::cin, line);
    return QString::fromLocal8Bit(line.c_str()).trimmed();
}

// При первом запуске, когда в БД нет ни одного админа,
// предлагаем интерактивно создать его.
qint64 ensureAdmin(AuthService& auth, Database& db) {
    Q_UNUSED(db);
    if (auth.hasAnyAdmin()) {
        // Просим залогиниться существующим админом
        for (int attempt = 1; attempt <= 3; ++attempt) {
            const auto login    = readLineFromStdin("Admin login: ");
            const auto password = readLineFromStdin("Admin password: ");
            auto r = auth.login(login, password);
            if (auto* rec = std::get_if<UserRecord>(&r)) {
                if (rec->role == UserRole::Admin) {
                    std::cout << "Welcome, " << rec->displayName.toLocal8Bit().constData()
                              << "!\n";
                    return rec->id;
                }
                std::cout << "This account is not an admin.\n";
            } else {
                std::cout << "Login failed.\n";
            }
        }
        std::cout << "Too many attempts.\n";
        return 0;
    }

    std::cout << "No admin account found. Let's create the first one.\n";
    const auto login = readLineFromStdin("New admin login: ");
    const auto name  = readLineFromStdin("Display name: ");
    const auto pass  = readLineFromStdin("Password: ");

    auto r = auth.registerAdmin(login, name, pass);
    if (auto* rec = std::get_if<UserRecord>(&r)) {
        std::cout << "Admin created.\n";
        return rec->id;
    }
    std::cout << "Cannot create admin.\n";
    return 0;
}

QString resolveConfigPath() {
    // Ищем server.ini рядом с exe (приоритет) или в config/
    const QStringList candidates = {
        "server.ini",
        "config/server.ini"
    };
    for (const auto& c : candidates) {
        if (QFileInfo::exists(c)) return c;
    }
    return "server.ini"; // нет — используем дефолты
}

} // namespace

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("MessengerQt Server");
    QCoreApplication::setApplicationVersion("1.0.0");

    std::cout << "MessengerQt Server starting...\n";

    const QString configPath = resolveConfigPath();
    const Config cfg = Config::loadFromFile(configPath);
    std::cout << "Config: " << configPath.toLocal8Bit().constData()
              << (QFileInfo::exists(configPath) ? " (loaded)" : " (defaults)") << "\n";

    Database db;
    QString err;
    if (!db.openPostgres(cfg.database, &err)) {
        std::cerr << "Cannot open Postgres: "
                  << err.toLocal8Bit().constData() << "\n";
        std::cerr << "Hint: check Database section in " << configPath.toLocal8Bit().constData()
                  << " and ensure messenger_db is reachable.\n";
        return 1;
    }
    std::cout << "Database connected: " << cfg.database.host.toLocal8Bit().constData()
              << ":" << cfg.database.port
              << "/" << cfg.database.name.toLocal8Bit().constData() << "\n";

    AuthService auth(db);

    const qint64 adminId = ensureAdmin(auth, db);
    if (adminId == 0) {
        std::cerr << "Cannot proceed without admin account.\n";
        return 1;
    }

    ChatServer server(&db, &auth, cfg.listen);

    QObject::connect(&server, &ChatServer::serverMessage,
        [](const QString& m) {
            std::cout << "[server] " << m.toLocal8Bit().constData() << "\n";
        });
    QObject::connect(&server, &ChatServer::clientConnected,
        [](const QString& peer) {
            std::cout << "[server] Client connected from "
                      << peer.toLocal8Bit().constData() << "\n";
        });
    QObject::connect(&server, &ChatServer::clientAuthenticated,
        [](qint64 id, const QString& login) {
            std::cout << "[server] Auth: " << login.toLocal8Bit().constData()
                      << " (id=" << id << ")\n";
        });
    QObject::connect(&server, &ChatServer::clientDisconnected,
        [](qint64 id, const QString& login) {
            if (id > 0) {
                std::cout << "[server] Disconnect: "
                          << login.toLocal8Bit().constData()
                          << " (id=" << id << ")\n";
            }
        });
    QObject::connect(&server, &ChatServer::newMessageDispatched,
        [](const Message& m) {
            std::cout << "[msg #" << m.id << "] "
                      << m.senderLogin.toLocal8Bit().constData()
                      << (m.isBroadcast ? " -> all" : QString(" -> %1").arg(m.receiverId).toLocal8Bit().constData())
                      << ": " << m.body.left(60).toLocal8Bit().constData()
                      << "\n";
        });

    if (!server.start(&err)) {
        std::cerr << "Cannot start TCP listener: "
                  << err.toLocal8Bit().constData() << "\n";
        return 1;
    }

    AdminConsole console(&server, &db, &auth, adminId);
    QObject::connect(&console, &AdminConsole::shutdownRequested,
                     &app, &QCoreApplication::quit);
    console.start();

    const int rc = app.exec();
    server.stop();
    db.close();
    std::cout << "Server stopped.\n";
    return rc;
}
