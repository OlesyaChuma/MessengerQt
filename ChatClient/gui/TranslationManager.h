#pragma once

#include <QObject>
#include <QTranslator>

namespace messenger::client::gui {

// Менеджер языка интерфейса клиента (English / Russian).
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
    QTranslator _appTranslator;
    QTranslator _qtTranslator;
};

} // namespace messenger::client::gui