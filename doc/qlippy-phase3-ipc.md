# Phase 3 — IPC layer

Use QLocalServer / QLocalSocket.

## Server name

```text
qlippy-ipc-v1
```

## Protocol

JSON + length prefix.

## Commands

```text
ping
show_popup
toggle_popup
hide_popup
list_items
copy_item
delete_item
pin_item
clear_history
get_status
```

## Example request

```json
{ "cmd": "show_popup" }
```

## Example response

```json
{ "ok": true }
```

## Events later

```text
history_changed
popup_shown
popup_hidden
```

## Required classes

```text
IpcServer
IpcClient
IpcMessage
JsonCodec
```
