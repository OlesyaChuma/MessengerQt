#include "ConnectionLogTab.h"
#include "ConnectionLogModel.h"

#include "Database.h"
#include "ChatServer.h"

#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace messenger::server::gui {

ConnectionLogTab::ConnectionLogTab(Database* db, ChatServer* server,
                                   QWidget* parent)
    : QWidget(parent), _db(db), _server(server) {

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    // ----- Filter toolbar -----
    auto* toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);

    toolbar->addWidget(new QLabel(tr("Category:")));
    _categoryCombo = new QComboBox;
    _categoryCombo->addItem(tr("All events"));
    _categoryCombo->addItem(tr("Logins only"));
    _categoryCombo->addItem(tr("Security events"));
    _categoryCombo->addItem(tr("Sanctions (ban/kick/unban)"));
    toolbar->addWidget(_categoryCombo);

    toolbar->addSpacing(12);

    toolbar->addWidget(new QLabel(tr("User:")));
    _userEdit = new QLineEdit;
    _userEdit->setPlaceholderText(tr("login or empty"));
    _userEdit->setMaximumWidth(140);
    toolbar->addWidget(_userEdit);

    _applyUserBtn = new QPushButton(tr("Apply"));
    toolbar->addWidget(_applyUserBtn);

    toolbar->addStretch(1);

    _refreshBtn = new QPushButton(tr("Refresh"));
    toolbar->addWidget(_refreshBtn);

    root->addLayout(toolbar);

    // ----- Table -----
    _model = new ConnectionLogModel(this);
    _model->setDatabase(_db);

    _table = new QTableView;
    _table->setModel(_model);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setSelectionMode(QAbstractItemView::SingleSelection);
    _table->setAlternatingRowColors(true);
    _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _table->verticalHeader()->setVisible(false);
    _table->horizontalHeader()->setStretchLastSection(true);
    _table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    root->addWidget(_table, 1);

    _summary = new QLabel;
    _summary->setStyleSheet("color: #7f8c8d; padding: 4px 0;");
    root->addWidget(_summary);

    // ----- Signals -----
    connect(_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConnectionLogTab::onCategoryChanged);
    connect(_applyUserBtn, &QPushButton::clicked,
            this, &ConnectionLogTab::onUserFilterApplied);
    connect(_userEdit, &QLineEdit::returnPressed,
            this, &ConnectionLogTab::onUserFilterApplied);
    connect(_refreshBtn, &QPushButton::clicked,
            this, &ConnectionLogTab::onRefreshClicked);

    // Polling каждые 3 секунды для live-обновления журнала
    _pollTimer.setInterval(3000);
    connect(&_pollTimer, &QTimer::timeout,
            this, &ConnectionLogTab::onAutoRefreshTick);
    _pollTimer.start();

    reload();
}

void ConnectionLogTab::reload() {
    const int idx = _categoryCombo->currentIndex();
    LogFilterCategory cat = LogFilterCategory::All;
    if (idx == 1) cat = LogFilterCategory::LoginsOnly;
    if (idx == 2) cat = LogFilterCategory::SecurityEvents;
    if (idx == 3) cat = LogFilterCategory::Sanctions;
    _model->setFilterCategory(cat);
    _model->setUserFilter(_userEdit->text());
    _model->reload(200);

    _summary->setText(tr("Loaded %1 events").arg(_model->rowCount()));

    // Обновляем границу для polling
    auto recent = _db->loadConnectionLog(1);
    _lastSeenId = recent.isEmpty() ? 0 : recent.first().id;
}

void ConnectionLogTab::onCategoryChanged() {
    reload();
}

void ConnectionLogTab::onUserFilterApplied() {
    reload();
}

void ConnectionLogTab::onRefreshClicked() {
    reload();
}

void ConnectionLogTab::onAutoRefreshTick() {
    if (!_db) return;
    // Берём свежие записи и добавляем те, что новее _lastSeenId
    auto recent = _db->loadConnectionLog(50);
    if (recent.isEmpty()) return;

    QList<Database::ConnectionLogRow> newer;
    for (const auto& r : recent) {
        if (r.id > _lastSeenId) newer.append(r);
        else break;
    }
    if (newer.isEmpty()) return;

    // newer пока в DESC (от самой новой к старой) — переворачиваем,
    // чтобы добавлять в естественном порядке
    std::reverse(newer.begin(), newer.end());
    for (const auto& r : newer) {
        _model->appendLive(r);
    }
    _lastSeenId = recent.first().id;
    _summary->setText(tr("Loaded %1 events (live)").arg(_model->rowCount()));
}

} // namespace messenger::server::gui