/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Verbatim port of legacy/insert.cpp: insert a warp / weft thread
    or insert a shaft / treadle slot. UpdateInsertMenu is skipped
    (Qt actions handle their own enablement / we rely on the
    kbd_field guard). */

#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "einzug.h"
#include "cursor.h"
#include "undoredo.h"

bool TDBWFRM::_IsEmptySchaft(int _j)
{
    for (int i = 0; i < Data->MAXX1; i++)
        if (einzug.feld.Get(i) == _j + 1)
            return false;
    if (!(ViewSchlagpatrone && ViewSchlagpatrone->isChecked())) {
        for (int i = 0; i < Data->MAXX2; i++)
            if (aufknuepfung.feld.Get(i, _j) > 0)
                return false;
    } else {
        for (int j = 0; j < Data->MAXY2; j++)
            if (trittfolge.feld.Get(_j, j) > 0)
                return false;
    }
    return true;
}

bool TDBWFRM::_IsEmptyTritt(int _i)
{
    for (int j = 0; j < Data->MAXY2; j++)
        if (trittfolge.feld.Get(_i, j) > 0)
            return false;
    if (!(ViewSchlagpatrone && ViewSchlagpatrone->isChecked())) {
        for (int j = 0; j < Data->MAXY1; j++)
            if (aufknuepfung.feld.Get(_i, j) > 0)
                return false;
    } else {
        for (int i = 0; i < Data->MAXX1; i++)
            if (einzug.feld.Get(i) == _i + 1)
                return false;
    }
    return true;
}

int TDBWFRM::GetEmptySchaft()
{
    int lastempty = -1;
    int j = Data->MAXY1 - 1;
    while (j >= 0 && _IsEmptySchaft(j)) {
        lastempty = j;
        --j;
    }
    return lastempty;
}

int TDBWFRM::GetEmptyTritt()
{
    int lastempty = -1;
    int i = Data->MAXX2 - 1;
    while (i >= 0 && _IsEmptyTritt(i)) {
        lastempty = i;
        --i;
    }
    return lastempty;
}

/*-----------------------------------------------------------------*/
void TDBWFRM::InsertSchaftClick()
{
    dbw3_assert(kbd_field == EINZUG || kbd_field == AUFKNUEPFUNG);
    if (!(kbd_field == EINZUG || kbd_field == AUFKNUEPFUNG))
        return;

    int currentschaft;
    if (kbd_field == EINZUG)
        currentschaft = scroll_y1 + einzug.kbd.j;
    else
        currentschaft = scroll_y1 + aufknuepfung.kbd.j;
    if (currentschaft == -1)
        return;

    const int emptyschaft = GetEmptySchaft();
    if (emptyschaft == -1)
        return;
    dbw3_assert(einzughandler);
    for (int i = emptyschaft; i > currentschaft; i--)
        einzughandler->SwitchSchaefte(i, i - 1);

    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}

void TDBWFRM::InsertTrittClick()
{
    dbw3_assert(kbd_field == TRITTFOLGE || kbd_field == AUFKNUEPFUNG);
    if (!(kbd_field == TRITTFOLGE || kbd_field == AUFKNUEPFUNG))
        return;

    int currenttritt;
    if (kbd_field == TRITTFOLGE)
        currenttritt = scroll_x2 + trittfolge.kbd.i;
    else
        currenttritt = scroll_x2 + aufknuepfung.kbd.i;
    if (currenttritt == -1)
        return;

    const int emptytritt = GetEmptyTritt();
    if (emptytritt == -1)
        return;
    for (int i = emptytritt; i > currenttritt; i--)
        SwitchTritte(i, i - 1);

    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::InsertKettfaden(int _i)
{
    /*  Alles eins nach rechts kopieren im Einzug. */
    for (int i = Data->MAXX1 - 1; i > _i; i--)
        einzug.feld.Set(i, einzug.feld.Get(i - 1));
    /*  Alles eins nach rechts kopieren im Gewebe. */
    for (int i = Data->MAXX1 - 1; i > _i; i--)
        for (int j = 0; j < Data->MAXY2; j++)
            gewebe.feld.Set(i, j, gewebe.feld.Get(i - 1, j));
    /*  Alles eins nach rechts kopieren in den Kettfarben. */
    for (int i = Data->MAXX1 - 1; i > _i; i--)
        kettfarben.feld.Set(i, kettfarben.feld.Get(i - 1));
    kettfarben.feld.Set(_i, (char)Data->color);
    /*  Neuen Faden initialisieren. */
    for (int j = 0; j < Data->MAXY2; j++)
        gewebe.feld.Set(_i, j, 0);
    einzug.feld.Set(_i, 0);

    CalcRangeKette();
    CalcRapport();
}

void TDBWFRM::InsertSchussfaden(int _j)
{
    /*  Alles eins nach oben kopieren in der Trittfolge. */
    for (int j = Data->MAXY2 - 1; j > _j; j--) {
        for (int i = 0; i < Data->MAXX2; i++)
            trittfolge.feld.Set(i, j, trittfolge.feld.Get(i, j - 1));
    }
    /*  Alles eins nach oben kopieren im Gewebe. */
    for (int j = Data->MAXY2 - 1; j > _j; j--)
        for (int i = 0; i < Data->MAXX1; i++)
            gewebe.feld.Set(i, j, gewebe.feld.Get(i, j - 1));
    /*  Alles eins nach oben kopieren in den Schussfarben. */
    for (int j = Data->MAXY2 - 1; j > _j; j--)
        schussfarben.feld.Set(j, schussfarben.feld.Get(j - 1));
    schussfarben.feld.Set(_j, (char)Data->color);
    /*  Neuen Faden initialisieren. */
    for (int i = 0; i < Data->MAXX1; i++)
        gewebe.feld.Set(i, _j, 0);
    for (int i = 0; i < Data->MAXX2; i++)
        trittfolge.feld.Set(i, _j, 0);

    CalcRangeSchuesse();
    CalcRapport();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::InsertKetteClick()
{
    dbw3_assert(kbd_field == EINZUG || kbd_field == GEWEBE);
    if (!(kbd_field == EINZUG || kbd_field == GEWEBE))
        return;
    /*  Not gated by GewebeLocked: inserts an empty column across
        every field in lockstep without triggering RecalcAll.    */

    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid() && (selection.feld == GEWEBE || selection.feld == EINZUG)) {
        for (int i = selection.begin.i; i <= selection.end.i; i++)
            InsertKettfaden(i);
    } else {
        int currentpos;
        if (kbd_field == EINZUG)
            currentpos = einzug.kbd.i;
        else
            currentpos = gewebe.kbd.i;
        if (currentpos == -1) {
            selection = savesel;
            return;
        }
        InsertKettfaden(scroll_x1 + currentpos);
    }

    SetModified();
    selection = savesel;
    refresh();
    if (undo)
        undo->Snapshot();
}

void TDBWFRM::InsertSchussClick()
{
    dbw3_assert(kbd_field == TRITTFOLGE || kbd_field == GEWEBE);
    if (!(kbd_field == TRITTFOLGE || kbd_field == GEWEBE))
        return;
    /*  Not gated by GewebeLocked: inserts an empty row across every
        field in lockstep without triggering RecalcAll.          */

    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid() && (selection.feld == GEWEBE || selection.feld == TRITTFOLGE)) {
        for (int j = selection.begin.j; j <= selection.end.j; j++)
            InsertSchussfaden(j);
    } else {
        int currentpos;
        if (kbd_field == TRITTFOLGE)
            currentpos = trittfolge.kbd.j;
        else
            currentpos = gewebe.kbd.j;
        if (currentpos == -1) {
            selection = savesel;
            return;
        }
        InsertSchussfaden(scroll_y2 + currentpos);
    }

    SetModified();
    selection = savesel;
    refresh();
    if (undo)
        undo->Snapshot();
}
