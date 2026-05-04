#include "ChatPanel.h"
#include "ChatClientCore.h"

#include <QVBoxLayout>
#include <QLabel>

namespace messenger::client::gui {

ChatPanel::ChatPanel(ChatClientCore* core, QWidget* parent)
    : QWidget(parent), _core(core) {
    auto* layout = new QVBoxLayout(this);
    auto* placeholder = new QLabel(tr("Chat view — coming in next package"));
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setObjectName("placeholderLabel");
    placeholder->setWordWrap(true);
    layout->addWidget(placeholder);
}

} // namespace messenger::client::gui