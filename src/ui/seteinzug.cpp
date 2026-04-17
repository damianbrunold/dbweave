/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "undoredo.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SetEinzug (int _i, int _j)
{
	if (ViewOnlyGewebe && ViewOnlyGewebe->isChecked()) {
		/*  Legacy rang the system bell here (MessageBeep). Qt has
		    QApplication::beep() but it's rarely what a porter wants;
		    the beep is dropped.                                   */
		return;
	}

	DoSetEinzug (_i, _j);

	// ggf. Punkt replizieren!
	if (RappViewRapport && RappViewRapport->isChecked() &&
	    (_i+scroll_x1)>=rapport.kr.a && (_i+scroll_x1)<=rapport.kr.b)
	{
		int i = _i+scroll_x1;

		if (i>kette.a) {
			while (i>kette.a) i -= rapport.kr.count();
			i += rapport.kr.count();
		}

		while (i<=kette.b) {
			if (i!=_i+scroll_x1) {
				// Einzug kopieren
				short neweinzug = einzug.feld.Get(scroll_x1+_i);
				einzug.feld.Set (i, neweinzug);

				// Kettfaden kopieren
				for (int jj=0; jj<Data->MAXY2; jj++)
					gewebe.feld.Set (i, jj, gewebe.feld.Get (_i+scroll_x1, jj));
			}
			i += rapport.kr.count();
		}
		RecalcFreieSchaefte();
	}

	UpdateRapport();
	SetModified();
	update();

	dbw3_assert (undo!=0);
	if (undo) undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DoSetEinzug (int _i, int _j)
{
	// Einzug neu setzen
	short oldeinzug = einzug.feld.Get(scroll_x1+_i);
	short neweinzug = (short)(scroll_y1+_j+1);
	if (oldeinzug==neweinzug) neweinzug = 0;
	einzug.feld.Set(scroll_x1+_i, neweinzug);
	if (neweinzug!=0)
		freieschaefte[neweinzug-1] = false;
	if (oldeinzug!=0) {
		freieschaefte[oldeinzug-1] = true;
		for (int i=0; i<Data->MAXX1; i++)
			if (einzug.feld.Get(i)==oldeinzug) {
				freieschaefte[oldeinzug-1] = false;
				break;
			}
	}

	// Kettfaden neuberechnen
	int i, j, k;
	for (j=0; j<Data->MAXY2; j++) gewebe.feld.Set (scroll_x1+_i, j, 0);
	for (i=0; i<Data->MAXX2; i++) {
		char s = aufknuepfung.feld.Get (i, neweinzug-1);
		if (s>0)
			for (k=0; k<Data->MAXY2; k++)
				if (trittfolge.feld.Get (i, k)>0)
					gewebe.feld.Set (scroll_x1+_i, k, s);
	}

	// Belegter Bereich nachfuehren
	CalcRangeKette();
	CalcRangeSchuesse();
}
/*-----------------------------------------------------------------*/
