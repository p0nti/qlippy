#include "Storage.h"

#include <QDir>
#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlError>

Storage::Storage(QObject* parent)
    : QObject(parent)
{}

Storage::~Storage()
{
    close();
}

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------
QString Storage::defaultPath()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    // AppLocalDataLocation uses the application name; ensure it ends with "qlippy"
    if (!base.endsWith("qlippy"))
        base = QDir::homePath() + "/.local/share/qlippy";
    return base + "/db.sqlite";
}

// ---------------------------------------------------------------------------
// open / close
// ---------------------------------------------------------------------------
bool Storage::open()
{
    const QString path = defaultPath();
    QDir dir = QFileInfo(path).absoluteDir();
    if (!dir.exists() && !dir.mkpath(".")) {
        m_lastError = "Cannot create data directory: " + dir.absolutePath();
        return false;
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", "qlippy_main");
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    return createSchema();
}

void Storage::close()
{
    if (m_db.isOpen())
        m_db.close();

    const QString connName = m_db.connectionName();
    m_db = QSqlDatabase(); // detach
    if (!connName.isEmpty())
        QSqlDatabase::removeDatabase(connName);
}

bool Storage::isOpen() const
{
    return m_db.isOpen();
}

// ---------------------------------------------------------------------------
// Schema
// ---------------------------------------------------------------------------
bool Storage::createSchema()
{
    QSqlQuery q(m_db);

    // Enable WAL for better concurrency
    if (!q.exec("PRAGMA journal_mode=WAL;")) {
        m_lastError = q.lastError().text();
        return false;
    }

    if (!q.exec(R"sql(
        CREATE TABLE IF NOT EXISTS clipboard_items (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            created_at      INTEGER NOT NULL,
            updated_at      INTEGER NOT NULL,
            last_copied_at  INTEGER NOT NULL DEFAULT 0,
            kind            TEXT    NOT NULL DEFAULT 'text',
            text_preview    TEXT    NOT NULL DEFAULT '',
            mime_types      TEXT    NOT NULL DEFAULT '',
            sha256          TEXT    NOT NULL DEFAULT '',
            is_pinned       INTEGER NOT NULL DEFAULT 0,
            byte_size       INTEGER NOT NULL DEFAULT 0
        )
    )sql")) {
        m_lastError = q.lastError().text();
        return false;
    }

    if (!q.exec(R"sql(
        CREATE TABLE IF NOT EXISTS clipboard_payloads (
            item_id     INTEGER PRIMARY KEY REFERENCES clipboard_items(id) ON DELETE CASCADE,
            text_plain  TEXT,
            image_png   BLOB,
            raw_payload BLOB
        )
    )sql")) {
        m_lastError = q.lastError().text();
        return false;
    }

    if (!q.exec(R"sql(
        CREATE TABLE IF NOT EXISTS settings (
            key     TEXT PRIMARY KEY,
            value   TEXT NOT NULL
        )
    )sql")) {
        m_lastError = q.lastError().text();
        return false;
    }

    // Indexes
    q.exec("CREATE INDEX IF NOT EXISTS idx_items_created  ON clipboard_items(created_at DESC)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_items_sha256   ON clipboard_items(sha256)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_items_pinned   ON clipboard_items(is_pinned, created_at DESC)");

    // Foreign key enforcement
    q.exec("PRAGMA foreign_keys = ON;");

    return true;
}
