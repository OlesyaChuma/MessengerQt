#include "ConnectionLogTab.h"

#include <QVBoxLayout>
#include <QLabel>

namespace messenger::server::gui {

ConnectionLogTab::ConnectionLogTab(Database* db, QWidget* parent)
    : QWidget(parent), _db(db) {
    auto* layout = new QVBoxLayout(this);
    auto* placeholder = new QLabel(tr("Connection log — coming in next package"));
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setObjectName("placeholderLabel");
    layout->addWidget(placeholder);
}

} // namespace messenger::server::gui