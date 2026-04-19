/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope: just MinimizeAufknuepfung from legacy/aufknuepfung.cpp.
    The nine Aux*Click menu handlers (ClearAufknuepfungClick,
    AufInvertClick, AufZentralsymmClick, four AufRoll*Click,
    AufSteigung{Inc,Dec}Click) are deferred -- they depend on the
    menu wiring plus selection state plus the ZentralSymmChecker
    integration which comes together in later slices.
*/

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "einzug.h"
#include <QAction>
/*-----------------------------------------------------------------*/
void TDBWFRM::MinimizeAufknuepfung()
{
	if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked()) return;

	// Leere Schaefte killen
	for (int j=0; j<Data->MAXY1; j++)
		if (einzughandler && einzughandler->IsEmptySchaft (j)) {
			for (int i=0; i<Data->MAXX2; i++)
				aufknuepfung.feld.Set (i, j, 0);
			einzughandler->RedrawAufknuepfungSchaft (j);
		}

	// Leere Tritte killen
	for (int i=0; i<Data->MAXX2; i++)
		if (IsEmptyTritt (i)) {
			for (int j=0; j<Data->MAXY1; j++)
				aufknuepfung.feld.Set (i, j, 0);
			RedrawAufknuepfungTritt (i);
		}
}
/*-----------------------------------------------------------------*/
