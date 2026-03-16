#pragma once

#include <QString>
#include <QByteArray>
#include <QDateTime>

enum class ItemKind {
    Text,
    Image,
};

struct Payload {
    QString    textPlain;
    QByteArray imagePng;
    QByteArray rawPayload;
};

struct Item {
    qint64    id          = 0;
    QDateTime createdAt;
    QDateTime updatedAt;
    QDateTime lastCopiedAt;
    ItemKind  kind        = ItemKind::Text;
    QString   textPreview;
    QString   mimeTypes;   // comma-separated
    QString   sha256;
    bool      isPinned    = false;
    qint64    byteSize    = 0;

    bool isValid() const { return id > 0; }
};
