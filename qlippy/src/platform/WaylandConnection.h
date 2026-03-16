#pragma once

#include <QObject>

struct wl_display;
class QSocketNotifier;

// Owns the wl_display connection and integrates Wayland event dispatch
// with Qt's event loop via QSocketNotifier.
class WaylandConnection : public QObject {
    Q_OBJECT
public:
    explicit WaylandConnection(QObject* parent = nullptr);
    ~WaylandConnection() override;

    // Connect to $WAYLAND_DISPLAY. Returns false on failure.
    bool connectToDisplay();
    void disconnectFromDisplay();

    bool        isConnected() const { return m_display != nullptr; }
    wl_display* display()     const { return m_display; }

    // Send all queued requests to the compositor.
    void flush();

signals:
    void displayError();

private slots:
    void onDisplayReadable();

private:
    wl_display*      m_display  = nullptr;
    QSocketNotifier* m_notifier = nullptr;
};
