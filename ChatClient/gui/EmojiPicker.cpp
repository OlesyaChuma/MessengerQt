#include "EmojiPicker.h"
#include "EmojiData.h"

#include <QTabWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPixmap>
#include <QIcon>

namespace messenger::client::gui {

EmojiPicker::EmojiPicker(QWidget* parent)
    : QFrame(parent, Qt::Popup) {
    setObjectName("emojiPicker");
    setFrameShape(QFrame::Panel);
    setFrameShadow(QFrame::Raised);
    setFixedSize(360, 160);

    setStyleSheet(R"(
        QFrame#emojiPicker {
            background-color: palette(window);
            border: 1px solid palette(mid);
            border-radius: 8px;
        }
        QPushButton#emojiButton {
            border: none;
            background: transparent;
            padding: 2px;
            border-radius: 4px;
        }
        QPushButton#emojiButton:hover {
            background-color: palette(highlight);
        }
        QTabWidget::pane {
            border: none;
        }
        QTabBar::tab {
            padding: 4px 10px;
            font-size: 12px;
        }
    )");

    buildUi();
}

void EmojiPicker::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(6, 6, 6, 6);
    root->setSpacing(4);

    _tabs = new QTabWidget;
    _tabs->setTabPosition(QTabWidget::North);

    for (const auto& cat : emojiCategories()) {
        auto* page = new QWidget;
        auto* grid = new QGridLayout(page);
        grid->setSpacing(2);
        grid->setContentsMargins(4, 4, 4, 4);

        int row = 0, col = 0;
        const int cols = 10;
        for (const auto& info : cat.emojis) {
            auto* btn = new QPushButton;
            btn->setObjectName("emojiButton");
            btn->setFixedSize(30, 30);
            btn->setIcon(QIcon(QString(":/client/emoji/%1.png").arg(info.code)));
            btn->setIconSize(QSize(22, 22));
            btn->setToolTip(info.code);

            connect(btn, &QPushButton::clicked, this, [this, u = info.unicode]() {
                emit emojiSelected(u);
                close();
            });

            grid->addWidget(btn, row, col);
            if (++col >= cols) { col = 0; ++row; }
        }
        // Заполнение пустых ячеек, чтобы сетка не растягивала кнопки
        grid->setRowStretch(row + 1, 1);

        _tabs->addTab(page, cat.title);
    }

    root->addWidget(_tabs, 1);
}

} // namespace messenger::client::gui
