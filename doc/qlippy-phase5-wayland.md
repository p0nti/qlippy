# Phase 5 — Wayland layer

## Use

- ext-data-control-v1
- fallback wlr-data-control

## Files

```text
WaylandConnection
WaylandRegistry
WaylandClipboardMonitor
WaylandOffer
```

## Capabilities

```text
extDataControl
wlrDataControl
primarySelection
```

## Monitor must

- detect selection
- read MIME list
- fetch payload
- emit item

## Priority

```text
text/plain;charset=utf-8
text/plain
image/png
```

## Normalize to

```text
TextItem
ImageItem
```

## Emit signal

```text
clipboardChanged(Item)
```

Daemon stores item.
