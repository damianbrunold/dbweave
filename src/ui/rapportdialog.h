/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy/rapport_form.cpp. Modal dialog driven by
    RappRapportierenClick to ask the user for the warp and weft
    repeat counts; "Repeat all" disables both fields and paints
    the whole kette / schuesse extent. */

#ifndef DBWEAVE_UI_RAPPORTDIALOG_H
#define DBWEAVE_UI_RAPPORTDIALOG_H

#include <QDialog>

class QCheckBox;
class QLineEdit;

class RapportDialog : public QDialog
{
	Q_OBJECT

public:
	explicit RapportDialog (QWidget* _parent = nullptr);

	int  horz()       const;    /* 0 or negative = unset / "Repeat all" */
	int  vert()       const;
	bool repeatAll()  const;
	bool repeatColors() const;

	void setHorz (int _n);
	void setVert (int _n);
	void setRepeatAll    (bool _on);
	void setRepeatColors (bool _on);

private:
	QLineEdit* rappHorz    = nullptr;
	QLineEdit* rappVert    = nullptr;
	QCheckBox* rappAll     = nullptr;
	QCheckBox* rappColors  = nullptr;
};

#endif
