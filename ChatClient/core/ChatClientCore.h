#pragma once

#include "Models.h"
#include "Protocol.h"
#include "FrameCodec.h"

#include <QObject>
#include <QTcpSocket>
#include <QHash>

namespace messenger::client {

// Сетевое ядро клиента. Не зависит от UI: общается через сигналы.
// Используется как консольным, так и GUI-клиентом.
class ChatClientCore : public QObject {
    Q_OBJECT
public:
    enum class State {
        Disconnected,
        Connecting,
        Connected,        // TCP установлен, но не залогинен
        Authenticated     // успешный login/register
    };
    Q_ENUM(State)

    explicit ChatClientCore(QObject* parent = nullptr);
    ~ChatClientCore() override;

    State state() const { return _state; }
    bool isConnected() const { return _state >= State::Connected; }
    bool isAuthenticated() const { return _state == State::Authenticated; }

    qint64 currentUserId() const { return _currentUserId; }
    QString currentLogin() const { return _currentLogin; }
    QString currentDisplayName() const { return _currentDisplayName; }
    UserRole currentRole() const { return _currentRole; }

public slots:
    void connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();

    // Запросы. Каждому возвращается уникальный requestId,
    // по которому потом приходит requestSucceeded / requestFailed.
    quint32 sendRegister(const QString& login,
                         const QString& displayName,
                         const QString& password);
    quint32 sendLogin(const QString& login, const QString& password);
    quint32 sendLogout();

    quint32 sendBroadcast(const QString& body, const Attachment& att = {});
    quint32 sendPrivateMessage(qint64 receiverId,
                               const QString& body,
                               const Attachment& att = {});

    quint32 sendEdit(qint64 messageId, const QString& newBody);
    quint32 sendDelete(qint64 messageId);

    quint32 requestHistory(qint64 peerId, qint64 beforeId = 0, int limit = 50);
    quint32 requestUsers();

signals:
    // Состояние и базовые события
    void stateChanged(State oldState, State newState);
    void serverError(const QString& description);   // ошибка соединения / сети

    // Ответы на запросы
    void requestSucceeded(quint32 requestId, const QJsonObject& payload);
    void requestFailed(quint32 requestId, ResultCode code, const QString& description);

    // Push-события от сервера
    void newMessageArrived(const Message& msg);
    void messageEdited(qint64 messageId, const QString& body, const QDateTime& at);
    void messageDeleted(qint64 messageId, const QDateTime& at);
    void userOnline(qint64 userId);
    void userOffline(qint64 userId);
    void kickedByServer(const QString& reason);
    void bannedByServer(const QString& reason);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError err);

private:
    void send(const Envelope& env);
    void handleEnvelope(const Envelope& env);
    quint32 nextRequestId() { return ++_lastRequestId; }
    void setState(State s);

    QTcpSocket* _socket = nullptr;
    FrameCodec _codec;

    State _state = State::Disconnected;

    // Активные запросы и их типы (нужно, чтобы корректно реагировать на Ok)
    QHash<quint32, CommandType> _inflight;

    quint32 _lastRequestId = 0;

    // Аутентификационные данные после успешного логина
    qint64   _currentUserId = 0;
    QString  _currentLogin;
    QString  _currentDisplayName;
    UserRole _currentRole = UserRole::Regular;
};

} // namespace messenger::client