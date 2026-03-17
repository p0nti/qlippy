#pragma once

#include <QCoreApplication>
#include <QObject>
#include <QtQml/qqml.h>
#include <memory>

class HistoryRepository;
class ClipboardModel;
class Settings;
class SettingsModel;
class QQuickWindow;
class QQmlApplicationEngine;

class PopupController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("PopupController is created in C++")
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
public:
    QString appVersion() const { return QCoreApplication::applicationVersion(); }

    explicit PopupController(HistoryRepository* repo, Settings* settings, QObject* parent = nullptr);
    ~PopupController() override;

    bool start();

    Q_INVOKABLE void show();
    Q_INVOKABLE void hide();
    Q_INVOKABLE void toggle();
    Q_INVOKABLE void refresh();

    bool isVisible() const;

signals:
    void copyRequested(qint64 id);
    // Emitted when popup is shown so QML can clear + focus the search box.
    void resetSearch();

private:
    void centerWindow();

    HistoryRepository* m_repo;
    Settings* m_settings;
    std::unique_ptr<ClipboardModel> m_model;
    std::unique_ptr<SettingsModel> m_settingsModel;
    std::unique_ptr<QQmlApplicationEngine> m_engine;
    QQuickWindow* m_window = nullptr;
};
