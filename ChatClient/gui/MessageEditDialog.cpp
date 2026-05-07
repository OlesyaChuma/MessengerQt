#include "MessageEditDialog.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>

namespace messenger::client::gui {

MessageEditDialog::MessageEditDialog(const QString& currentText, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Edit message"));
    setMinimumWidth(420);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(10);

    auto* hint = new QLabel(tr("Edit your message and click Save."));
    hint->setStyleSheet("color: palette(mid);");
    layout->addWidget(hint);

    _editor = new QTextEdit;
    _editor->setPlainText(currentText);
    _editor->setAcceptRichText(false);
    _editor->setMinimumHeight(80);
    _editor->moveCursor(QTextCursor::End);
    _editor->setFocus();
    layout->addWidget(_editor, 1);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText(tr("Save"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    buttons->button(QDialogButtonBox::Save)->setObjectName("primaryButton");

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    // Ctrl+Enter для быстрого сохранения
    auto* saveBtn = buttons->button(QDialogButtonBox::Save);
    saveBtn->setShortcut(QKeySequence("Ctrl+Return"));
}

QString MessageEditDialog::newText() const {
    return _editor->toPlainText().trimmed();
}

} // namespace messenger::client::gui