#include "Config.h"

#include <QSettings>
#include <QFileInfo>

namespace messenger::server {

Config Config::loadFromFile(const QString& path) {
    Config cfg;

    if (!QFileInfo::exists(path)) {
        return cfg; // дефолты
    }

    QSettings ini(path, QSettings::IniFormat);

    cfg.database.host     = ini.value("Database/host",     cfg.database.host).toString();
    cfg.database.port     = ini.value("Database/port",     cfg.database.port).toInt();
    cfg.database.name     = ini.value("Database/name",     cfg.database.name).toString();
    cfg.database.user     = ini.value("Database/user",     cfg.database.user).toString();
    cfg.database.password = ini.value("Database/password", cfg.database.password).toString();

    cfg.listen.host            = ini.value("Server/listen_host",
                                           cfg.listen.host).toString();
    cfg.listen.port            = ini.value("Server/listen_port",
                                           cfg.listen.port).toInt();
    cfg.listen.maxAttachmentMb = ini.value("Server/max_attachment_size_mb",
                                           cfg.listen.maxAttachmentMb).toInt();

    cfg.logging.logFile  = ini.value("Logging/log_file",  cfg.logging.logFile).toString();
    cfg.logging.logLevel = ini.value("Logging/log_level", cfg.logging.logLevel).toString();

    return cfg;
}

bool Config::saveToFile(const QString& path) const {
    QSettings ini(path, QSettings::IniFormat);

    ini.setValue("Database/host",     database.host);
    ini.setValue("Database/port",     database.port);
    ini.setValue("Database/name",     database.name);
    ini.setValue("Database/user",     database.user);
    ini.setValue("Database/password", database.password);

    ini.setValue("Server/listen_host",            listen.host);
    ini.setValue("Server/listen_port",            listen.port);
    ini.setValue("Server/max_attachment_size_mb", listen.maxAttachmentMb);

    ini.setValue("Logging/log_file",  logging.logFile);
    ini.setValue("Logging/log_level", logging.logLevel);

    ini.sync();
    return ini.status() == QSettings::NoError;
}

} // namespace messenger::server