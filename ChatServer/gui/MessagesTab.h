#pragma once

#include "Models.h"

#include <QWidget>

class QTableView;
class QPushButton;
class QComboBox;
class QLabel;
class QLineEdit;

namespace messenger::server {
class Database;
class ChatServer;
}

namespace messenger::server::gui {

class MessagesModel;

class MessagesTab : public QWidget {
    Q_OBJECT
public:
    MessagesTab(Database* db, ChatServer* server, QWidget* parent = nullptr);

public slots:
    void reload();

private slots:
    void onFilterChanged();
    void onLoadOlderClicked();
    void onRefreshClicked();
    void onSenderFilterApplied();

    // Live updates
    void onNewMessage(const Message& m);

private:
    void applyFilters();

    Database*   _db = nullptr;
    ChatServer* _server = nullptr;

    MessagesModel* _model = nullptr;
    QTableView*    _table = nullptr;

    QComboBox*  _typeCombo = nullptr;
    QLineEdit*  _senderEdit = nullptr;
    QPushButton* _applySenderBtn = nullptr;
    QPushButton* _refreshBtn = nullptr;
    QPushButton* _loadOlderBtn = nullptr;
    QLabel*      _summary = nullptr;
};

} // namespace messenger::server::gui