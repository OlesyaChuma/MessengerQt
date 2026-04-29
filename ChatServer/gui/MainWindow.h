#pragma once

#include "Models.h"
#include "Database.h"

#include <QMainWindow>

class QTabWidget;
class QLabel;
class QAction;

namespace messenger::server {
class Database;
class AuthService;
class ChatServer;
}

namespace messenger::server::gui {

class UsersTab;
class MessagesTab;
class ConnectionLogTab;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(Database* db, AuthService* auth, ChatServer* server,
               UserRecord adminRecord, QWidget* parent = nullptr);

private slots:
    void onThemeToggled();
    void onAboutTriggered();
    void onServerMessage(const QString& text);
    void onClientConnected(const QString& peer);
    void onClientAuthenticated(qint64 userId, const QString& login);
    void onClientDisconnected(qint64 userId, const QString& login);
    void onNewMessageDispatched(const Message& msg);

private:
    void setupUi();
    void setupMenu();
    void setupStatusBar();
    void wireServerSignals();
    void updateOnlineCounter();

    Database*    _db = nullptr;
    AuthService* _auth = nullptr;
    ChatServer*  _server = nullptr;
    UserRecord   _admin;

    QTabWidget* _tabs = nullptr;
    UsersTab*         _usersTab = nullptr;
    MessagesTab*      _messagesTab = nullptr;
    ConnectionLogTab* _logTab = nullptr;

    // Status bar widgets
    QLabel* _dbStatus = nullptr;
    QLabel* _serverStatus = nullptr;
    QLabel* _onlineCounter = nullptr;
    QLabel* _adminLabel = nullptr;

    QAction* _themeAction = nullptr;
};

} // namespace messenger::server::gui
