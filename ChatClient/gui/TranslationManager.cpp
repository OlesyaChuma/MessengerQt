#include "TranslationManager.h"

#include <QApplication>
#include <QSettings>
#include <QLibraryInfo>
#include <QLocale>

namespace messenger::client::gui {

TranslationManager& TranslationManager::instance() {
    static TranslationManager s;
    return s;
}

TranslationManager::TranslationManager(QObject* parent) : QObject(parent) {
    _current = loadSavedLanguage();
}

TranslationManager::Language TranslationManager::loadSavedLanguage() const {
    QSettings s;
    const QString name = s.value("client/ui/language").toString();
    if (name == "ru") return Language::Russian;
    if (name == "en") return Language::English;
    if (QLocale::system().language() == QLocale::Russian) return Language::Russian;
    return Language::English;
}

void TranslationManager::saveLanguage(Language lang) const {
    QSettings s;
    s.setValue("client/ui/language", lang == Language::Russian ? "ru" : "en");
}

QString TranslationManager::name(Language lang) {
    return lang == Language::Russian ? "Русский" : "English";
}

void TranslationManager::apply(Language lang) {
    qApp->removeTranslator(&_appTranslator);
    qApp->removeTranslator(&_qtTranslator);

    if (lang == Language::Russian) {
        const QString qtPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
        if (_qtTranslator.load("qt_ru", qtPath) ||
            _qtTranslator.load("qtbase_ru", qtPath)) {
            qApp->installTranslator(&_qtTranslator);
        }
        if (_appTranslator.load(":/client/i18n/messenger_client_ru.qm")) {
            qApp->installTranslator(&_appTranslator);
        }
    }

    if (lang != _current) {
        _current = lang;
        saveLanguage(lang);
    }
    emit languageChanged(lang);
}

} // namespace messenger::client::gui