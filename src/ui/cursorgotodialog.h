/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy/cursorgoto_form.cpp. Radio-group modal that
    lets the user jump the keyboard cursor to a different field. */

#ifndef DBWEAVE_UI_CURSORGOTODIALOG_H
#define DBWEAVE_UI_CURSORGOTODIALOG_H

#include "enums.h"

#include <QDialog>

class QRadioButton;

class CursorGotoDialog : public QDialog
{
	Q_OBJECT

public:
	/*  _initial picks which radio is pre-selected; _pegplan flips
	    the Trittfolge caption to "Pegplan" and disables the
	    Aufknuepfung choice to match legacy.                    */
	explicit CursorGotoDialog (QWidget* _parent, FELD _initial, bool _pegplan);

	FELD selected() const;

private:
	QRadioButton* rBindung       = nullptr;
	QRadioButton* rEinzug        = nullptr;
	QRadioButton* rTrittfolge    = nullptr;
	QRadioButton* rAufknuepfung  = nullptr;
	QRadioButton* rKettfarben    = nullptr;
	QRadioButton* rSchussfarben  = nullptr;
};

#endif
