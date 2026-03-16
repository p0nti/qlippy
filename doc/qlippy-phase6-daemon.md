# Phase 6 — Clipboard write back and daemon

## Clipboard write back

Use QClipboard only for writing.

### Class

```text
ClipboardWriter
```

### Methods

```text
setText
setImage
```

Never use QClipboard for monitoring.

## Daemon

### Class

```text
DaemonApp
```

### Responsibilities

- start Wayland
- start storage
- start IPC
- start popup controller

### On clipboard change

```text
store item
notify UI
```

### On IPC

```text
show_popup
copy_item
delete_item
list_items
```
