#pragma once

#include <QWidget>

namespace messenger::server {
class Database;
class ChatServer;
}

namespace messenger::server::gui {

class MessagesTab : public QWidget {
    Q_OBJECT
public:
    MessagesTab(Database* db, ChatServer* server, QWidget* parent = nullptr);

private:
    Database*   _db = nullptr;
    ChatServer* _server = nullptr;
};

} // namespace messenger::server::gui