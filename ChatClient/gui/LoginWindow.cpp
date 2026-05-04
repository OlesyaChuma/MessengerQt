#include "LoginWindow.h"
#include "ThemeManager.h"
#include "TranslationManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSettings>
#include <QInputDialog>

namespace messenger::client::gui {

LoginWindow::LoginWindow(ChatClientCore* core, QWidget* parent)
    : QDialog(parent), _core(core) {
    setWindowTitle(tr("MessengerQt — Sign in"));
    setModal(true);
    setMinimumWidth(440);

    setupUi();
    loadSavedSettings();

    connect(_core, &ChatClientCore::stateChanged,
            this, &LoginWindow::onCoreState);
    connect(_core, &ChatClientCore::serverError,
            this, &LoginWindow::onCoreError);
    connect(_core, &ChatClientCore::requestSucceeded,
            this, &LoginWindow::onRequestSucceeded);
    connect(_core, &ChatClientCore::requestFailed,
            this, &LoginWindow::onRequestFailed);

    connect(&TranslationManager::instance(),
            &TranslationManager::languageChanged,
            this, &LoginWindow::onLanguageChanged);
}

void LoginWindow::setupUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(14);

    // ---------- Top bar (theme + language) ----------
    auto* topBar = new QHBoxLayout;
    _themeBtn = new QPushButton(tr("Theme"));
    _themeBtn->setObjectName("flatButton");
    _themeBtn->setToolTip(tr("Toggle light / dark theme"));
    _langBtn = new QPushButton(tr("Language"));
    _langBtn->setObjectName("flatButton");
    _langBtn->setToolTip(tr("Switch interface language"));
    topBar->addStretch(1);
    topBar->addWidget(_themeBtn);
    topBar->addWidget(_langBtn);
    root->addLayout(topBar);

    // ---------- Title ----------
    _title = new QLabel(tr("Sign in"));
    _title->setObjectName("titleLabel");
    _title->setAlignment(Qt::AlignCenter);
    root->addWidget(_title);

    // ---------- Common server settings ----------
    auto* serverGroup = new QFormLayout;
    serverGroup->setSpacing(8);

    auto* serverRow = new QHBoxLayout;
    _hostEdit = new QLineEdit;
    _hostEdit->setPlaceholderText("localhost");
    _portEdit = new QSpinBox;
    _portEdit->setRange(1, 65535);
    _portEdit->setValue(54000);
    _portEdit->setMinimumWidth(80);
    serverRow->addWidget(_hostEdit, 3);
    serverRow->addWidget(new QLabel(":"));
    serverRow->addWidget(_portEdit, 1);

    _serverLbl = new QLabel(tr("Server:"));
    serverGroup->addRow(_serverLbl, serverRow);
    root->addLayout(serverGroup);

    // ---------- Stack: login / register ----------
    _stack = new QStackedWidget;

    // Login page
    auto* loginPage = new QWidget;
    auto* loginForm = new QFormLayout(loginPage);
    loginForm->setSpacing(10);

    _loginEdit = new QLineEdit;
    _loginEdit->setPlaceholderText(tr("your login"));
    _loginLbl = new QLabel(tr("Login:"));
    loginForm->addRow(_loginLbl, _loginEdit);

    _passEdit = new QLineEdit;
    _passEdit->setEchoMode(QLineEdit::Password);
    _passEdit->setPlaceholderText(tr("password"));
    _passLbl = new QLabel(tr("Password:"));
    loginForm->addRow(_passLbl, _passEdit);

    _rememberCheck = new QCheckBox(tr("Remember me"));
    _rememberCheck->setChecked(true);
    loginForm->addRow(QString(), _rememberCheck);

    auto* loginButtons = new QHBoxLayout;
    _switchToRegBtn = new QPushButton(tr("Create account"));
    _switchToRegBtn->setObjectName("flatButton");
    _loginBtn = new QPushButton(tr("Sign in"));
    _loginBtn->setObjectName("primaryButton");
    _loginBtn->setDefault(true);
    loginButtons->addWidget(_switchToRegBtn);
    loginButtons->addStretch(1);
    loginButtons->addWidget(_loginBtn);
    loginForm->addRow(QString(), loginButtons);

    _stack->addWidget(loginPage);

    // Register page
    auto* regPage = new QWidget;
    auto* regForm = new QFormLayout(regPage);
    regForm->setSpacing(10);

    _regLoginEdit = new QLineEdit;
    _regLoginEdit->setPlaceholderText(tr("choose a unique login"));
    _regLoginLbl = new QLabel(tr("Login:"));
    regForm->addRow(_regLoginLbl, _regLoginEdit);

    _regNameEdit = new QLineEdit;
    _regNameEdit->setPlaceholderText(tr("how others will see you"));
    _regNameLbl = new QLabel(tr("Display name:"));
    regForm->addRow(_regNameLbl, _regNameEdit);

    _regPassEdit = new QLineEdit;
    _regPassEdit->setEchoMode(QLineEdit::Password);
    _regPassLbl = new QLabel(tr("Password:"));
    regForm->addRow(_regPassLbl, _regPassEdit);

    _regPassRepeatEdit = new QLineEdit;
    _regPassRepeatEdit->setEchoMode(QLineEdit::Password);
    _regRepeatLbl = new QLabel(tr("Repeat:"));
    regForm->addRow(_regRepeatLbl, _regPassRepeatEdit);

    auto* regButtons = new QHBoxLayout;
    _switchToLoginBtn = new QPushButton(tr("Already have account"));
    _switchToLoginBtn->setObjectName("flatButton");
    _regBtn = new QPushButton(tr("Register"));
    _regBtn->setObjectName("primaryButton");
    _regBtn->setDefault(true);
    regButtons->addWidget(_switchToLoginBtn);
    regButtons->addStretch(1);
    regButtons->addWidget(_regBtn);
    regForm->addRow(QString(), regButtons);

    _stack->addWidget(regPage);

    root->addWidget(_stack, 1);

    // ---------- Status / error ----------
    _statusLabel = new QLabel;
    _statusLabel->setObjectName("statusLabel");
    _statusLabel->setAlignment(Qt::AlignCenter);
    root->addWidget(_statusLabel);

    _errorLabel = new QLabel;
    _errorLabel->setObjectName("errorLabel");
    _errorLabel->setAlignment(Qt::AlignCenter);
    _errorLabel->setWordWrap(true);
    _errorLabel->hide();
    root->addWidget(_errorLabel);

    // ---------- Connections ----------
    connect(_switchToRegBtn,   &QPushButton::clicked,
            this, &LoginWindow::onSwitchToRegister);
    connect(_switchToLoginBtn, &QPushButton::clicked,
            this, &LoginWindow::onSwitchToLogin);
    connect(_loginBtn, &QPushButton::clicked,
            this, &LoginWindow::onConnectAndLogin);
    connect(_regBtn,   &QPushButton::clicked,
            this, &LoginWindow::onConnectAndRegister);

    connect(_loginEdit, &QLineEdit::returnPressed,
            _loginBtn, &QPushButton::click);
    connect(_passEdit,  &QLineEdit::returnPressed,
            _loginBtn, &QPushButton::click);

    connect(_themeBtn, &QPushButton::clicked,
            this, &LoginWindow::onThemeToggled);
    connect(_langBtn,  &QPushButton::clicked,
            this, &LoginWindow::onSelectLanguage);

    setStyleSheet(R"(
        QLabel#titleLabel { font-size: 22px; font-weight: 600; padding: 8px 0; }
        QLabel#errorLabel { color: #c0392b; font-weight: 500; padding: 4px 0; }
        QLabel#statusLabel { color: #7f8c8d; padding: 2px 0; }
        QPushButton#primaryButton { min-height: 32px; padding: 4px 18px; font-weight: 600; }
        QPushButton#flatButton { background: transparent; border: none; color: #2980b9;
                                 padding: 4px 8px; }
        QPushButton#flatButton:hover { color: #1b5a85; text-decoration: underline; }
    )");
}

void LoginWindow::loadSavedSettings() {
    QSettings s;
    _hostEdit->setText(s.value("client/server/host", "localhost").toString());
    _portEdit->setValue(s.value("client/server/port", 54000).toInt());
    const QString lastLogin = s.value("client/lastLogin").toString();
    if (!lastLogin.isEmpty()) {
        _loginEdit->setText(lastLogin);
        _passEdit->setFocus();
    } else {
        _loginEdit->setFocus();
    }
}

void LoginWindow::saveSettings() {
    QSettings s;
    s.setValue("client/server/host", _hostEdit->text().trimmed());
    s.setValue("client/server/port", _portEdit->value());
    if (_rememberCheck->isChecked()) {
        s.setValue("client/lastLogin", _loginEdit->text().trimmed());
    }
}

void LoginWindow::setError(const QString& text) {
    _errorLabel->setText(text);
    _errorLabel->show();
    _statusLabel->clear();
}

void LoginWindow::clearError() {
    _errorLabel->clear();
    _errorLabel->hide();
}

void LoginWindow::setBusy(bool busy) {
    _busy = busy;
    _loginBtn->setEnabled(!busy);
    _regBtn->setEnabled(!busy);
    _switchToRegBtn->setEnabled(!busy);
    _switchToLoginBtn->setEnabled(!busy);
    _hostEdit->setEnabled(!busy);
    _portEdit->setEnabled(!busy);
}

void LoginWindow::onSwitchToLogin() {
    clearError();
    _stack->setCurrentIndex(0);
    _title->setText(tr("Sign in"));
    _loginEdit->setFocus();
}

void LoginWindow::onSwitchToRegister() {
    clearError();
    _stack->setCurrentIndex(1);
    _title->setText(tr("Create account"));
    _regLoginEdit->setFocus();
}

void LoginWindow::onConnectAndLogin() {
    clearError();
    const QString login = _loginEdit->text().trimmed();
    const QString pass  = _passEdit->text();
    if (login.isEmpty() || pass.isEmpty()) {
        setError(tr("Login and password cannot be empty."));
        return;
    }
    saveSettings();
    setBusy(true);
    _expectLogin = true;
    _chosenLogin = login;

    if (_core->isConnected()) {
        _statusLabel->setText(tr("Authenticating..."));
        _pendingRequest = _core->sendLogin(login, pass);
    } else {
        _statusLabel->setText(tr("Connecting to %1:%2...")
                              .arg(_hostEdit->text().trimmed())
                              .arg(_portEdit->value()));
        _core->connectToServer(_hostEdit->text().trimmed(),
                               static_cast<quint16>(_portEdit->value()));
        // Логин отправим в onCoreState → Connected
    }
}

void LoginWindow::onConnectAndRegister() {
    clearError();
    const QString login = _regLoginEdit->text().trimmed();
    const QString name  = _regNameEdit->text().trimmed();
    const QString pass  = _regPassEdit->text();
    const QString rep   = _regPassRepeatEdit->text();
    if (login.isEmpty() || name.isEmpty() || pass.isEmpty()) {
        setError(tr("All fields are required."));
        return;
    }
    if (pass != rep) {
        setError(tr("Passwords do not match."));
        return;
    }
    if (pass.size() < 4) {
        setError(tr("Password must be at least 4 characters."));
        return;
    }
    saveSettings();
    setBusy(true);
    _expectLogin = false;
    _chosenLogin = login;

    if (_core->isConnected()) {
        _statusLabel->setText(tr("Registering..."));
        _pendingRequest = _core->sendRegister(login, name, pass);
    } else {
        _statusLabel->setText(tr("Connecting to %1:%2...")
                              .arg(_hostEdit->text().trimmed())
                              .arg(_portEdit->value()));
        _core->connectToServer(_hostEdit->text().trimmed(),
                               static_cast<quint16>(_portEdit->value()));
    }
}

void LoginWindow::onCoreState(ChatClientCore::State, ChatClientCore::State newS) {
    if (newS == ChatClientCore::State::Connected && _busy) {
        // Только что подключились — продолжаем заявленный запрос
        if (_expectLogin) {
            _statusLabel->setText(tr("Authenticating..."));
            _pendingRequest = _core->sendLogin(_loginEdit->text().trimmed(),
                                               _passEdit->text());
        } else {
            _statusLabel->setText(tr("Registering..."));
            _pendingRequest = _core->sendRegister(
                _regLoginEdit->text().trimmed(),
                _regNameEdit->text().trimmed(),
                _regPassEdit->text());
        }
    }
    if (newS == ChatClientCore::State::Authenticated && _busy) {
        // Успешный логин — закрываем окно ТОЛЬКО если это активный запрос
        accept();
    }
    if (newS == ChatClientCore::State::Disconnected && _busy) {
        setBusy(false);
        setError(tr("Disconnected from server."));
    }
}

void LoginWindow::onCoreError(const QString& desc) {
    if (_busy) {
        setBusy(false);
        setError(tr("Network error: %1").arg(desc));
    }
}

void LoginWindow::onRequestSucceeded(quint32 rid, const QJsonObject& /*payload*/) {
    if (rid != _pendingRequest) return;
    _pendingRequest = 0;

    if (_expectLogin) {
        // Ждём stateChanged → Authenticated, accept() вызовется там
    } else {
        // Регистрация прошла — автоматически логинимся
        _statusLabel->setText(tr("Registered. Signing in..."));
        _expectLogin = true;
        _pendingRequest = _core->sendLogin(
            _regLoginEdit->text().trimmed(),
            _regPassEdit->text());
        _chosenLogin = _regLoginEdit->text().trimmed();
        QSettings s;
        s.setValue("client/lastLogin", _chosenLogin);
    }
}

void LoginWindow::onRequestFailed(quint32 rid, ResultCode code,
                                  const QString& desc) {
    if (rid != _pendingRequest) return;
    _pendingRequest = 0;
    setBusy(false);

    QString msg;
    switch (code) {
        case ResultCode::InvalidCredentials:
            msg = tr("Wrong login or password.");
            break;
        case ResultCode::UserBanned:
            msg = tr("This account is blocked.");
            break;
        case ResultCode::LoginAlreadyTaken:
            msg = tr("This login is already taken.");
            break;
        default:
            msg = desc.isEmpty()
                ? tr("Authentication failed.")
                : desc;
    }
    setError(msg);
}

void LoginWindow::onLanguageChanged() {
    retranslateUi();
}

void LoginWindow::retranslateUi() {
    setWindowTitle(tr("MessengerQt — Sign in"));
    _themeBtn->setText(tr("Theme"));
    _themeBtn->setToolTip(tr("Toggle light / dark theme"));
    _langBtn->setText(tr("Language"));
    _langBtn->setToolTip(tr("Switch interface language"));

    _title->setText(_stack->currentIndex() == 0
                    ? tr("Sign in") : tr("Create account"));

    _loginBtn->setText(tr("Sign in"));
    _regBtn->setText(tr("Register"));
    _switchToRegBtn->setText(tr("Create account"));
    _switchToLoginBtn->setText(tr("Already have account"));
    _rememberCheck->setText(tr("Remember me"));

    _loginEdit->setPlaceholderText(tr("your login"));
    _passEdit->setPlaceholderText(tr("password"));
    _regLoginEdit->setPlaceholderText(tr("choose a unique login"));
    _regNameEdit->setPlaceholderText(tr("how others will see you"));

    // Лейблы форм — вот это и решает проблему недопереведённых слов
    if (_serverLbl)    _serverLbl->setText(tr("Server:"));
    if (_loginLbl)     _loginLbl->setText(tr("Login:"));
    if (_passLbl)      _passLbl->setText(tr("Password:"));
    if (_regLoginLbl)  _regLoginLbl->setText(tr("Login:"));
    if (_regNameLbl)   _regNameLbl->setText(tr("Display name:"));
    if (_regPassLbl)   _regPassLbl->setText(tr("Password:"));
    if (_regRepeatLbl) _regRepeatLbl->setText(tr("Repeat:"));
}

void LoginWindow::onSelectLanguage() {
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

void LoginWindow::onThemeToggled() {
    ThemeManager::instance().toggle();
}

} // namespace messenger::client::gui
