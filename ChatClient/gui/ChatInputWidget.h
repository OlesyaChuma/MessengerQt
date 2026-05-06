#pragma once

#include <QWidget>

class QTextEdit;
class QPushButton;

namespace messenger::client::gui {

// Поле ввода сообщения с кнопкой Send.
// Enter = отправка, Shift+Enter = перенос строки.
// Высота автоматически растёт до 5 строк, потом скролл.
class ChatInputWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChatInputWidget(QWidget* parent = nullptr);

    QString text() const;
    void clear();
    void setEnabled(bool enabled);
    void setPlaceholder(const QString& text);

signals:
    void sendRequested(const QString& text);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onSendClicked();
    void onTextChanged();

private:
    void updateHeight();

    QTextEdit*   _editor = nullptr;
    QPushButton* _sendBtn = nullptr;
};

} // namespace messenger::client::gui