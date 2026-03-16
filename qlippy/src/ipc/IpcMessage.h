#pragma once

#include <QString>
#include <QJsonObject>

// Commands (client -> server)
namespace IpcCmd {
inline constexpr char Ping[]          = "ping";
inline constexpr char ShowPopup[]     = "show_popup";
inline constexpr char TogglePopup[]   = "toggle_popup";
inline constexpr char HidePopup[]     = "hide_popup";
inline constexpr char ListItems[]     = "list_items";
inline constexpr char SearchItems[]   = "search_items";
inline constexpr char CopyItem[]      = "copy_item";
inline constexpr char DeleteItem[]    = "delete_item";
inline constexpr char PinItem[]       = "pin_item";
inline constexpr char ClearHistory[]  = "clear_history";
inline constexpr char GetStatus[]     = "get_status";
inline constexpr char StopDaemon[]    = "stop_daemon";
} // namespace IpcCmd

struct IpcMessage {
    // request fields
    QString     cmd;
    QJsonObject params;

    // response fields
    bool        ok    = true;
    QString     error;
    QJsonObject data;

    // Factories
    static IpcMessage request(const QString& cmd, QJsonObject params = {});
    static IpcMessage success(QJsonObject data = {});
    static IpcMessage failure(const QString& error);

    bool isRequest()  const { return !cmd.isEmpty(); }
    bool isResponse() const { return cmd.isEmpty();  }
};
