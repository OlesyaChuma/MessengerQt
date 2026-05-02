#include "UsersTab.h"
#include "UsersModel.h"
#include "BanDialog.h"

#include "Database.h"
#include "ChatServer.h"

#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

namespace messenger::server::gui {

UsersTab::UsersTab(Database* db, ChatServer* server, qint64 adminId,
                   QWidget* parent)
    : QWidget(parent), _db(db), _server(server), _adminId(adminId) {

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    // ----- Toolbar -----
    auto* toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);

    _refreshBtn = new QPushButton(tr("Refresh"));
    _kickBtn    = new QPushButton(tr("Kick"));
    _banBtn     = new QPushButton(tr("Ban"));
    _unbanBtn   = new QPushButton(tr("Unban"));

    toolbar->addWidget(_refreshBtn);
    toolbar->addStretch(1);
    toolbar->addWidget(_kickBtn);
    toolbar->addWidget(_banBtn);
    toolbar->addWidget(_unbanBtn);

    root->addLayout(toolbar);

    // ----- Table -----
    _model = new UsersModel(this);
    _model->setDatabase(_db);

    _table = new QTableView;
    _table->setModel(_model);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setSelectionMode(QAbstractItemView::SingleSelection);
    _table->setAlternatingRowColors(true);
    _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _table->setSortingEnabled(true);
    _table->verticalHeader()->setVisible(false);
    _table->horizontalHeader()->setStretchLastSection(true);
    _table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _table->horizontalHeader()->setSectionResizeMode(
        UsersModel::ColBanned, QHeaderView::Stretch);

    root->addWidget(_table, 1);

    // ----- Summary footer -----
    _summary = new QLabel;
    _summary->setStyleSheet("color: #7f8c8d; padding: 4px 0;");
    root->addWidget(_summary);

    // ----- Signals -----
    connect(_refreshBtn, &QPushButton::clicked, this, &UsersTab::onRefreshClicked);
    connect(_kickBtn,    &QPushButton::clicked, this, &UsersTab::onKickClicked);
    connect(_banBtn,     &QPushButton::clicked, this, &UsersTab::onBanClicked);
    connect(_unbanBtn,   &QPushButton::clicked, this, &UsersTab::onUnbanClicked);

    connect(_table->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &UsersTab::onSelectionChanged);

    // Server live updates
    connect(_server, &ChatServer::clientAuthenticated,
            this, &UsersTab::onUserAuthenticated);
    connect(_server, &ChatServer::clientDisconnected,
            this, &UsersTab::onUserDisconnected);

    // Заполняем стартовое состояние
    QSet<qint64> online;
    for (auto id : _server->onlineUserIds()) online.insert(id);
    _model->setOnlineUsers(online);

    onSelectionChanged();
    reload();
}

void UsersTab::reload() {
    _model->reload();

    QSet<qint64> online;
    for (auto id : _server->onlineUserIds()) online.insert(id);
    _model->setOnlineUsers(online);

    refreshSummary();
    onSelectionChanged();
}

void UsersTab::refreshSummary() {
    int total = _model->rowCount();
    int onlineCount = 0;
    int bannedCount = 0;
    for (int i = 0; i < total; ++i) {
        const auto rec = _model->recordAt(i);
        if (_server->isUserOnline(rec.id)) ++onlineCount;
        if (rec.isBanned) ++bannedCount;
    }
    _summary->setText(tr("Total users: %1   |   Online: %2   |   Banned: %3")
                          .arg(total).arg(onlineCount).arg(bannedCount));
}

void UsersTab::onUserAuthenticated(qint64 userId, const QString& /*login*/) {
    // Если новый юзер только что зарегистрировался и сразу залогинился —
    // его строки в таблице ещё нет. Перезагружаем целиком.
    if (_model->rowOfUser(userId) < 0) {
        _model->reload();
    }
    _model->setUserOnline(userId, true);
    refreshSummary();
    onSelectionChanged();
}

void UsersTab::onUserDisconnected(qint64 userId, const QString& /*login*/) {
    _model->setUserOnline(userId, false);
    refreshSummary();
    onSelectionChanged();
}

qint64 UsersTab::selectedUserId() const {
    const auto idx = _table->selectionModel()->currentIndex();
    if (!idx.isValid()) return 0;
    return _model->recordAt(idx.row()).id;
}

QString UsersTab::selectedUserLogin() const {
    const auto idx = _table->selectionModel()->currentIndex();
    if (!idx.isValid()) return {};
    return _model->recordAt(idx.row()).login;
}

bool UsersTab::isSelectedUserBanned() const {
    const auto idx = _table->selectionModel()->currentIndex();
    if (!idx.isValid()) return false;
    return _model->recordAt(idx.row()).isBanned;
}

bool UsersTab::isSelectedUserOnline() const {
    const qint64 id = selectedUserId();
    if (id == 0) return false;
    return _server->isUserOnline(id);
}

void UsersTab::onSelectionChanged() {
    const qint64 id = selectedUserId();
    const bool any = (id != 0);
    const bool isAdminSelf = (id == _adminId);

    _kickBtn->setEnabled(any && !isAdminSelf && isSelectedUserOnline());
    _banBtn->setEnabled(any && !isAdminSelf && !isSelectedUserBanned());
    _unbanBtn->setEnabled(any && !isAdminSelf && isSelectedUserBanned());
}

void UsersTab::onRefreshClicked() {
    reload();
}

void UsersTab::onKickClicked() {
    const qint64 id = selectedUserId();
    if (id == 0) return;
    BanDialog dlg(BanDialog::Kick, selectedUserLogin(), this);
    if (dlg.exec() != QDialog::Accepted) return;
    if (_server->kickUser(id, dlg.reason())) {
        QMessageBox::information(this, tr("Kicked"),
            tr("User '%1' has been disconnected.").arg(selectedUserLogin()));
    } else {
        QMessageBox::warning(this, tr("Kick failed"),
            tr("Cannot kick '%1' (perhaps user is offline?).").arg(selectedUserLogin()));
    }
    reload();
}

void UsersTab::onBanClicked() {
    const qint64 id = selectedUserId();
    if (id == 0) return;
    BanDialog dlg(BanDialog::Ban, selectedUserLogin(), this);
    if (dlg.exec() != QDialog::Accepted) return;
    if (_server->banUser(id, _adminId, dlg.reason())) {
        QMessageBox::information(this, tr("Banned"),
            tr("User '%1' is now blocked.").arg(selectedUserLogin()));
    } else {
        QMessageBox::warning(this, tr("Ban failed"),
            tr("Cannot ban '%1'.").arg(selectedUserLogin()));
    }
    reload();
}

void UsersTab::onUnbanClicked() {
    const qint64 id = selectedUserId();
    if (id == 0) return;
    if (QMessageBox::question(this, tr("Unban user"),
            tr("Remove ban from '%1'?").arg(selectedUserLogin()))
            != QMessageBox::Yes) return;
    if (_server->unbanUser(id)) {
        QMessageBox::information(this, tr("Unbanned"),
            tr("User '%1' is unblocked.").arg(selectedUserLogin()));
    } else {
        QMessageBox::warning(this, tr("Unban failed"),
            tr("Cannot unban '%1'.").arg(selectedUserLogin()));
    }
    reload();
}

} // namespace messenger::server::gui
