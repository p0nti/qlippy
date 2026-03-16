#include "IpcServer.h"
#include "JsonCodec.h"

#include <QLocalSocket>
#include <QLocalServer>
#include <QStandardPaths>
#include <QDir>

QString IpcServer::socketPath()
{
    // Prefer $XDG_RUNTIME_DIR (user-private, mode 700) over /tmp.
    QString runtimeDir = QString::fromLocal8Bit(qgetenv("XDG_RUNTIME_DIR"));
    if (runtimeDir.isEmpty())
        runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (runtimeDir.isEmpty())
        runtimeDir = QDir::tempPath();
    return runtimeDir + "/" + QString::fromLatin1(kSocketName);
}

IpcServer::IpcServer(QObject* parent)
    : QObject(parent)
    , m_server(new QLocalServer(this))
{
    connect(m_server, &QLocalServer::newConnection,
            this,     &IpcServer::onNewConnection);
}

IpcServer::~IpcServer()
{
    stop();
}

bool IpcServer::start()
{
    const QString path = socketPath();
    QLocalServer::removeServer(path);
    if (!m_server->listen(path)) {
        return false;
    }
    return true;
}

void IpcServer::stop()
{
    m_server->close();
}

bool IpcServer::isListening() const
{
    return m_server->isListening();
}

void IpcServer::reply(QLocalSocket* conn, const IpcMessage& msg)
{
    if (conn && conn->isValid()) {
        conn->write(JsonCodec::encode(msg));
        conn->flush();
    }
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------
void IpcServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QLocalSocket* conn = m_server->nextPendingConnection();
        m_buffers[conn] = QByteArray{};

        connect(conn, &QLocalSocket::readyRead,
                this, &IpcServer::onReadyRead);
        connect(conn, &QLocalSocket::disconnected,
                this, &IpcServer::onDisconnected);
    }
}

void IpcServer::onReadyRead()
{
    auto* conn = qobject_cast<QLocalSocket*>(sender());
    if (!conn) return;

    QByteArray& buf = m_buffers[conn];
    buf.append(conn->readAll());

    while (true) {
        auto msg = JsonCodec::tryRead(buf);
        if (!msg.has_value()) break;
        emit commandReceived(*msg, conn);
    }
}

void IpcServer::onDisconnected()
{
    auto* conn = qobject_cast<QLocalSocket*>(sender());
    if (!conn) return;
    m_buffers.remove(conn);
    conn->deleteLater();
}
