#include "ConnectionLogModel.h"

#include <QBrush>
#include <QColor>
#include <QFont>

namespace messenger::server::gui {

namespace {

QString eventToText(ConnectionEvent e) {
    switch (e) {
        case ConnectionEvent::Login:       return "LOGIN";
        case ConnectionEvent::Logout:      return "LOGOUT";
        case ConnectionEvent::Kick:        return "KICK";
        case ConnectionEvent::Ban:         return "BAN";
        case ConnectionEvent::Unban:       return "UNBAN";
        case ConnectionEvent::FailedLogin: return "FAILED_LOGIN";
        case ConnectionEvent::Register:    return "REGISTER";
    }
    return "?";
}

QColor eventColor(ConnectionEvent e) {
    switch (e) {
        case ConnectionEvent::Login:       return QColor("#27ae60"); // green
        case ConnectionEvent::Logout:      return QColor("#7f8c8d"); // grey
        case ConnectionEvent::Kick:        return QColor("#e67e22"); // orange
        case ConnectionEvent::Ban:         return QColor("#c0392b"); // red
        case ConnectionEvent::Unban:       return QColor("#16a085"); // teal
        case ConnectionEvent::FailedLogin: return QColor("#d4ac0d"); // yellow
        case ConnectionEvent::Register:    return QColor("#2980b9"); // blue
    }
    return QColor();
}

} // namespace

ConnectionLogModel::ConnectionLogModel(QObject* parent)
    : QAbstractTableModel(parent) {}

void ConnectionLogModel::setDatabase(Database* db) {
    _db = db;
}

void ConnectionLogModel::setFilterCategory(LogFilterCategory c) {
    _category = c;
}

void ConnectionLogModel::setUserFilter(const QString& login) {
    _userFilter = login.trimmed();
}

bool ConnectionLogModel::passesFilter(const Database::ConnectionLogRow& r) const {
    switch (_category) {
        case LogFilterCategory::All:
            break;
        case LogFilterCategory::LoginsOnly:
            if (r.event != ConnectionEvent::Login &&
                r.event != ConnectionEvent::Logout) return false;
            break;
        case LogFilterCategory::SecurityEvents:
            if (r.event != ConnectionEvent::FailedLogin &&
                r.event != ConnectionEvent::Register) return false;
            break;
        case LogFilterCategory::Sanctions:
            if (r.event != ConnectionEvent::Kick &&
                r.event != ConnectionEvent::Ban &&
                r.event != ConnectionEvent::Unban) return false;
            break;
    }
    if (!_userFilter.isEmpty() &&
        r.loginCache.compare(_userFilter, Qt::CaseInsensitive) != 0) {
        return false;
    }
    return true;
}

void ConnectionLogModel::reload(int initialLimit) {
    if (!_db) return;
    beginResetModel();
    _rows.clear();
    // Берём с запасом, потом фильтруем — простая стратегия
    auto raw = _db->loadConnectionLog(initialLimit * 2);
    for (const auto& r : raw) {
        if (passesFilter(r)) _rows.append(r);
        if (_rows.size() >= initialLimit) break;
    }
    endResetModel();
}

void ConnectionLogModel::appendLive(const Database::ConnectionLogRow& row) {
    if (!passesFilter(row)) return;
    // Журнал отображается DESC (новые сверху) — добавляем в начало
    beginInsertRows({}, 0, 0);
    _rows.prepend(row);
    endInsertRows();
}

int ConnectionLogModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : _rows.size();
}

int ConnectionLogModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : ColCount;
}

QVariant ConnectionLogModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= _rows.size()) return {};
    const auto& r = _rows.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case ColTime:
                return r.occurredAt.toLocalTime().toString("yyyy-MM-dd HH:mm:ss");
            case ColEvent:
                return eventToText(r.event);
            case ColUser:
                return r.loginCache.isEmpty() ? QString("—") : r.loginCache;
            case ColIp:
                return r.ipAddress.isEmpty() ? QString("—") : r.ipAddress;
            case ColDetails:
                return r.details;
        }
    }

    if (role == Qt::ForegroundRole) {
        if (index.column() == ColEvent) {
            return QBrush(eventColor(r.event));
        }
    }

    if (role == Qt::FontRole) {
        if (index.column() == ColEvent) {
            QFont f; f.setBold(true); return f;
        }
        if (index.column() == ColTime) {
            QFont f; f.setFamily("Consolas"); return f;
        }
    }

    return {};
}

QVariant ConnectionLogModel::headerData(int section, Qt::Orientation orientation,
                                        int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return {};
    switch (section) {
        case ColTime:    return tr("Time");
        case ColEvent:   return tr("Event");
        case ColUser:    return tr("User");
        case ColIp:      return tr("IP address");
        case ColDetails: return tr("Details");
    }
    return {};
}

} // namespace messenger::server::gui