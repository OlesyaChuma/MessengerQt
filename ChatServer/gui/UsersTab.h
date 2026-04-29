#pragma once

#include <QWidget>

namespace messenger::server {
class Database;
class ChatServer;
}

namespace messenger::server::gui {

class UsersTab : public QWidget {
    Q_OBJECT
public:
    UsersTab(Database* db, ChatServer* server, qint64 adminId,
             QWidget* parent = nullptr);

private:
    Database*   _db = nullptr;
    ChatServer* _server = nullptr;
    qint64      _adminId = 0;
};

} // namespace messenger::server::gui