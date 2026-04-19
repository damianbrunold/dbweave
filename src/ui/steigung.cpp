/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope:
      * GetFirstTritt / GetLastTritt -- thin helpers over the
        freietritte availability array.
      * SpInvert -- schlagpatrone + gewebe wide invert across the
        used treadle column range.
      * IncrementSteigung / DecrementSteigung and the SteigungInc /
        SteigungDec wrappers -- diagonal cyclic shift ("slope")
        inside the selection.                                    */

#include "mainwindow.h"
#include "datamodule.h"
#include "undoredo.h"

#include <QByteArray>

int TDBWFRM::GetFirstTritt()
{
    for (int i = 0; i < Data->MAXX2; i++)
        if (freietritte && !freietritte[i])
            return i;
    return Data->MAXX2 - 1;
}

int TDBWFRM::GetLastTritt()
{
    for (int i = Data->MAXX2 - 1; i >= 0; i--)
        if (freietritte && !freietritte[i])
            return i;
    return 0;
}

void TDBWFRM::SpInvert()
{
    if (!(ViewSchlagpatrone && ViewSchlagpatrone->isChecked()))
        return;

    const int t1 = GetFirstTritt();
    const int t2 = GetLastTritt();
    if (t2 < t1)
        return;
    if (schuesse.a < 0 || schuesse.b < 0)
        return;

    for (int j = schuesse.a; j <= schuesse.b; j++) {
        for (int i = t1; i <= t2; i++) {
            const char s = trittfolge.feld.Get(i, j);
            trittfolge.feld.Set(i, j, char(s == 0 ? currentrange : -s));
        }
        if (kette.a >= 0 && kette.b >= 0)
            for (int i = kette.a; i <= kette.b; i++) {
                const char s = gewebe.feld.Get(i, j);
                gewebe.feld.Set(i, j, char(s == 0 ? currentrange : -s));
            }
    }

    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::IncrementSteigung(int _i, int _j, int _ii, int _jj, FELD _feld)
{
    FeldGridChar& feld = (_feld == GEWEBE)       ? gewebe.feld
                         : (_feld == TRITTFOLGE) ? trittfolge.feld
                                                 :
                                                 /* AUFKNUEPFUNG */ aufknuepfung.feld;

    const int ysize = _jj - _j + 1;
    if (ysize <= 0)
        return;
    QByteArray temp(ysize, char(0));

    for (int i = _i + 1; i <= _ii; i++) {
        const int inc = i - _i;
        for (int j = _j; j <= _jj; j++)
            temp[j - _j] = feld.Get(i, j);
        for (int j = _j; j <= _jj; j++)
            feld.Set(i, j, temp.at(((j - _j) + (_ii - _i) * ysize - inc) % ysize));
    }
    if (_feld == TRITTFOLGE)
        for (int j = _j; j <= _jj; j++)
            RecalcTrittfolgeEmpty(j);
}

void TDBWFRM::DecrementSteigung(int _i, int _j, int _ii, int _jj, FELD _feld)
{
    FeldGridChar& feld = (_feld == GEWEBE)       ? gewebe.feld
                         : (_feld == TRITTFOLGE) ? trittfolge.feld
                                                 :
                                                 /* AUFKNUEPFUNG */ aufknuepfung.feld;

    const int ysize = _jj - _j + 1;
    if (ysize <= 0)
        return;
    QByteArray temp(ysize, char(0));

    for (int i = _i + 1; i <= _ii; i++) {
        const int dec = i - _i;
        for (int j = _j; j <= _jj; j++)
            temp[j - _j] = feld.Get(i, j);
        for (int j = _j; j <= _jj; j++)
            feld.Set(i, j, temp.at(((j - _j) + dec) % ysize));
    }
    if (_feld == TRITTFOLGE)
        for (int j = _j; j <= _jj; j++)
            RecalcTrittfolgeEmpty(j);
}

static bool allowedForSteigung(TDBWFRM* frm, FELD f)
{
    if (f == GEWEBE || f == AUFKNUEPFUNG)
        return true;
    if (f == TRITTFOLGE && !frm->trittfolge.einzeltritt)
        return true;
    return false;
}

void TDBWFRM::SteigungInc()
{
    RANGE savesel = selection;
    selection.Normalize();
    if (!selection.Valid() || !allowedForSteigung(this, selection.feld)) {
        selection = savesel;
        return;
    }
    IncrementSteigung(selection.begin.i, selection.begin.j, selection.end.i, selection.end.j,
                      selection.feld);
    if (selection.feld != GEWEBE)
        RecalcGewebe();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    selection = savesel;
    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}

void TDBWFRM::SteigungDec()
{
    RANGE savesel = selection;
    selection.Normalize();
    if (!selection.Valid() || !allowedForSteigung(this, selection.feld)) {
        selection = savesel;
        return;
    }
    DecrementSteigung(selection.begin.i, selection.begin.j, selection.end.i, selection.end.j,
                      selection.feld);
    if (selection.feld != GEWEBE)
        RecalcGewebe();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    selection = savesel;
    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}
