#include "WaylandClipboardMonitor.h"
#include "WaylandConnection.h"
#include "WaylandRegistry.h"
#include "common/Logger.h"

#include <wayland-client.h>
#include "ext-data-control-v1-client-protocol.h"
#include "wlr-data-control-unstable-v1-client-protocol.h"

#include <QSocketNotifier>
#include <QCryptographicHash>
#include <QDateTime>

#include <unistd.h>
#include <fcntl.h>

// ---------------------------------------------------------------------------
// Internal offer state — heap-allocated per offer, freed after selection.
// ---------------------------------------------------------------------------
struct OfferData {
    WaylandClipboardMonitor* monitor = nullptr;
    QStringList              mimeTypes;
    // Set when selection fires:
    std::function<void(const char* mime, int fd)> receive;
    std::function<void()>                         destroy;
};

// ---------------------------------------------------------------------------
// MIME priority
// ---------------------------------------------------------------------------
static QString pickMime(const QStringList& types)
{
    static const QStringList kPriority = {
        "text/plain;charset=utf-8",
        "text/plain",
        "image/png",
    };
    for (const QString& p : kPriority) {
        if (types.contains(p, Qt::CaseInsensitive))
            return p;
    }
    return {};
}

// ===========================================================================
// ext-data-control-v1 listeners
// ===========================================================================

// --- offer listener ---------------------------------------------------------
static void ext_offer_mime(void* data, ext_data_control_offer_v1*, const char* mime)
{
    static_cast<OfferData*>(data)->mimeTypes << QString::fromUtf8(mime);
}

static const ext_data_control_offer_v1_listener s_extOfferListener = {
    .offer = ext_offer_mime,
};

// --- device listener --------------------------------------------------------
static void ext_device_data_offer(void* data, ext_data_control_device_v1*,
                                   ext_data_control_offer_v1* offer)
{
    auto* monitor = static_cast<WaylandClipboardMonitor*>(data);
    auto* state  = new OfferData{ monitor, {} };
    ext_data_control_offer_v1_add_listener(offer, &s_extOfferListener, state);
}

static void ext_device_selection(void* data, ext_data_control_device_v1*,
                                  ext_data_control_offer_v1* offer)
{
    auto* monitor = static_cast<WaylandClipboardMonitor*>(data);
    if (!offer) return;

    auto* state = static_cast<OfferData*>(ext_data_control_offer_v1_get_user_data(offer));
    state->receive = [offer](const char* mime, int fd) {
        ext_data_control_offer_v1_receive(offer, mime, fd);
    };
    state->destroy = [offer, state]() {
        ext_data_control_offer_v1_destroy(offer);
        delete state;
    };
    monitor->onSelection(state);
}

static void ext_device_finished(void* /*data*/, ext_data_control_device_v1* device)
{
    ext_data_control_device_v1_destroy(device);
}

static void ext_device_primary_selection(void* /*data*/, ext_data_control_device_v1*,
                                          ext_data_control_offer_v1* offer)
{
    if (offer) ext_data_control_offer_v1_destroy(offer);
}

static const ext_data_control_device_v1_listener s_extDeviceListener = {
    .data_offer        = ext_device_data_offer,
    .selection         = ext_device_selection,
    .finished          = ext_device_finished,
    .primary_selection = ext_device_primary_selection,
};

// ===========================================================================
// wlr-data-control-unstable-v1 listeners
// ===========================================================================

// --- offer listener ---------------------------------------------------------
static void wlr_offer_mime(void* data, zwlr_data_control_offer_v1*, const char* mime)
{
    static_cast<OfferData*>(data)->mimeTypes << QString::fromUtf8(mime);
}

static const zwlr_data_control_offer_v1_listener s_wlrOfferListener = {
    .offer = wlr_offer_mime,
};

// --- device listener --------------------------------------------------------
static void wlr_device_data_offer(void* data, zwlr_data_control_device_v1*,
                                   zwlr_data_control_offer_v1* offer)
{
    auto* monitor = static_cast<WaylandClipboardMonitor*>(data);
    auto* state   = new OfferData{ monitor, {} };
    zwlr_data_control_offer_v1_add_listener(offer, &s_wlrOfferListener, state);
}

static void wlr_device_selection(void* data, zwlr_data_control_device_v1*,
                                  zwlr_data_control_offer_v1* offer)
{
    auto* monitor = static_cast<WaylandClipboardMonitor*>(data);
    if (!offer) return;

    auto* state = static_cast<OfferData*>(zwlr_data_control_offer_v1_get_user_data(offer));
    state->receive = [offer](const char* mime, int fd) {
        zwlr_data_control_offer_v1_receive(offer, mime, fd);
    };
    state->destroy = [offer, state]() {
        zwlr_data_control_offer_v1_destroy(offer);
        delete state;
    };
    monitor->onSelection(state);
}

static void wlr_device_finished(void* /*data*/, zwlr_data_control_device_v1* device)
{
    zwlr_data_control_device_v1_destroy(device);
}

static void wlr_device_primary_selection(void* /*data*/, zwlr_data_control_device_v1*,
                                          zwlr_data_control_offer_v1* offer)
{
    if (offer) zwlr_data_control_offer_v1_destroy(offer);
}

static const zwlr_data_control_device_v1_listener s_wlrDeviceListener = {
    .data_offer        = wlr_device_data_offer,
    .selection         = wlr_device_selection,
    .finished          = wlr_device_finished,
    .primary_selection = wlr_device_primary_selection,
};

// ===========================================================================
// WaylandClipboardMonitor implementation
// ===========================================================================
WaylandClipboardMonitor::WaylandClipboardMonitor(WaylandConnection* conn,
                                                  WaylandRegistry*   registry,
                                                  QObject*           parent)
    : QObject(parent)
    , m_conn(conn)
    , m_registry(registry)
{}

WaylandClipboardMonitor::~WaylandClipboardMonitor()
{
    cancelPendingRead();

    if (m_device) {
        if (m_useExt)
            ext_data_control_device_v1_destroy(static_cast<ext_data_control_device_v1*>(m_device));
        else
            zwlr_data_control_device_v1_destroy(static_cast<zwlr_data_control_device_v1*>(m_device));
        m_device = nullptr;
    }
}

bool WaylandClipboardMonitor::start()
{
    if (!m_registry->seat()) {
        LOG_ERROR("WaylandClipboardMonitor: no wl_seat available");
        return false;
    }

    if (m_registry->extManager()) {
        m_useExt = true;
        auto* dev = ext_data_control_manager_v1_get_data_device(
            m_registry->extManager(), m_registry->seat());
        ext_data_control_device_v1_add_listener(dev, &s_extDeviceListener, this);
        m_device = dev;
        LOG_INFO("WaylandClipboardMonitor: using ext-data-control-v1");
    } else if (m_registry->wlrManager()) {
        m_useExt = false;
        auto* dev = zwlr_data_control_manager_v1_get_data_device(
            m_registry->wlrManager(), m_registry->seat());
        zwlr_data_control_device_v1_add_listener(dev, &s_wlrDeviceListener, this);
        m_device = dev;
        LOG_INFO("WaylandClipboardMonitor: using wlr-data-control-unstable-v1");
    } else {
        LOG_ERROR("WaylandClipboardMonitor: no data-control protocol");
        return false;
    }

    m_conn->flush();
    return true;
}

// ---------------------------------------------------------------------------
// Called from C listeners
// ---------------------------------------------------------------------------
void WaylandClipboardMonitor::onDataOffer(void* /*offer*/)
{
    // Nothing to do here — the listener already attached OfferData to the offer.
}

void WaylandClipboardMonitor::onOfferMime(void* offerState, const char* mime)
{
    static_cast<OfferData*>(offerState)->mimeTypes << QString::fromUtf8(mime);
}

void WaylandClipboardMonitor::onSelection(void* rawOfferState)
{
    processOffer(rawOfferState);
}

// ---------------------------------------------------------------------------
// Core: pick MIME, create pipe, request receive, set up async read
// ---------------------------------------------------------------------------
void WaylandClipboardMonitor::processOffer(void* rawOfferState)
{
    auto* state = static_cast<OfferData*>(rawOfferState);

    const QString mime = pickMime(state->mimeTypes);
    if (mime.isEmpty()) {
        LOG_DEBUG("WaylandClipboardMonitor: no supported MIME type, ignoring offer ("
                  + state->mimeTypes.join(", ") + ")");
        state->destroy();
        return;
    }

    // Cancel any previous in-progress read
    cancelPendingRead();

    int fds[2];
    if (pipe2(fds, O_CLOEXEC) < 0) {
        LOG_ERROR("WaylandClipboardMonitor: pipe2 failed");
        state->destroy();
        return;
    }

    // Ask compositor to write data to the write end of the pipe
    state->receive(mime.toUtf8().constData(), fds[1]);
    ::close(fds[1]);

    // Flush so the receive request reaches the compositor now
    m_conn->flush();

    m_currentMime       = mime;
    m_currentOfferState = state;

    // Read from the read end asynchronously
    m_pipeNotifier = new QSocketNotifier(fds[0], QSocketNotifier::Read, this);
    connect(m_pipeNotifier, &QSocketNotifier::activated,
            this,           &WaylandClipboardMonitor::onPipeReadable);
}

// ---------------------------------------------------------------------------
// Pipe read slot — accumulates data until EOF, then emits clipboardChanged
// ---------------------------------------------------------------------------
void WaylandClipboardMonitor::onPipeReadable()
{
    const int fd = m_pipeNotifier->socket();

    char buf[65536];
    const ssize_t n = ::read(fd, buf, sizeof(buf));

    if (n > 0) {
        m_pipeBuffer.append(buf, static_cast<int>(n));
        return; // wait for more data
    }

    // EOF (n == 0) or error (n < 0) — done reading
    m_pipeNotifier->setEnabled(false);
    m_pipeNotifier->deleteLater();
    m_pipeNotifier = nullptr;
    ::close(fd);

    if (m_currentOfferState) {
        static_cast<OfferData*>(m_currentOfferState)->destroy();
        m_currentOfferState = nullptr;
    }

    const QByteArray data = m_pipeBuffer;
    const QString    mime = m_currentMime;
    m_pipeBuffer.clear();
    m_currentMime.clear();

    if (n < 0 || data.isEmpty())
        return; // read error or empty clipboard

    // -----------------------------------------------------------------------
    // Build Item + Payload
    // -----------------------------------------------------------------------
    Item item;
    Payload payload;

    item.mimeTypes    = mime;
    item.byteSize     = data.size();
    item.createdAt    = QDateTime::currentDateTimeUtc();
    item.updatedAt    = item.createdAt;
    item.lastCopiedAt = item.createdAt;
    item.sha256       = QString::fromLatin1(
        QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());

    if (mime.startsWith("text/", Qt::CaseInsensitive)) {
        item.kind        = ItemKind::Text;
        const QString text = QString::fromUtf8(data);
        item.textPreview = text.trimmed().left(200);
        payload.textPlain = text;
    } else if (mime == "image/png") {
        item.kind        = ItemKind::Image;
        item.textPreview = QStringLiteral("[Image %1 bytes]").arg(data.size());
        payload.imagePng = data;
    } else {
        payload.rawPayload = data;
    }

    LOG_DEBUG(QStringLiteral("WaylandClipboardMonitor: captured %1  %2 bytes  sha256=%3")
              .arg(mime).arg(data.size()).arg(item.sha256.left(12)));

    emit clipboardChanged(item, payload);
}

// ---------------------------------------------------------------------------
// Cancel a pipe read that's in progress
// ---------------------------------------------------------------------------
void WaylandClipboardMonitor::cancelPendingRead()
{
    if (m_pipeNotifier) {
        const int fd = m_pipeNotifier->socket();
        m_pipeNotifier->setEnabled(false);
        m_pipeNotifier->deleteLater();
        m_pipeNotifier = nullptr;
        ::close(fd);
    }
    if (m_currentOfferState) {
        static_cast<OfferData*>(m_currentOfferState)->destroy();
        m_currentOfferState = nullptr;
    }
    m_pipeBuffer.clear();
    m_currentMime.clear();
}
