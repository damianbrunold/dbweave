/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy strgoptloom_form.cpp (TStrgOptLoomForm).
    Loom-type picker + serial-port picker. Built unconditionally
    so the UI offers the dropdown even when DBWEAVE_BUILD_LOOM is
    off; only the "Dummy" entry is live in that case and the other
    selections won't build a controller.                          */

#ifndef DBWEAVE_UI_LOOMOPTIONS_DIALOG_H
#define DBWEAVE_UI_LOOMOPTIONS_DIALOG_H

#include <QDialog>
#include "loom.h"      /* LOOMINTERFACE */

class QComboBox;
class QSpinBox;

class LoomOptionsDialog : public QDialog
{
	Q_OBJECT
public:
	explicit LoomOptionsDialog (QWidget* _parent = nullptr);

	LOOMINTERFACE interf () const;
	int           port   () const;   /*  1..8 matching legacy PORT enum */
	int           delay  () const;

	void setInterface (LOOMINTERFACE _i);
	void setPort      (int _p);
	void setDelay     (int _d);

private slots:
	void onInterfaceChanged (int _row);

private:
	QComboBox* cbInterface = nullptr;
	QComboBox* cbPort      = nullptr;
	QSpinBox*  spDelay     = nullptr;
};

#endif
