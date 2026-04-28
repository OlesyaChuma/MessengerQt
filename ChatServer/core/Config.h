#pragma once

#include <QString>

namespace messenger::server {

// Конфигурация сервера, читается из server.ini.
// Если файла нет, используются разумные значения по умолчанию.

struct DatabaseConfig {
    QString host       = "localhost";
    int     port       = 5432;
    QString name       = "messenger_db";
    QString user       = "messenger_user";
    QString password   = "messenger_pass_2026";
};

struct ListenConfig {
    QString host       = "0.0.0.0";
    int     port       = 54000;
    int     maxAttachmentMb = 10;
};

struct LoggingConfig {
    QString logFile    = "server.log";
    QString logLevel   = "info";   // debug | info | warning | error
};

class Config {
public:
    DatabaseConfig database;
    ListenConfig   listen;
    LoggingConfig  logging;

    // Загрузить из ini-файла. Если файл не найден или пуст, остаются значения по умолчанию.
    static Config loadFromFile(const QString& path);

    // Записать в ini-файл (для генерации при первом запуске).
    bool saveToFile(const QString& path) const;
};

} // namespace messenger::server