#pragma once

#include <QObject>
#include <cstdint>

struct wl_registry;
struct wl_seat;
struct ext_data_control_manager_v1;
struct zwlr_data_control_manager_v1;

class WaylandConnection;

// Binds well-known Wayland globals discovered via wl_registry.
// Prefers ext-data-control-v1; falls back to wlr-data-control-unstable-v1.
class WaylandRegistry : public QObject {
    Q_OBJECT
public:
    explicit WaylandRegistry(WaylandConnection* conn, QObject* parent = nullptr);
    ~WaylandRegistry() override;

    // Performs a roundtrip to discover all current globals.
    bool bind();

    wl_seat*                      seat()       const { return m_seat;       }
    ext_data_control_manager_v1*  extManager() const { return m_extManager; }
    zwlr_data_control_manager_v1* wlrManager() const { return m_wlrManager; }

    // Returns true if at least one data-control manager was bound.
    bool hasDataControl() const;

public:
    static void onGlobal(void* data, wl_registry* reg,
                         uint32_t name, const char* iface, uint32_t version);
    static void onGlobalRemove(void* data, wl_registry* reg, uint32_t name);

private:
    WaylandConnection*            m_conn       = nullptr;
    wl_registry*                  m_registry   = nullptr;
    wl_seat*                      m_seat       = nullptr;
    ext_data_control_manager_v1*  m_extManager = nullptr;
    zwlr_data_control_manager_v1* m_wlrManager = nullptr;
};
