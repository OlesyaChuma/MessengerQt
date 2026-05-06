#include "ChatInputWidget.h"

#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QKeyEvent>
#include <QFontMetrics>

#include "TranslationManager.h"

namespace messenger::client::gui {

ChatInputWidget::ChatInputWidget(QWidget* parent) : QWidget(parent) {
    setObjectName("chatInputWidget");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 12);
    layout->setSpacing(8);

    _editor = new QTextEdit;
    _editor->setObjectName("chatInputEditor");
    _editor->setPlaceholderText(tr("Type a message..."));
    _editor->setAcceptRichText(false);
    _editor->installEventFilter(this);
    _editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _editor->setTabChangesFocus(true);

    _sendBtn = new QPushButton("➤");
    _sendBtn->setObjectName("sendArrowButton");
    _sendBtn->setFixedSize(36, 36);
    _sendBtn->setToolTip(tr("Send"));
    _sendBtn->setCursor(Qt::PointingHandCursor);

    layout->addWidget(_editor, 1);
    layout->addWidget(_sendBtn, 0, Qt::AlignVCenter);

    updateHeight();

    connect(_sendBtn, &QPushButton::clicked,
            this, &ChatInputWidget::onSendClicked);
    connect(_editor, &QTextEdit::textChanged,
            this, &ChatInputWidget::onTextChanged);

    // При смене языка — обновляем placeholder, если он стандартный
    connect(&TranslationManager::instance(),
            &TranslationManager::languageChanged,
            this, [this]() {
                // Placeholder обновится из ChatPanel::retranslateUi через setPlaceholder.
                // Здесь мы только обновим базовый, если placeholder ещё пустой/дефолтный.
                if (_editor->placeholderText().isEmpty()) {
                    _editor->setPlaceholderText(tr("Type a message..."));
                }
            });
}

QString ChatInputWidget::text() const {
    return _editor->toPlainText().trimmed();
}

void ChatInputWidget::clear() {
    _editor->clear();
    updateHeight();
}

void ChatInputWidget::setEnabled(bool enabled) {
    _editor->setEnabled(enabled);
    _sendBtn->setEnabled(enabled);
}

void ChatInputWidget::setPlaceholder(const QString& text) {
    _editor->setPlaceholderText(text);
}

void ChatInputWidget::onSendClicked() {
    const QString t = text();
    if (t.isEmpty()) return;
    emit sendRequested(t);
    clear();
}

void ChatInputWidget::onTextChanged() {
    updateHeight();
}

void ChatInputWidget::updateHeight() {
    // Telegram-стиль: одна строка ~36px, расширяется до 4 строк, потом скролл.
    QFontMetrics fm(_editor->font());
    const int lineH = fm.lineSpacing();
    const int padding = 12;          // 6px сверху + 6px снизу внутри QTextEdit
    const int minHeight = 36;        // одна строка как стандарт мессенджера

    const int doc = qMax(lineH,
                         static_cast<int>(_editor->document()->size().height()));
    const int rows = qBound(1, (doc + lineH - 1) / lineH, 4);

    int target = rows * lineH + padding;
    target = qMax(target, minHeight);
    target = qMin(target, lineH * 4 + padding);

    _editor->setFixedHeight(target);
}

bool ChatInputWidget::eventFilter(QObject* watched, QEvent* event) {
    if (watched == _editor && event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        const bool isEnter = (keyEvent->key() == Qt::Key_Return ||
                              keyEvent->key() == Qt::Key_Enter);
        if (isEnter && !(keyEvent->modifiers() & Qt::ShiftModifier)) {
            onSendClicked();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

} // namespace messenger::client::gui
