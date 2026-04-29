#include "LoginDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QShowEvent>
#include <QMessageBox>

namespace messenger::server::gui {

LoginDialog::LoginDialog(AuthService* auth, QWidget* parent)
    : QDialog(parent), _auth(auth) {
    setWindowTitle(tr("MessengerQt Server — Login"));
    setModal(true);
    setFixedSize(420, 360);
    setupUi();
    setupStyles();
}

void LoginDialog::showEvent(QShowEvent* e) {
    QDialog::showEvent(e);
    // Решение режима делаем при показе, чтобы успели прогреться зависимости
    const bool hasAdmin = _auth->hasAnyAdmin();
    _stack->setCurrentIndex(hasAdmin ? 0 : 1);
    _titleLabel->setText(hasAdmin
        ? tr("Sign in as administrator")
        : tr("Create the first administrator"));
    if (hasAdmin) _loginEdit->setFocus();
    else          _createLoginEdit->setFocus();
}

void LoginDialog::setupUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(14);

    _titleLabel = new QLabel(tr("Sign in"));
    _titleLabel->setObjectName("titleLabel");
    _titleLabel->setAlignment(Qt::AlignCenter);
    root->addWidget(_titleLabel);

    _stack = new QStackedWidget;
    root->addWidget(_stack, 1);

    // ----- Login page -----
    auto* loginPage = new QWidget;
    auto* loginForm = new QFormLayout(loginPage);
    loginForm->setSpacing(10);
    loginForm->setLabelAlignment(Qt::AlignRight);

    _loginEdit = new QLineEdit;
    _loginEdit->setPlaceholderText(tr("admin"));
    loginForm->addRow(tr("Login:"), _loginEdit);

    _passEdit = new QLineEdit;
    _passEdit->setEchoMode(QLineEdit::Password);
    _passEdit->setPlaceholderText(tr("password"));
    loginForm->addRow(tr("Password:"), _passEdit);

    _loginBtn = new QPushButton(tr("Sign in"));
    _loginBtn->setDefault(true);
    _loginBtn->setObjectName("primaryButton");
    loginForm->addRow(QString(), _loginBtn);

    _stack->addWidget(loginPage);

    // ----- Create-admin page -----
    auto* createPage = new QWidget;
    auto* createForm = new QFormLayout(createPage);
    createForm->setSpacing(10);
    createForm->setLabelAlignment(Qt::AlignRight);

    _createLoginEdit = new QLineEdit;
    _createLoginEdit->setPlaceholderText(tr("admin"));
    createForm->addRow(tr("Login:"), _createLoginEdit);

    _createNameEdit = new QLineEdit;
    _createNameEdit->setPlaceholderText(tr("Administrator"));
    createForm->addRow(tr("Display name:"), _createNameEdit);

    _createPassEdit = new QLineEdit;
    _createPassEdit->setEchoMode(QLineEdit::Password);
    createForm->addRow(tr("Password:"), _createPassEdit);

    _createPassRepeatEdit = new QLineEdit;
    _createPassRepeatEdit->setEchoMode(QLineEdit::Password);
    createForm->addRow(tr("Repeat:"), _createPassRepeatEdit);

    _createBtn = new QPushButton(tr("Create administrator"));
    _createBtn->setDefault(true);
    _createBtn->setObjectName("primaryButton");
    createForm->addRow(QString(), _createBtn);

    _stack->addWidget(createPage);

    // ----- Error label -----
    _errorLabel = new QLabel;
    _errorLabel->setObjectName("errorLabel");
    _errorLabel->setWordWrap(true);
    _errorLabel->setAlignment(Qt::AlignCenter);
    _errorLabel->hide();
    root->addWidget(_errorLabel);

    connect(_loginBtn,  &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(_createBtn, &QPushButton::clicked, this, &LoginDialog::onCreateClicked);

    connect(_loginEdit, &QLineEdit::returnPressed, _loginBtn, &QPushButton::click);
    connect(_passEdit,  &QLineEdit::returnPressed, _loginBtn, &QPushButton::click);
}

void LoginDialog::setupStyles() {
    // Базовая стилизация — основной QSS темы примерится в main()
    setStyleSheet(R"(
        QLabel#titleLabel {
            font-size: 20px;
            font-weight: 600;
            padding: 8px 0 12px 0;
        }
        QLabel#errorLabel {
            color: #c0392b;
            font-weight: 500;
            padding: 6px 0;
        }
        QPushButton#primaryButton {
            min-height: 32px;
            padding: 4px 16px;
            font-weight: 600;
        }
    )");
}

void LoginDialog::showError(const QString& text) {
    _errorLabel->setText(text);
    _errorLabel->show();
}

void LoginDialog::clearError() {
    _errorLabel->clear();
    _errorLabel->hide();
}

void LoginDialog::onLoginClicked() {
    clearError();
    const QString login = _loginEdit->text().trimmed();
    const QString pass  = _passEdit->text();
    if (login.isEmpty() || pass.isEmpty()) {
        showError(tr("Login and password cannot be empty."));
        return;
    }

    _loginBtn->setEnabled(false);
    auto result = _auth->login(login, pass);
    _loginBtn->setEnabled(true);

    if (auto* code = std::get_if<ResultCode>(&result)) {
        const QString text = (*code == ResultCode::UserBanned)
            ? tr("This account is blocked.")
            : tr("Wrong login or password.");
        showError(text);
        return;
    }

    auto rec = std::get<UserRecord>(result);
    if (rec.role != UserRole::Admin) {
        showError(tr("This account is not an administrator."));
        return;
    }

    _admin = rec;
    accept();
}

void LoginDialog::onCreateClicked() {
    clearError();
    const QString login = _createLoginEdit->text().trimmed();
    const QString name  = _createNameEdit->text().trimmed();
    const QString pass  = _createPassEdit->text();
    const QString rep   = _createPassRepeatEdit->text();

    if (login.isEmpty() || name.isEmpty() || pass.isEmpty()) {
        showError(tr("All fields are required."));
        return;
    }
    if (pass != rep) {
        showError(tr("Passwords do not match."));
        return;
    }
    if (pass.size() < 4) {
        showError(tr("Password must be at least 4 characters."));
        return;
    }

    _createBtn->setEnabled(false);
    auto result = _auth->registerAdmin(login, name, pass);
    _createBtn->setEnabled(true);

    if (auto* code = std::get_if<ResultCode>(&result)) {
        showError((*code == ResultCode::LoginAlreadyTaken)
            ? tr("Login is already taken.")
            : tr("Cannot create administrator. Check the database."));
        return;
    }

    _admin = std::get<UserRecord>(result);
    QMessageBox::information(this, tr("Success"),
        tr("Administrator '%1' created. You are now signed in.")
            .arg(_admin.login));
    accept();
}

} // namespace messenger::server::gui