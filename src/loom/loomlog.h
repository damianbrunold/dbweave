/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Serial trace log for the loom drivers. The legacy code carried
    a hand-rolled writeLog() inside StAvlCdIIIController only; this
    generalises it to every controller and to the SerialPort
    wrapper itself, so a user who cannot get their loom to run can
    send back a byte-level transcript instead of "it doesn't work".

    Off by default; the user enables it from the loom options
    dialog (persisted as QSettings "Loom/Trace"). Every line is
    flushed immediately -- the failure modes we are chasing include
    hard hangs, and a buffered tail would be lost.                */

#ifndef DBWEAVE_LOOM_LOOMLOG_H
#define DBWEAVE_LOOM_LOOMLOG_H

#include <QString>

namespace LoomLog
{

/*  Enabling opens (and appends to) the trace file and stamps a
    session header. Disabling closes it. */
void SetEnabled(bool _on);
bool IsEnabled();

/*  Absolute path of the trace file, valid whether or not tracing
    is currently on. Shown in the loom options dialog so the user
    knows what to send back. */
QString FilePath();

/*  One timestamped line. */
void Write(const QString& _msg);

/*  One timestamped line with a hex dump of _len bytes, e.g.
        tx  4d 29 44 31 34 2e 35 34 4b 30 30 30 0d   "M)D14.54K000."
    _tag is a short direction/kind marker ("tx", "rx", "purge").  */
void WriteBytes(const QString& _tag, const char* _buf, int _len);

} /* namespace LoomLog */

#endif
