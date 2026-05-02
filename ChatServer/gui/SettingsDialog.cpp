#include "SettingsDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

namespace messenger::server::gui {

SettingsDialog::SettingsDialog(const Config& cfg, const QString& configPath,
                               QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Server settings"));
    setModal(true);
    setMinimumWidth(440);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(20, 16, 20, 16);
    root->setSpacing(12);

    auto* hint = new QLabel(tr("Settings are loaded from: <b>%1</b>"
                               "<br/>To change values, edit this file and restart the server.")
                            .arg(configPath));
    hint->setWordWrap(true);
    hint->setStyleSheet("color: #7f8c8d;");
    root->addWidget(hint);

    auto makeReadonly = [&](const QString& text) {
        auto* lbl = new QLabel(text);
        lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
        lbl->setStyleSheet("padding: 4px 8px; background-color: rgba(127,140,141,0.1);"
                           "border-radius: 3px;");
        return lbl;
    };

    // ----- Database group -----
    auto* dbGroup = new QGroupBox(tr("Database"));
    auto* dbForm = new QFormLayout(dbGroup);
    dbForm->setSpacing(8);
    dbForm->addRow(tr("Host:"),     makeReadonly(cfg.database.host));
    dbForm->addRow(tr("Port:"),     makeReadonly(QString::number(cfg.database.port)));
    dbForm->addRow(tr("Database:"), makeReadonly(cfg.database.name));
    dbForm->addRow(tr("User:"),     makeReadonly(cfg.database.user));
    dbForm->addRow(tr("Password:"), makeReadonly(QString(cfg.database.password.size(), '*')));
    root->addWidget(dbGroup);

    // ----- Server group -----
    auto* srvGroup = new QGroupBox(tr("Server"));
    auto* srvForm = new QFormLayout(srvGroup);
    srvForm->setSpacing(8);
    srvForm->addRow(tr("Listen host:"), makeReadonly(cfg.listen.host));
    srvForm->addRow(tr("Listen port:"), makeReadonly(QString::number(cfg.listen.port)));
    srvForm->addRow(tr("Max attachment (MB):"),
                    makeReadonly(QString::number(cfg.listen.maxAttachmentMb)));
    root->addWidget(srvGroup);

    // ----- Logging group -----
    auto* logGroup = new QGroupBox(tr("Logging"));
    auto* logForm = new QFormLayout(logGroup);
    logForm->setSpacing(8);
    logForm->addRow(tr("Log file:"),  makeReadonly(cfg.logging.logFile));
    logForm->addRow(tr("Log level:"), makeReadonly(cfg.logging.logLevel));
    root->addWidget(logGroup);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    buttons->button(QDialogButtonBox::Close)->setText(tr("Close"));
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);
}

} // namespace messenger::server::gui
