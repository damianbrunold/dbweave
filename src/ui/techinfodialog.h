/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy/techinfo_form.cpp (TTechinfoForm). Read-only
    modal listing DB-WEAVE version / install path, OS / kernel
    info, CPU architecture, and display colour depth. Legacy
    CPUID / GlobalMemoryStatus / GetDeviceCaps paths are replaced
    with the QSysInfo / QScreen / QStorageInfo equivalents. */

#ifndef DBWEAVE_UI_TECHINFODIALOG_H
#define DBWEAVE_UI_TECHINFODIALOG_H

#include <QDialog>

class TechinfoDialog : public QDialog
{
	Q_OBJECT

public:
	explicit TechinfoDialog (QWidget* _parent = nullptr);
};

#endif
