/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  File menu handlers (Open / Save / Save As) plus the surrounding
    QFileDialog + QMessageBox glue. The legacy unit filehandling.cpp
    had VCL TOpenDialog / TSaveDialog drag-and-drop handling, recent-
    files list (mru.cpp), and the "modified" prompt shown on close.
    This port lands just Open / Save / Save As -- the MRU list and
    the close-prompt are deferred along with the rest of the menu
    chrome.                                                         */

#include "mainwindow.h"
#include "fileformat.h"
#include "loadoptions.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

static QString fileFilter()
{
	return QStringLiteral("DB-WEAVE files (*.dbw);;All files (*.*)");
}

void __fastcall TDBWFRM::FileOpen ()
{
	const QString dir = filename.isEmpty()
	                  ? QString()
	                  : QFileInfo((QString)filename).absolutePath();
	const QString chosen = QFileDialog::getOpenFileName(
	    this, QStringLiteral("Open pattern"), dir, fileFilter());
	if (chosen.isEmpty()) return;

	/*  Close any file the loader from a previous Load left open. */
	if (file && file->IsOpen()) file->Close();

	filename = chosen;
	LOADSTAT stat = UNKNOWN_FAILURE;
	const bool ok = Load(stat, LOADALL);
	if (!ok) {
		QMessageBox::warning(this, QStringLiteral("DB-WEAVE"),
		    QStringLiteral("Could not open '%1' (status %2).")
		        .arg(chosen).arg(int(stat)));
		return;
	}
	SetAppTitle();
	update();
}

void __fastcall TDBWFRM::FileSaveAs ()
{
	const QString dir = filename.isEmpty()
	                  ? QString()
	                  : (QString)filename;
	const QString chosen = QFileDialog::getSaveFileName(
	    this, QStringLiteral("Save pattern as"), dir, fileFilter());
	if (chosen.isEmpty()) return;

	if (file && file->IsOpen()) file->Close();
	filename = chosen;
	FileSave();
}

void __fastcall TDBWFRM::FileSave ()
{
	if (filename.isEmpty()) { FileSaveAs(); return; }
	if (!Save()) {
		QMessageBox::warning(this, QStringLiteral("DB-WEAVE"),
		    QStringLiteral("Could not save '%1'.").arg((QString)filename));
		return;
	}
	SetModified(false);
	SetAppTitle();
}
