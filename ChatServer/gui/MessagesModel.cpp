#include "MessagesModel.h"
#include "Database.h"

#include <QBrush>
#include <QColor>
#include <QFont>

namespace messenger::server::gui {

MessagesModel::MessagesModel(QObject* parent) : QAbstractTableModel(parent) {}

void MessagesModel::setDatabase(Database* db) {
    _db = db;
}

void MessagesModel::setFilterMode(bool onlyBroadcast, bool onlyPrivate) {
    _onlyBroadcast = onlyBroadcast;
    _onlyPrivate = onlyPrivate;
}

void MessagesModel::setSenderFilter(qint64 senderId) {
    _senderFilter = senderId;
}

void MessagesModel::reload(int initialLimit) {
    if (!_db) return;
    beginResetModel();
    _rows = _db->loadAllMessages(_senderFilter, _onlyBroadcast, _onlyPrivate,
                                 0, initialLimit);
    endResetModel();
}

int MessagesModel::loadOlder(int extra) {
    if (!_db || _rows.isEmpty()) return 0;
    const qint64 oldestId = _rows.first().id;
    auto more = _db->loadAllMessages(_senderFilter, _onlyBroadcast, _onlyPrivate,
                                     oldestId, extra);
    if (more.isEmpty()) return 0;

    beginInsertRows({}, 0, more.size() - 1);
    QList<Message> combined = more;
    combined.append(_rows);
    _rows = combined;
    endInsertRows();
    return more.size();
}

int MessagesModel::rowOfMessage(qint64 id) const {
    for (int i = 0; i < _rows.size(); ++i) {
        if (_rows[i].id == id) return i;
    }
    return -1;
}

void MessagesModel::appendLive(const Message& m) {
    // Применяем фильтры
    if (_onlyBroadcast && !m.isBroadcast) return;
    if (_onlyPrivate   && m.isBroadcast)  return;
    if (_senderFilter > 0 && m.senderId != _senderFilter) return;

    // Защита от дублирования (если сообщение пришло и в reload, и через push)
    if (rowOfMessage(m.id) >= 0) return;

    beginInsertRows({}, _rows.size(), _rows.size());
    _rows.append(m);
    endInsertRows();
}

void MessagesModel::updateMessage(qint64 id, const QString& newBody,
                                  const QDateTime& editedAt) {
    int r = rowOfMessage(id);
    if (r < 0) return;
    _rows[r].body = newBody;
    _rows[r].editedAt = editedAt;
    emit dataChanged(index(r, 0), index(r, ColCount - 1));
}

void MessagesModel::markDeleted(qint64 id, const QDateTime& deletedAt) {
    int r = rowOfMessage(id);
    if (r < 0) return;
    _rows[r].deletedAt = deletedAt;
    emit dataChanged(index(r, 0), index(r, ColCount - 1));
}

Message MessagesModel::messageAt(int row) const {
    if (row < 0 || row >= _rows.size()) return {};
    return _rows.at(row);
}

int MessagesModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : _rows.size();
}

int MessagesModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : ColCount;
}

QVariant MessagesModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= _rows.size()) return {};
    const auto& m = _rows.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case ColTime:
                return m.createdAt.toLocalTime().toString("yyyy-MM-dd HH:mm:ss");
            case ColFrom:
                return m.senderLogin.isEmpty()
                    ? QString::number(m.senderId) : m.senderLogin;
            case ColTo:
                return m.isBroadcast ? tr("(broadcast)")
                                     : QString("→ id %1").arg(m.receiverId);
            case ColBody:
                return m.body;
            case ColFlags: {
                QStringList flags;
                if (m.editedAt.isValid())  flags << tr("edited");
                if (m.deletedAt.isValid()) flags << tr("deleted");
                return flags.join(", ");
            }
        }
    }

    if (role == Qt::ForegroundRole) {
        if (m.deletedAt.isValid()) return QBrush(QColor("#95a5a6"));
        if (index.column() == ColFlags && m.editedAt.isValid())
            return QBrush(QColor("#2980b9"));
        if (index.column() == ColFlags && m.deletedAt.isValid())
            return QBrush(QColor("#c0392b"));
        if (index.column() == ColTo && m.isBroadcast)
            return QBrush(QColor("#27ae60"));
    }

    if (role == Qt::FontRole) {
        QFont f;
        if (m.deletedAt.isValid()) f.setStrikeOut(true);
        else if (m.editedAt.isValid()) f.setItalic(true);
        return f;
    }

    if (role == Qt::ToolTipRole && !m.body.isEmpty()) {
        return m.body;
    }

    return {};
}

QVariant MessagesModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return {};
    switch (section) {
        case ColTime:  return tr("Time");
        case ColFrom:  return tr("From");
        case ColTo:    return tr("To / Type");
        case ColBody:  return tr("Body");
        case ColFlags: return tr("Flags");
    }
    return {};
}

} // namespace messenger::server::gui