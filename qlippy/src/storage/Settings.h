#pragma once

#include <QObject>
#include <QString>
#include <optional>

class Storage;

// Key/value settings backed by the `settings` table.
class Settings : public QObject
{
    Q_OBJECT
public:
    // Keys
    static constexpr char KeyLayout[] = "layout";
    static constexpr char KeyOpacity[] = "opacity";
    static constexpr char KeyTheme[] = "theme";
    static constexpr char KeyExpandMode[] = "expand_mode";
    static constexpr char KeyCompactImageExpand[] = "compact_image_expand";
    static constexpr char KeyMaxHistory[] = "max_history";
    static constexpr char KeyMaxBytes[] = "max_bytes";
    static constexpr char KeySaveImages[] = "save_images";
    static constexpr char KeyDedupe[] = "dedupe";
    static constexpr char KeyAllowDeletionItems[] = "allow_deletion_items";

    explicit Settings(Storage *storage, QObject *parent = nullptr);

    // Typed getters with defaults
    QString layout() const;          // default normal
    double opacity() const;          // default 1.0, clamped to 0.6-1.0
    QString theme() const;           // default teal
    bool expandMode() const;         // default true
    bool compactImageExpand() const; // default false
    int maxHistory() const;          // default 500
    qint64 maxBytes() const;         // default 50 MB
    bool saveImages() const;         // default true
    bool dedupe() const;             // default true
    bool allowDeletionItems() const; // default false

    // Typed setters
    void setLayout(const QString &value);
    void setOpacity(double value);
    void setTheme(const QString &value);
    void setExpandMode(bool value);
    void setCompactImageExpand(bool value);
    void setMaxHistory(int value);
    void setSaveImages(bool value);
    void setDedupe(bool value);
    void setAllowDeletionItems(bool value);

    // Generic accessors
    QString get(const QString &key, const QString &defaultValue = {}) const;
    void set(const QString &key, const QString &value);

signals:
    void layoutChanged();
    void opacityChanged();
    void themeChanged();
    void expandModeChanged();
    void compactImageExpandChanged();
    void maxHistoryChanged();
    void saveImagesChanged();
    void dedupeChanged();
    void allowDeletionItemsChanged();

private:
    Storage *m_storage;
};
