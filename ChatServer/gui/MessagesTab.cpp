#include "MessagesTab.h"
#include "MessagesModel.h"

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
#include <QScrollBar>

namespace messenger::server::gui {

MessagesTab::MessagesTab(Database* db, ChatServer* server, QWidget* parent)
    : QWidget(parent), _db(db), _server(server) {

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    // ----- Filter toolbar -----
    auto* toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);

    toolbar->addWidget(new QLabel(tr("Type:")));
    _typeCombo = new QComboBox;
    _typeCombo->addItem(tr("All"));
    _typeCombo->addItem(tr("Broadcast only"));
    _typeCombo->addItem(tr("Private only"));
    toolbar->addWidget(_typeCombo);

    toolbar->addSpacing(12);

    toolbar->addWidget(new QLabel(tr("Sender id:")));
    _senderEdit = new QLineEdit;
    _senderEdit->setPlaceholderText(tr("0 = all"));
    _senderEdit->setMaximumWidth(80);
    toolbar->addWidget(_senderEdit);

    _applySenderBtn = new QPushButton(tr("Apply"));
    toolbar->addWidget(_applySenderBtn);

    toolbar->addStretch(1);

    _loadOlderBtn = new QPushButton(tr("Load older"));
    _refreshBtn   = new QPushButton(tr("Refresh"));
    toolbar->addWidget(_loadOlderBtn);
    toolbar->addWidget(_refreshBtn);

    root->addLayout(toolbar);

    // ----- Table -----
    _model = new MessagesModel(this);
    _model->setDatabase(_db);

    _table = new QTableView;
    _table->setModel(_model);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setSelectionMode(QAbstractItemView::SingleSelection);
    _table->setAlternatingRowColors(true);
    _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _table->verticalHeader()->setVisible(false);
    _table->horizontalHeader()->setStretchLastSection(false);
    _table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _table->horizontalHeader()->setSectionResizeMode(
        MessagesModel::ColBody, QHeaderView::Stretch);

    root->addWidget(_table, 1);

    _summary = new QLabel;
    _summary->setStyleSheet("color: #7f8c8d; padding: 4px 0;");
    root->addWidget(_summary);

    // ----- Signals -----
    connect(_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MessagesTab::onFilterChanged);
    connect(_applySenderBtn, &QPushButton::clicked,
            this, &MessagesTab::onSenderFilterApplied);
    connect(_senderEdit, &QLineEdit::returnPressed,
            this, &MessagesTab::onSenderFilterApplied);
    connect(_refreshBtn, &QPushButton::clicked,
            this, &MessagesTab::onRefreshClicked);
    connect(_loadOlderBtn, &QPushButton::clicked,
            this, &MessagesTab::onLoadOlderClicked);

    // Live updates от сервера
    connect(_server, &ChatServer::newMessageDispatched,
            this, &MessagesTab::onNewMessage);

    reload();
}

void MessagesTab::applyFilters() {
    const int idx = _typeCombo->currentIndex();
    _model->setFilterMode(idx == 1, idx == 2);

    const qint64 sender = _senderEdit->text().toLongLong();
    _model->setSenderFilter(sender);
}

void MessagesTab::reload() {
    applyFilters();
    _model->reload(100);
    _summary->setText(tr("Loaded %1 messages").arg(_model->rowCount()));
    // Прокрутка к концу — самые свежие
    _table->scrollToBottom();
}

void MessagesTab::onFilterChanged() {
    reload();
}

void MessagesTab::onLoadOlderClicked() {
    const int loaded = _model->loadOlder(100);
    if (loaded == 0) {
        _summary->setText(tr("No more older messages. Total: %1")
                          .arg(_model->rowCount()));
    } else {
        _summary->setText(tr("Loaded %1 more (total: %2)")
                          .arg(loaded).arg(_model->rowCount()));
    }
}

void MessagesTab::onRefreshClicked() {
    reload();
}

void MessagesTab::onSenderFilterApplied() {
    reload();
}

void MessagesTab::onNewMessage(const Message& m) {
    // Проверим, надо ли скроллить — если пользователь смотрит самый низ
    const auto* sb = _table->verticalScrollBar();
    const bool wasAtBottom = sb && (sb->value() >= sb->maximum() - 4);

    _model->appendLive(m);

    if (wasAtBottom) {
        _table->scrollToBottom();
    }
    _summary->setText(tr("Loaded %1 messages (last: live update)")
                      .arg(_model->rowCount()));
}

} // namespace messenger::server::gui