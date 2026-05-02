#pragma once

#include <QObject>
#include <QTranslator>

namespace messenger::server::gui {

// Управление языком интерфейса (English / Russian).
// Переводы загружаются из ресурсов (qm-файлы).
class TranslationManager : public QObject {
    Q_OBJECT
public:
    enum class Language { English, Russian };
    Q_ENUM(Language)

    static TranslationManager& instance();

    Language current() const { return _current; }
    void apply(Language lang);

    static QString name(Language lang);

signals:
    void languageChanged(Language newLang);

private:
    explicit TranslationManager(QObject* parent = nullptr);

    Language loadSavedLanguage() const;
    void saveLanguage(Language lang) const;

    Language _current = Language::English;
    QTranslator _appTranslator;       // наши переводы
    QTranslator _qtTranslator;        // системные строки Qt (Ok/Cancel и т.п.)
};

} // namespace messenger::server::gui