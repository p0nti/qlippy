#include "WaylandRegistry.h"
#include "WaylandConnection.h"
#include "common/Logger.h"

#include <wayland-client.h>
#include "ext-data-control-v1-client-protocol.h"
#include "wlr-data-control-unstable-v1-client-protocol.h"

#include <algorithm>
#include <cstring>

static const wl_registry_listener s_registryListener = {
    WaylandRegistry::onGlobal,
    WaylandRegistry::onGlobalRemove,
};

WaylandRegistry::WaylandRegistry(WaylandConnection* conn, QObject* parent)
    : QObject(parent)
    , m_conn(conn)
{}

WaylandRegistry::~WaylandRegistry()
{
    if (m_extManager) { ext_data_control_manager_v1_destroy(m_extManager);  m_extManager = nullptr; }
    if (m_wlrManager) { zwlr_data_control_manager_v1_destroy(m_wlrManager); m_wlrManager = nullptr; }
    if (m_seat)       { wl_seat_destroy(m_seat);                             m_seat       = nullptr; }
    if (m_registry)   { wl_registry_destroy(m_registry);                    m_registry   = nullptr; }
}

bool WaylandRegistry::bind()
{
    m_registry = wl_display_get_registry(m_conn->display());
    if (!m_registry) {
        LOG_ERROR("WaylandRegistry: wl_display_get_registry failed");
        return false;
    }

    wl_registry_add_listener(m_registry, &s_registryListener, this);
    wl_display_roundtrip(m_conn->display());

    if (!hasDataControl()) {
        LOG_ERROR("WaylandRegistry: neither ext-data-control-v1 nor wlr-data-control found");
        return false;
    }

    LOG_INFO(QString("WaylandRegistry: bound  seat=%1  ext=%2  wlr=%3")
             .arg(m_seat       ? "yes" : "no")
             .arg(m_extManager ? "yes" : "no")
             .arg(m_wlrManager ? "yes" : "no"));
    return true;
}

bool WaylandRegistry::hasDataControl() const
{
    return m_extManager != nullptr || m_wlrManager != nullptr;
}

void WaylandRegistry::onGlobal(void* data, wl_registry* reg,
                                uint32_t name, const char* iface, uint32_t version)
{
    auto* self = static_cast<WaylandRegistry*>(data);

    if (!std::strcmp(iface, wl_seat_interface.name) && !self->m_seat) {
        self->m_seat = static_cast<wl_seat*>(
            wl_registry_bind(reg, name, &wl_seat_interface, 1));
    }
    else if (!std::strcmp(iface, ext_data_control_manager_v1_interface.name)
             && !self->m_extManager) {
        const uint32_t v = std::min(version, 1u);
        self->m_extManager = static_cast<ext_data_control_manager_v1*>(
            wl_registry_bind(reg, name, &ext_data_control_manager_v1_interface, v));
    }
    else if (!std::strcmp(iface, zwlr_data_control_manager_v1_interface.name)
             && !self->m_wlrManager) {
        const uint32_t v = std::min(version, 2u);
        self->m_wlrManager = static_cast<zwlr_data_control_manager_v1*>(
            wl_registry_bind(reg, name, &zwlr_data_control_manager_v1_interface, v));
    }
}

void WaylandRegistry::onGlobalRemove(void* /*data*/, wl_registry* /*reg*/, uint32_t /*name*/)
{
    // Not handled in v1
}
