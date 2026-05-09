#include "EmojiData.h"

#include <QHash>
#include <QObject>

namespace messenger::client::gui {

namespace {

// Helper: создать EmojiInfo по hex-коду
EmojiInfo make(const QString& code) {
    bool ok = false;
    const auto value = code.toUInt(&ok, 16);
    if (!ok) return EmojiInfo{code, QString()};
    return EmojiInfo{code, QString::fromUcs4(reinterpret_cast<const char32_t*>(&value), 1)};
}

} // namespace

QList<EmojiCategory> emojiCategories() {
    return {
        {
            QObject::tr("Smileys"),
            {
                make("1F600"), make("1F601"), make("1F602"), make("1F923"),
                make("1F60A"), make("1F60D"), make("1F618"), make("1F61C"),
                make("1F914"), make("1F60E"), make("1F62D"), make("1F622"),
                make("1F621"), make("1F634"), make("1F92F"), make("1F973"),
            }
        },
        {
            QObject::tr("Hands"),
            {
                make("1F44D"), make("1F44E"), make("1F44F"), make("1F64C"),
                make("1F64F"), make("1F44A"), make("270A"),  make("270C"),
                make("1F91D"), make("1F91F"), make("1F90C"), make("1F44B"),
                make("1F4AA"), make("1F91E"), make("1F446"), make("1F447"),
            }
        },
        {
            QObject::tr("Hearts"),
            {
                make("2764"),  make("1F9E1"), make("1F49B"), make("1F49A"),
                make("1F499"), make("1F49C"), make("1F5A4"), make("1F90D"),
                make("1F498"), make("1F495"), make("1F496"), make("1F497"),
                make("1F49E"), make("1F49F"), make("1F494"), make("1F49D"),
            }
        },
        {
            QObject::tr("Animals"),
            {
                make("1F436"), make("1F431"), make("1F42D"), make("1F430"),
                make("1F98A"), make("1F43B"), make("1F43C"), make("1F428"),
                make("1F981"), make("1F42F"), make("1F434"), make("1F42E"),
                make("1F437"), make("1F438"), make("1F435"), make("1F414"),
            }
        },
        {
            QObject::tr("Symbols"),
            {
                make("1F525"), make("2B50"),  make("2728"),  make("1F4A5"),
                make("1F389"), make("1F38A"), make("1F381"), make("2615"),
                make("1F37A"), make("1F355"), make("1F354"), make("1F370"),
                make("2705"),  make("274C"),  make("2753"),  make("2757"),
            }
        },
    };
}

QString codeForUnicode(const QString& unicode) {
    static QHash<QString, QString> cache;
    if (cache.isEmpty()) {
        for (const auto& cat : emojiCategories()) {
            for (const auto& e : cat.emojis) {
                cache.insert(e.unicode, e.code);
            }
        }
    }
    return cache.value(unicode);
}

} // namespace messenger::client::gui