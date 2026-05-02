#pragma once

#include "Database.h"

#include <QAbstractTableModel>
#include <QSet>

namespace messenger::server::gui {

// Модель таблицы пользователей.
// Загружает из БД, поддерживает обновление онлайн-статуса по push.
class UsersModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column {
        ColId = 0,
        ColLogin,
        ColDisplayName,
        ColRole,
        ColStatus,
        ColBanned,
        ColLastSeen,
        ColCount
    };

    explicit UsersModel(QObject* parent = nullptr);

    void setDatabase(Database* db);

    void reload();
    void setOnlineUsers(const QSet<qint64>& ids);
    void setUserOnline(qint64 id, bool online);

    // Данные строки по индексу
    UserRecord recordAt(int row) const;
    int rowOfUser(qint64 userId) const;

    // QAbstractTableModel
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    Database* _db = nullptr;
    QList<UserRecord> _rows;
    QSet<qint64>      _online;
};

} // namespace messenger::server::gui