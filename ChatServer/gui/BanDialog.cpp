#include "BanDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>

namespace messenger::server::gui {

BanDialog::BanDialog(Mode mode, const QString& userLogin, QWidget* parent)
    : QDialog(parent), _mode(mode) {
    setModal(true);
    setMinimumWidth(380);
    setWindowTitle(_mode == Ban ? tr("Ban user") : tr("Kick user"));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(20, 16, 20, 16);
    root->setSpacing(10);

    auto* title = new QLabel(_mode == Ban
        ? tr("Block user '%1' permanently").arg(userLogin)
        : tr("Disconnect user '%1' (one-time)").arg(userLogin));
    title->setStyleSheet("font-size: 14px; font-weight: 600;");
    root->addWidget(title);

    auto* hint = new QLabel(_mode == Ban
        ? tr("The user will not be able to log in until unbanned.")
        : tr("The user can reconnect immediately, but the current session ends."));
    hint->setWordWrap(true);
    hint->setStyleSheet("color: #7f8c8d;");
    root->addWidget(hint);

    auto* form = new QFormLayout;
    form->setSpacing(8);
    _reasonEdit = new QLineEdit;
    _reasonEdit->setPlaceholderText(_mode == Ban
        ? tr("Why are you banning this user?")
        : tr("Optional reason"));
    form->addRow(tr("Reason:"), _reasonEdit);
    root->addLayout(form);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText(
        _mode == Ban ? tr("Ban") : tr("Kick"));
    buttons->button(QDialogButtonBox::Ok)->setObjectName("primaryButton");
    root->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    _reasonEdit->setFocus();
}

QString BanDialog::reason() const {
    QString r = _reasonEdit->text().trimmed();
    if (r.isEmpty()) {
        r = (_mode == Ban) ? tr("Banned by admin")
                           : tr("Kicked by admin");
    }
    return r;
}

} // namespace messenger::server::gui