/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port note: legacy DoSetBlatteinzug paints directly onto the form's
    Canvas with three Rectangle()s + a MoveTo/LineTo pair. Qt uses an
    event-driven paintEvent so we drop the direct paint code and issue
    a deferred update() in SetBlatteinzug. The visual output is the
    same once the rendering slice lands and paintEvent is wired up.
*/

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "undoredo.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SetBlatteinzug (int _i)
{
	DoSetBlatteinzug (_i);
	refresh();    /* legacy painted inline; defer to paintEvent */

	dbw3_assert (undo!=0);
	if (undo) undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DoSetBlatteinzug (int _i)
{
	// Feld setzen
	blatteinzug.feld.Set (_i+scroll_x1, !blatteinzug.feld.Get(_i+scroll_x1));
	SetModified();
}
/*-----------------------------------------------------------------*/
