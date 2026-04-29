#pragma once

#include <QWidget>

namespace messenger::server {
class Database;
}

namespace messenger::server::gui {

class ConnectionLogTab : public QWidget {
    Q_OBJECT
public:
    ConnectionLogTab(Database* db, QWidget* parent = nullptr);

private:
    Database* _db = nullptr;
};

} // namespace messenger::server::gui