#include "MainWindow.h"
#include "UsersTab.h"
#include "MessagesTab.h"
#include "ConnectionLogTab.h"

#include "ThemeManager.h"
#include "Database.h"
#include "AuthService.h"
#include "ChatServer.h"

#include <QTabWidget>
#include <QLabel>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QApplication>
#include <QStyle>
#include <QMessageBox>

namespace messenger::server::gui {

MainWindow::MainWindow(Database* db, AuthService* auth, ChatServer* server,
                       UserRecord adminRecord, QWidget* parent)
    : QMainWindow(parent),
      _db(db), _auth(auth), _server(server), _admin(adminRecord) {
    setWindowTitle(tr("MessengerQt — Server Administration"));
    resize(1100, 720);

    setupUi();
    setupMenu();
    setupStatusBar();
    wireServerSignals();
}

void MainWindow::setupUi() {
    _tabs = new QTabWidget;
    _tabs->setDocumentMode(true);
    _tabs->setMovable(false);

    _usersTab    = new UsersTab(_db, _server, _admin.id, this);
    _messagesTab = new MessagesTab(_db, _server, this);
    _logTab      = new ConnectionLogTab(_db, this);

    const auto userIcon = QIcon(":/server/icons/user.svg");
    const auto msgIcon  = QIcon(":/server/icons/message.svg");
    const auto logIcon  = QIcon(":/server/icons/log.svg");

    _tabs->addTab(_usersTab,    userIcon, tr("Users"));
    _tabs->addTab(_messagesTab, msgIcon,  tr("Messages"));
    _tabs->addTab(_logTab,      logIcon,  tr("Connection log"));

    setCentralWidget(_tabs);
}

void MainWindow::setupMenu() {
    auto* menuFile   = menuBar()->addMenu(tr("&File"));
    auto* menuView   = menuBar()->addMenu(tr("&View"));
    auto* menuHelp   = menuBar()->addMenu(tr("&Help"));

    auto* exitAct = menuFile->addAction(tr("E&xit"));
    exitAct->setShortcut(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, qApp, &QApplication::quit);

    _themeAction = menuView->addAction(tr("Toggle &theme (Light/Dark)"));
    _themeAction->setShortcut(QKeySequence("Ctrl+T"));
    connect(_themeAction, &QAction::triggered, this, &MainWindow::onThemeToggled);

    auto* refreshAct = menuView->addAction(tr("&Refresh"));
    refreshAct->setShortcut(QKeySequence::Refresh);
    connect(refreshAct, &QAction::triggered, this, [this]() {
        // Обновление вкладок реализуем в Пакете 4Б/4В
    });

    auto* aboutAct = menuHelp->addAction(tr("&About..."));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::onAboutTriggered);
}

void MainWindow::setupStatusBar() {
    auto* sb = statusBar();
    sb->setSizeGripEnabled(false);

    _dbStatus = new QLabel(tr("DB: connected"));
    _dbStatus->setObjectName("statusOk");
    sb->addWidget(_dbStatus);

    sb->addWidget(new QLabel("|"));

    _serverStatus = new QLabel(tr("Server: starting..."));
    sb->addWidget(_serverStatus);

    sb->addWidget(new QLabel("|"));

    _onlineCounter = new QLabel(tr("Online: 0"));
    _onlineCounter->setObjectName("onlineCounter");
    sb->addWidget(_onlineCounter);

    _adminLabel = new QLabel(tr("Signed in as: %1").arg(_admin.login));
    sb->addPermanentWidget(_adminLabel);
}

void MainWindow::wireServerSignals() {
    connect(_server, &ChatServer::serverMessage,
            this, &MainWindow::onServerMessage);
    connect(_server, &ChatServer::clientConnected,
            this, &MainWindow::onClientConnected);
    connect(_server, &ChatServer::clientAuthenticated,
            this, &MainWindow::onClientAuthenticated);
    connect(_server, &ChatServer::clientDisconnected,
            this, &MainWindow::onClientDisconnected);
    connect(_server, &ChatServer::newMessageDispatched,
            this, &MainWindow::onNewMessageDispatched);
}

void MainWindow::onThemeToggled() {
    ThemeManager::instance().toggle();
}

void MainWindow::onAboutTriggered() {
    QMessageBox::about(this, tr("About MessengerQt Server"),
        tr("<h3>MessengerQt — Server Administration</h3>"
           "<p>Version %1</p>"
           "<p>Multi-user network messenger.<br>"
           "Final qualifying project — C++ developer programme.</p>"
           "<p>Built with Qt 6.5, PostgreSQL, MSVC 2022.</p>")
        .arg(QApplication::applicationVersion()));
}

void MainWindow::onServerMessage(const QString& text) {
    _serverStatus->setText(text);
}

void MainWindow::onClientConnected(const QString& peer) {
    statusBar()->showMessage(tr("New connection from %1").arg(peer), 4000);
}

void MainWindow::onClientAuthenticated(qint64 userId, const QString& login) {
    statusBar()->showMessage(tr("User logged in: %1 (id=%2)").arg(login).arg(userId), 4000);
    updateOnlineCounter();
}

void MainWindow::onClientDisconnected(qint64 userId, const QString& login) {
    if (userId > 0) {
        statusBar()->showMessage(
            tr("User logged out: %1").arg(login), 4000);
    }
    updateOnlineCounter();
}

void MainWindow::onNewMessageDispatched(const Message& /*msg*/) {
    // Обработка живой ленты — в MessagesTab (Пакет 4Б)
}

void MainWindow::updateOnlineCounter() {
    _onlineCounter->setText(tr("Online: %1").arg(_server->sessionsCount()));
}

} // namespace messenger::server::gui
