#include "UsersPanel.h"
#include "UsersListModel.h"
#include "UserItemDelegate.h"
#include "ThemeManager.h"
#include "TranslationManager.h"

#include "ChatClientCore.h"
#include "Models.h"

#include <QListView>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QCoreApplication>

namespace messenger::client::gui {

UsersPanel::UsersPanel(ChatClientCore* core, QWidget* parent)
    : QWidget(parent), _core(core) {
    setMinimumWidth(260);
    setObjectName("usersPanel");
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ----- Header -----
    _header = new QLabel(tr("Conversations"));
    _header->setObjectName("usersPanelHeader");
    _header->setContentsMargins(12, 10, 12, 10);
    layout->addWidget(_header);

    // ----- Search -----
    _search = new QLineEdit;
    _search->setPlaceholderText(tr("Search users..."));
    _search->setClearButtonEnabled(true);
    _search->setContentsMargins(8, 0, 8, 8);
    layout->addWidget(_search);

    // ----- List -----
    _model = new UsersListModel(this);
    _list = new QListView;
    _list->setModel(_model);
    _list->setItemDelegate(new UserItemDelegate(_list));
    _list->setSelectionMode(QAbstractItemView::SingleSelection);
    _list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _list->setMouseTracking(true);
    _list->setUniformItemSizes(true);
    _list->setAlternatingRowColors(false);
    _list->setFrameShape(QFrame::NoFrame);
    layout->addWidget(_list, 1);

    // ----- Connections -----
    connect(_list->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &UsersPanel::onSelectionChanged);
    connect(_search, &QLineEdit::textChanged,
            this, &UsersPanel::onSearchChanged);

    connect(_core, &ChatClientCore::userOnline,
            this, &UsersPanel::onCoreUserOnline);
    connect(_core, &ChatClientCore::userOffline,
            this, &UsersPanel::onCoreUserOffline);
    connect(_core, &ChatClientCore::requestSucceeded,
            this, [this](quint32 rid, const QJsonObject& payload) {
                if (rid == _pendingUsersRequest && payload.contains("users")) {
                    onUsersListReceived(payload);
                }
            });

    // Сразу загружаем
    requestUsers();

    // По умолчанию выбран broadcast
    auto firstIdx = _model->index(0, 0);
    if (firstIdx.isValid()) {
        _list->setCurrentIndex(firstIdx);
    }

    setStyleSheet(R"(
        QListView {
            background-color: transparent;
            outline: none;
            selection-background-color: #5dade2;
            selection-color: white;
        }
    )");

    // Применяет цвет заголовка и переводы в зависимости от темы и языка
    auto applyHeader = [this]() {
        const bool dark = (palette().color(QPalette::Window).lightness() < 128);
        _header->setText(tr("Conversations"));
        _header->setStyleSheet(
            QString("font-size: 14px; font-weight: 600; color: %1;")
                .arg(dark ? "#d4d8de" : "#2c3e50"));
        _search->setPlaceholderText(tr("Search users..."));
    };
    applyHeader();

    // При смене темы — пересчитываем цвет
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [applyHeader]() { applyHeader(); });

    // При смене языка — обновляем тексты и цвет (через очередь, чтобы tr() уже применился)
    connect(&TranslationManager::instance(),
            &TranslationManager::languageChanged,
            this, [this, applyHeader]() {
                QMetaObject::invokeMethod(this, [this, applyHeader]() {
                    applyHeader();
                    _model->onLanguageChanged();
                }, Qt::QueuedConnection);
            });
}

void UsersPanel::requestUsers() {
    if (!_core->isAuthenticated()) return;
    _pendingUsersRequest = _core->requestUsers();
}

void UsersPanel::reloadUsers() {
    requestUsers();
}

void UsersPanel::onUsersListReceived(const QJsonObject& payload) {
    QList<User> users;
    QSet<qint64> onlineIds;
    const auto arr = payload.value("users").toArray();
    for (const auto& v : arr) {
        const auto u = userFromJson(v.toObject());
        if (u.id == _core->currentUserId()) continue;
        users.append(u);
        if (u.isOnline) onlineIds.insert(u.id);
    }
    _model->setUsers(users);
    _model->setOnlineSet(onlineIds);

    // Восстанавливаем выбор:
    if (_currentPeerId == 0) {
        auto idx = _model->index(0, 0);
        if (idx.isValid()) _list->setCurrentIndex(idx);
    } else {
        const int row = _model->rowOfUser(_currentPeerId);
        if (row >= 0) _list->setCurrentIndex(_model->index(row, 0));
    }
    _pendingUsersRequest = 0;
}

void UsersPanel::onSelectionChanged() {
    const auto idx = _list->currentIndex();
    if (!idx.isValid()) return;

    const bool isBroadcast = idx.data(UsersListModel::IsBroadcastRole).toBool();
    if (isBroadcast) {
        _currentPeerId = 0;
        emit chatSelected(0, tr("# General chat"));
        _model->clearUnread(0);
        return;
    }

    const qint64 peerId = idx.data(UsersListModel::UserIdRole).toLongLong();
    const QString name  = idx.data(UsersListModel::DisplayNameRole).toString();
    _currentPeerId = peerId;
    emit chatSelected(peerId, name);
    _model->clearUnread(peerId);
}

void UsersPanel::onSearchChanged(const QString& text) {
    _model->setSearchFilter(text);
}

void UsersPanel::onCoreUserOnline(qint64 userId) {
    _model->setUserOnline(userId, true);
}

void UsersPanel::onCoreUserOffline(qint64 userId) {
    _model->setUserOnline(userId, false);
}

void UsersPanel::onUserOnlineChanged(qint64 userId, bool online) {
    _model->setUserOnline(userId, online);
}

void UsersPanel::markUnread(qint64 peerId, bool unread) {
    _model->setUnread(peerId, unread);
}

void UsersPanel::clearUnread(qint64 peerId) {
    _model->clearUnread(peerId);
}

} // namespace messenger::client::gui
