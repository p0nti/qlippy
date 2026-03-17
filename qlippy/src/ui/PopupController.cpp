#include "PopupController.h"

#include "ClipboardModel.h"
#include "SettingsModel.h"

#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QScreen>
#include <QGuiApplication>
#include <QUrl>

PopupController::PopupController(HistoryRepository* repo, Settings* settings, QObject* parent)
    : QObject(parent)
    , m_repo(repo)
    , m_settings(settings)
{}

PopupController::~PopupController() = default;

bool PopupController::start()
{
    m_model = std::make_unique<ClipboardModel>(m_repo);
    m_settingsModel = std::make_unique<SettingsModel>(m_settings);
    m_model->setCopyHandler([this](qint64 id) {
        emit copyRequested(id);
        hide();
    });

    m_engine = std::make_unique<QQmlApplicationEngine>();
    m_engine->setInitialProperties({
        {"clipboardModel",  QVariant::fromValue(m_model.get())},
        {"settingsModel",   QVariant::fromValue(m_settingsModel.get())},
        {"popupController", QVariant::fromValue(this)},
    });
    m_engine->load(QUrl(QStringLiteral("qrc:/qt/qml/Qlippy/ClipboardWindow.qml")));

    if (m_engine->rootObjects().isEmpty())
        return false;

    m_window = qobject_cast<QQuickWindow*>(m_engine->rootObjects().first());
    if (!m_window)
        return false;

    m_window->setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    m_window->hide();
    return true;
}

void PopupController::show()
{
    if (!m_window)
        return;

    emit resetSearch();
    m_model->refresh();
    centerWindow();
    m_window->show();
    m_window->raise();
    m_window->requestActivate();
}

void PopupController::hide()
{
    if (m_window)
        m_window->hide();
}

void PopupController::toggle()
{
    if (!m_window)
        return;

    if (m_window->isVisible())
        hide();
    else
        show();
}

void PopupController::refresh()
{
    if (m_model)
        m_model->refresh();
}

bool PopupController::isVisible() const
{
    return m_window && m_window->isVisible();
}

void PopupController::centerWindow()
{
    if (!m_window)
        return;

    QScreen* screen = m_window->screen();
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    const QRect avail = screen->availableGeometry();
    const QSize size = m_window->size();
    const int x = avail.x() + (avail.width() - size.width()) / 2;
    const int y = avail.y() + (avail.height() - size.height()) / 2;
    m_window->setPosition(x, y);
}
