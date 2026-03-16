# Phase 1 — Repository layout

```text
qlippy/
  CMakeLists.txt
  cmake/
  protocols/
  qml/
  src/
    app/
    ipc/
    platform/
    storage/
    ui/
    common/
  tests/
  docs/
```

Targets:

- qlippy (single binary)

Modes:

```bash
qlippy --daemon
qlippy --popup
qlippy --toggle
qlippy --copy <id>
qlippy --delete <id>
qlippy --clear
```

Dependencies:

- Qt6 Core
- Qt6 Gui
- Qt6 Qml
- Qt6 Quick
- Qt6 QuickControls2
- Qt6 Network
- Qt6 Sql
- Wayland client
- wayland-protocols
- sqlite3

C++20 required.

Rules:

- no UI code in platform layer
- no raw owning pointers
- RAII only
- storage isolated
- IPC isolated
- protocol isolated
