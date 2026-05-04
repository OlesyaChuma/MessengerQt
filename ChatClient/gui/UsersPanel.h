#pragma once

#include <QWidget>

namespace messenger::client {
class ChatClientCore;
}

namespace messenger::client::gui {

class UsersPanel : public QWidget {
    Q_OBJECT
public:
    UsersPanel(ChatClientCore* core, QWidget* parent = nullptr);

private:
    ChatClientCore* _core = nullptr;
};

} // namespace messenger::client::gui