#include "SettingsModel.h"

#include "storage/Settings.h"

SettingsModel::SettingsModel(Settings* settings, QObject* parent)
    : QObject(parent)
    , m_settings(settings)
{
    if (!m_settings)
        return;

    connect(m_settings, &Settings::layoutChanged, this, &SettingsModel::layoutChanged);
    connect(m_settings, &Settings::opacityChanged, this, &SettingsModel::opacityChanged);
    connect(m_settings, &Settings::themeChanged, this, &SettingsModel::themeChanged);
    connect(m_settings, &Settings::expandModeChanged, this, &SettingsModel::expandModeChanged);
    connect(m_settings, &Settings::compactImageExpandChanged, this, &SettingsModel::compactImageExpandChanged);
    connect(m_settings, &Settings::dedupeChanged, this, &SettingsModel::dedupeChanged);
    connect(m_settings, &Settings::saveImagesChanged, this, &SettingsModel::saveImagesChanged);
    connect(m_settings, &Settings::maxHistoryChanged, this, &SettingsModel::maxHistoryChanged);
}

QString SettingsModel::layout() const
{
    return m_settings ? m_settings->layout() : QStringLiteral("normal");
}

double SettingsModel::opacity() const
{
    return m_settings ? m_settings->opacity() : 1.0;
}

QString SettingsModel::theme() const
{
    return m_settings ? m_settings->theme() : QStringLiteral("teal");
}

bool SettingsModel::expandMode() const
{
    return m_settings ? m_settings->expandMode() : true;
}

bool SettingsModel::compactImageExpand() const
{
    return m_settings ? m_settings->compactImageExpand() : false;
}

bool SettingsModel::dedupe() const
{
    return m_settings ? m_settings->dedupe() : true;
}

bool SettingsModel::saveImages() const
{
    return m_settings ? m_settings->saveImages() : true;
}

int SettingsModel::maxHistory() const
{
    return m_settings ? m_settings->maxHistory() : 500;
}

void SettingsModel::setLayout(const QString& value)
{
    if (!m_settings)
        return;
    m_settings->setLayout(value);
}

void SettingsModel::setOpacity(double value)
{
    if (!m_settings)
        return;
    m_settings->setOpacity(value);
}

void SettingsModel::setTheme(const QString& value)
{
    if (!m_settings)
        return;
    m_settings->setTheme(value);
}

void SettingsModel::setExpandMode(bool value)
{
    if (!m_settings)
        return;
    m_settings->setExpandMode(value);
}

void SettingsModel::setCompactImageExpand(bool value)
{
    if (!m_settings)
        return;
    m_settings->setCompactImageExpand(value);
}

void SettingsModel::setDedupe(bool value)
{
    if (!m_settings)
        return;
    m_settings->setDedupe(value);
}

void SettingsModel::setSaveImages(bool value)
{
    if (!m_settings)
        return;
    m_settings->setSaveImages(value);
}

void SettingsModel::setMaxHistory(int value)
{
    if (!m_settings)
        return;
    m_settings->setMaxHistory(value);
}