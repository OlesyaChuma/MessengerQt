#pragma once

#include <QWidget>
#include <QTimer>

class QTableView;
class QPushButton;
class QComboBox;
class QLineEdit;
class QLabel;

namespace messenger::server {
class Database;
class ChatServer;
}

namespace messenger::server::gui {

class ConnectionLogModel;

class ConnectionLogTab : public QWidget {
    Q_OBJECT
public:
    ConnectionLogTab(Database* db, ChatServer* server, QWidget* parent = nullptr);

public slots:
    void reload();

private slots:
    void onCategoryChanged();
    void onUserFilterApplied();
    void onRefreshClicked();
    void onAutoRefreshTick();

private:
    Database*   _db = nullptr;
    ChatServer* _server = nullptr;

    ConnectionLogModel* _model = nullptr;
    QTableView*         _table = nullptr;

    QComboBox*   _categoryCombo = nullptr;
    QLineEdit*   _userEdit = nullptr;
    QPushButton* _applyUserBtn = nullptr;
    QPushButton* _refreshBtn = nullptr;
    QLabel*      _summary = nullptr;

    // Журнал в БД пишется не через сигнал, поэтому делаем мягкий polling
    // раз в 3 секунды. Это эффективно для админ-интерфейса.
    QTimer _pollTimer;
    qint64 _lastSeenId = 0;
};

} // namespace messenger::server::gui