#pragma once

#include "ChatClientCore.h"

#include <QMainWindow>

class QSplitter;
class QLabel;
class QAction;

namespace messenger::client::gui {

class UsersPanel;
class ChatPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(ChatClientCore* core, QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onThemeToggled();
    void onSelectLanguage();
    void onLanguageChanged();
    void onAboutTriggered();

    void onCoreState(ChatClientCore::State oldS, ChatClientCore::State newS);
    void onCoreError(const QString& desc);
    void onKickedByServer(const QString& reason);
    void onBannedByServer(const QString& reason);

private:
    void setupUi();
    void setupMenu();
    void setupStatusBar();
    void retranslateUi();

    void loadWindowState();
    void saveWindowState();

    ChatClientCore* _core = nullptr;

    QSplitter*  _splitter = nullptr;
    UsersPanel* _usersPanel = nullptr;
    ChatPanel*  _chatPanel = nullptr;

    QLabel* _connectionStatus = nullptr;
    QLabel* _userLabel = nullptr;

    QAction* _themeAction = nullptr;
    QAction* _languageAction = nullptr;
};

} // namespace messenger::client::gui