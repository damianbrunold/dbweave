/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#include <stdio.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "felddef.h"
#include "datamodule.h"
#include "undoredo.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SteigungIncClick(TObject *Sender)
{
    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid() && (selection.feld==GEWEBE || selection.feld==AUFKNUEPFUNG || (selection.feld==TRITTFOLGE && !trittfolge.einzeltritt))) {
        IncrementSteigung (selection.begin.i, selection.begin.j, selection.end.i, selection.end.j, selection.feld);

        if (selection.feld==GEWEBE) RecalcAll();
        else RecalcGewebe();
        CalcRangeSchuesse();
        CalcRangeKette();
        UpdateRapport();

        Invalidate();
        SetModified();
        dbw3_assert(undo);
        undo->Snapshot();
    }

    selection = savesel;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SteigungDecClick(TObject *Sender)
{
    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid() && (selection.feld==GEWEBE || selection.feld==AUFKNUEPFUNG || (selection.feld==TRITTFOLGE && !trittfolge.einzeltritt))) {
        DecrementSteigung (selection.begin.i, selection.begin.j, selection.end.i, selection.end.j, selection.feld);

        if (selection.feld==GEWEBE) RecalcAll();
        else RecalcGewebe();
        CalcRangeSchuesse();
        CalcRangeKette();
        UpdateRapport();

        Invalidate();
        SetModified();
        dbw3_assert(undo);
        undo->Snapshot();
    }

    selection = savesel;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::IncrementSteigung (int _i, int _j, int _ii, int _jj, FELD _feld)
{
    FeldGridChar& feld = _feld==GEWEBE ? gewebe.feld :
                            (_feld==TRITTFOLGE ? trittfolge.feld : aufknuepfung.feld);

    try {
        int ysize = _jj-_j+1;
        char* temp = new char[ysize];
        memset (temp, 0, sizeof(char)*ysize);

        for (int i=_i+1; i<=_ii; i++) {
            int inc = i-_i;
            // Spalte kopieren
            for (int j=_j; j<=_jj; j++)
                temp[j-_j] = feld.Get (i, j);
            // versetzt zurücklesen
            for (int j=_j; j<=_jj; j++)
                feld.Set (i, j, temp[(j-_j+(_ii-_i)*ysize-inc) % ysize]);
        }

        if (_feld==TRITTFOLGE) UpdateIsEmpty (_j, _jj);

        delete[] temp;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DecrementSteigung (int _i, int _j, int _ii, int _jj, FELD _feld)
{
    FeldGridChar& feld = _feld==GEWEBE ? gewebe.feld :
                            (_feld==TRITTFOLGE ? trittfolge.feld : aufknuepfung.feld);

    try {
        int ysize = _jj-_j+1;
        char* temp = new char[ysize];
        memset (temp, 0, sizeof(char)*ysize);

        for (int i=_i+1; i<=_ii; i++) {
            int dec = i-_i;
            // Spalte kopieren
            for (int j=_j; j<=_jj; j++)
                temp[j-_j] = feld.Get (i, j);
            // versetzt zurücklesen
            for (int j=_j; j<=_jj; j++)
                feld.Set (i, j, temp[(j-_j+dec) % ysize]);
        }

        delete[] temp;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
