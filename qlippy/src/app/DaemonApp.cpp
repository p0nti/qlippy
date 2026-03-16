#include "DaemonApp.h"

#include "ipc/IpcServer.h"
#include "ipc/IpcMessage.h"
#include "storage/Storage.h"
#include "storage/HistoryRepository.h"
#include "storage/Settings.h"
#include "storage/Item.h"
#include "common/Logger.h"
#include "platform/WaylandConnection.h"
#include "platform/WaylandRegistry.h"
#include "platform/WaylandClipboardMonitor.h"
#include "ui/PopupController.h"
#include "platform/ClipboardWriter.h"

#include <QLocalSocket>
#include <QJsonArray>
#include <QJsonObject>
#include <QCoreApplication>
#include <QGuiApplication>

DaemonApp::DaemonApp(QObject* parent)
    : QObject(parent)
    , m_storage(std::make_unique<Storage>())
    , m_ipcServer(std::make_unique<IpcServer>())
    , m_waylandConn(std::make_unique<WaylandConnection>())
{}

DaemonApp::~DaemonApp()
{
    stop();
}

// ---------------------------------------------------------------------------
// start / stop
// ---------------------------------------------------------------------------
bool DaemonApp::start()
{
    LOG_INFO("DaemonApp: starting");

    // Storage
    if (!m_storage->open()) {
        const QString err = "Storage failed to open: " + m_storage->lastError();
        LOG_ERROR(err);
        emit startupFailed(err);
        return false;
    }
    LOG_INFO("DaemonApp: storage open at " + Storage::defaultPath());

    m_repo     = std::make_unique<HistoryRepository>(m_storage.get());
    m_settings = std::make_unique<Settings>(m_storage.get());

    connect(m_settings.get(), &Settings::maxHistoryChanged, this, [this]() {
        const int pruned = m_repo->prune(m_settings->maxHistory());
        if (pruned > 0)
            LOG_DEBUG("DaemonApp: pruned " + QString::number(pruned) + " old items after max_history change");
        if (m_popup)
            m_popup->refresh();
    });

    // IPC
    connect(m_ipcServer.get(), &IpcServer::commandReceived,
            this,              &DaemonApp::onCommandReceived);

    if (!m_ipcServer->start()) {
        const QString err = "IPC server failed to start on socket: " +
                            QString(IpcServer::kSocketName);
        LOG_ERROR(err);
        emit startupFailed(err);
        return false;
    }
    LOG_INFO("DaemonApp: IPC listening on " + QString(IpcServer::kSocketName));

    // Popup controller lives in daemon and is shown/hidden via IPC.
    if (qobject_cast<QGuiApplication*>(QCoreApplication::instance())) {
        m_popup = std::make_unique<PopupController>(m_repo.get(), m_settings.get());
        connect(m_popup.get(), &PopupController::copyRequested, this, [this](qint64 id) {
            IpcMessage msg = IpcMessage::request(IpcCmd::CopyItem, {{"id", id}});
            handleCopyItem(msg, nullptr);
        });
        if (!m_popup->start()) {
            LOG_WARN("DaemonApp: failed to initialize popup UI");
            m_popup.reset();
        }
    } else {
        LOG_WARN("DaemonApp: popup UI disabled (requires QGuiApplication)");
    }

    // Wayland monitor (not fatal if unavailable — e.g. running headlessly)
    if (m_waylandConn->connectToDisplay()) {
        m_registry = std::make_unique<WaylandRegistry>(m_waylandConn.get());
        if (m_registry->bind()) {
            m_monitor = std::make_unique<WaylandClipboardMonitor>(
                m_waylandConn.get(), m_registry.get());
            connect(m_monitor.get(), &WaylandClipboardMonitor::clipboardChanged,
                    this,            &DaemonApp::onClipboardChanged);
            if (!m_monitor->start())
                LOG_WARN("DaemonApp: clipboard monitor failed to start");

            // Clipboard writer shares the Wayland connection + registry
            m_writer = std::make_unique<ClipboardWriter>(
                m_waylandConn.get(), m_registry.get());
        } else {
            LOG_WARN("DaemonApp: no data-control protocol, clipboard monitoring disabled");
        }
    } else {
        LOG_WARN("DaemonApp: no Wayland display, clipboard monitoring disabled");
    }

    return true;
}

void DaemonApp::stop()
{
    LOG_INFO("DaemonApp: stopping");
    if (m_popup)        { m_popup.reset(); }
    if (m_writer)       { m_writer.reset(); }
    if (m_monitor)      { m_monitor.reset(); }
    if (m_registry)     { m_registry.reset(); }
    if (m_waylandConn)  { m_waylandConn->disconnectFromDisplay(); }
    if (m_ipcServer) m_ipcServer->stop();
    if (m_storage)   m_storage->close();
}

// ---------------------------------------------------------------------------
// IPC dispatch
// ---------------------------------------------------------------------------
void DaemonApp::onCommandReceived(IpcMessage msg, QLocalSocket* conn)
{
    LOG_DEBUG("IPC command: " + msg.cmd);

    if      (msg.cmd == IpcCmd::Ping)         handlePing(conn);
    else if (msg.cmd == IpcCmd::ListItems)    handleListItems(conn);
    else if (msg.cmd == IpcCmd::SearchItems)  handleSearchItems(msg, conn);
    else if (msg.cmd == IpcCmd::CopyItem)     handleCopyItem(msg, conn);
    else if (msg.cmd == IpcCmd::DeleteItem)   handleDeleteItem(msg, conn);
    else if (msg.cmd == IpcCmd::PinItem)      handlePinItem(msg, conn);
    else if (msg.cmd == IpcCmd::ClearHistory) handleClearHistory(conn);
    else if (msg.cmd == IpcCmd::GetStatus)    handleGetStatus(conn);
    else if (msg.cmd == IpcCmd::ShowPopup) {
        if (!m_popup) {
            m_ipcServer->reply(conn, IpcMessage::failure("popup UI unavailable"));
            return;
        }
        m_popup->show();
        m_ipcServer->reply(conn, IpcMessage::success());
    }
    else if (msg.cmd == IpcCmd::TogglePopup) {
        if (!m_popup) {
            m_ipcServer->reply(conn, IpcMessage::failure("popup UI unavailable"));
            return;
        }
        m_popup->toggle();
        m_ipcServer->reply(conn, IpcMessage::success());
    }
    else if (msg.cmd == IpcCmd::HidePopup) {
        if (!m_popup) {
            m_ipcServer->reply(conn, IpcMessage::failure("popup UI unavailable"));
            return;
        }
        m_popup->hide();
        m_ipcServer->reply(conn, IpcMessage::success());
    }
    else if (msg.cmd == IpcCmd::StopDaemon) {
        m_ipcServer->reply(conn, IpcMessage::success());
        LOG_INFO("DaemonApp: stop requested via IPC");
        QCoreApplication::quit();
    }
    else {
        m_ipcServer->reply(conn, IpcMessage::failure("unknown command: " + msg.cmd));
    }
}

// ---------------------------------------------------------------------------
// Handlers
// ---------------------------------------------------------------------------
void DaemonApp::handlePing(QLocalSocket* conn)
{
    m_ipcServer->reply(conn, IpcMessage::success({{"pong", true}}));
}

void DaemonApp::handleGetStatus(QLocalSocket* conn)
{
    QJsonObject data;
    data["storage_open"] = m_storage->isOpen();
    data["ipc_listening"] = m_ipcServer->isListening();
    m_ipcServer->reply(conn, IpcMessage::success(data));
}

void DaemonApp::handleListItems(QLocalSocket* conn)
{
    const int limit = m_settings->maxHistory();
    const QList<Item> items = m_repo->fetchRecent(limit);

    QJsonArray arr;
    for (const Item& it : items) {
        QJsonObject obj;
        obj["id"]          = it.id;
        obj["kind"]        = it.kind == ItemKind::Image ? "image" : "text";
        obj["preview"]     = it.textPreview;
        obj["timestamp"]   = it.createdAt.toSecsSinceEpoch();
        obj["pinned"]      = it.isPinned;
        obj["byte_size"]   = it.byteSize;
        arr.append(obj);
    }
    m_ipcServer->reply(conn, IpcMessage::success({{"items", arr}}));
}

void DaemonApp::handleSearchItems(const IpcMessage& msg, QLocalSocket* conn)
{
    const QString needle = msg.params["q"].toString().trimmed();
    if (needle.isEmpty()) {
        handleListItems(conn);
        return;
    }

    const int limit = m_settings->maxHistory();
    const QList<Item> items = m_repo->searchRecent(needle, limit);

    QJsonArray arr;
    for (const Item& it : items) {
        QJsonObject obj;
        obj["id"]          = it.id;
        obj["kind"]        = it.kind == ItemKind::Image ? "image" : "text";
        obj["preview"]     = it.textPreview;
        obj["timestamp"]   = it.createdAt.toSecsSinceEpoch();
        obj["pinned"]      = it.isPinned;
        obj["byte_size"]   = it.byteSize;
        arr.append(obj);
    }
    m_ipcServer->reply(conn, IpcMessage::success({{"items", arr}, {"query", needle}}));
}

void DaemonApp::handleCopyItem(const IpcMessage& msg, QLocalSocket* conn)
{
    const qint64 id = static_cast<qint64>(msg.params["id"].toDouble());
    if (id <= 0) {
        m_ipcServer->reply(conn, IpcMessage::failure("missing or invalid id"));
        return;
    }

    auto payload = m_repo->fetchPayload(id);
    if (!payload.has_value()) {
        m_ipcServer->reply(conn, IpcMessage::failure("item not found"));
        return;
    }

    m_repo->touchCopied(id);

    // Write the content back to the Wayland clipboard
    if (m_writer) {
        if (!payload->textPlain.isEmpty()) {
            m_writer->setText(payload->textPlain);
            LOG_INFO(QStringLiteral("copy_item %1: wrote text (%2 chars) to clipboard")
                     .arg(id).arg(payload->textPlain.size()));
        } else if (!payload->imagePng.isEmpty()) {
            m_writer->setImage(payload->imagePng);
            LOG_INFO(QStringLiteral("copy_item %1: wrote image (%2 bytes) to clipboard")
                     .arg(id).arg(payload->imagePng.size()));
        } else {
            LOG_WARN(QStringLiteral("copy_item %1: payload has no writable content").arg(id));
        }
    }

    if (m_popup)
        m_popup->refresh();

    if (conn)
        m_ipcServer->reply(conn, IpcMessage::success());
}

void DaemonApp::handleDeleteItem(const IpcMessage& msg, QLocalSocket* conn)
{
    const qint64 id = static_cast<qint64>(msg.params["id"].toDouble());
    if (id <= 0) {
        m_ipcServer->reply(conn, IpcMessage::failure("missing or invalid id"));
        return;
    }

    if (!m_repo->remove(id)) {
        m_ipcServer->reply(conn, IpcMessage::failure(m_repo->lastError()));
        return;
    }
    if (m_popup)
        m_popup->refresh();
    m_ipcServer->reply(conn, IpcMessage::success());
}

void DaemonApp::handlePinItem(const IpcMessage& msg, QLocalSocket* conn)
{
    const qint64 id     = static_cast<qint64>(msg.params["id"].toDouble());
    const bool   pinned = msg.params["pinned"].toBool(true);

    if (id <= 0) {
        m_ipcServer->reply(conn, IpcMessage::failure("missing or invalid id"));
        return;
    }

    if (!m_repo->setPin(id, pinned)) {
        m_ipcServer->reply(conn, IpcMessage::failure(m_repo->lastError()));
        return;
    }
    if (m_popup)
        m_popup->refresh();
    m_ipcServer->reply(conn, IpcMessage::success());
}

void DaemonApp::handleClearHistory(QLocalSocket* conn)
{
    // Prune all non-pinned by capping to 0
    m_repo->prune(0);
    if (m_popup)
        m_popup->refresh();
    m_ipcServer->reply(conn, IpcMessage::success());
}

// ---------------------------------------------------------------------------
// Clipboard monitor slot
// ---------------------------------------------------------------------------
void DaemonApp::onClipboardChanged(Item item, Payload payload)
{
    if (m_settings->dedupe()) {
        const qint64 existing = m_repo->findDuplicate(item.sha256);
        if (existing > 0) {
            LOG_DEBUG("DaemonApp: dedup hit, updating last_copied_at for id=" +
                      QString::number(existing));
            m_repo->touchCopied(existing);
            return;
        }
    }

    if (!m_settings->saveImages() && item.kind == ItemKind::Image) {
        LOG_DEBUG("DaemonApp: image capture disabled, skipping");
        return;
    }

    const qint64 newId = m_repo->insert(item, payload);
    if (newId <= 0) {
        LOG_WARN("DaemonApp: failed to insert clipboard item: " + m_repo->lastError());
        return;
    }

    LOG_INFO(QStringLiteral("DaemonApp: stored item id=%1  kind=%2  bytes=%3")
             .arg(newId)
             .arg(item.kind == ItemKind::Image ? "image" : "text")
             .arg(item.byteSize));

    // Prune to max_history after insert
    const int pruned = m_repo->prune(m_settings->maxHistory());
    if (pruned > 0)
        LOG_DEBUG("DaemonApp: pruned " + QString::number(pruned) + " old items (count)");

    // Prune to max_bytes after insert
    const qint64 maxBytes = m_settings->maxBytes();
    if (maxBytes > 0) {
        const int prunedBytes = m_repo->pruneToBytes(maxBytes);
        if (prunedBytes > 0)
            LOG_DEBUG("DaemonApp: pruned " + QString::number(prunedBytes) + " old items (bytes)");
    }

    if (m_popup)
        m_popup->refresh();
}
