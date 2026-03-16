#include <QtTest>

#include "Storage.h"
#include "HistoryRepository.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTimeZone>

namespace
{

    class EnvGuard
    {
    public:
        explicit EnvGuard(const QString &key)
            : m_key(key.toLocal8Bit()), m_hadValue(qEnvironmentVariableIsSet(m_key.constData()))
        {
            if (m_hadValue)
                m_oldValue = qgetenv(m_key.constData());
        }

        ~EnvGuard()
        {
            if (m_hadValue)
                qputenv(m_key.constData(), m_oldValue);
            else
                qunsetenv(m_key.constData());
        }

    private:
        QByteArray m_key;
        QByteArray m_oldValue;
        bool m_hadValue = false;
    };

    struct SeededEnvironment
    {
        QProcessEnvironment processEnvironment;
        qint64 newestId = 0;
        qint64 olderId = 0;
    };

    class ScopedDaemon
    {
    public:
        ScopedDaemon(const QString &binaryPath, const QProcessEnvironment &env)
            : m_binaryPath(binaryPath), m_environment(env)
        {
            m_process.setProcessEnvironment(m_environment);
            m_process.start(m_binaryPath, {QStringLiteral("--daemon")});
            QVERIFY2(m_process.waitForStarted(5000), "Failed to start qlippy daemon");

            bool ready = false;
            for (int attempt = 0; attempt < 30; ++attempt)
            {
                QProcess probe;
                probe.setProcessEnvironment(m_environment);
                probe.start(m_binaryPath, {QStringLiteral("--list")});
                if (probe.waitForFinished(1000) && probe.exitStatus() == QProcess::NormalExit && probe.exitCode() == 0)
                {
                    ready = true;
                    break;
                }
                QTest::qWait(100);
            }
            QVERIFY2(ready, "qlippy daemon did not become IPC-ready in time");
        }

        ~ScopedDaemon()
        {
            if (m_process.state() == QProcess::NotRunning)
                return;

            QProcess stopProcess;
            stopProcess.setProcessEnvironment(m_environment);
            stopProcess.start(m_binaryPath, {QStringLiteral("--stop")});
            stopProcess.waitForFinished(3000);

            if (!m_process.waitForFinished(3000))
            {
                m_process.kill();
                m_process.waitForFinished(3000);
            }
        }

    private:
        QString m_binaryPath;
        QProcessEnvironment m_environment;
        QProcess m_process;
    };

    static QString testBinaryPath()
    {
        return QString::fromUtf8(QLIPPY_TEST_BIN_PATH);
    }

    static void failNow(const QString &message)
    {
        qFatal("%s", qPrintable(message));
    }

    static void ensureDirectory(const QString &path)
    {
        QDir dir;
        if (!dir.mkpath(path))
            failNow(QStringLiteral("Failed to create directory: %1").arg(path));
    }

    static QByteArray runCommand(const QProcessEnvironment &env, const QStringList &args)
    {
        QProcess process;
        process.setProcessEnvironment(env);
        process.start(testBinaryPath(), args);
        if (!process.waitForFinished(5000))
            failNow(QStringLiteral("Command timed out: %1").arg(args.join(' ')));
        if (process.exitStatus() != QProcess::NormalExit)
            failNow(process.errorString());

        const QByteArray stdoutData = process.readAllStandardOutput();
        const QByteArray stderrData = process.readAllStandardError();
        if (process.exitCode() != 0)
        {
            failNow(QStringLiteral("Command failed: %1\nstdout: %2\nstderr: %3")
                        .arg(args.join(' '),
                             QString::fromUtf8(stdoutData),
                             QString::fromUtf8(stderrData)));
        }
        return stdoutData.trimmed();
    }

    static QByteArray runZshSnippet(const QProcessEnvironment &env, const QString &snippet)
    {
        QTemporaryFile scriptFile;
        scriptFile.setAutoRemove(true);
        if (!scriptFile.open())
            failNow(QStringLiteral("Failed to create temporary zsh script"));

        scriptFile.write(snippet.toUtf8());
        scriptFile.flush();

        QProcess process;
        process.setProcessEnvironment(env);
        process.start(QStringLiteral("zsh"), {QStringLiteral("-f"), scriptFile.fileName()});
        if (!process.waitForFinished(5000))
            failNow(QStringLiteral("zsh snippet timed out"));
        if (process.exitStatus() != QProcess::NormalExit)
            failNow(process.errorString());

        const QByteArray stdoutData = process.readAllStandardOutput();
        const QByteArray stderrData = process.readAllStandardError();
        if (process.exitCode() != 0)
        {
            failNow(QStringLiteral("zsh snippet failed\nstdout: %1\nstderr: %2")
                        .arg(QString::fromUtf8(stdoutData),
                             QString::fromUtf8(stderrData)));
        }
        return stdoutData.trimmed();
    }

    static QJsonObject runListCommand(const QProcessEnvironment &env)
    {
        const QByteArray stdoutData = runCommand(env, {QStringLiteral("--list")});
        QJsonParseError error;
        const QJsonDocument document = QJsonDocument::fromJson(stdoutData, &error);
        if (error.error != QJsonParseError::NoError)
            failNow(error.errorString());
        if (!document.isObject())
            failNow(QStringLiteral("Expected --list output to be a JSON object"));
        return document.object();
    }

    static SeededEnvironment createSeededEnvironment(QTemporaryDir &tempDir)
    {
        EnvGuard homeGuard(QStringLiteral("HOME"));
        EnvGuard dataGuard(QStringLiteral("XDG_DATA_HOME"));
        EnvGuard runtimeGuard(QStringLiteral("XDG_RUNTIME_DIR"));

        const QString homePath = tempDir.path() + QStringLiteral("/home");
        const QString dataPath = tempDir.path() + QStringLiteral("/data");
        const QString runtimePath = tempDir.path() + QStringLiteral("/runtime");

        ensureDirectory(homePath);
        ensureDirectory(dataPath);
        ensureDirectory(runtimePath);

        QFile::setPermissions(runtimePath, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

        qputenv("HOME", homePath.toUtf8());
        qputenv("XDG_DATA_HOME", dataPath.toUtf8());
        qputenv("XDG_RUNTIME_DIR", runtimePath.toUtf8());

        QCoreApplication::setApplicationName(QStringLiteral("qlippy"));

        Storage storage;
        if (!storage.open())
            failNow(storage.lastError());
        HistoryRepository repo(&storage);

        const qint64 now = QDateTime::currentSecsSinceEpoch();

        Item older;
        older.createdAt = QDateTime::fromSecsSinceEpoch(now - 10, QTimeZone::UTC);
        older.updatedAt = older.createdAt;
        older.lastCopiedAt = older.createdAt;
        older.kind = ItemKind::Text;
        older.textPreview = QStringLiteral("older-secret");
        older.mimeTypes = QStringLiteral("text/plain");
        older.sha256 = QStringLiteral("older-hash");
        older.byteSize = older.textPreview.toUtf8().size();

        Payload olderPayload;
        olderPayload.textPlain = older.textPreview;

        Item newer;
        newer.createdAt = QDateTime::fromSecsSinceEpoch(now - 1, QTimeZone::UTC);
        newer.updatedAt = newer.createdAt;
        newer.lastCopiedAt = newer.createdAt;
        newer.kind = ItemKind::Text;
        newer.textPreview = QStringLiteral("newer-secret");
        newer.mimeTypes = QStringLiteral("text/plain");
        newer.sha256 = QStringLiteral("newer-hash");
        newer.byteSize = newer.textPreview.toUtf8().size();

        Payload newerPayload;
        newerPayload.textPlain = newer.textPreview;

        SeededEnvironment seeded;
        seeded.olderId = repo.insert(older, olderPayload);
        seeded.newestId = repo.insert(newer, newerPayload);

        if (seeded.olderId <= 0 || seeded.newestId <= 0)
            failNow(QStringLiteral("Failed to seed clipboard history items"));

        storage.close();

        seeded.processEnvironment = QProcessEnvironment::systemEnvironment();
        seeded.processEnvironment.insert(QStringLiteral("HOME"), homePath);
        seeded.processEnvironment.insert(QStringLiteral("XDG_DATA_HOME"), dataPath);
        seeded.processEnvironment.insert(QStringLiteral("XDG_RUNTIME_DIR"), runtimePath);
        seeded.processEnvironment.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));

        return seeded;
    }

} // namespace

class CliCommandTests : public QObject
{
    Q_OBJECT

private slots:
    void listCommandUsesItemsArrayShape();
    void deleteCommandRemovesListedItem();
    void zshCleanupFunctionDeletesNewestListedItem();
};

void CliCommandTests::listCommandUsesItemsArrayShape()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const SeededEnvironment seeded = createSeededEnvironment(tempDir);
    const ScopedDaemon daemon(testBinaryPath(), seeded.processEnvironment);

    const QJsonObject listObject = runListCommand(seeded.processEnvironment);
    QVERIFY2(listObject.contains(QStringLiteral("items")), "Expected --list output to contain top-level 'items'");
    QVERIFY2(listObject.value(QStringLiteral("items")).isArray(), "Expected --list output 'items' value to be an array");

    const QJsonArray items = listObject.value(QStringLiteral("items")).toArray();
    QVERIFY(items.size() >= 2);
    QCOMPARE(items.at(0).toObject().value(QStringLiteral("id")).toInteger(), seeded.newestId);
    QCOMPARE(items.at(1).toObject().value(QStringLiteral("id")).toInteger(), seeded.olderId);
}

void CliCommandTests::deleteCommandRemovesListedItem()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const SeededEnvironment seeded = createSeededEnvironment(tempDir);
    const ScopedDaemon daemon(testBinaryPath(), seeded.processEnvironment);

    runCommand(seeded.processEnvironment, {QStringLiteral("--delete"), QString::number(seeded.newestId)});

    const QJsonObject listObject = runListCommand(seeded.processEnvironment);
    const QJsonArray items = listObject.value(QStringLiteral("items")).toArray();

    QVERIFY(!items.isEmpty());
    QCOMPARE(items.at(0).toObject().value(QStringLiteral("id")).toInteger(), seeded.olderId);

    for (const QJsonValue &itemValue : items)
        QVERIFY(itemValue.toObject().value(QStringLiteral("id")).toInteger() != seeded.newestId);
}

void CliCommandTests::zshCleanupFunctionDeletesNewestListedItem()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SeededEnvironment seeded = createSeededEnvironment(tempDir);
    const ScopedDaemon daemon(testBinaryPath(), seeded.processEnvironment);

    const QString snippet = QStringLiteral(R"ZSH(
pass() {
    return 0
}

_cliphist() {
    local qlippy_bin="%1"
    local id

    pass -c "$1" || return 1

    if command -v jq >/dev/null 2>&1; then
        id="$($qlippy_bin --list 2>/dev/null | jq -r '.items[0].id // empty')"
    else
        id="$($qlippy_bin --list 2>/dev/null | grep -oE '"id":[0-9]+' | head -n1 | cut -d: -f2)"
    fi

    [ -n "$id" ] && "$qlippy_bin" --delete "$id" >/dev/null 2>&1
}

_cliphist dummy-entry
)ZSH")
                                .arg(testBinaryPath());

    runZshSnippet(seeded.processEnvironment, snippet);

    const QJsonObject listObject = runListCommand(seeded.processEnvironment);
    const QJsonArray items = listObject.value(QStringLiteral("items")).toArray();

    QVERIFY(!items.isEmpty());
    QCOMPARE(items.at(0).toObject().value(QStringLiteral("id")).toInteger(), seeded.olderId);

    for (const QJsonValue &itemValue : items)
        QVERIFY(itemValue.toObject().value(QStringLiteral("id")).toInteger() != seeded.newestId);
}

QTEST_MAIN(CliCommandTests)
#include "CliCommandTests.moc"