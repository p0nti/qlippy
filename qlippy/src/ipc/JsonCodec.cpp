#include "JsonCodec.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>

static constexpr int kHeaderSize = 4; // uint32 big-endian

// ---------------------------------------------------------------------------
// Encode
// ---------------------------------------------------------------------------
QByteArray JsonCodec::encode(const IpcMessage& msg)
{
    QJsonObject obj = toJson(msg);
    QByteArray  body = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    QByteArray frame;
    frame.resize(kHeaderSize);
    quint32 len = static_cast<quint32>(body.size());
    qToBigEndian(len, reinterpret_cast<uchar*>(frame.data()));
    frame.append(body);
    return frame;
}

// ---------------------------------------------------------------------------
// Decode
// ---------------------------------------------------------------------------
std::optional<IpcMessage> JsonCodec::tryRead(QByteArray& buffer)
{
    if (buffer.size() < kHeaderSize)
        return std::nullopt;

    quint32 bodyLen = qFromBigEndian<quint32>(buffer.constData());

    if (buffer.size() < static_cast<int>(kHeaderSize + bodyLen))
        return std::nullopt;

    QByteArray body = buffer.mid(kHeaderSize, static_cast<int>(bodyLen));
    buffer.remove(0, static_cast<int>(kHeaderSize + bodyLen));

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(body, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return std::nullopt;

    return fromJson(doc.object());
}

// ---------------------------------------------------------------------------
// JSON <-> IpcMessage helpers
// ---------------------------------------------------------------------------
QJsonObject JsonCodec::toJson(const IpcMessage& msg)
{
    QJsonObject obj;
    if (msg.isRequest()) {
        obj["cmd"] = msg.cmd;
        if (!msg.params.isEmpty())
            obj["params"] = msg.params;
    } else {
        obj["ok"] = msg.ok;
        if (!msg.ok)
            obj["error"] = msg.error;
        if (!msg.data.isEmpty())
            obj["data"] = msg.data;
    }
    return obj;
}

IpcMessage JsonCodec::fromJson(const QJsonObject& obj)
{
    IpcMessage m;
    if (obj.contains("cmd")) {
        // request
        m.cmd    = obj["cmd"].toString();
        m.params = obj["params"].toObject();
    } else {
        // response
        m.ok    = obj["ok"].toBool(true);
        m.error = obj["error"].toString();
        m.data  = obj["data"].toObject();
    }
    return m;
}
