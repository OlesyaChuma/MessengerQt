#include "core/ChatClientCore.h"
#include "ConsoleUi.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QStringList>
#include <iostream>

using namespace messenger;
using namespace messenger::client;

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("MessengerQt Console Client");
    QCoreApplication::setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Console chat client (debug build).");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption hostOpt({"H", "host"},
        "Server host (default: localhost)", "host", "localhost");
    QCommandLineOption portOpt({"p", "port"},
        "Server port (default: 54000)", "port", "54000");

    parser.addOption(hostOpt);
    parser.addOption(portOpt);
    parser.process(app);

    const QString host = parser.value(hostOpt);
    const quint16 port = static_cast<quint16>(parser.value(portOpt).toUInt());

    ChatClientCore core;
    ConsoleUi ui(&core, host, port);

    QObject::connect(&ui, &ConsoleUi::shutdownRequested,
                     &app, &QCoreApplication::quit);

    ui.start();

    const int rc = app.exec();
    core.disconnectFromServer();
    std::cout << "Client stopped.\n";
    return rc;
}