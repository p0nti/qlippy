#pragma once

#include "IpcMessage.h"
#include <QByteArray>
#include <optional>

// Wire format: [uint32 big-endian length][UTF-8 JSON body]
class JsonCodec {
public:
    // Encode a message to wire bytes (length prefix + JSON).
    static QByteArray encode(const IpcMessage& msg);

    // Attempt to extract and decode one complete message from `buffer`.
    // On success, consumes the bytes and returns the message.
    // Returns std::nullopt when the buffer holds an incomplete frame.
    static std::optional<IpcMessage> tryRead(QByteArray& buffer);

private:
    static IpcMessage fromJson(const QJsonObject& obj);
    static QJsonObject toJson(const IpcMessage& msg);
};
