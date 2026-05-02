#include "TrayIcon.h"

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QApplication>

namespace messenger::server::gui {

TrayIcon::TrayIcon(QMainWindow* mainWindow, QObject* parent)
    : QObject(parent), _main(mainWindow) {
    _tray = new QSystemTrayIcon(this);
    _tray->setIcon(QIcon(":/server/icons/server.svg"));
    setupMenu();
    updateTooltip();

    connect(_tray, &QSystemTrayIcon::activated,
            this, &TrayIcon::onActivated);
}

void TrayIcon::setupMenu() {
    _menu = new QMenu;

    _showAct = _menu->addAction(tr("Show window"));
    _hideAct = _menu->addAction(tr("Hide window"));
    _menu->addSeparator();
    _aboutAct = _menu->addAction(tr("About..."));
    _menu->addSeparator();
    _quitAct = _menu->addAction(tr("Quit"));

    connect(_showAct,  &QAction::triggered, this, &TrayIcon::onShowMain);
    connect(_hideAct,  &QAction::triggered, this, &TrayIcon::onHideMain);
    connect(_aboutAct, &QAction::triggered, this, &TrayIcon::aboutRequested);
    connect(_quitAct,  &QAction::triggered, this, &TrayIcon::quitRequested);

    _tray->setContextMenu(_menu);
}

void TrayIcon::updateTooltip() {
    _tray->setToolTip(tr("MessengerQt Server\nOnline: %1").arg(_onlineCount));
}

void TrayIcon::show() {
    if (!isAvailable()) return;
    _tray->show();
}

void TrayIcon::hide() {
    _tray->hide();
}

void TrayIcon::setOnlineCount(int count) {
    _onlineCount = count;
    updateTooltip();
}

void TrayIcon::notify(const QString& title, const QString& body,
                      QSystemTrayIcon::MessageIcon icon, int durationMs) {
    if (!isAvailable() || !_tray->isVisible()) return;
    _tray->showMessage(title, body, icon, durationMs);
}

void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger ||
        reason == QSystemTrayIcon::DoubleClick) {
        onShowMain();
    }
}

void TrayIcon::onShowMain() {
    if (!_main) return;
    _main->showNormal();
    _main->raise();
    _main->activateWindow();
}

void TrayIcon::onHideMain() {
    if (_main) _main->hide();
}

} // namespace messenger::server::gui