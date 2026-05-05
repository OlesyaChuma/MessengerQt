#include "UserItemDelegate.h"
#include "UsersListModel.h"
#include "AvatarRenderer.h"

#include <QPainter>
#include <QApplication>
#include <QStyle>

namespace messenger::client::gui {

UserItemDelegate::UserItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

QSize UserItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                 const QModelIndex& /*index*/) const {
    return QSize(option.rect.width(), 56);
}

void UserItemDelegate::paint(QPainter* painter,
                             const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    const QRect r = option.rect;
    const bool selected = option.state & QStyle::State_Selected;
    const bool hovered  = option.state & QStyle::State_MouseOver;

    // Фон строки
    if (selected) {
        painter->fillRect(r, option.palette.highlight());
    } else if (hovered) {
        QColor h = option.palette.highlight().color();
        h.setAlpha(40);
        painter->fillRect(r, h);
    }

    // Извлекаем данные из модели
    const QString name  = index.data(UsersListModel::DisplayNameRole).toString();
    const QString login = index.data(UsersListModel::LoginRole).toString();
    const bool online   = index.data(UsersListModel::OnlineRole).toBool();
    const bool isBroadcast = index.data(UsersListModel::IsBroadcastRole).toBool();
    const bool unread   = index.data(UsersListModel::HasUnreadRole).toBool();

    const int padding = 10;
    const int avatarSize = 40;

    // Аватарка
    const QPixmap avatar = isBroadcast
        ? AvatarRenderer::renderBroadcast(avatarSize)
        : AvatarRenderer::renderWithStatus(login, name, online, avatarSize);
    const int avatarY = r.top() + (r.height() - avatarSize) / 2;
    painter->drawPixmap(r.left() + padding, avatarY, avatar);

    // Имя
    QFont nameFont = option.font;
    nameFont.setPointSizeF(nameFont.pointSizeF() + 0.5);
    if (unread) nameFont.setBold(true);
    painter->setFont(nameFont);

    QColor textColor = selected
        ? option.palette.highlightedText().color()
        : option.palette.text().color();
    painter->setPen(textColor);

    const int textLeft = r.left() + padding + avatarSize + padding;
    const int textRight = r.right() - padding;
    const QRect nameRect(textLeft, r.top(),
                         textRight - textLeft, r.height());
    painter->drawText(nameRect, Qt::AlignVCenter | Qt::AlignLeft, name);

    // Маркер непрочитанного — синяя точка справа
    if (unread && !selected) {
        const int dotSize = 9;
        const int dotX = r.right() - padding - dotSize;
        const int dotY = r.top() + (r.height() - dotSize) / 2;
        painter->setBrush(QColor("#2980b9"));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(dotX, dotY, dotSize, dotSize);
    }

    painter->restore();
}

} // namespace messenger::client::gui