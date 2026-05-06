#pragma once

#include "Models.h"

#include <QAbstractListModel>
#include <QList>
#include <QSet>

namespace messenger::client::gui {

// Модель боковой панели юзеров.
// Первый элемент — всегда виртуальный «# Общий чат» (broadcast).
// Остальные — реальные пользователи, отсортированные:
//   1. Online сверху, offline ниже
//   2. Внутри группы — по displayName.
class UsersListModel : public QAbstractListModel {
    Q_OBJECT
public:
    // Спец-роли для делегата
    enum Role {
        UserIdRole = Qt::UserRole + 1,
        LoginRole,
        DisplayNameRole,
        OnlineRole,
        IsBroadcastRole,
        HasUnreadRole
    };

    explicit UsersListModel(QObject* parent = nullptr);

    void setUsers(const QList<User>& users);
    void setUserOnline(qint64 userId, bool online);
    void setOnlineSet(const QSet<qint64>& ids);

    // Маркер «непрочитано» — например, при новом сообщении в чате,
    // который сейчас не активен.
    void setUnread(qint64 peerId, bool unread); // 0 = broadcast
    void clearUnread(qint64 peerId);            // 0 = broadcast

    // Поиск по подстроке (логин и displayName)
    void setSearchFilter(const QString& filter);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    // Найти строку по userId. -1 = broadcast не считается.
    int rowOfUser(qint64 userId) const;

    // Получить юзера по строке (broadcast вернёт User{} с id=0)
    User userAt(int row) const;
    bool isBroadcastRow(int row) const;

public slots:
    void onLanguageChanged();

private:
    void rebuildVisible();
    void sortUsers();

    QList<User> _all;             // все юзеры (без фильтра)
    QList<User> _visible;         // отфильтрованные/отсортированные
    QSet<qint64> _online;
    QSet<qint64> _unread;         // peerId с непрочитанным
    bool _broadcastUnread = false;
    QString _filter;
};

} // namespace messenger::client::gui
