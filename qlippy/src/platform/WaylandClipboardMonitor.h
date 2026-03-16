#pragma once

#include "storage/Item.h"
#include <QObject>
#include <QString>

class WaylandConnection;
class WaylandRegistry;
class QSocketNotifier;

// Monitors the Wayland clipboard using ext-data-control-v1 (preferred)
// or wlr-data-control-unstable-v1 (fallback).
//
// Emits clipboardChanged(Item, Payload) whenever a new selection is detected.
class WaylandClipboardMonitor : public QObject {
    Q_OBJECT
public:
    explicit WaylandClipboardMonitor(WaylandConnection* conn,
                                     WaylandRegistry*   registry,
                                     QObject*           parent = nullptr);
    ~WaylandClipboardMonitor() override;

    bool start();

signals:
    void clipboardChanged(Item item, Payload payload);

public:
    // Called from static C listener callbacks — do not call directly.
    void onDataOffer(void* offer);
    void onOfferMime(void* offerState, const char* mime);
    void onSelection(void* offer);

private slots:
    void onPipeReadable();

private:
    void processOffer(void* offerState);
    void cancelPendingRead();

    WaylandConnection* m_conn;
    WaylandRegistry*   m_registry;

    // Raw protocol device handle (ext or wlr)
    void* m_device = nullptr;
    bool  m_useExt = false;

    // Current in-progress pipe read
    QSocketNotifier* m_pipeNotifier = nullptr;
    QByteArray       m_pipeBuffer;
    QString          m_currentMime;
    void*            m_currentOfferState = nullptr; // OfferData* that owns the active offer
};
