#pragma once

#include "Models.h"

#include <QWidget>
#include <QHash>

class QLabel;

namespace messenger::client {
class ChatClientCore;
}

namespace messenger::client::gui {

class MessagesView;
class ChatInputWidget;

// Центральная панель чата:
//  - шапка с именем собеседника / # General chat
//  - лента сообщений (MessagesView)
//  - поле ввода (ChatInputWidget)
class ChatPanel : public QWidget {
    Q_OBJECT
public:
    ChatPanel(ChatClientCore* core, QWidget* parent = nullptr);

public slots:
    // peerId == 0 — broadcast, иначе приватный чат с этим userId
    void switchToChat(qint64 peerId, const QString& displayLabel);

signals:
    // Когда пришло сообщение из НЕ активного чата — показать индикатор.
    // peerId — кому/откуда (для приватки — собеседник, для broadcast — 0).
    void unreadInChat(qint64 peerId);

private slots:
    void onSendRequested(const QString& text);

    void onCoreNewMessage(const Message& m);
    void onCoreMessageEdited(qint64 messageId, const QString& body,
                             const QDateTime& at);
    void onCoreMessageDeleted(qint64 messageId, const QDateTime& at);
    void onRequestSucceeded(quint32 rid, const QJsonObject& payload);
    void onRequestFailed(quint32 rid, ResultCode code, const QString& desc);

    void onLanguageChanged();

    void onEditRequested(qint64 messageId);
    void onDeleteRequested(qint64 messageId);

private:
    void setupUi();
    void retranslateUi();
    void requestHistory(qint64 peerId);
    bool messageBelongsToActiveChat(const Message& m) const;

    ChatClientCore* _core = nullptr;

    QLabel*          _headerLbl = nullptr;
    MessagesView*    _messages = nullptr;
    ChatInputWidget* _input = nullptr;
    QWidget*         _emptyHint = nullptr;

    qint64  _activePeerId = -1;        // -1 = ничего ещё не выбрано
    QString _activeLabel;
    quint32 _pendingHistoryRequest = 0;

    quint32 _pendingEditRequest = 0;
    quint32 _pendingDeleteRequest = 0;
};

} // namespace messenger::client::gui
