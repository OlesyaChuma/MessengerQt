#pragma once

#include "AuthService.h"
#include "Database.h"
#include "Models.h"

#include <QDialog>

class QLineEdit;
class QPushButton;
class QLabel;
class QStackedWidget;

namespace messenger::server::gui {

// Диалог входа администратора в GUI-сервер.
// При первом запуске (нет ни одного админа) — режим создания первого админа.
// Иначе — режим входа.
class LoginDialog : public QDialog {
    Q_OBJECT
public:
    LoginDialog(AuthService* auth, QWidget* parent = nullptr);

    UserRecord adminRecord() const { return _admin; }

protected:
    void showEvent(QShowEvent* e) override;

private slots:
    void onLoginClicked();
    void onCreateClicked();

private:
    void setupUi();
    void setupStyles();
    void showError(const QString& text);
    void clearError();

    AuthService* _auth = nullptr;
    UserRecord _admin;

    QStackedWidget* _stack = nullptr;

    // Login page
    QLineEdit*  _loginEdit = nullptr;
    QLineEdit*  _passEdit = nullptr;
    QPushButton* _loginBtn = nullptr;

    // Create-admin page
    QLineEdit*  _createLoginEdit = nullptr;
    QLineEdit*  _createNameEdit = nullptr;
    QLineEdit*  _createPassEdit = nullptr;
    QLineEdit*  _createPassRepeatEdit = nullptr;
    QPushButton* _createBtn = nullptr;

    QLabel* _errorLabel = nullptr;
    QLabel* _titleLabel = nullptr;
};

} // namespace messenger::server::gui