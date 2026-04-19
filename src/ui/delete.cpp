/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Verbatim port of legacy/delete.cpp. UpdateDeleteMenu is
    skipped (handled by the kbd_field guards at the entry points). */

#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "einzug.h"
#include "cursor.h"
#include "undoredo.h"

void TDBWFRM::DeleteSchaftClick()
{
    dbw3_assert(kbd_field == EINZUG || kbd_field == AUFKNUEPFUNG);
    if (!(kbd_field == EINZUG || kbd_field == AUFKNUEPFUNG))
        return;

    int currentpos;
    if (kbd_field == EINZUG)
        currentpos = einzug.kbd.j;
    else
        currentpos = gewebe.kbd.j;
    if (currentpos == -1)
        return;

    /*  Einzug loeschen. */
    for (int i = 0; i < Data->MAXX1; i++)
        if (einzug.feld.Get(i) == currentpos + 1)
            einzug.feld.Set(i, 0);

    /*  Aufknuepfung / Trittfolge loeschen. */
    if (!(ViewSchlagpatrone && ViewSchlagpatrone->isChecked())) {
        for (int i = 0; i < Data->MAXX2; i++)
            aufknuepfung.feld.Set(i, currentpos, 0);
    } else {
        for (int j = 0; j < Data->MAXY2; j++) {
            trittfolge.feld.Set(currentpos, j, 0);
            trittfolge.isempty.Set(j, true);
            for (int i = 0; i < Data->MAXX2; i++)
                if (trittfolge.feld.Get(i, j) > 0) {
                    trittfolge.isempty.Set(j, false);
                    break;
                }
        }
    }

    RecalcGewebe();
    if (einzughandler)
        einzughandler->EliminateEmptySchaft();

    CalcRangeKette();
    CalcRangeSchuesse();
    CalcRapport();

    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}

void TDBWFRM::DeleteTrittClick()
{
    dbw3_assert(kbd_field == TRITTFOLGE || kbd_field == AUFKNUEPFUNG);
    if (!(kbd_field == TRITTFOLGE || kbd_field == AUFKNUEPFUNG))
        return;

    int currentpos;
    if (kbd_field == TRITTFOLGE)
        currentpos = trittfolge.kbd.i;
    else
        currentpos = gewebe.kbd.i;
    if (currentpos == -1)
        return;

    /*  Tritt loeschen. */
    for (int j = 0; j < Data->MAXY2; j++) {
        trittfolge.feld.Set(currentpos, j, 0);
        trittfolge.isempty.Set(j, true);
        for (int i = 0; i < Data->MAXX2; i++)
            if (trittfolge.feld.Get(i, j) > 0) {
                trittfolge.isempty.Set(j, false);
                break;
            }
    }

    /*  Aufknuepfung / Einzug loeschen. */
    if (!(ViewSchlagpatrone && ViewSchlagpatrone->isChecked())) {
        for (int j = 0; j < Data->MAXY1; j++)
            aufknuepfung.feld.Set(currentpos, j, 0);
    } else {
        for (int i = 0; i < Data->MAXX1; i++)
            if (einzug.feld.Get(i) == currentpos + 1)
                einzug.feld.Set(i, 0);
    }

    RecalcGewebe();
    if (!(ViewSchlagpatrone && ViewSchlagpatrone->isChecked()))
        EliminateEmptyTritt();

    CalcRangeKette();
    CalcRangeSchuesse();
    CalcRapport();

    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::DeleteKettfaden(int _i)
{
    /*  Alles eins nach links kopieren im Einzug. */
    for (int i = _i + 1; i < Data->MAXX1; i++)
        einzug.feld.Set(i - 1, einzug.feld.Get(i));
    /*  Alles eins nach links kopieren im Gewebe. */
    for (int i = _i + 1; i < Data->MAXX1; i++)
        for (int j = 0; j < Data->MAXY2; j++)
            gewebe.feld.Set(i - 1, j, gewebe.feld.Get(i, j));
    /*  Aeusserster Faden initialisieren. */
    for (int j = 0; j < Data->MAXY2; j++)
        gewebe.feld.Set(Data->MAXX1 - 1, j, 0);
    einzug.feld.Set(Data->MAXX1 - 1, 0);

    RecalcFreieSchaefte();
    CalcRangeKette();
    CalcRapport();
}

void TDBWFRM::DeleteSchussfaden(int _j)
{
    /*  Alles eins nach unten kopieren in der Trittfolge. */
    for (int j = _j + 1; j < Data->MAXY2; j++) {
        for (int i = 0; i < Data->MAXX2; i++)
            trittfolge.feld.Set(i, j - 1, trittfolge.feld.Get(i, j));
        trittfolge.isempty.Set(j - 1, trittfolge.isempty.Get(j));
    }
    /*  Alles eins nach unten kopieren im Gewebe. */
    for (int j = _j + 1; j < Data->MAXY2; j++)
        for (int i = 0; i < Data->MAXX1; i++)
            gewebe.feld.Set(i, j - 1, gewebe.feld.Get(i, j));
    /*  Oberster Faden initialisieren. */
    for (int i = 0; i < Data->MAXX1; i++)
        gewebe.feld.Set(i, Data->MAXY2 - 1, 0);
    for (int i = 0; i < Data->MAXX2; i++)
        trittfolge.feld.Set(i, Data->MAXY2 - 1, 0);
    trittfolge.isempty.Set(Data->MAXY2 - 1, true);

    RecalcFreieTritte();
    CalcRangeSchuesse();
    CalcRapport();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::DeleteKetteClick()
{
    dbw3_assert(kbd_field == EINZUG || kbd_field == GEWEBE);
    if (!(kbd_field == EINZUG || kbd_field == GEWEBE))
        return;

    if (selection.Valid() && (selection.feld == GEWEBE || selection.feld == EINZUG)) {
        for (int i = selection.begin.i; i <= selection.end.i; i++)
            DeleteKettfaden(selection.begin.i);
    } else {
        int currentpos;
        if (kbd_field == EINZUG)
            currentpos = einzug.kbd.i;
        else
            currentpos = gewebe.kbd.i;
        if (currentpos == -1)
            return;
        DeleteKettfaden(currentpos);
    }

    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}

void TDBWFRM::DeleteSchussClick()
{
    dbw3_assert(kbd_field == TRITTFOLGE || kbd_field == GEWEBE);
    if (!(kbd_field == TRITTFOLGE || kbd_field == GEWEBE))
        return;

    if (selection.Valid() && (selection.feld == GEWEBE || selection.feld == TRITTFOLGE)) {
        for (int j = selection.begin.j; j <= selection.end.j; j++)
            DeleteSchussfaden(selection.begin.j);
    } else {
        int currentpos;
        if (kbd_field == TRITTFOLGE)
            currentpos = trittfolge.kbd.j;
        else
            currentpos = gewebe.kbd.j;
        if (currentpos == -1)
            return;
        DeleteSchussfaden(currentpos);
    }

    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}
