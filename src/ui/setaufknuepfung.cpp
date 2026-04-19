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
#include "einzug.h"
/*-----------------------------------------------------------------*/
void TDBWFRM::SetAufknuepfung (int _i, int _j, bool _set, int _range)
{
	if (ViewOnlyGewebe && ViewOnlyGewebe->isChecked()) return;

	DoSetAufknuepfung (_i, _j, _set, _range);

	dbw3_assert (undo!=0);
	if (undo) undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DoSetAufknuepfung (int _i, int _j, bool _set, int _range)
{
	int i, j, k, l;
	if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked()) return;

	// Punkt toggeln und neuzeichnen
	if (!_set) ToggleAufknuepfung (scroll_x2+_i, scroll_y1+_j);
	else aufknuepfung.feld.Set (scroll_x2+_i, scroll_y1+_j, char(_range));
	RedrawAufknuepfung (scroll_x2+_i, scroll_y1+_j);

	// Muss ueberhaupt neuberechnet werden?!
	if ((einzughandler && einzughandler->IsEmptySchaft (scroll_y1+_j)) ||
	    IsEmptyTritt (scroll_x2+_i))
	{
		SetModified();
		refresh();
		return;
	}

	// Zeilen und Spalten loeschen
	for (i=0; i<Data->MAXX1; i++)
		if (einzug.feld.Get (i)==scroll_y1+_j+1)
			for (j=0; j<Data->MAXY2; j++) {
				char s = gewebe.feld.Get (i, j);
				if (s>0) {
					gewebe.feld.Set (i, j, 0);
					RedrawGewebe (i, j);
				}
			}
	for (j=0; j<Data->MAXY2; j++)
		if (trittfolge.feld.Get (scroll_x2+_i, j)>0)
			for (i=0; i<Data->MAXX1; i++) {
				char s = gewebe.feld.Get (i, j);
				if (s>0) {
					gewebe.feld.Set (i, j, 0);
					RedrawGewebe (i, j);
				}
			}

	// Neuberechnen und -zeichnen
	for (i=0; i<Data->MAXX2; i++)
		for (l=0; l<Data->MAXY1; l++) {
			char s = aufknuepfung.feld.Get (i, l);
			if (s>0) {
				for (j=0; j<Data->MAXY2; j++)
					if (trittfolge.feld.Get (i, j)>0)
						for (k=0; k<Data->MAXX1; k++)
							if (einzug.feld.Get(k)==l+1)
								if (gewebe.feld.Get(k, j)<=0) {
									gewebe.feld.Set (k, j, s);
									RedrawGewebe (k, j);
								}
			}
		}

	CalcRange();
	UpdateRapport();

	SetModified();
	refresh();
}
/*-----------------------------------------------------------------*/
