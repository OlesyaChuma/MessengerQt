#pragma once

#include <QObject>
#include <QSystemTrayIcon>

class QMenu;
class QAction;
class QMainWindow;

namespace messenger::server::gui {

// Иконка в системном трее.
// При активации (двойной клик / клик по уведомлению) разворачивает окно.
// Контекстное меню: Show / Hide / About / Quit.
class TrayIcon : public QObject {
    Q_OBJECT
public:
    TrayIcon(QMainWindow* mainWindow, QObject* parent = nullptr);

    bool isAvailable() const { return QSystemTrayIcon::isSystemTrayAvailable(); }

    void show();
    void hide();

    void setOnlineCount(int count);
    void notify(const QString& title, const QString& body,
                QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,
                int durationMs = 4000);

signals:
    void aboutRequested();
    void quitRequested();

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowMain();
    void onHideMain();

private:
    void setupMenu();
    void updateTooltip();

    QMainWindow*     _main = nullptr;
    QSystemTrayIcon* _tray = nullptr;
    QMenu*           _menu = nullptr;

    QAction* _showAct = nullptr;
    QAction* _hideAct = nullptr;
    QAction* _aboutAct = nullptr;
    QAction* _quitAct = nullptr;

    int _onlineCount = 0;
};

} // namespace messenger::server::gui