#include "MainWindow.h"
#include "UsersPanel.h"
#include "ChatPanel.h"
#include "ThemeManager.h"
#include "TranslationManager.h"

#include <QSplitter>
#include <QLabel>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QInputDialog>
#include <QApplication>

namespace messenger::client::gui {

MainWindow::MainWindow(ChatClientCore* core, QWidget* parent)
    : QMainWindow(parent), _core(core) {
    setWindowTitle(tr("MessengerQt — %1").arg(_core->currentDisplayName()));
    setWindowIcon(QIcon(":/client/icons/app.svg"));
    resize(960, 640);

    setupUi();
    setupMenu();
    setupStatusBar();

    connect(_core, &ChatClientCore::stateChanged,
            this, &MainWindow::onCoreState);
    connect(_core, &ChatClientCore::serverError,
            this, &MainWindow::onCoreError);
    connect(_core, &ChatClientCore::kickedByServer,
            this, &MainWindow::onKickedByServer);
    connect(_core, &ChatClientCore::bannedByServer,
            this, &MainWindow::onBannedByServer);

    connect(&TranslationManager::instance(),
            &TranslationManager::languageChanged,
            this, &MainWindow::onLanguageChanged);

    loadWindowState();
}

MainWindow::~MainWindow() {
    saveWindowState();
}

void MainWindow::setupUi() {
    _splitter = new QSplitter(Qt::Horizontal);
    _usersPanel = new UsersPanel(_core, this);
    _chatPanel  = new ChatPanel(_core, this);
    _splitter->addWidget(_usersPanel);
    _splitter->addWidget(_chatPanel);
    _splitter->setStretchFactor(0, 0);
    _splitter->setStretchFactor(1, 1);
    _splitter->setSizes({260, 700});

    setCentralWidget(_splitter);

    // При выборе диалога в боковой панели — открываем его в чате
    connect(_usersPanel, &UsersPanel::chatSelected,
            _chatPanel, &ChatPanel::switchToChat);

    // Когда сообщение пришло в неактивный чат — показываем индикатор «непрочитано»
    connect(_chatPanel, &ChatPanel::unreadInChat,
            this, [this](qint64 peerId) {
                if (_usersPanel->currentPeerId() != peerId) {
                    _usersPanel->markUnread(peerId, true);
                    incrementUnread(peerId);
                }
            });

    // Сбрасываем непрочитанные при выборе чата
    connect(_usersPanel, &UsersPanel::chatSelected,
            this, [this](qint64 peerId, const QString&) {
                clearUnreadFor(peerId);
            });

    // Слушаем все новые сообщения для tray-уведомлений и звуковых сигналов
    connect(_core, &ChatClientCore::newMessageArrived,
            this, [this](const Message& m) {
                // Не уведомляем о собственных сообщениях
                if (m.senderId == _core->currentUserId()) return;

                const qint64 chatId = m.isBroadcast ? 0 : m.senderId;
                const bool windowActive = isActiveWindow();
                const bool sameChat =
                    (windowActive && _usersPanel->currentPeerId() == chatId);

                // Если юзер уже смотрит этот чат — не дёргаем
                if (sameChat) return;

                notifyNewMessage(m.senderLogin, m.body);
            });
}

void MainWindow::setupMenu() {
    auto* menuFile = menuBar()->addMenu(tr("&File"));
    auto* menuView = menuBar()->addMenu(tr("&View"));
    auto* menuHelp = menuBar()->addMenu(tr("&Help"));

    auto* logoutAct = menuFile->addAction(tr("&Sign out"));
    logoutAct->setShortcut(QKeySequence("Ctrl+L"));
    connect(logoutAct, &QAction::triggered, this, [this]() {
        _core->disconnectFromServer();
        close();
    });
    menuFile->addSeparator();
    auto* exitAct = menuFile->addAction(tr("E&xit"));
    exitAct->setShortcut(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    _themeAction = menuView->addAction(tr("Toggle &theme (Light/Dark)"));
    _themeAction->setShortcut(QKeySequence("Ctrl+T"));
    connect(_themeAction, &QAction::triggered,
            this, &MainWindow::onThemeToggled);

    _languageAction = menuView->addAction(tr("&Language..."));
    connect(_languageAction, &QAction::triggered,
            this, &MainWindow::onSelectLanguage);

    auto* aboutAct = menuHelp->addAction(tr("&About..."));
    connect(aboutAct, &QAction::triggered,
            this, &MainWindow::onAboutTriggered);
}

void MainWindow::setupStatusBar() {
    auto* sb = statusBar();
    sb->setSizeGripEnabled(false);

    _connectionStatus = new QLabel(tr("Connected"));
    _connectionStatus->setObjectName("statusOk");
    sb->addWidget(_connectionStatus);

    sb->addWidget(new QLabel("|"));

    _userLabel = new QLabel(tr("Signed in as: %1").arg(_core->currentLogin()));
    sb->addPermanentWidget(_userLabel);
}

void MainWindow::onThemeToggled() {
    ThemeManager::instance().toggle();
}

void MainWindow::onSelectLanguage() {
    auto& tm = TranslationManager::instance();
    QStringList items = {
        TranslationManager::name(TranslationManager::Language::English),
        TranslationManager::name(TranslationManager::Language::Russian)
    };
    int currentIdx = (tm.current() == TranslationManager::Language::Russian)
                     ? 1 : 0;
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
    // Заголовок окна (с учётом счётчика непрочитанных)
    updateWindowTitle();

    // Статус-бар
    if (_core->isConnected()) {
        _connectionStatus->setText(tr("Connected"));
    } else {
        _connectionStatus->setText(tr("Disconnected"));
    }
    _userLabel->setText(tr("Signed in as: %1").arg(_core->currentLogin()));

    // Меню — пересоздаём целиком
    menuBar()->clear();
    setupMenu();
}

void MainWindow::onAboutTriggered() {
    QMessageBox::about(this, tr("About MessengerQt"),
        tr("<h3>MessengerQt — Client</h3>"
           "<p>Version %1</p>"
           "<p>Multi-user network messenger.<br>"
           "Final qualifying project — C++ developer programme.</p>"
           "<p>Built with Qt 6.5, MSVC 2022.</p>")
        .arg(QApplication::applicationVersion()));
}

void MainWindow::onCoreState(ChatClientCore::State,
                             ChatClientCore::State newS) {
    if (newS == ChatClientCore::State::Disconnected) {
        _connectionStatus->setText(tr("Disconnected"));
        _connectionStatus->setStyleSheet("color: #c0392b; font-weight: 500;");
    } else if (newS == ChatClientCore::State::Authenticated ||
               newS == ChatClientCore::State::Connected) {
        _connectionStatus->setText(tr("Connected"));
        _connectionStatus->setStyleSheet("color: #27ae60; font-weight: 500;");
    }
}

void MainWindow::onCoreError(const QString& desc) {
    statusBar()->showMessage(tr("Error: %1").arg(desc), 5000);
}

void MainWindow::onKickedByServer(const QString& reason) {
    QMessageBox::warning(this, tr("Disconnected by server"),
        tr("You were disconnected by an administrator.\n\nReason: %1").arg(reason));
    close();
}

void MainWindow::onBannedByServer(const QString& reason) {
    QMessageBox::critical(this, tr("Account blocked"),
        tr("Your account has been blocked.\n\nReason: %1").arg(reason));
    close();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    saveWindowState();
    _core->disconnectFromServer();
    event->accept();
}

void MainWindow::loadWindowState() {
    QSettings s;
    const auto geom = s.value("client/ui/mainwindow/geometry").toByteArray();
    if (!geom.isEmpty()) restoreGeometry(geom);

    const auto splitter = s.value("client/ui/mainwindow/splitter").toByteArray();
    if (!splitter.isEmpty()) _splitter->restoreState(splitter);
}

void MainWindow::saveWindowState() {
    QSettings s;
    s.setValue("client/ui/mainwindow/geometry", saveGeometry());
    s.setValue("client/ui/mainwindow/splitter", _splitter->saveState());
}

void MainWindow::incrementUnread(qint64 peerId) {
    _unreadByPeer[peerId] = _unreadByPeer.value(peerId, 0) + 1;
    _unreadTotal++;
    updateWindowTitle();
}

void MainWindow::clearUnreadFor(qint64 peerId) {
    const int n = _unreadByPeer.value(peerId, 0);
    if (n > 0) {
        _unreadTotal -= n;
        if (_unreadTotal < 0) _unreadTotal = 0;
        _unreadByPeer.remove(peerId);
        updateWindowTitle();
    }
}

void MainWindow::updateWindowTitle() {
    const QString name = _core->currentDisplayName();
    if (_unreadTotal > 0) {
        setWindowTitle(tr("MessengerQt — %1 (%2)")
                           .arg(name).arg(_unreadTotal));
    } else {
        setWindowTitle(tr("MessengerQt — %1").arg(name));
    }
}

void MainWindow::notifyNewMessage(const QString& /*senderName*/,
                                  const QString& /*body*/) {
    // 1) Мигание иконки в панели задач — стандарт Windows
    QApplication::alert(this);

    // 2) Звуковой сигнал — системный «дзинь»
    QApplication::beep();
}

} // namespace messenger::client::gui
