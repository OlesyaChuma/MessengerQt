#include "ChatPanel.h"
#include "MessagesView.h"
#include "ChatInputWidget.h"
#include "TranslationManager.h"

#include "ChatClientCore.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QJsonArray>
#include <QCoreApplication>

namespace messenger::client::gui {

ChatPanel::ChatPanel(ChatClientCore* core, QWidget* parent)
    : QWidget(parent), _core(core) {
    setObjectName("chatPanel");
    setupUi();

    connect(_core, &ChatClientCore::newMessageArrived,
            this, &ChatPanel::onCoreNewMessage);
    connect(_core, &ChatClientCore::messageEdited,
            this, &ChatPanel::onCoreMessageEdited);
    connect(_core, &ChatClientCore::messageDeleted,
            this, &ChatPanel::onCoreMessageDeleted);
    connect(_core, &ChatClientCore::requestSucceeded,
            this, &ChatPanel::onRequestSucceeded);
    connect(_core, &ChatClientCore::requestFailed,
            this, &ChatPanel::onRequestFailed);

    connect(&TranslationManager::instance(),
            &TranslationManager::languageChanged,
            this, &ChatPanel::onLanguageChanged);
}

void ChatPanel::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ----- Header -----
    auto* headerWrap = new QWidget;
    headerWrap->setObjectName("chatHeader");
    auto* headerLayout = new QHBoxLayout(headerWrap);
    headerLayout->setContentsMargins(16, 12, 16, 12);

    _headerLbl = new QLabel(tr("Select a conversation to start"));
    _headerLbl->setObjectName("chatHeaderLabel");
    headerLayout->addWidget(_headerLbl);
    headerLayout->addStretch(1);

    layout->addWidget(headerWrap);

    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("chatSeparator");
    layout->addWidget(sep);

    // ----- Messages view -----
    _messages = new MessagesView;
    _messages->showPlaceholder(
        tr("Choose a user or # General chat from the list to start messaging."));
    layout->addWidget(_messages, 1);

    // ----- Input -----
    _input = new ChatInputWidget;
    _input->setEnabled(false);
    layout->addWidget(_input);

    connect(_input, &ChatInputWidget::sendRequested,
            this, &ChatPanel::onSendRequested);

    setStyleSheet(R"(
        QWidget#chatHeader {
            background-color: #5dade2;
            border-bottom: 1px solid #4a9bcd;
        }
        QLabel#chatHeaderLabel {
            font-size: 15px;
            font-weight: 600;
            background: transparent;
            color: white;
        }
        QFrame#chatSeparator { color: palette(mid); }
        QLabel#messagesPlaceholder {
            color: palette(mid);
            font-size: 14px;
            font-style: italic;
            padding: 32px;
        }

        /* Поле ввода — компактное, как в Telegram */
        QWidget#chatInputWidget { background-color: palette(window); }
        QTextEdit#chatInputEditor {
            border: 1px solid palette(mid);
            border-radius: 18px;
            padding: 6px 14px;
            background-color: palette(alternate-base);
            color: palette(text);
            selection-background-color: #5dade2;
            selection-color: white;
            font-size: 13px;
        }
        QTextEdit#chatInputEditor:focus { border-color: #5dade2; }

        /* Круглая кнопка отправки со стрелкой */
        QPushButton#sendArrowButton {
            background-color: #5dade2;
            color: white;
            border: none;
            border-radius: 18px;
            font-size: 18px;
            font-weight: bold;
            padding: 0 0 1px 2px;
        }
        QPushButton#sendArrowButton:hover {
            background-color: #4a9bcd;
        }
        QPushButton#sendArrowButton:pressed {
            background-color: #3a89bb;
        }
        QPushButton#sendArrowButton:disabled {
            background-color: #95a5a6;
        }
    )");
}

void ChatPanel::switchToChat(qint64 peerId, const QString& displayLabel) {
    _activePeerId = peerId;
    _activeLabel = displayLabel;
    _headerLbl->setText(displayLabel);
    _input->setEnabled(true);
    _input->setPlaceholder(peerId == 0
        ? tr("Message everyone in # General chat")
        : tr("Message %1").arg(displayLabel));
    requestHistory(peerId);
}

void ChatPanel::requestHistory(qint64 peerId) {
    _messages->showPlaceholder(tr("Loading messages..."));
    _pendingHistoryRequest = _core->requestHistory(peerId, 0, 50);
}

void ChatPanel::onRequestSucceeded(quint32 rid, const QJsonObject& payload) {
    // 1) Ответ на загрузку истории
    if (rid == _pendingHistoryRequest) {
        _pendingHistoryRequest = 0;

        QList<Message> messages;
        const auto arr = payload.value("messages").toArray();
        for (const auto& v : arr) {
            messages.append(messageFromJson(v.toObject()));
        }
        if (messages.isEmpty()) {
            _messages->showPlaceholder(_activePeerId == 0
                                           ? tr("# General chat is empty. Be the first to write!")
                                           : tr("No messages with %1 yet. Say hi!").arg(_activeLabel));
        } else {
            _messages->setMessages(messages, _core->currentUserId());
        }
        return;
    }

    // 2) Ответ на send_message — добавляем своё сообщение в ленту,
    //    потому что сервер не шлёт push отправителю.
    if (payload.contains("message")) {
        const auto m = messageFromJson(payload.value("message").toObject());
        if (messageBelongsToActiveChat(m)) {
            _messages->appendMessage(m, _core->currentUserId());
        }
    }
}

void ChatPanel::onRequestFailed(quint32 rid, ResultCode /*code*/,
                                const QString& desc) {
    if (rid == _pendingHistoryRequest) {
        _pendingHistoryRequest = 0;
        _messages->showPlaceholder(tr("Cannot load history: %1").arg(desc));
    }
}

void ChatPanel::onSendRequested(const QString& text) {
    if (_activePeerId < 0 || text.isEmpty()) return;
    if (_activePeerId == 0) {
        _core->sendBroadcast(text);
    } else {
        _core->sendPrivateMessage(_activePeerId, text);
    }
}

bool ChatPanel::messageBelongsToActiveChat(const Message& m) const {
    if (_activePeerId < 0) return false;
    if (_activePeerId == 0) {
        return m.isBroadcast;
    }
    if (m.isBroadcast) return false;
    const qint64 me = _core->currentUserId();
    // Приватный чат: либо я отправил собеседнику, либо собеседник прислал мне.
    return (m.senderId == _activePeerId && m.receiverId == me) ||
           (m.senderId == me && m.receiverId == _activePeerId);
}

void ChatPanel::onCoreNewMessage(const Message& m) {
    if (messageBelongsToActiveChat(m)) {
        _messages->appendMessage(m, _core->currentUserId());
        return;
    }
    // Сигналим, что в другом чате есть непрочитанное.
    if (m.isBroadcast) {
        emit unreadInChat(0);
    } else {
        const qint64 me = _core->currentUserId();
        const qint64 peer = (m.senderId == me) ? m.receiverId : m.senderId;
        emit unreadInChat(peer);
    }
}

void ChatPanel::onCoreMessageEdited(qint64 messageId, const QString& body,
                                    const QDateTime& at) {
    _messages->applyEdited(messageId, body, at);
}

void ChatPanel::onCoreMessageDeleted(qint64 messageId, const QDateTime& at) {
    _messages->applyDeleted(messageId, at);
}

void ChatPanel::onLanguageChanged() {
    // Откладываем обновление до следующего цикла обработки событий —
    // в этот момент Qt уже точно применит новые .qm-файлы переводов.
    QMetaObject::invokeMethod(this, &ChatPanel::retranslateUi,
                              Qt::QueuedConnection);
}

void ChatPanel::retranslateUi() {
    if (_activePeerId < 0) {
        _headerLbl->setText(tr("Select a conversation to start"));
        _messages->showPlaceholder(
            tr("Choose a user or # General chat from the list to start messaging."));
        return;
    }

    // Активный broadcast-чат — пересчитываем имя чата по новому языку
    if (_activePeerId == 0) {
        _activeLabel = tr("# General chat");
        _headerLbl->setText(_activeLabel);
        _input->setPlaceholder(tr("Message everyone in # General chat"));
    } else {
        // Приватный — имя пользователя не зависит от языка, только placeholder
        _headerLbl->setText(_activeLabel);
        _input->setPlaceholder(tr("Message %1").arg(_activeLabel));
    }
}

} // namespace messenger::client::gui
