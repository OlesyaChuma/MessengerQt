#include "MessageBubble.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QMenu>
#include <QContextMenuEvent>
#include <QApplication>
#include "ThemeManager.h"

namespace messenger::client::gui {

namespace {

// Инкапсулированный «пузырь» — тёплый прямоугольник со скруглёнными углами.
class BubbleFrame : public QWidget {
public:
    BubbleFrame(bool mine, QWidget* parent = nullptr)
        : QWidget(parent), _mine(mine) {
        setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        setMinimumWidth(40);
        setMaximumWidth(560);

        // Отключаем глобальные QSS-стили фона, потому что мы сами
        // рисуем закруглённый пузырь в paintEvent.
        setAttribute(Qt::WA_StyledBackground, false);

        // Прозрачный фон под нашей рисовкой — иначе глобальный QSS
        // QWidget { background-color: #f5f6f8 } будет рисовать прямоугольник.
        setStyleSheet(
            "background: transparent;"
            "border: none;"
            );
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QColor bg;
        if (_mine) {
            bg = QColor("#5dade2");  // мягкий приятный голубой
        } else {
            const bool dark =
                (ThemeManager::instance().current() == ThemeManager::Theme::Dark);
            bg = dark ? QColor("#3a4150") : QColor("#e8eaed");
        }

        p.setBrush(bg);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(rect(), 12, 12);
    }

private:
    bool _mine;
};

} // namespace

MessageBubble::MessageBubble(const Message& msg, bool isMine, QWidget* parent)
    : QWidget(parent), _msg(msg), _mine(isMine) {
    setObjectName("messageBubbleRow");

    auto* outer = new QHBoxLayout(this);
    outer->setContentsMargins(8, 4, 8, 4);
    outer->setSpacing(0);

    // Контейнер «колонки»: имя отправителя -> пузырь -> время.
    auto* column = new QVBoxLayout;
    column->setSpacing(2);
    column->setContentsMargins(0, 0, 0, 0);

    // Имя отправителя — только для чужих
    _senderLbl = new QLabel(_msg.senderLogin);
    _senderLbl->setObjectName("bubbleSender");
    _senderLbl->setVisible(!_mine);
    column->addWidget(_senderLbl, 0,
                      _mine ? Qt::AlignRight : Qt::AlignLeft);

    // Сам пузырь
    auto* bubble = new BubbleFrame(_mine);
    auto* bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(12, 8, 12, 8);
    bubbleLayout->setSpacing(2);

    _bodyLbl = new QLabel(_msg.body);
    _bodyLbl->setObjectName("bubbleBody");
    _bodyLbl->setWordWrap(true);
    _bodyLbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    // Отключаем системное контекстное меню QLabel — у нас своё в MessageBubble.
    _bodyLbl->setContextMenuPolicy(Qt::NoContextMenu);
    bubbleLayout->addWidget(_bodyLbl);

    column->addWidget(bubble, 0,
                      _mine ? Qt::AlignRight : Qt::AlignLeft);

    // Время
    _metaLbl = new QLabel(formatTime());
    _metaLbl->setObjectName("bubbleMeta");
    column->addWidget(_metaLbl, 0,
                      _mine ? Qt::AlignRight : Qt::AlignLeft);

    if (_mine) {
        outer->addStretch(1);
        outer->addLayout(column);
    } else {
        outer->addLayout(column);
        outer->addStretch(1);
    }

    rebuild();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this]() { rebuild(); update(); });
}

QString MessageBubble::formatTime() const {
    QString time = _msg.createdAt.toLocalTime().toString("HH:mm");
    QStringList parts;
    parts << time;
    if (_msg.editedAt.isValid())  parts << tr("edited");
    if (_msg.deletedAt.isValid()) parts << tr("deleted");
    return parts.join(" · ");
}

void MessageBubble::rebuild() {
    if (_msg.deletedAt.isValid()) {
        _bodyLbl->setText(tr("[message deleted]"));
        QFont f = _bodyLbl->font();
        f.setItalic(true);
        _bodyLbl->setFont(f);
    } else {
        _bodyLbl->setText(_msg.body);
        QFont f = _bodyLbl->font();
        f.setItalic(false);
        _bodyLbl->setFont(f);
    }

    const bool dark = (ThemeManager::instance().current() == ThemeManager::Theme::Dark);

    // Цвет текста сообщения
    if (_mine) {
        // Чёрный текст на голубом — максимальный контраст
        _bodyLbl->setStyleSheet("color: #000000; background: transparent;");
    } else {
        // Чужие пузыри: чёрный текст на сером в светлой, белый — в тёмной
        _bodyLbl->setStyleSheet(dark
                                    ? "color: #ffffff; background: transparent;"
                                    : "color: #000000; background: transparent;");
    }

    // Имя отправителя — таким же цветом, как имена в списке пользователей
    _senderLbl->setStyleSheet(QString(
                                  "font-size: 12px; font-weight: 700; padding: 0 4px; "
                                  "background: transparent; color: %1;")
                                  .arg(dark ? "#d4d8de" : "#2c3e50"));

    // Время — приглушённый тон, но читаемый
    _metaLbl->setStyleSheet(QString(
                                "font-size: 11px; padding: 0 4px; "
                                "background: transparent; color: %1;")
                                .arg(dark ? "#a0a5ae" : "#5a6470"));

    _metaLbl->setText(formatTime());
}

void MessageBubble::updateBody(const QString& newBody,
                               const QDateTime& editedAt) {
    _msg.body = newBody;
    _msg.editedAt = editedAt;
    rebuild();
}

void MessageBubble::markDeleted(const QDateTime& deletedAt) {
    _msg.deletedAt = deletedAt;
    rebuild();
}

void MessageBubble::contextMenuEvent(QContextMenuEvent* event) {
    if (!_mine || _msg.deletedAt.isValid()) {
        QWidget::contextMenuEvent(event);
        return;
    }
    QMenu menu(this);
    auto* editAct = menu.addAction(tr("Edit"));
    auto* delAct  = menu.addAction(tr("Delete"));
    auto* chosen = menu.exec(event->globalPos());
    if (chosen == editAct)        emit editRequested(_msg.id);
    else if (chosen == delAct)    emit deleteRequested(_msg.id);
}

} // namespace messenger::client::gui
