#pragma once

#include "Config.h"

#include <QDialog>

namespace messenger::server::gui {

// Просмотр текущих настроек (только чтение, изменения через server.ini).
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(const Config& cfg, const QString& configPath,
                   QWidget* parent = nullptr);
};

} // namespace messenger::server::gui