#include "DaemonApp.h"
#include "ipc/IpcClient.h"
#include "ipc/IpcMessage.h"
#include "common/Logger.h"

#include <QtQml/qqmlextensionplugin.h>
Q_IMPORT_QML_PLUGIN(QlippyPlugin)

#include <QCoreApplication>
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QJsonDocument>
#include <QMessageBox>
#include <QQuickStyle>
#include <QProcess>
#include <QThread>
#include <QTextStream>
#include <csignal>
#include <cstdlib>

#ifndef QLIPPY_VERSION_STR
#define QLIPPY_VERSION_STR "0.1.0"
#endif

#ifndef QLIPPY_BUILD_TYPE_STR
#define QLIPPY_BUILD_TYPE_STR "dev"
#endif

static bool hasArg(int argc, char *argv[], const char *arg)
{
    for (int i = 1; i < argc; ++i)
    {
        if (QString::fromLocal8Bit(argv[i]) == QString::fromLatin1(arg))
            return true;
    }
    return false;
}

// Attempt to launch the daemon in the background and wait for it to accept
// IPC connections (up to ~2 s).  Returns true if the daemon became reachable.
static bool tryAutoStart()
{
    const QString exe = QCoreApplication::applicationFilePath();
    if (!QProcess::startDetached(exe, {"--daemon"}))
        return false;

    // Poll until the socket is available, max 2 s
    for (int i = 0; i < 20; ++i)
    {
        QThread::msleep(100);
        IpcClient probe;
        if (probe.connectToServer(200))
            return true;
    }
    return false;
}

// Connect to daemon, auto-starting it if necessary.
static bool ensureConnected(IpcClient &client)
{
    if (client.connectToServer(500))
        return true;

    QTextStream(stderr) << "qlippy: daemon not running, starting it...\n";
    if (!tryAutoStart())
    {
        QTextStream(stderr) << "error: failed to start daemon\n";
        return false;
    }
    return client.connectToServer(500);
}

static int cliSend(const IpcMessage &msg)
{
    IpcClient client;
    if (!ensureConnected(client))
    {
        QTextStream(stderr) << "error: cannot connect to daemon\n";
        return EXIT_FAILURE;
    }

    auto resp = client.sendSync(msg, 3000);
    if (!resp.has_value())
    {
        QTextStream(stderr) << "error: no response from daemon\n";
        return EXIT_FAILURE;
    }

    if (!resp->ok)
    {
        QTextStream(stderr) << "error: " << resp->error << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// Like cliSend but prints the response data as JSON to stdout.
static int cliSendPrint(const IpcMessage &msg)
{
    IpcClient client;
    if (!ensureConnected(client))
    {
        QTextStream(stderr) << "error: cannot connect to daemon\n";
        return EXIT_FAILURE;
    }

    auto resp = client.sendSync(msg, 3000);
    if (!resp.has_value())
    {
        QTextStream(stderr) << "error: no response from daemon\n";
        return EXIT_FAILURE;
    }

    if (!resp->ok)
    {
        QTextStream(stderr) << "error: " << resp->error << "\n";
        return EXIT_FAILURE;
    }

    QTextStream(stdout) << QString::fromUtf8(
                               QJsonDocument(resp->data).toJson(QJsonDocument::Compact))
                        << "\n";
    return EXIT_SUCCESS;
}

static int runWithApp(QCoreApplication &app)
{
    QCoreApplication::setApplicationName("qlippy");

    QString buildType = QString::fromLatin1(QLIPPY_BUILD_TYPE_STR).trimmed().toLower();
    if (buildType.isEmpty())
        buildType = QStringLiteral("dev");
    QCoreApplication::setApplicationVersion(
        QString::fromLatin1(QLIPPY_VERSION_STR) + QStringLiteral("+") + buildType);

    QCommandLineParser parser;
    parser.setApplicationDescription("Wayland clipboard manager");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption daemonOpt("daemon", "Run as background daemon");
    QCommandLineOption popupOpt("popup", "Show clipboard popup");
    QCommandLineOption toggleOpt("toggle", "Toggle clipboard popup");
    QCommandLineOption listOpt("list", "List clipboard history (JSON)");
    QCommandLineOption searchOpt("search", "Search clipboard history (JSON)", "query");
    QCommandLineOption copyOpt("copy", "Copy item by id to clipboard", "id");
    QCommandLineOption deleteOpt("delete", "Delete item by id", "id");
    QCommandLineOption pinOpt("pin", "Pin item by id", "id");
    QCommandLineOption unpinOpt("unpin", "Unpin item by id", "id");
    QCommandLineOption clearOpt("clear", "Clear all history");
    QCommandLineOption stopOpt("stop", "Stop the running daemon");
    QCommandLineOption verboseOpt("verbose", "Enable debug logging");

    parser.addOption(daemonOpt);
    parser.addOption(popupOpt);
    parser.addOption(toggleOpt);
    parser.addOption(listOpt);
    parser.addOption(searchOpt);
    parser.addOption(copyOpt);
    parser.addOption(deleteOpt);
    parser.addOption(pinOpt);
    parser.addOption(unpinOpt);
    parser.addOption(clearOpt);
    parser.addOption(stopOpt);
    parser.addOption(verboseOpt);

    parser.process(app);

    if (parser.isSet(verboseOpt))
        Logger::instance().setMinLevel(LogLevel::Debug);

    if (parser.isSet(popupOpt))
        return cliSend(IpcMessage::request(IpcCmd::ShowPopup));

    if (parser.isSet(toggleOpt))
        return cliSend(IpcMessage::request(IpcCmd::TogglePopup));

    if (parser.isSet(listOpt))
        return cliSendPrint(IpcMessage::request(IpcCmd::ListItems));

    if (parser.isSet(searchOpt))
        return cliSendPrint(IpcMessage::request(IpcCmd::SearchItems,
                                                {{"q", parser.value(searchOpt)}}));

    if (parser.isSet(clearOpt))
        return cliSend(IpcMessage::request(IpcCmd::ClearHistory));

    if (parser.isSet(stopOpt))
        return cliSend(IpcMessage::request(IpcCmd::StopDaemon));

    if (parser.isSet(copyOpt))
    {
        bool ok = false;
        const qint64 id = parser.value(copyOpt).toLongLong(&ok);
        if (!ok || id <= 0)
        {
            QTextStream(stderr) << "error: invalid id\n";
            return EXIT_FAILURE;
        }
        return cliSend(IpcMessage::request(IpcCmd::CopyItem, {{"id", id}}));
    }

    if (parser.isSet(deleteOpt))
    {
        bool ok = false;
        const qint64 id = parser.value(deleteOpt).toLongLong(&ok);
        if (!ok || id <= 0)
        {
            QTextStream(stderr) << "error: invalid id\n";
            return EXIT_FAILURE;
        }
        return cliSend(IpcMessage::request(IpcCmd::DeleteItem, {{"id", id}}));
    }

    if (parser.isSet(pinOpt))
    {
        bool ok = false;
        const qint64 id = parser.value(pinOpt).toLongLong(&ok);
        if (!ok || id <= 0)
        {
            QTextStream(stderr) << "error: invalid id\n";
            return EXIT_FAILURE;
        }
        return cliSend(IpcMessage::request(IpcCmd::PinItem, {{"id", id}, {"pinned", true}}));
    }

    if (parser.isSet(unpinOpt))
    {
        bool ok = false;
        const qint64 id = parser.value(unpinOpt).toLongLong(&ok);
        if (!ok || id <= 0)
        {
            QTextStream(stderr) << "error: invalid id\n";
            return EXIT_FAILURE;
        }
        return cliSend(IpcMessage::request(IpcCmd::PinItem, {{"id", id}, {"pinned", false}}));
    }

    if (parser.isSet(daemonOpt))
    {
        // Install SIGTERM/SIGINT handlers so the daemon shuts down gracefully.
        std::signal(SIGTERM, [](int)
                    { QCoreApplication::quit(); });
        std::signal(SIGINT, [](int)
                    { QCoreApplication::quit(); });

        Logger::instance().openFile();
        LOG_INFO("qlippy daemon starting (pid " + QString::number(QCoreApplication::applicationPid()) + ")");

        DaemonApp daemon;

        QObject::connect(&daemon, &DaemonApp::startupFailed, [](const QString &reason)
                         {
            QTextStream(stderr) << "fatal: " << reason << "\n";

            // When user manually starts a second daemon instance, provide a clear GUI hint.
            if (reason.contains(QStringLiteral("another qlippy daemon is already active"), Qt::CaseInsensitive)
                && qobject_cast<QGuiApplication*>(QCoreApplication::instance())) {
                QMessageBox::information(nullptr,
                                         QStringLiteral("qlippy"),
                                         QStringLiteral("Another qlippy daemon is already active."));
            }

            QCoreApplication::exit(EXIT_FAILURE); });

        if (!daemon.start())
            return EXIT_FAILURE;

        QObject::connect(&app, &QCoreApplication::aboutToQuit, &daemon, &DaemonApp::stop);
        return app.exec();
    }

    parser.showHelp(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    const bool daemonMode = hasArg(argc, argv, "--daemon");

    if (daemonMode)
    {
        // Keep Qt Quick Controls independent from desktop widget style plugins
        // such as kvantum, which may not provide a QML style module.
        QQuickStyle::setStyle(QStringLiteral("Fusion"));
        QApplication app(argc, argv);
        return runWithApp(app);
    }

    QCoreApplication app(argc, argv);
    return runWithApp(app);
}
