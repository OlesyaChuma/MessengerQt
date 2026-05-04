#pragma once

#include "ChatClientCore.h"

#include <QDialog>

class QLineEdit;
class QPushButton;
class QLabel;
class QStackedWidget;
class QSpinBox;
class QCheckBox;

namespace messenger::client::gui {

// Окно входа/регистрации клиента.
// Сохраняет в QSettings адрес сервера, порт и последний логин,
// чтобы пользователю не нужно было каждый раз вводить заново.
class LoginWindow : public QDialog {
    Q_OBJECT
public:
    explicit LoginWindow(ChatClientCore* core, QWidget* parent = nullptr);

    QString chosenLogin() const { return _chosenLogin; }

private slots:
    void onSwitchToLogin();
    void onSwitchToRegister();
    void onConnectAndLogin();
    void onConnectAndRegister();

    void onCoreState(ChatClientCore::State oldS, ChatClientCore::State newS);
    void onCoreError(const QString& desc);
    void onRequestSucceeded(quint32 rid, const QJsonObject& payload);
    void onRequestFailed(quint32 rid, ResultCode code, const QString& desc);

    void onLanguageChanged();
    void onSelectLanguage();
    void onThemeToggled();

private:
    void setupUi();
    void retranslateUi();
    void loadSavedSettings();
    void saveSettings();
    void setError(const QString& text);
    void clearError();
    void setBusy(bool busy);

    ChatClientCore* _core = nullptr;

    QStackedWidget* _stack = nullptr;
    QLabel*  _title = nullptr;

    // Server settings
    QLineEdit* _hostEdit = nullptr;
    QSpinBox*  _portEdit = nullptr;

    // Login page
    QLineEdit*   _loginEdit = nullptr;
    QLineEdit*   _passEdit = nullptr;
    QCheckBox*   _rememberCheck = nullptr;
    QPushButton* _loginBtn = nullptr;
    QPushButton* _switchToRegBtn = nullptr;

    // Register page
    QLineEdit*   _regLoginEdit = nullptr;
    QLineEdit*   _regNameEdit = nullptr;
    QLineEdit*   _regPassEdit = nullptr;
    QLineEdit*   _regPassRepeatEdit = nullptr;
    QPushButton* _regBtn = nullptr;
    QPushButton* _switchToLoginBtn = nullptr;

    // Лейблы форм — нужны для retranslateUi
    QLabel* _serverLbl = nullptr;
    QLabel* _loginLbl = nullptr;
    QLabel* _passLbl = nullptr;
    QLabel* _regLoginLbl = nullptr;
    QLabel* _regNameLbl = nullptr;
    QLabel* _regPassLbl = nullptr;
    QLabel* _regRepeatLbl = nullptr;

    QLabel* _errorLabel = nullptr;
    QLabel* _statusLabel = nullptr;

    QPushButton* _themeBtn = nullptr;
    QPushButton* _langBtn = nullptr;

    bool _busy = false;
    bool _expectLogin = true;
    QString _chosenLogin;
    quint32 _pendingRequest = 0;
};

} // namespace messenger::client::gui
