/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "loomlog.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>

namespace
{

bool g_enabled = false;
QFile* g_file = nullptr;

QString traceFilePath()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty())
        dir = QDir::homePath();
    return dir + QStringLiteral("/loom-trace.log");
}

/*  Appends one already-composed line, prefixed with a millisecond
    timestamp, and flushes. */
void emitLine(const QString& _line)
{
    if (!g_enabled || !g_file || !g_file->isOpen())
        return;
    QTextStream out(g_file);
    out << QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"))
        << QStringLiteral("  ") << _line << Qt::endl;
    out.flush();
    g_file->flush();
}

} /* namespace */

/*-----------------------------------------------------------------*/
void LoomLog::SetEnabled(bool _on)
{
    if (_on == g_enabled)
        return;

    if (!_on) {
        emitLine(QStringLiteral("--- trace stopped ---"));
        g_enabled = false;
        if (g_file) {
            g_file->close();
            delete g_file;
            g_file = nullptr;
        }
        return;
    }

    const QString path = traceFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    g_file = new QFile(path);
    if (!g_file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        delete g_file;
        g_file = nullptr;
        return;
    }
    g_enabled = true;
    emitLine(QStringLiteral("--- trace started, DB-WEAVE %1 ---")
                 .arg(QCoreApplication::applicationVersion()));
}

bool LoomLog::IsEnabled()
{
    return g_enabled;
}

QString LoomLog::FilePath()
{
    return traceFilePath();
}

void LoomLog::Write(const QString& _msg)
{
    emitLine(_msg);
}

void LoomLog::WriteBytes(const QString& _tag, const char* _buf, int _len)
{
    if (!g_enabled)
        return;
    QString hex;
    QString ascii;
    for (int i = 0; i < _len; i++) {
        const unsigned char c = static_cast<unsigned char>(_buf[i]);
        hex += QStringLiteral("%1 ").arg(c, 2, 16, QLatin1Char('0'));
        ascii += (c >= 0x20 && c < 0x7f) ? QLatin1Char(char(c)) : QLatin1Char('.');
    }
    emitLine(
        QStringLiteral("%1 %2 [%3] \"%4\"").arg(_tag, -5).arg(_len, 3).arg(hex.trimmed(), ascii));
}
