#pragma once

#include <QString>
#include <QList>

namespace messenger::client::gui {

// Описание одного эмодзи: код (имя файла) + Unicode-символ.
// Один и тот же набор используется и в picker'е, и при замене в пузырях.
struct EmojiInfo {
    QString code;     // например "1F600"
    QString unicode;  // например "😀"
};

// Категория с заголовком и набором эмодзи
struct EmojiCategory {
    QString title;       // tr-ключ — переводится в TranslationManager
    QList<EmojiInfo> emojis;
};

// Полный набор всех 5 категорий
QList<EmojiCategory> emojiCategories();

// Соответствие Unicode → код (для замены в пузырях)
QString codeForUnicode(const QString& unicode);

} // namespace messenger::client::gui