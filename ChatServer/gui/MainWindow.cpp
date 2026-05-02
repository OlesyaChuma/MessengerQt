#include "MainWindow.h"
#include "UsersTab.h"
#include "MessagesTab.h"
#include "ConnectionLogTab.h"
#include "TrayIcon.h"
#include "SettingsDialog.h"
#include "ThemeManager.h"

#include "Database.h"
#include "AuthService.h"
#include "ChatServer.h"
#include "TranslationManager.h"

#include <QTabWidget>
#include <QLabel>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QApplication>
#include <QStyle>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QInputDialog>
#include <QPushButton>
#include <QAbstractButton>

namespace messenger::server::gui {

MainWindow::MainWindow(Database* db, AuthService* auth, ChatServer* server,
                       UserRecord adminRecord,
                       const Config& cfg, const QString& configPath,
                       QWidget* parent)
    : QMainWindow(parent),
      _db(db), _auth(auth), _server(server), _admin(adminRecord),
      _cfg(cfg), _configPath(configPath) {
    setWindowTitle(tr("MessengerQt — Server Administration"));
    setWindowIcon(QIcon(":/server/icons/server.svg"));
    resize(1100, 720);

    setupUi();
    setupMenu();
    setupStatusBar();
    wireServerSignals();
    setupTray();

    loadWindowState();

    // Реакция на смену языка во время работы
    connect(&TranslationManager::instance(),
            &TranslationManager::languageChanged,
            this, &MainWindow::onLanguageChanged);
}

MainWindow::~MainWindow() {
    saveWindowState();
}

void MainWindow::setupUi() {
    _tabs = new QTabWidget;
    _tabs->setDocumentMode(true);
    _tabs->setMovable(false);

    _usersTab    = new UsersTab(_db, _server, _admin.id, this);
    _messagesTab = new MessagesTab(_db, _server, this);
    _logTab      = new ConnectionLogTab(_db, _server, this);

    const auto userIcon = QIcon(":/server/icons/user.svg");
    const auto msgIcon  = QIcon(":/server/icons/message.svg");
    const auto logIcon  = QIcon(":/server/icons/log.svg");

    _tabs->addTab(_usersTab,    userIcon, tr("Users"));
    _tabs->addTab(_messagesTab, msgIcon,  tr("Messages"));
    _tabs->addTab(_logTab,      logIcon,  tr("Connection log"));

    setCentralWidget(_tabs);
}

void MainWindow::setupMenu() {
    auto* menuFile = menuBar()->addMenu(tr("&File"));
    auto* menuView = menuBar()->addMenu(tr("&View"));
    auto* menuHelp = menuBar()->addMenu(tr("&Help"));

    _settingsAction = menuFile->addAction(tr("&Settings..."));
    _settingsAction->setShortcut(QKeySequence("Ctrl+,"));
    connect(_settingsAction, &QAction::triggered,
            this, &MainWindow::onSettingsTriggered);

    menuFile->addSeparator();

    auto* hideAct = menuFile->addAction(tr("Minimize to &tray"));
    hideAct->setShortcut(QKeySequence("Ctrl+H"));
    connect(hideAct, &QAction::triggered, this, &MainWindow::hide);

    auto* exitAct = menuFile->addAction(tr("E&xit"));
    exitAct->setShortcut(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, [this]() {
        _reallyQuit = true;
        close();
    });

    _themeAction = menuView->addAction(tr("Toggle &theme (Light/Dark)"));
    _themeAction->setShortcut(QKeySequence("Ctrl+T"));
    connect(_themeAction, &QAction::triggered, this, &MainWindow::onThemeToggled);

    _languageAction = menuView->addAction(tr("&Language..."));
    connect(_languageAction, &QAction::triggered, this, &MainWindow::onSelectLanguage);

    menuView->addSeparator();

    auto* refreshAct = menuView->addAction(tr("&Refresh current tab"));
    refreshAct->setShortcut(QKeySequence::Refresh);
    connect(refreshAct, &QAction::triggered, this, [this]() {
        const int idx = _tabs->currentIndex();
        if (idx == 0) _usersTab->reload();
        else if (idx == 1) _messagesTab->reload();
        else if (idx == 2) _logTab->reload();
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

void MainWindow::setupTray() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return; // на системах без трея просто пропускаем
    }
    _tray = new TrayIcon(this, this);
    connect(_tray, &TrayIcon::aboutRequested,
            this, &MainWindow::onTrayAboutRequested);
    connect(_tray, &TrayIcon::quitRequested,
            this, &MainWindow::onTrayQuitRequested);
    _tray->setOnlineCount(0);
    _tray->show();
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

void MainWindow::onSettingsTriggered() {
    SettingsDialog dlg(_cfg, _configPath, this);
    dlg.exec();
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
    if (_tray) {
        _tray->notify(tr("User connected"),
                      tr("%1 logged in").arg(login),
                      QSystemTrayIcon::Information, 3000);
    }
}

void MainWindow::onClientDisconnected(qint64 userId, const QString& login) {
    if (userId > 0) {
        statusBar()->showMessage(tr("User logged out: %1").arg(login), 4000);
    }
    updateOnlineCounter();
}

void MainWindow::onNewMessageDispatched(const Message& /*msg*/) {
    // Лента сама обновляется через MessagesTab
}

void MainWindow::updateOnlineCounter() {
    const int count = _server->sessionsCount();
    _onlineCounter->setText(tr("Online: %1").arg(count));
    if (_tray) _tray->setOnlineCount(count);
}

void MainWindow::onTrayAboutRequested() {
    onAboutTriggered();
}

void MainWindow::onTrayQuitRequested() {
    _reallyQuit = true;
    QApplication::quit();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (_reallyQuit) {
        // Программный выход (через меню/трей) — закрываемся без вопросов
        saveWindowState();
        event->accept();
        return;
    }

    // Пользователь нажал крестик — спрашиваем, что делать
    QMessageBox box(this);
    box.setWindowTitle(tr("Close server"));
    box.setIcon(QMessageBox::Question);
    box.setText(tr("<b>Close MessengerQt Server?</b>"));
    box.setInformativeText(tr(
        "If you choose <b>Quit</b>, the server will stop and all connected "
        "clients will be disconnected.<br><br>"
        "If you choose <b>Hide to tray</b>, the server keeps running in the "
        "background and you can restore the window from the system tray icon."));

    QPushButton* hideBtn   = nullptr;
    QPushButton* quitBtn   = box.addButton(tr("Quit server"), QMessageBox::DestructiveRole);
    if (_tray && QSystemTrayIcon::isSystemTrayAvailable()) {
        hideBtn = box.addButton(tr("Hide to tray"), QMessageBox::AcceptRole);
        box.setDefaultButton(hideBtn);
    } else {
        box.setDefaultButton(quitBtn);
    }
    box.addButton(tr("Cancel"), QMessageBox::RejectRole);

    box.exec();
    QAbstractButton* clicked = box.clickedButton();

    if (clicked == static_cast<QAbstractButton*>(quitBtn)) {
        _reallyQuit = true;
        saveWindowState();
        event->accept();
        QApplication::quit();
    } else if (hideBtn && clicked == static_cast<QAbstractButton*>(hideBtn)) {
        hide();
        event->ignore();
    } else {
        // Cancel или закрытие диалога — отменяем закрытие
        event->ignore();
    }
}

void MainWindow::loadWindowState() {
    QSettings s;
    const auto geom = s.value("ui/mainwindow/geometry").toByteArray();
    const auto state = s.value("ui/mainwindow/state").toByteArray();
    if (!geom.isEmpty()) restoreGeometry(geom);
    if (!state.isEmpty()) restoreState(state);
}

void MainWindow::saveWindowState() {
    QSettings s;
    s.setValue("ui/mainwindow/geometry", saveGeometry());
    s.setValue("ui/mainwindow/state", saveState());
}

void MainWindow::onSelectLanguage() {
    auto& tm = TranslationManager::instance();
    QStringList items = {
        TranslationManager::name(TranslationManager::Language::English),
        TranslationManager::name(TranslationManager::Language::Russian)
    };
    int currentIdx = (tm.current() == TranslationManager::Language::Russian) ? 1 : 0;

    bool ok = false;
    QString choice = QInputDialog::getItem(this,
                                           tr("Select language"),
                                           tr("Interface language:"),
                                           items, currentIdx, false, &ok);
    if (!ok) return;

    auto target = (choice == items[1])
                      ? TranslationManager::Language::Russian
                      : TranslationManager::Language::English;
    tm.apply(target);
}

void MainWindow::onLanguageChanged() {
    retranslateUi();
}

void MainWindow::retranslateUi() {
    setWindowTitle(tr("MessengerQt — Server Administration"));
    _adminLabel->setText(tr("Signed in as: %1").arg(_admin.login));
    updateOnlineCounter();

    // Перестроим меню целиком — простой и надёжный способ
    menuBar()->clear();
    setupMenu();

    // Названия вкладок
    _tabs->setTabText(0, tr("Users"));
    _tabs->setTabText(1, tr("Messages"));
    _tabs->setTabText(2, tr("Connection log"));
}

} // namespace messenger::server::gui
