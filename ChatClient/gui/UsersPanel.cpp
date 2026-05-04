#include "UsersPanel.h"
#include "ChatClientCore.h"

#include <QVBoxLayout>
#include <QLabel>

namespace messenger::client::gui {

UsersPanel::UsersPanel(ChatClientCore* core, QWidget* parent)
    : QWidget(parent), _core(core) {
    setMinimumWidth(240);
    auto* layout = new QVBoxLayout(this);
    auto* placeholder = new QLabel(tr("Users list — coming in next package"));
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setObjectName("placeholderLabel");
    placeholder->setWordWrap(true);
    layout->addWidget(placeholder);
}

} // namespace messenger::client::gui