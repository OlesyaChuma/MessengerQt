#include "core/Config.h"
#include "core/Database.h"
#include "core/AuthService.h"
#include "core/ChatServer.h"
#include "gui/LoginDialog.h"
#include "gui/MainWindow.h"
#include "gui/ThemeManager.h"
#include "gui/TranslationManager.h"

#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QIcon>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shobjidl.h>
#endif

using namespace messenger;
using namespace messenger::server;
using namespace messenger::server::gui;

namespace {

QString resolveConfigPath() {
    const QStringList candidates = {
        "server.ini",
        "config/server.ini"
    };
    for (const auto& c : candidates) {
        if (QFileInfo::exists(c)) return c;
    }
    return "server.ini";
}

} // namespace

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("MessengerQt Server");
    QApplication::setOrganizationName("MessengerQt");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setWindowIcon(QIcon(":/server/icons/server.svg"));

#ifdef Q_OS_WIN
    // Регистрируем уникальный AppUserModelID — Windows сможет
    // показывать уведомления и группировать иконку в taskbar.
    SetCurrentProcessExplicitAppUserModelID(L"MessengerQt.Server.1");
#endif

    // Применяем сохранённую тему сразу при старте
    ThemeManager::instance().apply(ThemeManager::instance().current());
    TranslationManager::instance().apply(TranslationManager::instance().current());

    // ----- Конфиг -----
    const QString configPath = resolveConfigPath();
    const Config cfg = Config::loadFromFile(configPath);

    // ----- БД -----
    Database db;
    QString err;
    if (!db.openPostgres(cfg.database, &err)) {
        QMessageBox::critical(nullptr,
            QObject::tr("Database connection failed"),
            QObject::tr("Cannot open Postgres database:\n%1\n\n"
                        "Check Database section in %2 and ensure messenger_db is reachable.")
                .arg(err, configPath));
        return 1;
    }

    AuthService auth(db);

    // ----- Логин админа / создание первого -----
    LoginDialog dlg(&auth);
    if (dlg.exec() != QDialog::Accepted) {
        return 0; // пользователь закрыл окно
    }
    const UserRecord admin = dlg.adminRecord();

    // ----- TCP-сервер -----
    ChatServer server(&db, &auth, cfg.listen);
    if (!server.start(&err)) {
        QMessageBox::critical(nullptr,
            QObject::tr("Server start failed"),
            QObject::tr("Cannot start TCP listener on %1:%2:\n%3")
                .arg(cfg.listen.host).arg(cfg.listen.port).arg(err));
        return 1;
    }

    // ----- Главное окно -----
    MainWindow window(&db, &auth, &server, admin, cfg, configPath);
    window.show();

    const int rc = app.exec();
    server.stop();
    db.close();
    return rc;
}
