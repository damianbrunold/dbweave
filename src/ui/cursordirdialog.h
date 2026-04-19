/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy/cursordir_form.cpp. Small modal with four
    checkable direction toggles (Up / Down / Left / Right) plus
    OK / Cancel. Alt-click on an axis button un-presses the
    opposite-axis button. Click in the gap between two adjacent
    buttons activates both for diagonal auto-advance. */

#ifndef DBWEAVE_UI_CURSORDIRDIALOG_H
#define DBWEAVE_UI_CURSORDIRDIALOG_H

#include "cursor.h"

#include <QDialog>

class QToolButton;

class CursorDirDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CursorDirDialog (QWidget* _parent, CURSORDIRECTION _cd);

	CURSORDIRECTION cursordirection = CD_NONE;

protected:
	void mouseReleaseEvent (QMouseEvent* _e) override;

private:
	QToolButton* cdUp    = nullptr;
	QToolButton* cdDown  = nullptr;
	QToolButton* cdLeft  = nullptr;
	QToolButton* cdRight = nullptr;

	void readFromButtons();
};

#endif
