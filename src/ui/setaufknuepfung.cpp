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
void TDBWFRM::SetAufknuepfung(int _i, int _j, bool _set, int _range)
{
    if (ViewOnlyGewebe && ViewOnlyGewebe->isChecked())
        return;

    DoSetAufknuepfung(_i, _j, _set, _range);
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
        return;

    RecalcGewebe();
    CalcRange();
    UpdateRapport();
    SetModified();
    refresh();

    dbw3_assert(undo != 0);
    if (undo)
        undo->Snapshot();
}
/*-----------------------------------------------------------------*/
/*  DoSetAufknuepfung only toggles / sets the aufknuepfung cell. The
    caller runs RecalcGewebe + CalcRange + UpdateRapport once after
    all related writes.                                             */
void TDBWFRM::DoSetAufknuepfung(int _i, int _j, bool _set, int _range)
{
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
        return;

    if (!_set)
        ToggleAufknuepfung(scroll_x2 + _i, scroll_y1 + _j);
    else
        aufknuepfung.feld.Set(scroll_x2 + _i, scroll_y1 + _j, char(_range));
}
/*-----------------------------------------------------------------*/
