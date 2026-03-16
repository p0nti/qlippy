#include "Settings.h"
#include "Storage.h"

#include <algorithm>
#include <QSqlQuery>
#include <QSqlError>

namespace
{

    QString normalizeLayout(const QString &value)
    {
        if (value == QLatin1String("compact") || value == QLatin1String("normal") || value == QLatin1String("big"))
            return value;
        return QStringLiteral("normal");
    }

    QString normalizeTheme(const QString &value)
    {
        if (value == QLatin1String("system-dark") || value == QLatin1String("system_light") || value == QLatin1String("system-light"))
            return value == QLatin1String("system_light") ? QStringLiteral("system-light") : value;
        if (value == QLatin1String("system"))
            return QStringLiteral("system-dark");
        if (value == QLatin1String("catppuccin"))
            return value;
        if (value == QLatin1String("nord"))
            return QStringLiteral("teal");
        return QStringLiteral("teal");
    }

    QString boolToString(bool value)
    {
        return value ? QStringLiteral("1") : QStringLiteral("0");
    }

    bool stringToBool(const QString &value, bool defaultValue)
    {
        if (value == QLatin1String("1") || value == QLatin1String("true") || value == QLatin1String("on"))
            return true;
        if (value == QLatin1String("0") || value == QLatin1String("false") || value == QLatin1String("off"))
            return false;
        return defaultValue;
    }

}

Settings::Settings(Storage *storage, QObject *parent)
    : QObject(parent), m_storage(storage)
{
    const QString storedTheme = get(KeyTheme, QStringLiteral("teal"));
    if (storedTheme != normalizeTheme(storedTheme))
        set(KeyTheme, normalizeTheme(storedTheme));
}

// ---------------------------------------------------------------------------
// Typed getters
// ---------------------------------------------------------------------------
QString Settings::layout() const
{
    return normalizeLayout(get(KeyLayout, "normal"));
}

double Settings::opacity() const
{
    bool ok = false;
    const double value = get(KeyOpacity, "1.0").toDouble(&ok);
    if (!ok)
        return 1.0;
    return std::clamp(value, 0.6, 1.0);
}

QString Settings::theme() const
{
    return normalizeTheme(get(KeyTheme, "teal"));
}

bool Settings::expandMode() const
{
    return stringToBool(get(KeyExpandMode, "1"), true);
}

bool Settings::compactImageExpand() const
{
    return stringToBool(get(KeyCompactImageExpand, "0"), false);
}

int Settings::maxHistory() const
{
    return get(KeyMaxHistory, "500").toInt();
}

qint64 Settings::maxBytes() const
{
    return get(KeyMaxBytes, QString::number(50LL * 1024 * 1024)).toLongLong();
}

bool Settings::saveImages() const
{
    return stringToBool(get(KeySaveImages, "1"), true);
}

bool Settings::dedupe() const
{
    return stringToBool(get(KeyDedupe, "1"), true);
}

bool Settings::allowDeletionItems() const
{
    return stringToBool(get(KeyAllowDeletionItems, "0"), false);
}

void Settings::setLayout(const QString &value)
{
    const QString normalized = normalizeLayout(value);
    if (layout() == normalized)
        return;
    set(KeyLayout, normalized);
    emit layoutChanged();
}

void Settings::setOpacity(double value)
{
    const double normalized = std::clamp(value, 0.6, 1.0);
    if (qFuzzyCompare(opacity(), normalized))
        return;
    set(KeyOpacity, QString::number(normalized, 'f', 2));
    emit opacityChanged();
}

void Settings::setTheme(const QString &value)
{
    const QString normalized = normalizeTheme(value);
    if (theme() == normalized)
        return;
    set(KeyTheme, normalized);
    emit themeChanged();
}

void Settings::setExpandMode(bool value)
{
    if (expandMode() == value)
        return;
    set(KeyExpandMode, boolToString(value));
    emit expandModeChanged();
}

void Settings::setCompactImageExpand(bool value)
{
    if (compactImageExpand() == value)
        return;
    set(KeyCompactImageExpand, boolToString(value));
    emit compactImageExpandChanged();
}

void Settings::setMaxHistory(int value)
{
    const int normalized = std::max(1, value);
    if (maxHistory() == normalized)
        return;
    set(KeyMaxHistory, QString::number(normalized));
    emit maxHistoryChanged();
}

void Settings::setSaveImages(bool value)
{
    if (saveImages() == value)
        return;
    set(KeySaveImages, boolToString(value));
    emit saveImagesChanged();
}

void Settings::setDedupe(bool value)
{
    if (dedupe() == value)
        return;
    set(KeyDedupe, boolToString(value));
    emit dedupeChanged();
}

void Settings::setAllowDeletionItems(bool value)
{
    if (allowDeletionItems() == value)
        return;
    set(KeyAllowDeletionItems, boolToString(value));
    emit allowDeletionItemsChanged();
}

// ---------------------------------------------------------------------------
// Generic
// ---------------------------------------------------------------------------
QString Settings::get(const QString &key, const QString &defaultValue) const
{
    QSqlQuery q(m_storage->db());
    q.prepare("SELECT value FROM settings WHERE key = :key");
    q.bindValue(":key", key);
    if (q.exec() && q.next())
        return q.value(0).toString();
    return defaultValue;
}

void Settings::set(const QString &key, const QString &value)
{
    QSqlQuery q(m_storage->db());
    q.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (:key, :value)");
    q.bindValue(":key", key);
    q.bindValue(":value", value);
    q.exec();
}
