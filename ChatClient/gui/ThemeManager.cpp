#include "ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QTextStream>

namespace messenger::client::gui {

ThemeManager& ThemeManager::instance() {
    static ThemeManager s;
    return s;
}

ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {
    _current = loadSavedTheme();
}

ThemeManager::Theme ThemeManager::loadSavedTheme() const {
    QSettings s;
    const QString name = s.value("client/ui/theme", "light").toString();
    return name == "dark" ? Theme::Dark : Theme::Light;
}

void ThemeManager::saveTheme(Theme t) const {
    QSettings s;
    s.setValue("client/ui/theme", t == Theme::Dark ? "dark" : "light");
}

void ThemeManager::apply(Theme t) {
    const QString path = (t == Theme::Dark)
        ? ":/client/client_dark.qss"
        : ":/client/client_light.qss";

    QFile f(path);
    QString css;
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        css = QTextStream(&f).readAll();
    }
    qApp->setStyleSheet(css);

    if (t != _current) {
        _current = t;
        saveTheme(t);
        emit themeChanged(t);
    }
}

void ThemeManager::toggle() {
    apply(_current == Theme::Light ? Theme::Dark : Theme::Light);
}

} // namespace messenger::client::gui