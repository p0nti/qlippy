#include "IpcMessage.h"

IpcMessage IpcMessage::request(const QString& cmd, QJsonObject params)
{
    IpcMessage m;
    m.cmd    = cmd;
    m.params = std::move(params);
    return m;
}

IpcMessage IpcMessage::success(QJsonObject data)
{
    IpcMessage m;
    m.ok   = true;
    m.data = std::move(data);
    return m;
}

IpcMessage IpcMessage::failure(const QString& error)
{
    IpcMessage m;
    m.ok    = false;
    m.error = error;
    return m;
}
