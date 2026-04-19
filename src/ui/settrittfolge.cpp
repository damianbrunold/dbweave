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
void TDBWFRM::SetTrittfolge(int _i, int _j, bool _set, int _range)
{
    if (ViewOnlyGewebe && ViewOnlyGewebe->isChecked())
        return;

    DoSetTrittfolge(_i, _j, _set, _range);

    // ggf. Punkt replizieren!
    if (RappViewRapport && RappViewRapport->isChecked() && (_j + scroll_y2) >= rapport.sr.a
        && (_j + scroll_y2) <= rapport.sr.b) {
        int j = _j + scroll_y2;

        if (j > schuesse.a) {
            while (j > schuesse.a)
                j -= rapport.sr.count();
            j += rapport.sr.count();
        }

        while (j <= schuesse.b) {
            if (j != _j + scroll_y2) {
                // Tritt kopieren
                for (int ii = 0; ii < Data->MAXX2; ii++)
                    trittfolge.feld.Set(ii, j, trittfolge.feld.Get(ii, _j + scroll_y2));
                trittfolge.isempty.Set(j, trittfolge.isempty.Get(_j + scroll_y2));

                // Schussfaden kopieren
                for (int ii = 0; ii < Data->MAXX1; ii++)
                    gewebe.feld.Set(ii, j, gewebe.feld.Get(ii, _j + scroll_y2));
            }
            j += rapport.sr.count();
        }
        RecalcFreieTritte();
    }

    UpdateRapport();
    SetModified();
    refresh();

    dbw3_assert(undo != 0);
    if (undo)
        undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DoSetTrittfolge(int _i, int _j, bool _set, int _range)
{
    // Tritt neu setzen
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked()) {
        char oldstate = trittfolge.feld.Get(scroll_x2 + _i, scroll_y2 + _j);
        if (!_set)
            trittfolge.feld.Set(scroll_x2 + _i, scroll_y2 + _j,
                                char(oldstate <= 0 ? currentrange : -oldstate));
        else
            trittfolge.feld.Set(scroll_x2 + _i, scroll_y2 + _j, char(_range));
    } else if (!trittfolge.einzeltritt) {
        char oldstate = trittfolge.feld.Get(scroll_x2 + _i, scroll_y2 + _j);
        trittfolge.feld.Set(scroll_x2 + _i, scroll_y2 + _j, char(oldstate <= 0 ? 1 : -oldstate));
    } else {
        bool bSet = trittfolge.feld.Get(scroll_x2 + _i, scroll_y2 + _j);
        for (int i = 0; i < Data->MAXX2; i++)
            trittfolge.feld.Set(i, scroll_y2 + _j, 0);
        trittfolge.feld.Set(scroll_x2 + _i, scroll_y2 + _j, bSet ? (char)0 : (char)1);
    }

    RecalcGewebe();

    // Belegter Bereich nachfuehren
    CalcRangeKette();
    CalcRangeSchuesse();
}
/*-----------------------------------------------------------------*/
