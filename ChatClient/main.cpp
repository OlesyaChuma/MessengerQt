#include "core/ChatClientCore.h"
#include "gui/LoginWindow.h"
#include "gui/MainWindow.h"
#include "gui/ThemeManager.h"
#include "gui/TranslationManager.h"

#include <QApplication>
#include <QIcon>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shobjidl.h>
#endif

using namespace messenger;
using namespace messenger::client;
using namespace messenger::client::gui;

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("MessengerQt Client");
    QApplication::setOrganizationName("MessengerQt");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setWindowIcon(QIcon(":/client/icons/app.svg"));

#ifdef Q_OS_WIN
    SetCurrentProcessExplicitAppUserModelID(L"MessengerQt.Client.1");
#endif

    ThemeManager::instance().apply(ThemeManager::instance().current());
    TranslationManager::instance().apply(TranslationManager::instance().current());

    ChatClientCore core;

    LoginWindow login(&core);
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow window(&core);
    window.show();

    return app.exec();
}