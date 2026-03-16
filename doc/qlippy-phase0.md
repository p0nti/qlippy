# Phase 0 — Goal

Build a Wayland-native clipboard manager similar to Ditto but modern, with:

- C++ backend
- QML UI
- SQLite storage
- Wayland protocol access (NOT QClipboard for capture)
- IPC between daemon and UI
- popup triggered by compositor shortcut
- single binary, multi-mode

Supported in v1:

- text clipboard
- PNG images
- history search
- pin items
- delete items
- copy back to clipboard
- max history limit
- dedupe

Not required in v1:

- primary selection
- global shortcut API
- portals
- flatpak
- HTML preview UI
- multi-seat
- tray integration
