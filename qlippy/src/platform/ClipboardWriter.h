#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QStringList>

class WaylandConnection;
class WaylandRegistry;

// Writes content back to the Wayland clipboard using the data-control protocol.
// Does not require compositor focus (unlike QClipboard on Wayland).
class ClipboardWriter : public QObject {
    Q_OBJECT
public:
    explicit ClipboardWriter(WaylandConnection* conn,
                             WaylandRegistry*   registry,
                             QObject*           parent = nullptr);
    ~ClipboardWriter() override;

    void setText(const QString& text);
    void setImage(const QByteArray& pngData);

public:
    // Called from static C listener callbacks — do not call directly.
    void onSourceSend(const char* mime, int fd);
    void onSourceCancelled();

private:
    void setSelection(const QStringList& mimes);
    void destroySource();

    WaylandConnection* m_conn;
    WaylandRegistry*   m_registry;

    bool          m_useExt   = false;
    void*         m_device   = nullptr;  // ext or wlr device (owned)
    void*         m_source   = nullptr;  // ext or wlr source (current selection)

    QString       m_text;
    QByteArray    m_imagePng;
};
