#include "UsersTab.h"

#include <QVBoxLayout>
#include <QLabel>

namespace messenger::server::gui {

UsersTab::UsersTab(Database* db, ChatServer* server, qint64 adminId,
                   QWidget* parent)
    : QWidget(parent), _db(db), _server(server), _adminId(adminId) {
    auto* layout = new QVBoxLayout(this);
    auto* placeholder = new QLabel(tr("Users table — coming in next package"));
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setObjectName("placeholderLabel");
    layout->addWidget(placeholder);
}

} // namespace messenger::server::gui