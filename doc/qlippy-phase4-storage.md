# Phase 4 — Storage

## SQLite file

```text
~/.local/share/qlippy/db.sqlite
```

## Schema

### clipboard_items

```sql
id INTEGER PRIMARY KEY
created_at INTEGER
updated_at INTEGER
kind TEXT
text_preview TEXT
mime_types TEXT
sha256 TEXT
is_pinned INTEGER
byte_size INTEGER
last_copied_at INTEGER
```

### clipboard_payloads

```sql
item_id INTEGER PRIMARY KEY
text_plain TEXT
image_png BLOB
raw_payload BLOB
```

## Indexes

- created_at desc
- sha256
- is_pinned, created_at

## Rules

- dedupe by sha256
- do not dedupe pinned
- cap history
- prune oldest non-pinned

## Classes

```text
Storage
HistoryRepository
Item
Payload
```
