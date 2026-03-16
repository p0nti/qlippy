#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QString>

// Owns the database connection, creates schema, runs migrations.
class Storage : public QObject {
    Q_OBJECT
public:
    explicit Storage(QObject* parent = nullptr);
    ~Storage() override;

    // Opens (and creates if needed) the database at the canonical path.
    // Returns false on error; call lastError() for details.
    bool open();
    void close();
    bool isOpen() const;

    QSqlDatabase& db() { return m_db; }

    QString lastError() const { return m_lastError; }

    // The canonical path: ~/.local/share/qlippy/db.sqlite
    static QString defaultPath();

private:
    bool createSchema();

    QSqlDatabase m_db;
    QString      m_lastError;
};
