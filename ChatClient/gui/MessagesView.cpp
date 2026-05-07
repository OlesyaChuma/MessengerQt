#include "MessagesView.h"
#include "MessageBubble.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QTimer>

namespace messenger::client::gui {

MessagesView::MessagesView(QWidget* parent) : QScrollArea(parent) {
    setObjectName("messagesView");
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);

    _container = new QWidget;
    _container->setObjectName("messagesContainer");
    _layout = new QVBoxLayout(_container);
    _layout->setContentsMargins(8, 8, 8, 8);
    _layout->setSpacing(2);
    _layout->addStretch(1);

    setWidget(_container);
}

void MessagesView::clear() {
    for (auto* b : _bubbles) b->deleteLater();
    _bubbles.clear();
    if (_placeholder) {
        _placeholder->deleteLater();
        _placeholder = nullptr;
    }
}

void MessagesView::showPlaceholder(const QString& text) {
    clear();
    _placeholder = new QLabel(text);
    _placeholder->setObjectName("messagesPlaceholder");
    _placeholder->setAlignment(Qt::AlignCenter);
    _placeholder->setWordWrap(true);
    // Вставляем перед stretch-spacer-ом (последним элементом)
    _layout->insertWidget(_layout->count() - 1, _placeholder, 1, Qt::AlignCenter);
}

Message MessagesView::findMessage(qint64 messageId) const {
    for (auto* b : _bubbles) {
        if (b->messageId() == messageId) {
            return b->message();
        }
    }
    return Message{};
}

void MessagesView::setMessages(const QList<Message>& messages,
                               qint64 currentUserId) {
    clear();
    for (const auto& m : messages) {
        const bool mine = (m.senderId == currentUserId);
        auto* b = new MessageBubble(m, mine, _container);
        connectBubble(b);
        _bubbles.append(b);
        _layout->insertWidget(_layout->count() - 1, b);
    }
    QTimer::singleShot(0, this, [this]() {
        scrollToBottom(false);
        QTimer::singleShot(80, this, [this]() {
            scrollToBottom(false);
        });
    });
}

void MessagesView::appendMessage(const Message& m, qint64 currentUserId) {
    if (_placeholder) {
        _placeholder->deleteLater();
        _placeholder = nullptr;
    }
    const bool mine = (m.senderId == currentUserId);
    auto* b = new MessageBubble(m, mine, _container);
    connectBubble(b);
    _bubbles.append(b);
    _layout->insertWidget(_layout->count() - 1, b);

    // Автоскролл к последнему сообщению — всегда, и через два этапа,
    // чтобы layout успел пересчитать высоту нового пузыря.
    QTimer::singleShot(0, this, [this]() {
        scrollToBottom(false);
        QTimer::singleShot(50, this, [this]() {
            scrollToBottom(false);
        });
    });
}

void MessagesView::applyEdited(qint64 messageId, const QString& newBody,
                               const QDateTime& editedAt) {
    for (auto* b : _bubbles) {
        if (b->messageId() == messageId) {
            b->updateBody(newBody, editedAt);
            return;
        }
    }
}

void MessagesView::applyDeleted(qint64 messageId, const QDateTime& deletedAt) {
    for (auto* b : _bubbles) {
        if (b->messageId() == messageId) {
            b->markDeleted(deletedAt);
            return;
        }
    }
}

bool MessagesView::isAtBottom() const {
    auto* sb = verticalScrollBar();
    return sb && (sb->value() >= sb->maximum() - 8);
}

void MessagesView::scrollToBottom(bool /*smooth*/) {
    auto* sb = verticalScrollBar();
    if (!sb) return;
    sb->setValue(sb->maximum());
}

void MessagesView::onScrolledToBottomCheck() {
    // Зарезервировано для подгрузки старых сообщений при scroll-up.
    // Реализуем в Пакете 5Б-3.
}

void MessagesView::connectBubble(MessageBubble* b) {
    connect(b, &MessageBubble::editRequested,
            this, &MessagesView::editRequested);
    connect(b, &MessageBubble::deleteRequested,
            this, &MessagesView::deleteRequested);
}

} // namespace messenger::client::gui
