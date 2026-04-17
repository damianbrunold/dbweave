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
#include "palette.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SetKettfarben (int _i)
{
	DoSetKettfarben (_i);
	update();

	dbw3_assert (undo!=0);
	if (undo) undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DoSetKettfarben (int _i)
{
	kettfarben.feld.Set (_i+scroll_x1, Data->color);

	/*  Kettfaden neu zeichnen falls FE oder GS. The legacy body painted
	    the kettfarben cell directly via Canvas-> then called
	    DrawGewebeKette; the direct paint is dropped (deferred repaint)
	    and DrawGewebeKette remains as a stub until rendering lands. */
	if ((GewebeFarbeffekt && GewebeFarbeffekt->isChecked()) ||
	    (GewebeSimulation && GewebeSimulation->isChecked()))
		DrawGewebeKette (_i);

	SetModified();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SetSchussfarben (int _j)
{
	DoSetSchussfarben (_j);
	update();

	dbw3_assert (undo!=0);
	if (undo) undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DoSetSchussfarben (int _j)
{
	schussfarben.feld.Set (_j+scroll_y2, Data->color);

	if ((GewebeFarbeffekt && GewebeFarbeffekt->isChecked()) ||
	    (GewebeSimulation && GewebeSimulation->isChecked()))
		DrawGewebeSchuss (_j);

	SetModified();
}
/*-----------------------------------------------------------------*/
