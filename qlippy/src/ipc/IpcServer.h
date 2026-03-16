#pragma once

#include "IpcMessage.h"
#include <QObject>
#include <QLocalServer>
#include <QHash>
#include <QByteArray>
#include <QString>

class QLocalSocket;

// Listens on the well-known socket name and dispatches incoming commands.
class IpcServer : public QObject {
    Q_OBJECT
public:
    static constexpr char kSocketName[] = "qlippy-ipc-v1";

    // Returns the full absolute path to the socket, honouring XDG_RUNTIME_DIR.
    // Both server and client must use this to agree on the path.
    static QString socketPath();

    explicit IpcServer(QObject* parent = nullptr);
    ~IpcServer() override;

    bool start();
    void stop();
    bool isListening() const;
    QString lastError() const { return m_lastError; }

    // Send a response back to the connection that sent a command.
    void reply(QLocalSocket* conn, const IpcMessage& msg);

signals:
    void commandReceived(IpcMessage msg, QLocalSocket* conn);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QLocalServer*                   m_server;
    QHash<QLocalSocket*, QByteArray> m_buffers;
    QString                          m_lastError;
};
