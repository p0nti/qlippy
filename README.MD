# qlippy

qlippy is a Wayland clipboard manager built with Qt 6 + QML.
It runs as a background daemon, captures clipboard history, and exposes a popup UI for search, preview, pinning, copy/delete actions, and runtime behavior tuning.

## What it does

- Runs a long-lived daemon that watches clipboard changes on Wayland.
- Stores clipboard history in SQLite with metadata and payload tables.
- Exposes a local IPC API over a Unix socket for CLI and popup control.
- Provides a Qt Quick popup UI with:
  - Search and keyboard-first navigation
  - Pin/unpin and delete actions
  - Expand/collapse text and image previews
  - An options slider for live settings (theme, layout, opacity, behavior, max history)
- Supports command-line operations for list/search/copy/delete/pin/clear/toggle popup/stop daemon.

## Repository layout

- `qlippy/`: Main C++/Qt project

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

A convenience script is provided:

```bash
qlippy/build.sh
```

Or manually from repository root:

```bash
cmake -S qlippy -B qlippy/build
cmake --build qlippy/build
```

A VS Code task is also available: **Build qlippy (build.sh)** — kills any running instance, builds, and launches the daemon.

Executable path after build:

```
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
--daemon                Run as background daemon
--popup                 Show clipboard popup
--toggle                Toggle clipboard popup
--list                  List history (JSON)
--search <query>        Search history (JSON)
--copy <id>             Copy entry to clipboard
--delete <id>           Delete entry
--pin <id>              Pin entry
--unpin <id>            Unpin entry
--clear                 Clear all history
--stop                  Stop the running daemon
--verbose               Enable debug logging
```

## Data and runtime paths

- Database: `~/.local/share/qlippy/db.sqlite`
- Logs: `~/.local/state/qlippy/log.txt`
- IPC socket: `$XDG_RUNTIME_DIR/qlippy-ipc-v1`
  - Falls back to Qt RuntimeLocation, then `/tmp` if `$XDG_RUNTIME_DIR` is unset

## Settings model (runtime, persisted)

Settings are persisted in the SQLite `settings` table and exposed live to QML:

- `layout`: `compact | normal | big`
- `theme`: `teal | catppuccin | system-dark | system-light`
- `opacity`: `0.6` to `1.0`
- `expand_mode`: enable expanded preview mode
- `compact_image_expand`: image fit mode in compact layout
- `max_history`: max retained entries (default: 500)
- `save_images`: persist image payloads
- `dedupe`: deduplicate repeated content
- `allow_deletion_items`: allow items to be deleted from history

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
- After QML or source edits, rebuild and restart the daemon to load updated resources.
- The project uses a single executable for both daemon and client commands.
- The VS Code task (`.vscode/tasks.json`) handles kill, build, and launch in one step.

## Quick troubleshooting

- Popup does not open:
  - Ensure daemon is running: `qlippy/build/src/app/qlippy --daemon`
  - Check logs: `~/.local/state/qlippy/log.txt`
- CLI cannot connect:
  - Verify socket exists: `ls $XDG_RUNTIME_DIR/qlippy-ipc-v1`
  - Restart the daemon
- No history persisted:
  - Verify `~/.local/share/qlippy/db.sqlite` exists and is writable
