#pragma once

#include <QWidget>

class QTableView;
class QPushButton;
class QLabel;

namespace messenger::server {
class Database;
class ChatServer;
}

namespace messenger::server::gui {

class UsersModel;

class UsersTab : public QWidget {
    Q_OBJECT
public:
    UsersTab(Database* db, ChatServer* server, qint64 adminId,
             QWidget* parent = nullptr);

public slots:
    void reload();
    void onUserAuthenticated(qint64 userId, const QString& login);
    void onUserDisconnected(qint64 userId, const QString& login);

private slots:
    void onSelectionChanged();
    void onKickClicked();
    void onBanClicked();
    void onUnbanClicked();
    void onRefreshClicked();

private:
    void refreshSummary();
    qint64 selectedUserId() const;
    QString selectedUserLogin() const;
    bool isSelectedUserBanned() const;
    bool isSelectedUserOnline() const;

    Database*   _db = nullptr;
    ChatServer* _server = nullptr;
    qint64      _adminId = 0;

    UsersModel* _model = nullptr;
    QTableView* _table = nullptr;

    QPushButton* _kickBtn = nullptr;
    QPushButton* _banBtn = nullptr;
    QPushButton* _unbanBtn = nullptr;
    QPushButton* _refreshBtn = nullptr;
    QLabel*      _summary = nullptr;
};

} // namespace messenger::server::gui
