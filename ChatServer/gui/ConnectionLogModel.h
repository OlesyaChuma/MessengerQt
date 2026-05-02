#pragma once

#include "Database.h"
#include "Models.h"

#include <QAbstractTableModel>
#include <QList>

namespace messenger::server::gui {

// Категория для фильтра
enum class LogFilterCategory {
    All,
    LoginsOnly,        // login + logout
    SecurityEvents,    // failed_login + register
    Sanctions          // kick + ban + unban
};

// Модель таблицы журнала подключений.
class ConnectionLogModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column {
        ColTime = 0,
        ColEvent,
        ColUser,
        ColIp,
        ColDetails,
        ColCount
    };

    explicit ConnectionLogModel(QObject* parent = nullptr);

    void setDatabase(Database* db);

    void setFilterCategory(LogFilterCategory c);
    void setUserFilter(const QString& loginOrEmpty);

    void reload(int initialLimit = 200);
    void appendLive(const Database::ConnectionLogRow& row);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    bool passesFilter(const Database::ConnectionLogRow& r) const;

    Database* _db = nullptr;
    QList<Database::ConnectionLogRow> _rows;

    LogFilterCategory _category = LogFilterCategory::All;
    QString _userFilter;
};

} // namespace messenger::server::gui