#include "IpcClient.h"
#include "JsonCodec.h"

#include <QLocalSocket>
#include <QElapsedTimer>

IpcClient::IpcClient(QObject* parent)
    : QObject(parent)
    , m_socket(new QLocalSocket(this))
{
    connect(m_socket, &QLocalSocket::readyRead,
            this,     &IpcClient::onReadyRead);
    connect(m_socket, &QLocalSocket::errorOccurred,
            this,     &IpcClient::onError);
}

IpcClient::~IpcClient()
{
    disconnectFromServer();
}

bool IpcClient::connectToServer(int timeoutMs)
{
    m_socket->connectToServer(IpcServer::socketPath());
    return m_socket->waitForConnected(timeoutMs);
}

void IpcClient::disconnectFromServer()
{
    if (m_socket->state() != QLocalSocket::UnconnectedState) {
        m_socket->disconnectFromServer();
        if (m_socket->state() != QLocalSocket::UnconnectedState)
            m_socket->waitForDisconnected(500);
    }
}

bool IpcClient::isConnected() const
{
    return m_socket->state() == QLocalSocket::ConnectedState;
}

void IpcClient::send(const IpcMessage& msg)
{
    m_socket->write(JsonCodec::encode(msg));
    m_socket->flush();
}

std::optional<IpcMessage> IpcClient::sendSync(const IpcMessage& msg, int timeoutMs)
{
    m_inbox.clear();
    send(msg);

    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < timeoutMs) {
        if (!m_inbox.isEmpty())
            return m_inbox.takeFirst();

        auto decoded = JsonCodec::tryRead(m_buffer);
        if (decoded.has_value())
            return decoded;

        const int remaining = timeoutMs - static_cast<int>(timer.elapsed());
        if (!m_socket->waitForReadyRead(remaining))
            return std::nullopt;

        m_buffer.append(m_socket->readAll());
        decoded = JsonCodec::tryRead(m_buffer);
        if (decoded.has_value())
            return decoded;
    }

    return std::nullopt;
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------
void IpcClient::onReadyRead()
{
    m_buffer.append(m_socket->readAll());
    while (true) {
        auto msg = JsonCodec::tryRead(m_buffer);
        if (!msg.has_value()) break;
        m_inbox.append(*msg);
        emit responseReceived(*msg);
    }
}

void IpcClient::onError(QLocalSocket::LocalSocketError /*err*/)
{
    emit errorOccurred(m_socket->errorString());
}
