#pragma once

#include "ipc/IpcMessage.h"
#include "storage/Item.h"
#include <QObject>
#include <memory>

class Storage;
class HistoryRepository;
class Settings;
class IpcServer;
class QLocalSocket;
class WaylandConnection;
class WaylandRegistry;
class WaylandClipboardMonitor;
class ClipboardWriter;
class PopupController;

// Owns all daemon subsystems and wires them together.
// Created in main() when --daemon is passed.
class DaemonApp : public QObject {
    Q_OBJECT
public:
    explicit DaemonApp(QObject* parent = nullptr);
    ~DaemonApp() override;

    // Start all subsystems. Returns false if a required subsystem fails.
    bool start();

    // Graceful shutdown (called from SIGTERM / QCoreApplication::quit).
    void stop();

signals:
    void startupFailed(QString reason);

private slots:
    void onCommandReceived(IpcMessage msg, QLocalSocket* conn);
    void onClipboardChanged(Item item, Payload payload);

private:
    void handlePing(QLocalSocket* conn);
    void handleListItems(QLocalSocket* conn);
    void handleSearchItems(const IpcMessage& msg, QLocalSocket* conn);
    void handleCopyItem(const IpcMessage& msg, QLocalSocket* conn);
    void handleDeleteItem(const IpcMessage& msg, QLocalSocket* conn);
    void handlePinItem(const IpcMessage& msg, QLocalSocket* conn);
    void handleClearHistory(QLocalSocket* conn);
    void handleGetStatus(QLocalSocket* conn);

    // Subsystems (owned)
    std::unique_ptr<Storage>           m_storage;
    std::unique_ptr<HistoryRepository> m_repo;
    std::unique_ptr<Settings>          m_settings;
    std::unique_ptr<IpcServer>         m_ipcServer;
    std::unique_ptr<WaylandConnection>        m_waylandConn;
    std::unique_ptr<WaylandRegistry>          m_registry;
    std::unique_ptr<WaylandClipboardMonitor>  m_monitor;
    std::unique_ptr<ClipboardWriter>           m_writer;
    std::unique_ptr<PopupController>           m_popup;
};
