#pragma once

#include <QPixmap>
#include <QString>

namespace messenger::client::gui {

// Генератор круглых аватарок с инициалами.
// Цвет генерируется детерминированно по логину - один и тот же
// пользователь всегда получает одинаковый цвет аватарки.
class AvatarRenderer {
public:
    // Получить аватарку для пользователя
    // login — для генерации цвета (стабильный hash)
    // displayName — для извлечения инициалов
    // size — диаметр круга в пикселях (по умолчанию 36)
    static QPixmap render(const QString& login,
                          const QString& displayName,
                          int size = 36);

    // Аватарка с маленькой зелёной точкой online-статуса в правом нижнем углу
    static QPixmap renderWithStatus(const QString& login,
                                    const QString& displayName,
                                    bool online,
                                    int size = 36);

    // Специальная аватарка для broadcast-чата — иконка #
    static QPixmap renderBroadcast(int size = 36);

private:
    static QString extractInitials(const QString& displayName,
                                   const QString& fallbackLogin);
    static QColor colorForLogin(const QString& login);
};

} // namespace messenger::client::gui