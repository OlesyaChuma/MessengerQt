#pragma once

#include "ChatClientCore.h"

#include <QMainWindow>
#include <QHash>
#include <QSoundEffect>

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

    int _unreadTotal = 0;
    QHash<qint64, int> _unreadByPeer;  // peerId - counter

    QAction* _muteAction = nullptr;     // галочка «без звука» в меню View
    QSoundEffect* _notifySound = nullptr;
    bool _soundEnabled = true;

    void setupNotificationSound();      // загрузка WAV
    void setSoundEnabled(bool on);      // переключение + сохранение в QSettings

    void incrementUnread(qint64 peerId);
    void clearUnreadFor(qint64 peerId);
    void updateWindowTitle();
    void notifyNewMessage(const QString& senderName, const QString& body);
};

} // namespace messenger::client::gui
