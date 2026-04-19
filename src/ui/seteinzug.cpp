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
void TDBWFRM::SetEinzug(int _i, int _j)
{
    if (ViewOnlyGewebe && ViewOnlyGewebe->isChecked()) {
        /*  Legacy rang the system bell here (MessageBeep). Qt has
            QApplication::beep() but it's rarely what a porter wants;
            the beep is dropped.                                   */
        return;
    }

    DoSetEinzug(_i, _j);

    // ggf. Punkt replizieren!
    if (RappViewRapport && RappViewRapport->isChecked() && (_i + scroll_x1) >= rapport.kr.a
        && (_i + scroll_x1) <= rapport.kr.b) {
        int i = _i + scroll_x1;

        if (i > kette.a) {
            while (i > kette.a)
                i -= rapport.kr.count();
            i += rapport.kr.count();
        }

        const short neweinzug = einzug.feld.Get(scroll_x1 + _i);
        while (i <= kette.b) {
            if (i != _i + scroll_x1)
                einzug.feld.Set(i, neweinzug);
            i += rapport.kr.count();
        }
    }

    /*  Single recalc across the final einzug. RecalcGewebe rebuilds
        gewebe from einzug x aufknuepfung x trittfolge for every warp,
        so the replicated columns get their gewebe columns rebuilt
        automatically.                                              */
    RecalcGewebe();
    CalcRangeKette();
    CalcRangeSchuesse();
    UpdateRapport();
    SetModified();
    refresh();

    dbw3_assert(undo != 0);
    if (undo)
        undo->Snapshot();
}
/*-----------------------------------------------------------------*/
/*  DoSetEinzug only writes the einzug cell. The caller runs
    RecalcGewebe + CalcRange* once after all related writes.        */
void TDBWFRM::DoSetEinzug(int _i, int _j)
{
    short oldeinzug = einzug.feld.Get(scroll_x1 + _i);
    short neweinzug = (short)(scroll_y1 + _j + 1);
    if (oldeinzug == neweinzug)
        neweinzug = 0;
    einzug.feld.Set(scroll_x1 + _i, neweinzug);
}
/*-----------------------------------------------------------------*/
