# Phase 2 — Runtime architecture

Single binary, two roles.

## Daemon role

- Wayland monitor
- SQLite
- IPC server
- popup controller

## Popup role

- sends IPC
- shows UI

## Flow

```text
compositor shortcut
 -> qlippy --popup
 -> IPC to daemon
 -> daemon shows popup
```
