#pragma once

#include <QWidget>

namespace messenger::client {
class ChatClientCore;
}

namespace messenger::client::gui {

class ChatPanel : public QWidget {
    Q_OBJECT
public:
    ChatPanel(ChatClientCore* core, QWidget* parent = nullptr);

private:
    ChatClientCore* _core = nullptr;
};

} // namespace messenger::client::gui