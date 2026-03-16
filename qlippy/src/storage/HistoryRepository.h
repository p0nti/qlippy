#pragma once

#include "Item.h"
#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QList>
#include <optional>

class Storage;

// All CRUD operations against clipboard_items + clipboard_payloads.
class HistoryRepository : public QObject {
    Q_OBJECT
public:
    explicit HistoryRepository(Storage* storage, QObject* parent = nullptr);

    // Insert a new item + payload.  Returns the assigned id, or 0 on error.
    qint64 insert(const Item& item, const Payload& payload);

    // Dedupe: returns the existing id if sha256 already present (and not pinned),
    // updating last_copied_at.  Returns 0 when no duplicate found.
    qint64 findDuplicate(const QString& sha256) const;

    // Fetch recent items (non-payload), newest first.
    QList<Item> fetchRecent(int limit = 200) const;

    // Full-text search on text_preview using LIKE, newest first.
    QList<Item> searchRecent(const QString& needle, int limit = 200) const;

    // Fetch a single item by id.
    std::optional<Item>    fetchItem(qint64 id) const;
    std::optional<Payload> fetchPayload(qint64 id) const;

    // Mutations
    bool remove(qint64 id);
    bool setPin(qint64 id, bool pinned);
    bool touchCopied(qint64 id);

    // Pruning: delete oldest non-pinned rows exceeding maxCount.
    // Returns number of rows deleted.
    int prune(int maxCount);

    // Pruning: delete oldest non-pinned rows until total byte_size <= maxBytes.
    // Returns number of rows deleted.
    int pruneToBytes(qint64 maxBytes);

    QString lastError() const { return m_lastError; }

private:
    Item    rowToItem(const QSqlQuery& q) const;
    QSqlDatabase db() const;

    Storage* m_storage;
    mutable QString m_lastError;
};
