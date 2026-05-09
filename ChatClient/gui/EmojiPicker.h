#pragma once

#include <QFrame>

class QTabWidget;

namespace messenger::client::gui {

// Всплывающее окно выбора эмодзи: 5 категорий, ~16 эмодзи в каждой.
// Клик по эмодзи испускает emojiSelected(unicodeChar) и закрывает окно.
class EmojiPicker : public QFrame {
    Q_OBJECT
public:
    explicit EmojiPicker(QWidget* parent = nullptr);

signals:
    void emojiSelected(const QString& unicode);

private:
    void buildUi();

    QTabWidget* _tabs = nullptr;
};

} // namespace messenger::client::gui