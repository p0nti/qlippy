#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QStringList>

#include <queue>

class QSocketNotifier;
class WaylandConnection;
class WaylandRegistry;

// Writes content back to the Wayland clipboard using the data-control protocol.
// Does not require compositor focus (unlike QClipboard on Wayland).
//
// send events are served asynchronously via QSocketNotifier to avoid blocking
// the Qt event loop while the receiving application drains its pipe buffer.
// Concurrent send events (multiple MIME types from the same paste) are queued
// and dispatched in order.
class ClipboardWriter : public QObject {
    Q_OBJECT
public:
    explicit ClipboardWriter(WaylandConnection* conn,
                             WaylandRegistry*   registry,
                             QObject*           parent = nullptr);
    ~ClipboardWriter() override;

    void setText(const QString& text);
    void setImage(const QByteArray& pngData);

    // Called from static C listener callbacks — do not call directly.
    void onSourceSend(const char* mime, int fd);
    void onSourceCancelled();

private:
    void setSelection(const QStringList& mimes);
    void destroySource();

    // Async write helpers
    void startNextWrite();
    void onWriteReady();
    void abortPendingWrites();

    struct PendingWrite {
        int        fd;
        QByteArray data;
        qint64     pos{0};
    };

    WaylandConnection* m_conn;
    WaylandRegistry*   m_registry;

    bool          m_useExt   = false;
    void*         m_device   = nullptr;  // ext or wlr device (owned)
    void*         m_source   = nullptr;  // ext or wlr source (current selection)

    QString       m_text;
    QByteArray    m_imagePng;

    std::queue<PendingWrite> m_writeQueue;
    QSocketNotifier*         m_writeNotifier = nullptr;
};
