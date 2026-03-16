#pragma once

#include "IpcMessage.h"
#include "IpcServer.h"
#include <QObject>
#include <QLocalSocket>
#include <QByteArray>
#include <QList>
#include <functional>

// Connects to the daemon's IPC server and sends commands.
// Supports both async (signal-based) and blocking-sync usage for CLI mode.
class IpcClient : public QObject {
    Q_OBJECT
public:
    explicit IpcClient(QObject* parent = nullptr);
    ~IpcClient() override;

    // Connect to the server. Returns true when connected.
    bool connectToServer(int timeoutMs = 2000);
    void disconnectFromServer();
    bool isConnected() const;

    // Send a request (non-blocking). The responseReceived signal fires on reply.
    void send(const IpcMessage& msg);

    // Blocking send + wait for one response. Returns nullopt on timeout.
    // Intended for CLI-mode where there is no running event loop.
    std::optional<IpcMessage> sendSync(const IpcMessage& msg, int timeoutMs = 3000);

signals:
    void responseReceived(IpcMessage msg);
    void errorOccurred(QString errorString);

private slots:
    void onReadyRead();
    void onError(QLocalSocket::LocalSocketError err);

private:
    QLocalSocket* m_socket;
    QByteArray    m_buffer;
    QList<IpcMessage> m_inbox;
};
