#pragma once

#include "Models.h"

#include <QScrollArea>
#include <QList>

class QVBoxLayout;
class QLabel;

namespace messenger::client::gui {

class MessageBubble;

// Прокручиваемая лента пузырей сообщений.
// Поддерживает автоскролл при добавлении новых, обнуление при смене чата
// и подгрузку старых сообщений по прокрутке вверх (в Пакете 5Б-3 включим).
class MessagesView : public QScrollArea {
    Q_OBJECT
public:
    explicit MessagesView(QWidget* parent = nullptr);

    // Полная замена ленты (например, при переключении чата)
    void setMessages(const QList<Message>& messages, qint64 currentUserId);
    void clear();

    // Добавить одно сообщение в конец (live)
    void appendMessage(const Message& m, qint64 currentUserId);

    // Применить редактирование/удаление по id
    void applyEdited(qint64 messageId, const QString& newBody,
                     const QDateTime& editedAt);
    void applyDeleted(qint64 messageId, const QDateTime& deletedAt);

    // Заглушка-приветствие, пока чат не выбран
    void showPlaceholder(const QString& text);

signals:
    void editRequested(qint64 messageId);
    void deleteRequested(qint64 messageId);

private slots:
    void onScrolledToBottomCheck();

private:
    void scrollToBottom(bool smooth = false);
    void connectBubble(MessageBubble* b);
    bool isAtBottom() const;

    QWidget*     _container = nullptr;
    QVBoxLayout* _layout = nullptr;
    QList<MessageBubble*> _bubbles;

    QLabel* _placeholder = nullptr;
};

} // namespace messenger::client::gui
