#include "AvatarRenderer.h"

#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QCryptographicHash>

namespace messenger::client::gui {

namespace {

// Палитра приятных тёплых/холодных цветов для аватарок.
// Выбираем по hash от логина, чтобы цвет был стабилен.
const QList<QColor>& palette() {
    static const QList<QColor> p = {
        QColor("#e74c3c"),  // red
        QColor("#e67e22"),  // orange
        QColor("#f39c12"),  // amber
        QColor("#27ae60"),  // green
        QColor("#16a085"),  // teal
        QColor("#2980b9"),  // blue
        QColor("#8e44ad"),  // purple
        QColor("#c0392b"),  // dark red
        QColor("#d35400"),  // dark orange
        QColor("#2c3e50"),  // dark blue-gray
        QColor("#7f8c8d"),  // gray
        QColor("#1abc9c"),  // emerald
    };
    return p;
}

} // namespace

QString AvatarRenderer::extractInitials(const QString& displayName,
                                        const QString& fallbackLogin) {
    QString src = displayName.trimmed().isEmpty()
        ? fallbackLogin.trimmed()
        : displayName.trimmed();
    if (src.isEmpty()) return "?";

    // Пробуем взять первые буквы первых двух слов
    const QStringList parts = src.split(' ', Qt::SkipEmptyParts);
    QString result;
    if (parts.size() >= 2) {
        result += parts[0].at(0);
        result += parts[1].at(0);
    } else if (parts.size() == 1) {
        const QString& w = parts[0];
        result += w.at(0);
        if (w.size() > 1) {
            // вторая буква - тоже первая, если нет пробелов
            result += w.at(1);
        }
    }
    return result.toUpper();
}

QColor AvatarRenderer::colorForLogin(const QString& login) {
    if (login.isEmpty()) return palette().first();
    // SHA-1 для стабильного hash — байтов хватает для индекса
    const QByteArray digest = QCryptographicHash::hash(
        login.toUtf8(), QCryptographicHash::Sha1);
    const int index = static_cast<unsigned char>(digest.at(0)) % palette().size();
    return palette().at(index);
}

QPixmap AvatarRenderer::render(const QString& login,
                               const QString& displayName,
                               int size) {
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Фон-кружок
    const QColor bg = colorForLogin(login);
    p.setBrush(bg);
    p.setPen(Qt::NoPen);
    p.drawEllipse(0, 0, size, size);

    // Инициалы
    const QString initials = extractInitials(displayName, login);
    QFont f = p.font();
    f.setPointSize(qMax(8, static_cast<int>(size * 0.4)));
    f.setBold(true);
    p.setFont(f);
    p.setPen(Qt::white);
    p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, initials);

    return pm;
}

QPixmap AvatarRenderer::renderWithStatus(const QString& login,
                                         const QString& displayName,
                                         bool online,
                                         int size) {
    QPixmap pm = render(login, displayName, size);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int dotSize = qMax(8, static_cast<int>(size * 0.28));
    const int x = size - dotSize - 1;
    const int y = size - dotSize - 1;

    // Белая обводка для контраста
    p.setPen(QPen(QColor("#ffffff"), 2));
    p.setBrush(online ? QColor("#27ae60") : QColor("#95a5a6"));
    p.drawEllipse(x, y, dotSize, dotSize);

    return pm;
}

QPixmap AvatarRenderer::renderBroadcast(int size) {
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);

    p.setBrush(QColor("#2980b9"));
    p.setPen(Qt::NoPen);
    p.drawEllipse(0, 0, size, size);

    QFont f = p.font();
    f.setPointSize(qMax(10, static_cast<int>(size * 0.5)));
    f.setBold(true);
    p.setFont(f);
    p.setPen(Qt::white);
    p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, "#");

    return pm;
}

} // namespace messenger::client::gui