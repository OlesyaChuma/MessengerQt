#pragma once

#include <QObject>
#include <QString>

namespace messenger::server::gui {

// Управление темами оформления (light/dark) через QSS из ресурсов.
// Сохраняет выбор в QSettings, чтобы запомнить между запусками.
class ThemeManager : public QObject {
    Q_OBJECT
public:
    enum class Theme { Light, Dark };
    Q_ENUM(Theme)

    static ThemeManager& instance();

    Theme current() const { return _current; }
    void apply(Theme t);
    void toggle();

signals:
    void themeChanged(Theme newTheme);

private:
    explicit ThemeManager(QObject* parent = nullptr);

    Theme loadSavedTheme() const;
    void saveTheme(Theme t) const;

    Theme _current = Theme::Light;
};

} // namespace messenger::server::gui