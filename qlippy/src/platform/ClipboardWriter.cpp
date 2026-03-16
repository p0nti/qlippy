#include "ClipboardWriter.h"
#include "WaylandConnection.h"
#include "WaylandRegistry.h"
#include "common/Logger.h"

#include <wayland-client.h>
#include "ext-data-control-v1-client-protocol.h"
#include "wlr-data-control-unstable-v1-client-protocol.h"

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
    const QByteArray data = (mimeStr == "image/png") ? m_imagePng
                                                      : m_text.toUtf8();

    qint64 written = 0;
    while (written < data.size()) {
        ssize_t n = ::write(fd, data.constData() + written,
                            static_cast<size_t>(data.size() - written));
        if (n < 0) {
            if (errno == EINTR) continue;
            LOG_WARN("ClipboardWriter: write to paste fd failed");
            break;
        }
        written += n;
    }
    ::close(fd);
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

