#pragma once

#include <QObject>

class Settings;

class SettingsModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString layout READ layout WRITE setLayout NOTIFY layoutChanged)
    Q_PROPERTY(double opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(bool expandMode READ expandMode WRITE setExpandMode NOTIFY expandModeChanged)
    Q_PROPERTY(bool compactImageExpand READ compactImageExpand WRITE setCompactImageExpand NOTIFY compactImageExpandChanged)
    Q_PROPERTY(bool dedupe READ dedupe WRITE setDedupe NOTIFY dedupeChanged)
    Q_PROPERTY(bool saveImages READ saveImages WRITE setSaveImages NOTIFY saveImagesChanged)
    Q_PROPERTY(bool allowDeletionItems READ allowDeletionItems WRITE setAllowDeletionItems NOTIFY allowDeletionItemsChanged)
    Q_PROPERTY(int maxHistory READ maxHistory WRITE setMaxHistory NOTIFY maxHistoryChanged)

public:
    explicit SettingsModel(Settings* settings, QObject* parent = nullptr);

    QString layout() const;
    double opacity() const;
    QString theme() const;
    bool expandMode() const;
    bool compactImageExpand() const;
    bool dedupe() const;
    bool saveImages() const;
    bool allowDeletionItems() const;
    int maxHistory() const;

public slots:
    void setLayout(const QString& value);
    void setOpacity(double value);
    void setTheme(const QString& value);
    void setExpandMode(bool value);
    void setCompactImageExpand(bool value);
    void setDedupe(bool value);
    void setSaveImages(bool value);
    void setAllowDeletionItems(bool value);
    void setMaxHistory(int value);

signals:
    void layoutChanged();
    void opacityChanged();
    void themeChanged();
    void expandModeChanged();
    void compactImageExpandChanged();
    void dedupeChanged();
    void saveImagesChanged();
    void allowDeletionItemsChanged();
    void maxHistoryChanged();

private:
    Settings* m_settings;
};