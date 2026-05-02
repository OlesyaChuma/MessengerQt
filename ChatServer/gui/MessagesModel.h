#pragma once

#include "Models.h"

#include <QAbstractTableModel>
#include <QList>

namespace messenger::server {
class Database;
}

namespace messenger::server::gui {

// Модель ленты сообщений для админ-просмотра.
// Поддерживает фильтры и пагинацию (load older).
class MessagesModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column {
        ColTime = 0,
        ColFrom,
        ColTo,
        ColBody,
        ColFlags,    // edited / deleted
        ColCount
    };

    explicit MessagesModel(QObject* parent = nullptr);

    void setDatabase(Database* db);

    // Фильтры
    void setFilterMode(bool onlyBroadcast, bool onlyPrivate);
    void setSenderFilter(qint64 senderId);

    // Перезагрузка с учётом фильтров
    void reload(int initialLimit = 100);

    // Постраничная подгрузка более старых
    int loadOlder(int extra = 100);

    // Live-добавление
    void appendLive(const Message& m);
    void updateMessage(qint64 id, const QString& newBody, const QDateTime& editedAt);
    void markDeleted(qint64 id, const QDateTime& deletedAt);

    Message messageAt(int row) const;

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    int rowOfMessage(qint64 id) const;

    Database* _db = nullptr;
    QList<Message> _rows;

    bool _onlyBroadcast = false;
    bool _onlyPrivate = false;
    qint64 _senderFilter = 0;
};

} // namespace messenger::server::gui