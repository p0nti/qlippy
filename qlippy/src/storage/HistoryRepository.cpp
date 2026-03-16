#include "HistoryRepository.h"
#include "Storage.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>

HistoryRepository::HistoryRepository(Storage* storage, QObject* parent)
    : QObject(parent)
    , m_storage(storage)
{}

QSqlDatabase HistoryRepository::db() const
{
    return m_storage->db();
}

// ---------------------------------------------------------------------------
// Insert
// ---------------------------------------------------------------------------
qint64 HistoryRepository::insert(const Item& item, const Payload& payload)
{
    QSqlDatabase d = db();
    d.transaction();

    QSqlQuery q(d);
    q.prepare(R"sql(
        INSERT INTO clipboard_items
            (created_at, updated_at, last_copied_at, kind, text_preview,
             mime_types, sha256, is_pinned, byte_size)
        VALUES
            (:created_at, :updated_at, :last_copied_at, :kind, :text_preview,
             :mime_types, :sha256, :is_pinned, :byte_size)
    )sql");

    const qint64 now = QDateTime::currentSecsSinceEpoch();
    q.bindValue(":created_at",     item.createdAt.isValid()    ? item.createdAt.toSecsSinceEpoch()    : now);
    q.bindValue(":updated_at",     item.updatedAt.isValid()    ? item.updatedAt.toSecsSinceEpoch()    : now);
    q.bindValue(":last_copied_at", item.lastCopiedAt.isValid() ? item.lastCopiedAt.toSecsSinceEpoch() : now);
    q.bindValue(":kind",           item.kind == ItemKind::Image ? "image" : "text");
    q.bindValue(":text_preview",   item.textPreview);
    q.bindValue(":mime_types",     item.mimeTypes);
    q.bindValue(":sha256",         item.sha256);
    q.bindValue(":is_pinned",      item.isPinned ? 1 : 0);
    q.bindValue(":byte_size",      item.byteSize);

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        d.rollback();
        return 0;
    }

    const qint64 newId = q.lastInsertId().toLongLong();

    QSqlQuery qp(d);
    qp.prepare(R"sql(
        INSERT INTO clipboard_payloads (item_id, text_plain, image_png, raw_payload)
        VALUES (:item_id, :text_plain, :image_png, :raw_payload)
    )sql");
    qp.bindValue(":item_id",     newId);
    qp.bindValue(":text_plain",  payload.textPlain.isNull() ? QVariant() : payload.textPlain);
    qp.bindValue(":image_png",   payload.imagePng.isEmpty() ? QVariant() : payload.imagePng);
    qp.bindValue(":raw_payload", payload.rawPayload.isEmpty() ? QVariant() : payload.rawPayload);

    if (!qp.exec()) {
        m_lastError = qp.lastError().text();
        d.rollback();
        return 0;
    }

    d.commit();
    return newId;
}

// ---------------------------------------------------------------------------
// Dedupe
// ---------------------------------------------------------------------------
qint64 HistoryRepository::findDuplicate(const QString& sha256) const
{
    QSqlQuery q(db());
    q.prepare("SELECT id FROM clipboard_items WHERE sha256 = :sha256 AND is_pinned = 0 LIMIT 1");
    q.bindValue(":sha256", sha256);
    if (q.exec() && q.next())
        return q.value(0).toLongLong();
    return 0;
}

// ---------------------------------------------------------------------------
// Fetch
// ---------------------------------------------------------------------------
QList<Item> HistoryRepository::fetchRecent(int limit) const
{
    QSqlQuery q(db());
    q.prepare("SELECT * FROM clipboard_items ORDER BY last_copied_at DESC, id DESC LIMIT :limit");
    q.bindValue(":limit", limit);

    QList<Item> items;
    if (q.exec()) {
        while (q.next())
            items.append(rowToItem(q));
    }
    return items;
}

QList<Item> HistoryRepository::searchRecent(const QString& needle, int limit) const
{
    QSqlQuery q(db());
    q.prepare(R"sql(
        SELECT * FROM clipboard_items
        WHERE text_preview LIKE :needle ESCAPE '\'
        ORDER BY last_copied_at DESC, id DESC
        LIMIT :limit
    )sql");
    // Escape any LIKE special chars in the needle, then wrap in wildcards
    QString escaped = needle;
    escaped.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
    escaped.replace(QLatin1Char('%'),  QLatin1String("\\%"));
    escaped.replace(QLatin1Char('_'),  QLatin1String("\\_"));
    q.bindValue(":needle", QLatin1Char('%') + escaped + QLatin1Char('%'));
    q.bindValue(":limit", limit);

    QList<Item> items;
    if (q.exec()) {
        while (q.next())
            items.append(rowToItem(q));
    }
    return items;
}

std::optional<Item> HistoryRepository::fetchItem(qint64 id) const
{
    QSqlQuery q(db());
    q.prepare("SELECT * FROM clipboard_items WHERE id = :id");
    q.bindValue(":id", id);
    if (q.exec() && q.next())
        return rowToItem(q);
    return std::nullopt;
}

std::optional<Payload> HistoryRepository::fetchPayload(qint64 id) const
{
    QSqlQuery q(db());
    q.prepare("SELECT text_plain, image_png, raw_payload FROM clipboard_payloads WHERE item_id = :id");
    q.bindValue(":id", id);
    if (!q.exec() || !q.next())
        return std::nullopt;

    Payload p;
    if (!q.value(0).isNull()) p.textPlain   = q.value(0).toString();
    if (!q.value(1).isNull()) p.imagePng    = q.value(1).toByteArray();
    if (!q.value(2).isNull()) p.rawPayload  = q.value(2).toByteArray();
    return p;
}

// ---------------------------------------------------------------------------
// Mutations
// ---------------------------------------------------------------------------
bool HistoryRepository::remove(qint64 id)
{
    QSqlQuery q(db());
    q.prepare("DELETE FROM clipboard_items WHERE id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool HistoryRepository::setPin(qint64 id, bool pinned)
{
    QSqlQuery q(db());
    q.prepare("UPDATE clipboard_items SET is_pinned = :pin, updated_at = :now WHERE id = :id");
    q.bindValue(":pin", pinned ? 1 : 0);
    q.bindValue(":now", QDateTime::currentSecsSinceEpoch());
    q.bindValue(":id",  id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool HistoryRepository::touchCopied(qint64 id)
{
    QSqlQuery q(db());
    q.prepare("UPDATE clipboard_items SET last_copied_at = :now WHERE id = :id");
    q.bindValue(":now", QDateTime::currentSecsSinceEpoch());
    q.bindValue(":id",  id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Pruning: by count
// ---------------------------------------------------------------------------
int HistoryRepository::prune(int maxCount)
{
    // Count non-pinned rows
    QSqlQuery qc(db());
    qc.prepare("SELECT COUNT(*) FROM clipboard_items WHERE is_pinned = 0");
    if (!qc.exec() || !qc.next()) return 0;

    const int total    = qc.value(0).toInt();
    const int toDelete = total - maxCount;
    if (toDelete <= 0) return 0;

    QSqlQuery q(db());
    q.prepare(R"sql(
        DELETE FROM clipboard_items
        WHERE id IN (
            SELECT id FROM clipboard_items
            WHERE is_pinned = 0
            ORDER BY created_at ASC
            LIMIT :n
        )
    )sql");
    q.bindValue(":n", toDelete);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return 0;
    }
    return q.numRowsAffected();
}

// ---------------------------------------------------------------------------
// Pruning: by total byte size
// ---------------------------------------------------------------------------
int HistoryRepository::pruneToBytes(qint64 maxBytes)
{
    // Sum byte_size of all non-pinned rows
    QSqlQuery qs(db());
    qs.prepare("SELECT COALESCE(SUM(byte_size), 0) FROM clipboard_items WHERE is_pinned = 0");
    if (!qs.exec() || !qs.next()) return 0;

    qint64 total = qs.value(0).toLongLong();
    if (total <= maxBytes) return 0;

    // Delete oldest non-pinned items one batch at a time until under budget.
    // Uses a single DELETE … IN (SELECT … ORDER BY created_at LIMIT n) query
    // by estimating how many rows to drop based on average size.
    QSqlQuery qcount(db());
    qcount.prepare("SELECT COUNT(*) FROM clipboard_items WHERE is_pinned = 0");
    if (!qcount.exec() || !qcount.next()) return 0;

    const int rowCount = qcount.value(0).toInt();
    if (rowCount == 0) return 0;

    // Rough estimate: delete enough rows to cover the overage.
    const qint64 avgBytes  = total / rowCount;
    const qint64 overage   = total - maxBytes;
    const int    estimate  = static_cast<int>((overage + avgBytes - 1) / std::max<qint64>(avgBytes, 1));
    const int    toDelete  = std::max(1, estimate);

    QSqlQuery q(db());
    q.prepare(R"sql(
        DELETE FROM clipboard_items
        WHERE id IN (
            SELECT id FROM clipboard_items
            WHERE is_pinned = 0
            ORDER BY created_at ASC
            LIMIT :n
        )
    )sql");
    q.bindValue(":n", toDelete);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return 0;
    }
    return q.numRowsAffected();
}

// ---------------------------------------------------------------------------
// Row mapping
// ---------------------------------------------------------------------------
Item HistoryRepository::rowToItem(const QSqlQuery& q) const
{
    Item item;
    item.id            = q.value("id").toLongLong();
    item.createdAt     = QDateTime::fromSecsSinceEpoch(q.value("created_at").toLongLong());
    item.updatedAt     = QDateTime::fromSecsSinceEpoch(q.value("updated_at").toLongLong());
    item.lastCopiedAt  = QDateTime::fromSecsSinceEpoch(q.value("last_copied_at").toLongLong());
    item.kind          = q.value("kind").toString() == "image" ? ItemKind::Image : ItemKind::Text;
    item.textPreview   = q.value("text_preview").toString();
    item.mimeTypes     = q.value("mime_types").toString();
    item.sha256        = q.value("sha256").toString();
    item.isPinned      = q.value("is_pinned").toInt() != 0;
    item.byteSize      = q.value("byte_size").toLongLong();
    return item;
}
