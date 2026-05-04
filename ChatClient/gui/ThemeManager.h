#pragma once

#include <QObject>

namespace messenger::client::gui {

// Менеджер тем клиентской части (light/dark).
// Параллельный аналог серверного ThemeManager — отдельные QSS-файлы.
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

} // namespace messenger::client::gui