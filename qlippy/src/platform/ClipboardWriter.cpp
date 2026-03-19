#include "ClipboardWriter.h"
#include "WaylandConnection.h"
#include "WaylandRegistry.h"
#include "common/Logger.h"

#include <wayland-client.h>
#include "ext-data-control-v1-client-protocol.h"
#include "wlr-data-control-unstable-v1-client-protocol.h"

#include <QSocketNotifier>

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

// ===========================================================================
// Discard listeners for the writer's own device
// (We own a second device just to issue set_selection; incoming offers are
// properly destroyed to avoid Wayland proxy leaks.)
// ===========================================================================

static void w_ext_device_offer(void*, ext_data_control_device_v1*,
                                ext_data_control_offer_v1* offer)
    { if (offer) ext_data_control_offer_v1_destroy(offer); }
static void w_ext_device_selection(void*, ext_data_control_device_v1*,
                                    ext_data_control_offer_v1*) {}
static void w_ext_device_finished(void*, ext_data_control_device_v1* dev)
    { ext_data_control_device_v1_destroy(dev); }
static void w_ext_device_primary(void*, ext_data_control_device_v1*,
                                  ext_data_control_offer_v1* offer)
    { if (offer) ext_data_control_offer_v1_destroy(offer); }

static const ext_data_control_device_v1_listener s_extDeviceDiscard = {
    .data_offer        = w_ext_device_offer,
    .selection         = w_ext_device_selection,
    .finished          = w_ext_device_finished,
    .primary_selection = w_ext_device_primary,
};

static void w_wlr_device_offer(void*, zwlr_data_control_device_v1*,
                                zwlr_data_control_offer_v1* offer)
    { if (offer) zwlr_data_control_offer_v1_destroy(offer); }
static void w_wlr_device_selection(void*, zwlr_data_control_device_v1*,
                                    zwlr_data_control_offer_v1*) {}
static void w_wlr_device_finished(void*, zwlr_data_control_device_v1* dev)
    { zwlr_data_control_device_v1_destroy(dev); }
static void w_wlr_device_primary(void*, zwlr_data_control_device_v1*,
                                  zwlr_data_control_offer_v1* offer)
    { if (offer) zwlr_data_control_offer_v1_destroy(offer); }

static const zwlr_data_control_device_v1_listener s_wlrDeviceDiscard = {
    .data_offer        = w_wlr_device_offer,
    .selection         = w_wlr_device_selection,
    .finished          = w_wlr_device_finished,
    .primary_selection = w_wlr_device_primary,
};

// ===========================================================================
// Source listeners — respond to paste requests
// ===========================================================================

static void ext_source_send(void* data, ext_data_control_source_v1*,
                             const char* mime, int32_t fd)
    { static_cast<ClipboardWriter*>(data)->onSourceSend(mime, fd); }

static void ext_source_cancelled(void* data, ext_data_control_source_v1*)
    { static_cast<ClipboardWriter*>(data)->onSourceCancelled(); }

static const ext_data_control_source_v1_listener s_extSourceListener = {
    .send      = ext_source_send,
    .cancelled = ext_source_cancelled,
};

static void wlr_source_send(void* data, zwlr_data_control_source_v1*,
                             const char* mime, int32_t fd)
    { static_cast<ClipboardWriter*>(data)->onSourceSend(mime, fd); }

static void wlr_source_cancelled(void* data, zwlr_data_control_source_v1*)
    { static_cast<ClipboardWriter*>(data)->onSourceCancelled(); }

static const zwlr_data_control_source_v1_listener s_wlrSourceListener = {
    .send      = wlr_source_send,
    .cancelled = wlr_source_cancelled,
};

// ===========================================================================
// ClipboardWriter
// ===========================================================================

ClipboardWriter::ClipboardWriter(WaylandConnection* conn,
                                  WaylandRegistry*   registry,
                                  QObject*           parent)
    : QObject(parent)
    , m_conn(conn)
    , m_registry(registry)
{
    if (registry->extManager()) {
        m_useExt = true;
        auto* dev = ext_data_control_manager_v1_get_data_device(
            registry->extManager(), registry->seat());
        ext_data_control_device_v1_add_listener(dev, &s_extDeviceDiscard, nullptr);
        m_device = dev;
        LOG_DEBUG("ClipboardWriter: using ext-data-control-v1");
    } else if (registry->wlrManager()) {
        m_useExt = false;
        auto* dev = zwlr_data_control_manager_v1_get_data_device(
            registry->wlrManager(), registry->seat());
        zwlr_data_control_device_v1_add_listener(dev, &s_wlrDeviceDiscard, nullptr);
        m_device = dev;
        LOG_DEBUG("ClipboardWriter: using wlr-data-control-unstable-v1");
    } else {
        LOG_WARN("ClipboardWriter: no data-control protocol available");
    }
}

ClipboardWriter::~ClipboardWriter()
{
    abortPendingWrites();
    destroySource();
    if (m_device) {
        if (m_useExt)
            ext_data_control_device_v1_destroy(
                static_cast<ext_data_control_device_v1*>(m_device));
        else
            zwlr_data_control_device_v1_destroy(
                static_cast<zwlr_data_control_device_v1*>(m_device));
        m_device = nullptr;
    }
}

void ClipboardWriter::setText(const QString& text)
{
    if (!m_device) return;
    m_text     = text;
    m_imagePng.clear();
    setSelection({"text/plain;charset=utf-8", "text/plain"});
}

void ClipboardWriter::setImage(const QByteArray& pngData)
{
    if (!m_device) return;
    m_imagePng = pngData;
    m_text.clear();
    setSelection({"image/png"});
}

void ClipboardWriter::setSelection(const QStringList& mimes)
{
    destroySource();

    if (m_useExt) {
        auto* src = ext_data_control_manager_v1_create_data_source(
            m_registry->extManager());
        for (const QString& m : mimes)
            ext_data_control_source_v1_offer(src, m.toUtf8().constData());
        ext_data_control_source_v1_add_listener(src, &s_extSourceListener, this);
        ext_data_control_device_v1_set_selection(
            static_cast<ext_data_control_device_v1*>(m_device), src);
        m_source = src;
    } else {
        auto* src = zwlr_data_control_manager_v1_create_data_source(
            m_registry->wlrManager());
        for (const QString& m : mimes)
            zwlr_data_control_source_v1_offer(src, m.toUtf8().constData());
        zwlr_data_control_source_v1_add_listener(src, &s_wlrSourceListener, this);
        zwlr_data_control_device_v1_set_selection(
            static_cast<zwlr_data_control_device_v1*>(m_device), src);
        m_source = src;
    }

    m_conn->flush();
}

void ClipboardWriter::onSourceSend(const char* mime, int fd)
{
    const QString mimeStr = QString::fromUtf8(mime);
    QByteArray data = (mimeStr == QLatin1String("image/png")) ? m_imagePng
                                                               : m_text.toUtf8();

    // Non-blocking so writes never stall the event loop
    ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

    const bool idle = m_writeQueue.empty();
    m_writeQueue.push({fd, std::move(data), 0});
    if (idle)
        startNextWrite();
}

void ClipboardWriter::startNextWrite()
{
    Q_ASSERT(!m_writeQueue.empty());
    Q_ASSERT(!m_writeNotifier);

    m_writeNotifier = new QSocketNotifier(
        m_writeQueue.front().fd, QSocketNotifier::Write, this);
    connect(m_writeNotifier, &QSocketNotifier::activated,
            this, &ClipboardWriter::onWriteReady);
}

void ClipboardWriter::onWriteReady()
{
    Q_ASSERT(!m_writeQueue.empty());
    PendingWrite& pw = m_writeQueue.front();

    while (pw.pos < pw.data.size()) {
        ssize_t n = ::write(pw.fd,
                            pw.data.constData() + pw.pos,
                            static_cast<std::size_t>(pw.data.size() - pw.pos));
        if (n < 0) {
            if (errno == EINTR)                          continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) return; // re-armed automatically
            LOG_WARN("ClipboardWriter: write to paste fd failed");
            break;
        }
        pw.pos += n;
    }

    // Done (or unrecoverable error) — retire this entry
    m_writeNotifier->setEnabled(false);
    m_writeNotifier->deleteLater();
    m_writeNotifier = nullptr;
    ::close(pw.fd);
    m_writeQueue.pop();

    if (!m_writeQueue.empty())
        startNextWrite();
}

void ClipboardWriter::abortPendingWrites()
{
    if (m_writeNotifier) {
        m_writeNotifier->setEnabled(false);
        m_writeNotifier->deleteLater();
        m_writeNotifier = nullptr;
    }
    while (!m_writeQueue.empty()) {
        ::close(m_writeQueue.front().fd);
        m_writeQueue.pop();
    }
}

void ClipboardWriter::onSourceCancelled()
{
    LOG_DEBUG("ClipboardWriter: selection replaced by another source");
    destroySource();
}

void ClipboardWriter::destroySource()
{
    if (!m_source) return;
    if (m_useExt)
        ext_data_control_source_v1_destroy(
            static_cast<ext_data_control_source_v1*>(m_source));
    else
        zwlr_data_control_source_v1_destroy(
            static_cast<zwlr_data_control_source_v1*>(m_source));
    m_source = nullptr;
}

