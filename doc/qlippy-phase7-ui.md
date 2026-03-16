# Phase 7 — Popup UI and popup control

## Popup UI

Qt Quick

### Window

```text
frameless
always on top
centered
```

### Model

```text
ClipboardModel : QAbstractListModel
```

### Roles

```text
id
preview
timestamp
pinned
type
```

### Features

- search
- keyboard navigation
- enter = copy
- del = delete
- p = pin

### QML files

```text
Popup.qml
ItemDelegate.qml
SearchBox.qml
Preview.qml
```

Popup controlled from daemon.

## Popup control

### Class

```text
PopupController
```

### Functions

```text
show()
hide()
toggle()
```

Popup created once.

Hidden when not used.
