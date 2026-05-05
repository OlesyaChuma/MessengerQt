#pragma once

#include <QStyledItemDelegate>

namespace messenger::client::gui {

// Кастомная отрисовка строки списка юзеров:
// круглая аватарка слева, имя пользователя посередине,
// маленькая точка о непрочитанном справа.
class UserItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit UserItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;
};

} // namespace messenger::client::gui