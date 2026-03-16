# qlippy

qlippy is a Wayland clipboard manager built with Qt 6 + QML.
It runs as a background daemon, captures clipboard history, and exposes a popup UI for search, preview, pinning, copy/delete actions, and runtime behavior tuning.

## What it does

- Runs a long-lived daemon that watches clipboard changes on Wayland.
- Stores clipboard history in SQLite with metadata and payload tables.
- Exposes a local IPC API over a Unix local socket for CLI and popup control.
- Provides a Qt Quick popup UI with:
  - Search and keyboard-first navigation
  - Pin/unpin and delete actions
  - Expand/collapse text and image previews
  - An options slider for live settings (theme, layout, opacity, behavior, max history)
- Supports command-line operations for list/search/copy/delete/pin/clear/toggle popup/stop daemon.

## Repository layout

- `qlippy/`: Main C++/Qt project
- `doc/`: Design notes, phased implementation docs, and options specification
- `doc/Wayland Clipboard Manager Options.html`: Captured option UI reference

Inside `qlippy/`:

- `src/app/`: Entry point, daemon orchestration, CLI handling
- `src/platform/`: Wayland integration and clipboard capture/write
- `src/ipc/`: Local IPC protocol and server/client transport
- `src/storage/`: SQLite schema, repository, and settings backend
- `src/ui/`: Popup controller and Qt model bridge for QML
- `qml/`: Popup UI and reusable QML components
- `docs/qlippy.service`: user-level systemd service file installed with the app
- `tests/`: test targets

## Requirements

- Linux with Wayland session
- CMake >= 3.22
- C++20 compiler
- Qt 6 dev packages: Core, Gui, Qml, Quick, QuickControls2, Network, Sql
- pkg-config
- wayland-client
- wayland-protocols
- SQLite Qt driver (QSQLITE)

## Build

From repository root:

```bash
cmake -S qlippy -B qlippy/build -DCMAKE_BUILD_TYPE=Release
cmake --build qlippy/build --target qlippy -j$(nproc)
```

Executable path:

```bash
qlippy/build/src/app/qlippy
```

## Run modes

Start daemon:

```bash
qlippy/build/src/app/qlippy --daemon
```

Show popup:

```bash
qlippy/build/src/app/qlippy --popup
```

Toggle popup:

```bash
qlippy/build/src/app/qlippy --toggle
```

## CLI reference

```text
--daemon                Run as daemon
--popup                 Show popup
--toggle                Toggle popup
--list                  List history (JSON)
--search <query>        Search history (JSON)
--copy <id>             Copy entry to clipboard
--delete <id>           Delete entry
--pin <id>              Pin entry
--unpin <id>            Unpin entry
--clear                 Clear history
--stop                  Stop daemon
--verbose               Debug logging
```

## Data and runtime paths

- Database: `~/.local/share/qlippy/db.sqlite`
- Logs: `~/.local/state/qlippy/log.txt`
- IPC socket name: `qlippy-ipc-v1`
  - Socket path is resolved under `$XDG_RUNTIME_DIR` (fallbacks apply)

## Settings model (runtime, persisted)

Settings are persisted in the SQLite `settings` table and exposed live to QML:

- `layout`: `compact | normal | big`
- `theme`: `nord | catppuccin`
- `opacity`: 0.6 to 1.0
- `expand_mode`: enable expanded preview mode
- `compact_image_expand`: image fit mode behavior
- `max_history`: max retained entries
- `save_images`: persist image payloads
- `dedupe`: deduplicate repeated content

## systemd user service

Install target includes:

- Service file: `docs/qlippy.service`
- Install destination: `${CMAKE_INSTALL_DATAROOTDIR}/systemd/user`

Typical user flow:

```bash
cmake --install qlippy/build --prefix ~/.local
systemctl --user daemon-reload
systemctl --user enable --now qlippy.service
```

## Development notes

- The popup UI is loaded from Qt resources (`qml/qlippy.qrc`).
- After QML/source edits, rebuild and restart daemon to load updated resources.
- The project currently uses a single executable for daemon + client commands.

## Quick troubleshooting

- Popup does not open:
  - Ensure daemon is running (`qlippy --daemon`)
  - Check logs in `~/.local/state/qlippy/log.txt`
- CLI cannot connect:
  - Verify runtime socket exists under `$XDG_RUNTIME_DIR`
  - Restart daemon
- No history persisted:
  - Verify `~/.local/share/qlippy/db.sqlite` exists and is writable
