#pragma once

#include <QDialog>

class QTextEdit;
class QDialogButtonBox;

namespace messenger::client::gui {

// Модальный диалог редактирования сообщения.
// Принимает старый текст, возвращает отредактированный через newText().
class MessageEditDialog : public QDialog {
    Q_OBJECT
public:
    MessageEditDialog(const QString& currentText, QWidget* parent = nullptr);

    QString newText() const;

private:
    QTextEdit* _editor = nullptr;
};

} // namespace messenger::client::gui