/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Verbatim port of legacy/insertbindung.cpp. Stamps a twill
    (Koeper) or satin (Atlas) binding on the gewebe at the
    cursor, then calls RecalcAll so einzug / trittfolge /
    aufknuepfung catch up. The 13 legacy *Click wrappers are
    absorbed into plain parameterised calls at the menu wiring
    site.                                                     */

#include "mainwindow.h"
#include "datamodule.h"
#include "undoredo.h"
#include "assert_compat.h"

void TDBWFRM::KoeperEinfuegen(int _h, int _s)
{
    if (kbd_field != GEWEBE)
        return;

    const int posi = gewebe.kbd.i + scroll_x1;
    const int posj = gewebe.kbd.j + scroll_y2;

    const int n = _h + _s;
    if (_h <= _s) {
        for (int i = 0; i < n; i++)
            for (int j = i; j < i + _h; j++)
                if (posi + i < Data->MAXX1 && posj + (j % n) < Data->MAXY2)
                    gewebe.feld.Set(posi + i, posj + (j % n), char(currentrange));
    } else {
        for (int i = 0; i < n; i++)
            for (int j = i + _s; j < i + n; j++)
                if (posi + i < Data->MAXX1 && posj + (j % n) < Data->MAXY2)
                    gewebe.feld.Set(posi + i, posj + (j % n), char(currentrange));
    }

    /*  Selektion setzen. */
    selection.begin = PT(posi, posj);
    selection.end = PT(posi + n - 1, posj + n - 1);
    if (gewebe.gw > 0 && selection.end.i >= scroll_x1 + gewebe.pos.width / gewebe.gw)
        selection.end.i = scroll_x1 + gewebe.pos.width / gewebe.gw - 1;
    if (gewebe.gh > 0 && selection.end.j >= scroll_y2 + gewebe.pos.height / gewebe.gh)
        selection.end.j = scroll_y2 + gewebe.pos.height / gewebe.gh - 1;
    selection.feld = GEWEBE;

    RecalcAll();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
static inline void FSet(FeldGridChar& _m, int _i, int _j, char _s)
{
    if (_i < Data->MAXX1 && _j < Data->MAXY2)
        _m.Set(_i, _j, _s);
}

void TDBWFRM::AtlasEinfuegen(int _n)
{
    if (kbd_field != GEWEBE)
        return;

    const int posi = gewebe.kbd.i + scroll_x1;
    const int posj = gewebe.kbd.j + scroll_y2;

    const char s = (char)currentrange;
    FeldGridChar& m = gewebe.feld;
    switch (_n) {
    case 5:
        FSet(m, posi + 0, posj + 0, s);
        FSet(m, posi + 1, posj + 2, s);
        FSet(m, posi + 2, posj + 4, s);
        FSet(m, posi + 3, posj + 1, s);
        FSet(m, posi + 4, posj + 3, s);
        break;
    case 6:
        FSet(m, posi + 0, posj + 0, s);
        FSet(m, posi + 1, posj + 2, s);
        FSet(m, posi + 2, posj + 4, s);
        FSet(m, posi + 3, posj + 1, s);
        FSet(m, posi + 4, posj + 5, s);
        FSet(m, posi + 5, posj + 3, s);
        break;
    case 7:
        FSet(m, posi + 0, posj + 2, s);
        FSet(m, posi + 1, posj + 6, s);
        FSet(m, posi + 2, posj + 3, s);
        FSet(m, posi + 3, posj + 0, s);
        FSet(m, posi + 4, posj + 4, s);
        FSet(m, posi + 5, posj + 1, s);
        FSet(m, posi + 6, posj + 5, s);
        break;
    case 8:
        FSet(m, posi + 0, posj + 2, s);
        FSet(m, posi + 1, posj + 5, s);
        FSet(m, posi + 2, posj + 0, s);
        FSet(m, posi + 3, posj + 3, s);
        FSet(m, posi + 4, posj + 6, s);
        FSet(m, posi + 5, posj + 1, s);
        FSet(m, posi + 6, posj + 4, s);
        FSet(m, posi + 7, posj + 7, s);
        break;
    case 9:
        FSet(m, posi + 0, posj + 0, s);
        FSet(m, posi + 1, posj + 2, s);
        FSet(m, posi + 2, posj + 4, s);
        FSet(m, posi + 3, posj + 6, s);
        FSet(m, posi + 4, posj + 8, s);
        FSet(m, posi + 5, posj + 1, s);
        FSet(m, posi + 6, posj + 3, s);
        FSet(m, posi + 7, posj + 5, s);
        FSet(m, posi + 8, posj + 7, s);
        break;
    case 10:
        FSet(m, posi + 0, posj + 0, s);
        FSet(m, posi + 1, posj + 7, s);
        FSet(m, posi + 2, posj + 4, s);
        FSet(m, posi + 3, posj + 1, s);
        FSet(m, posi + 4, posj + 8, s);
        FSet(m, posi + 5, posj + 5, s);
        FSet(m, posi + 6, posj + 2, s);
        FSet(m, posi + 7, posj + 9, s);
        FSet(m, posi + 8, posj + 6, s);
        FSet(m, posi + 9, posj + 3, s);
        break;
    }

    /*  Selektion setzen. */
    selection.begin = PT(posi, posj);
    selection.end = PT(posi + _n - 1, posj + _n - 1);
    if (gewebe.gw > 0 && selection.end.i >= scroll_x1 + gewebe.pos.width / gewebe.gw)
        selection.end.i = scroll_x1 + gewebe.pos.width / gewebe.gw - 1;
    if (gewebe.gh > 0 && selection.end.j >= scroll_y2 + gewebe.pos.height / gewebe.gh)
        selection.end.j = scroll_y2 + gewebe.pos.height / gewebe.gh - 1;
    selection.feld = GEWEBE;

    RecalcAll();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}
