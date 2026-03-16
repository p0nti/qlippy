#include "ClipboardModel.h"

#include "storage/HistoryRepository.h"

#include <algorithm>

ClipboardModel::ClipboardModel(HistoryRepository* repo, QObject* parent)
    : QAbstractListModel(parent)
    , m_repo(repo)
{
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(150);
    connect(&m_debounce, &QTimer::timeout, this, &ClipboardModel::applySearch);
}

int ClipboardModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_visible.size();
}

QVariant ClipboardModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_visible.size())
        return {};

    const Item& item = m_all[m_visible[index.row()]];
    switch (role) {
    case IdRole: return item.id;
    case PreviewRole: return item.textPreview;
    case TimestampRole: return item.createdAt.toSecsSinceEpoch();
    case PinnedRole: return item.isPinned;
    case TypeRole: return item.kind == ItemKind::Image ? "image" : "text";
    default: return {};
    }
}

QHash<int, QByteArray> ClipboardModel::roleNames() const
{
    return {
        { IdRole, "id" },
        { PreviewRole, "preview" },
        { TimestampRole, "timestamp" },
        { PinnedRole, "pinned" },
        { TypeRole, "type" },
    };
}

void ClipboardModel::setSearchText(const QString& text)
{
    if (m_searchText == text)
        return;

    m_searchText = text;
    emit searchTextChanged();
    m_debounce.start();
}

void ClipboardModel::setResultLimit(int limit)
{
    const int normalized = std::max(1, limit);
    if (m_resultLimit == normalized)
        return;

    m_resultLimit = normalized;
    emit resultLimitChanged();
    refresh();
}

void ClipboardModel::applySearch()
{
    beginResetModel();
    const QString needle = m_searchText.trimmed();
    if (needle.isEmpty())
        m_all = m_repo->fetchRecent(m_resultLimit);
    else
        m_all = m_repo->searchRecent(needle, m_resultLimit);
    // After a DB-level search all rows match; m_visible = all indices
    m_visible.clear();
    for (int i = 0; i < m_all.size(); ++i)
        m_visible.append(i);
    endResetModel();
    emit countChanged();
}

void ClipboardModel::refresh()
{
    m_debounce.stop();
    beginResetModel();
    const QString needle = m_searchText.trimmed();
    if (needle.isEmpty())
        m_all = m_repo->fetchRecent(m_resultLimit);
    else
        m_all = m_repo->searchRecent(needle, m_resultLimit);
    m_visible.clear();
    for (int i = 0; i < m_all.size(); ++i)
        m_visible.append(i);
    endResetModel();
    emit countChanged();
}

qint64 ClipboardModel::idAt(int row) const
{
    if (row < 0 || row >= m_visible.size())
        return 0;
    return m_all[m_visible[row]].id;
}

QString ClipboardModel::typeAt(int row) const
{
    if (row < 0 || row >= m_visible.size())
        return {};

    const Item& item = m_all[m_visible[row]];
    return item.kind == ItemKind::Image ? QStringLiteral("image") : QStringLiteral("text");
}

QString ClipboardModel::fullTextAt(int row) const
{
    if (row < 0 || row >= m_visible.size())
        return {};

    const Item& item = m_all[m_visible[row]];
    if (item.kind != ItemKind::Text)
        return item.textPreview;

    const auto payload = m_repo->fetchPayload(item.id);
    QString text = payload && !payload->textPlain.isNull()
        ? payload->textPlain
        : item.textPreview;

    return text;
}

QString ClipboardModel::imageDataUrlAt(int row) const
{
    if (row < 0 || row >= m_visible.size())
        return {};

    const Item& item = m_all[m_visible[row]];
    if (item.kind != ItemKind::Image)
        return {};

    const auto payload = m_repo->fetchPayload(item.id);
    if (!payload || payload->imagePng.isEmpty())
        return {};

    return QStringLiteral("data:image/png;base64,")
        + QString::fromLatin1(payload->imagePng.toBase64());
}

void ClipboardModel::activate(int row)
{
    const qint64 id = idAt(row);
    if (id <= 0)
        return;
    if (m_copyHandler)
        m_copyHandler(id);
}

void ClipboardModel::deleteAt(int row)
{
    const qint64 id = idAt(row);
    if (id <= 0)
        return;

    if (m_repo->remove(id))
        refresh();
}

void ClipboardModel::togglePinAt(int row)
{
    if (row < 0 || row >= m_visible.size())
        return;

    Item& item = m_all[m_visible[row]];
    if (m_repo->setPin(item.id, !item.isPinned))
        refresh();
}

bool ClipboardModel::isPinnedAt(int row) const
{
    if (row < 0 || row >= m_visible.size())
        return false;
    return m_all[m_visible[row]].isPinned;
}

void ClipboardModel::rebuildFilter()
{
    // DB-level search is now used; this just rebuilds m_visible from the
    // already-fetched m_all (all rows match after a searchRecent call).
    m_visible.clear();
    for (int i = 0; i < m_all.size(); ++i)
        m_visible.append(i);
}
