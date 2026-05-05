#pragma once

#include "Models.h"

#include <QWidget>

class QListView;
class QLineEdit;
class QLabel;

namespace messenger::client {
class ChatClientCore;
}

namespace messenger::client::gui {

class UsersListModel;

class UsersPanel : public QWidget {
    Q_OBJECT
public:
    UsersPanel(ChatClientCore* core, QWidget* parent = nullptr);

    // Текущая выбранная цель чата:
    //   peerId == 0 — broadcast;
    //   peerId > 0 — приватный чат с этим пользователем.
    qint64 currentPeerId() const { return _currentPeerId; }

public slots:
    void reloadUsers();
    void onUserOnlineChanged(qint64 userId, bool online);
    void markUnread(qint64 peerId, bool unread);
    void clearUnread(qint64 peerId);

signals:
    // peerId == 0 — broadcast, иначе — приватный
    void chatSelected(qint64 peerId, const QString& displayLabel);

private slots:
    void onSelectionChanged();
    void onSearchChanged(const QString& text);

    void onCoreUserOnline(qint64 userId);
    void onCoreUserOffline(qint64 userId);
    void onUsersListReceived(const QJsonObject& payload);

private:
    void requestUsers();

    ChatClientCore* _core = nullptr;

    UsersListModel* _model = nullptr;
    QListView*      _list = nullptr;
    QLineEdit*      _search = nullptr;
    QLabel*         _header = nullptr;

    qint64 _currentPeerId = 0;
    quint32 _pendingUsersRequest = 0;
};

} // namespace messenger::client::gui