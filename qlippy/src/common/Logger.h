#pragma once

#include <QString>
#include <QFile>
#include <QMutex>

enum class LogLevel { Debug, Info, Warn, Error };

class Logger {
public:
    static Logger& instance();

    // Open log file at ~/.local/state/qlippy/log.txt
    // Falls back to stderr only if this is not called.
    bool openFile();

    void setMinLevel(LogLevel level) { m_minLevel = level; }

    void log(LogLevel level, const QString& msg);

    void debug(const QString& msg) { log(LogLevel::Debug, msg); }
    void info (const QString& msg) { log(LogLevel::Info,  msg); }
    void warn (const QString& msg) { log(LogLevel::Warn,  msg); }
    void error(const QString& msg) { log(LogLevel::Error, msg); }

    static QString defaultLogPath();

private:
    Logger() = default;
    void rotateIfNeeded();

    static constexpr qint64 kMaxLogBytes = 1 * 1024 * 1024; // 1 MB
    QFile    m_file;
    QMutex   m_mutex;
    LogLevel m_minLevel = LogLevel::Info;
    qint64   m_bytesWritten = 0;
};

// Convenience macros
#define LOG_DEBUG(msg) Logger::instance().debug(msg)
#define LOG_INFO(msg)  Logger::instance().info(msg)
#define LOG_WARN(msg)  Logger::instance().warn(msg)
#define LOG_ERROR(msg) Logger::instance().error(msg)
