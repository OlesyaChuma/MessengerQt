#include "UsersModel.h"
#include "Protocol.h"

#include <QBrush>
#include <QColor>
#include <QFont>

namespace messenger::server::gui {

UsersModel::UsersModel(QObject* parent) : QAbstractTableModel(parent) {}

void UsersModel::setDatabase(Database* db) {
    _db = db;
    reload();
}

void UsersModel::reload() {
    if (!_db) return;
    beginResetModel();
    _rows = _db->listAllUsers();
    endResetModel();
}

void UsersModel::setOnlineUsers(const QSet<qint64>& ids) {
    _online = ids;
    if (!_rows.isEmpty()) {
        emit dataChanged(index(0, ColStatus),
                         index(_rows.size() - 1, ColStatus),
                         {Qt::DisplayRole, Qt::DecorationRole, Qt::ForegroundRole});
    }
}

void UsersModel::setUserOnline(qint64 id, bool online) {
    if (online) _online.insert(id);
    else        _online.remove(id);

    const int r = rowOfUser(id);
    if (r >= 0) {
        emit dataChanged(index(r, ColStatus), index(r, ColStatus),
                         {Qt::DisplayRole, Qt::DecorationRole, Qt::ForegroundRole});
    }
}

UserRecord UsersModel::recordAt(int row) const {
    if (row < 0 || row >= _rows.size()) return {};
    return _rows.at(row);
}

int UsersModel::rowOfUser(qint64 userId) const {
    for (int i = 0; i < _rows.size(); ++i) {
        if (_rows[i].id == userId) return i;
    }
    return -1;
}

int UsersModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : _rows.size();
}

int UsersModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : ColCount;
}

QVariant UsersModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= _rows.size()) return {};
    const auto& u = _rows.at(index.row());
    const bool online = _online.contains(u.id);

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case ColId:           return u.id;
            case ColLogin:        return u.login;
            case ColDisplayName:  return u.displayName;
            case ColRole:         return userRoleToString(u.role);
            case ColStatus:       return online ? tr("online") : tr("offline");
            case ColBanned:
                return u.isBanned
                    ? (u.banReason.isEmpty() ? tr("yes")
                                             : tr("yes (%1)").arg(u.banReason))
                    : tr("no");
            case ColLastSeen:
                return u.lastSeen.isValid()
                    ? u.lastSeen.toLocalTime().toString("yyyy-MM-dd HH:mm:ss")
                    : QString("—");
        }
    }

    if (role == Qt::ForegroundRole) {
        if (index.column() == ColStatus) {
            return QBrush(online ? QColor("#27ae60") : QColor("#95a5a6"));
        }
        if (index.column() == ColBanned && u.isBanned) {
            return QBrush(QColor("#c0392b"));
        }
        if (index.column() == ColRole && u.role == UserRole::Admin) {
            return QBrush(QColor("#2980b9"));
        }
    }

    if (role == Qt::FontRole) {
        if (index.column() == ColRole && u.role == UserRole::Admin) {
            QFont f; f.setBold(true); return f;
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == ColId)
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }

    return {};
}

QVariant UsersModel::headerData(int section, Qt::Orientation orientation,
                                int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return {};
    switch (section) {
        case ColId:           return tr("ID");
        case ColLogin:        return tr("Login");
        case ColDisplayName:  return tr("Display name");
        case ColRole:         return tr("Role");
        case ColStatus:       return tr("Status");
        case ColBanned:       return tr("Banned");
        case ColLastSeen:     return tr("Last seen");
    }
    return {};
}

} // namespace messenger::server::gui