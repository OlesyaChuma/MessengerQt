#pragma once

#include <QObject>
#include <QHash>
#include <QPointer>
#include <atomic>

#include "core/ChatClientCore.h"
#include "Models.h"

namespace messenger::client {

// Консольный текстовый UI поверх ChatClientCore.
// Используется в этом проекте только как тестовая утилита для интеграции;
// финальный продукт — GUI на Qt Widgets (Пакет 5).
class ConsoleUi : public QObject {
    Q_OBJECT
public:
    ConsoleUi(ChatClientCore* core,
              const QString& host, quint16 port,
              QObject* parent = nullptr);
    ~ConsoleUi() override;

    void start();

signals:
    void shutdownRequested();

public slots:
    void onLineRead(const QString& line);

private slots:
    void onStateChanged(ChatClientCore::State oldS, ChatClientCore::State newS);
    void onServerError(const QString& desc);
    void onRequestSucceeded(quint32 rid, const QJsonObject& payload);
    void onRequestFailed(quint32 rid, ResultCode code, const QString& desc);

    void onNewMessageArrived(const Message& m);
    void onMessageEdited(qint64 id, const QString& body, const QDateTime& at);
    void onMessageDeleted(qint64 id, const QDateTime& at);
    void onUserOnline(qint64 userId);
    void onUserOffline(qint64 userId);
    void onKicked(const QString& reason);
    void onBanned(const QString& reason);

private:
    void printHelp() const;
    void prompt() const;
    void println(const QString& s) const;

    void cmdRegister(const QStringList& args);
    void cmdLogin(const QStringList& args);
    void cmdLogout();
    void cmdUsers();
    void cmdHistory(const QStringList& args);
    void cmdSendBroadcast(const QStringList& args);
    void cmdSendPrivate(const QStringList& args);
    void cmdEdit(const QStringList& args);
    void cmdDelete(const QStringList& args);

    void renderMessage(const Message& m) const;
    void rememberUsers(const QJsonArray& arr);
    QString loginForId(qint64 id) const;

    ChatClientCore* _core = nullptr;
    QString _host;
    quint16 _port = 0;

    // Кэш пользователей (id -> login) для удобного отображения
    QHash<qint64, QString> _userLoginCache;

    // Какой запрос за что отвечает (чтобы красиво печатать ответы)
    QHash<quint32, QString> _requestLabel;

    std::atomic<bool> _shuttingDown{false};
};

} // namespace messenger::client