#include "UsersListModel.h"

#include <algorithm>

namespace messenger::client::gui {

UsersListModel::UsersListModel(QObject* parent)
    : QAbstractListModel(parent) {}

void UsersListModel::setUsers(const QList<User>& users) {
    beginResetModel();
    _all = users;
    sortUsers();
    rebuildVisible();
    endResetModel();
}

void UsersListModel::setUserOnline(qint64 userId, bool online) {
    if (online) _online.insert(userId);
    else        _online.remove(userId);
    // Полная пересортировка нужна, потому что online меняет позицию
    beginResetModel();
    sortUsers();
    rebuildVisible();
    endResetModel();
}

void UsersListModel::setOnlineSet(const QSet<qint64>& ids) {
    _online = ids;
    beginResetModel();
    sortUsers();
    rebuildVisible();
    endResetModel();
}

void UsersListModel::setUnread(qint64 peerId, bool unread) {
    if (peerId == 0) {
        if (_broadcastUnread == unread) return;
        _broadcastUnread = unread;
    } else {
        if (unread) _unread.insert(peerId);
        else        _unread.remove(peerId);
    }
    // Перерисовываем только индикатор непрочитанного
    if (_visible.isEmpty()) return;
    emit dataChanged(index(0), index(_visible.size()), {HasUnreadRole});
}

void UsersListModel::clearUnread(qint64 peerId) {
    setUnread(peerId, false);
}

void UsersListModel::setSearchFilter(const QString& filter) {
    if (_filter == filter) return;
    _filter = filter;
    beginResetModel();
    rebuildVisible();
    endResetModel();
}

void UsersListModel::sortUsers() {
    auto isOnline = [this](const User& u) { return _online.contains(u.id); };
    std::sort(_all.begin(), _all.end(), [&](const User& a, const User& b) {
        const bool aOn = isOnline(a);
        const bool bOn = isOnline(b);
        if (aOn != bOn) return aOn;  // online сначала
        // Внутри группы — по displayName, регистронезависимо
        const QString an = a.displayName.isEmpty() ? a.login : a.displayName;
        const QString bn = b.displayName.isEmpty() ? b.login : b.displayName;
        return an.compare(bn, Qt::CaseInsensitive) < 0;
    });
}

void UsersListModel::rebuildVisible() {
    _visible.clear();
    if (_filter.isEmpty()) {
        _visible = _all;
        return;
    }
    for (const auto& u : _all) {
        if (u.login.contains(_filter, Qt::CaseInsensitive) ||
            u.displayName.contains(_filter, Qt::CaseInsensitive)) {
            _visible.append(u);
        }
    }
}

int UsersListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    // +1 для строки «# Общий чат» (только если без фильтра, либо если он ему соответствует)
    const bool broadcastVisible = _filter.isEmpty() ||
        QString("общий чат").contains(_filter, Qt::CaseInsensitive) ||
        QString("general chat").contains(_filter, Qt::CaseInsensitive) ||
        QString("broadcast").contains(_filter, Qt::CaseInsensitive);
    return _visible.size() + (broadcastVisible ? 1 : 0);
}

bool UsersListModel::isBroadcastRow(int row) const {
    if (row != 0) return false;
    const bool broadcastVisible = _filter.isEmpty() ||
        QString("общий чат").contains(_filter, Qt::CaseInsensitive) ||
        QString("general chat").contains(_filter, Qt::CaseInsensitive) ||
        QString("broadcast").contains(_filter, Qt::CaseInsensitive);
    return broadcastVisible;
}

User UsersListModel::userAt(int row) const {
    if (isBroadcastRow(row)) return User{}; // id=0
    int realIndex = isBroadcastRow(0) ? row - 1 : row;
    if (realIndex < 0 || realIndex >= _visible.size()) return User{};
    return _visible.at(realIndex);
}

int UsersListModel::rowOfUser(qint64 userId) const {
    if (userId == 0) return isBroadcastRow(0) ? 0 : -1;
    for (int i = 0; i < _visible.size(); ++i) {
        if (_visible[i].id == userId) {
            return isBroadcastRow(0) ? i + 1 : i;
        }
    }
    return -1;
}

QVariant UsersListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const int row = index.row();

    if (isBroadcastRow(row)) {
        switch (role) {
            case Qt::DisplayRole:
            case DisplayNameRole:
                return tr("# General chat");
            case LoginRole:
                return QString("__broadcast__");
            case UserIdRole:
                return qint64(0);
            case OnlineRole:
                return false;
            case IsBroadcastRole:
                return true;
            case HasUnreadRole:
                return _broadcastUnread;
        }
        return {};
    }

    const auto u = userAt(row);
    if (u.id == 0) return {};

    switch (role) {
        case Qt::DisplayRole:
        case DisplayNameRole:
            return u.displayName.isEmpty() ? u.login : u.displayName;
        case LoginRole:
            return u.login;
        case UserIdRole:
            return u.id;
        case OnlineRole:
            return _online.contains(u.id);
        case IsBroadcastRole:
            return false;
        case HasUnreadRole:
            return _unread.contains(u.id);
    }
    return {};
}

} // namespace messenger::client::gui