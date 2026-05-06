#pragma once

#include "Models.h"

#include <QWidget>

class QLabel;

namespace messenger::client::gui {

// Один пузырь сообщения.
// Свои сообщения — справа синие, чужие — слева серые.
// Над пузырём имя отправителя (только для чужих),
// под пузырём время отправки маленьким серым.
class MessageBubble : public QWidget {
    Q_OBJECT
public:
    MessageBubble(const Message& msg, bool isMine, QWidget* parent = nullptr);

    qint64 messageId() const { return _msg.id; }
    const Message& message() const { return _msg; }
    bool isMine() const { return _mine; }

    void updateBody(const QString& newBody, const QDateTime& editedAt);
    void markDeleted(const QDateTime& deletedAt);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

signals:
    void editRequested(qint64 messageId);
    void deleteRequested(qint64 messageId);

private:
    void rebuild();
    QString formatTime() const;

    Message _msg;
    bool _mine = false;

    QLabel* _senderLbl = nullptr;
    QLabel* _bodyLbl = nullptr;
    QLabel* _metaLbl = nullptr;
};

} // namespace messenger::client::gui