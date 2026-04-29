#include "MessagesTab.h"

#include <QVBoxLayout>
#include <QLabel>

namespace messenger::server::gui {

MessagesTab::MessagesTab(Database* db, ChatServer* server, QWidget* parent)
    : QWidget(parent), _db(db), _server(server) {
    auto* layout = new QVBoxLayout(this);
    auto* placeholder = new QLabel(tr("Messages feed — coming in next package"));
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setObjectName("placeholderLabel");
    layout->addWidget(placeholder);
}

} // namespace messenger::server::gui