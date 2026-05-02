#pragma once

#include "Models.h"
#include "Database.h"
#include "Config.h"

#include <QMainWindow>

class QTabWidget;
class QLabel;
class QAction;
class QCloseEvent;

namespace messenger::server {
class AuthService;
class ChatServer;
}

namespace messenger::server::gui {

class UsersTab;
class MessagesTab;
class ConnectionLogTab;
class TrayIcon;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(Database* db, AuthService* auth, ChatServer* server,
               UserRecord adminRecord,
               const Config& cfg, const QString& configPath,
               QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onThemeToggled();
    void onAboutTriggered();
    void onSettingsTriggered();
    void onServerMessage(const QString& text);
    void onClientConnected(const QString& peer);
    void onClientAuthenticated(qint64 userId, const QString& login);
    void onClientDisconnected(qint64 userId, const QString& login);
    void onNewMessageDispatched(const Message& msg);

    void onTrayAboutRequested();
    void onTrayQuitRequested();

    void onLanguageChanged();
    void onSelectLanguage();

private:
    void setupUi();
    void setupMenu();
    void setupStatusBar();
    void setupTray();
    void wireServerSignals();
    void updateOnlineCounter();

    void loadWindowState();
    void saveWindowState();

    QAction* _languageAction = nullptr;
    void retranslateUi();

    Database*    _db = nullptr;
    AuthService* _auth = nullptr;
    ChatServer*  _server = nullptr;
    UserRecord   _admin;
    Config       _cfg;
    QString      _configPath;

    QTabWidget* _tabs = nullptr;
    UsersTab*         _usersTab = nullptr;
    MessagesTab*      _messagesTab = nullptr;
    ConnectionLogTab* _logTab = nullptr;

    QLabel* _dbStatus = nullptr;
    QLabel* _serverStatus = nullptr;
    QLabel* _onlineCounter = nullptr;
    QLabel* _adminLabel = nullptr;

    QAction* _themeAction = nullptr;
    QAction* _settingsAction = nullptr;

    TrayIcon* _tray = nullptr;
    bool      _reallyQuit = false;
};

} // namespace messenger::server::gui
