#include "Logger.h"

#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QMutexLocker>
#include <QTextStream>
#include <QtGlobal>
#include <cstdio>

Logger& Logger::instance()
{
    static Logger s_instance;
    return s_instance;
}

QString Logger::defaultLogPath()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::StateLocation);
    if (!base.endsWith("qlippy"))
        base = QDir::homePath() + "/.local/state/qlippy";
    return base + "/log.txt";
}

bool Logger::openFile()
{
    const QString path = defaultLogPath();
    QDir dir = QFileInfo(path).absoluteDir();
    if (!dir.exists())
        dir.mkpath(".");

    m_file.setFileName(path);
    return m_file.open(QIODevice::Append | QIODevice::Text);
}

void Logger::log(LogLevel level, const QString& msg)
{
    if (level < m_minLevel)
        return;

    static const char* labels[] = { "DEBUG", "INFO ", "WARN ", "ERROR" };
    const char* label = labels[static_cast<int>(level)];

    const QString line = QStringLiteral("[%1] [%2] %3\n")
        .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs))
        .arg(label)
        .arg(msg);

    QMutexLocker lock(&m_mutex);

    // Rotate before writing if log is getting large
    rotateIfNeeded();

    if (m_file.isOpen()) {
        const QByteArray bytes = line.toUtf8();
        m_file.write(bytes);
        m_bytesWritten += bytes.size();
        m_file.flush();
    }

    // Always echo WARN/ERROR to stderr
    if (level >= LogLevel::Warn) {
        std::fputs(qPrintable(line), stderr);
    }
}

void Logger::rotateIfNeeded()
{
    // m_mutex is already held by the caller
    if (!m_file.isOpen() || m_bytesWritten < kMaxLogBytes)
        return;

    const QString path = m_file.fileName();
    m_file.close();
    QFile::remove(path + ".1");
    QFile::rename(path, path + ".1");
    m_file.setFileName(path);
    m_file.open(QIODevice::WriteOnly | QIODevice::Text);
    m_bytesWritten = 0;
}
