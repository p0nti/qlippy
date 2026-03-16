#include "WaylandConnection.h"
#include "common/Logger.h"

#include <wayland-client.h>
#include <QSocketNotifier>

WaylandConnection::WaylandConnection(QObject* parent)
    : QObject(parent)
{}

WaylandConnection::~WaylandConnection()
{
    disconnectFromDisplay();
}

bool WaylandConnection::connectToDisplay()
{
    m_display = wl_display_connect(nullptr);
    if (!m_display) {
        LOG_ERROR("WaylandConnection: failed to connect to Wayland display");
        return false;
    }

    const int fd = wl_display_get_fd(m_display);
    m_notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated,
            this,       &WaylandConnection::onDisplayReadable);

    LOG_INFO("WaylandConnection: connected to Wayland display");
    return true;
}

void WaylandConnection::disconnectFromDisplay()
{
    if (m_notifier) {
        m_notifier->setEnabled(false);
        m_notifier->deleteLater();
        m_notifier = nullptr;
    }
    if (m_display) {
        wl_display_disconnect(m_display);
        m_display = nullptr;
    }
}

void WaylandConnection::flush()
{
    if (m_display)
        wl_display_flush(m_display);
}

void WaylandConnection::onDisplayReadable()
{
    if (wl_display_dispatch(m_display) < 0) {
        LOG_ERROR("WaylandConnection: display error, disconnecting");
        disconnectFromDisplay();
        emit displayError();
        return;
    }
    wl_display_flush(m_display);
}
