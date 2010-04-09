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
#include "einzug.h"
#include "zentralsymm.h"
#include "dbw3_strings.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ClearAufknuepfungClick(TObject *Sender)
{
    if (ViewSchlagpatrone->Checked) return;

    // Auknuepfung löschen
    for (int i=0; i<Data->MAXX2; i++)
        for (int j=0; j<Data->MAXY1; j++)
            aufknuepfung.feld.Set (i, j, 0);

    // Gewebe löschen
    for (int i=0; i<Data->MAXX1; i++)
        for (int j=0; j<Data->MAXY2; j++)
            gewebe.feld.Set (i, j, 0);

    CalcRangeKette();
    CalcRangeSchuesse();
    UpdateRapport();
    Invalidate();
    SetModified();

    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::AufInvertClick(TObject *Sender)
{
    if (ViewSchlagpatrone->Checked) return;

    int s1 = GetFirstSchaft();
    int s2 = GetLastSchaft();
    if (s2<s1) return;

    int t1 = GetFirstTritt();
    int t2 = GetLastTritt();
    if (t2<t1) return;

    for (int i=t1; i<=t2; i++)
        for (int j=s1; j<=s2; j++) {
            char s = aufknuepfung.feld.Get (i, j);
            aufknuepfung.feld.Set (i, j, char(s==0 ? currentrange : -s));
        }

    for (int i=kette.a; i<=kette.b; i++)
        for (int j=schuesse.a; j<=schuesse.b; j++) {
            char s = gewebe.feld.Get (i, j);
            gewebe.feld.Set (i, j, char(s==0 ? currentrange : -s));
        }

    Invalidate();
    SetModified();

    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::AufZentralsymmClick(TObject *Sender)
{
    if (ViewSchlagpatrone->Checked) return;

    int s1 = GetFirstSchaft();
    int s2 = GetLastSchaft();
    if (s2<s1) return;

    int t1 = GetFirstTritt();
    int t2 = GetLastTritt();
    if (t2<t1) return;

    if ((t2-t1)!=(s2-s1)) return; // nicht quadratisch

    ZentralSymmChecker symchecker(t2-t1+1, s2-s1+1);

    // Aufknüpfung reinkopieren
    for (int i=t1; i<=t2; i++)
        for (int j=s1; j<=s2; j++)
            symchecker.SetData (i-t1, j-s1, aufknuepfung.feld.Get (i, j));

    if (!symchecker.IsAlreadySymmetric()) {
        // Falls Symmetrie vorhanden Aufknüpfung entsprechend ändern
        if (symchecker.SearchSymmetry()) {
            for (int i=t1; i<=t2; i++)
                for (int j=s1; j<=s2; j++)
                    aufknuepfung.feld.Set (i, j, symchecker.GetData (i-t1, j-s1));
        } else {
            Application->MessageBox (NOCENTRALSYMMFOUND.c_str(), APP_TITLE, MB_OK);
        }

        RecalcGewebe();
        CalcRangeSchuesse();
        CalcRangeKette();
        UpdateRapport();

        Invalidate();
        SetModified();
        dbw3_assert(undo);
        undo->Snapshot();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::AufRollUpClick(TObject *Sender)
{
    if (ViewSchlagpatrone->Checked) return;

    int s1 = GetFirstSchaft();
    int s2 = GetLastSchaft();
    if (s2<s1) return;

    int t1 = GetFirstTritt();
    int t2 = GetLastTritt();
    if (t2<t1) return;

    try {
        char* data = new char[t2-t1+1];

        for (int i=t1; i<=t2; i++)
            data[i-t1] = aufknuepfung.feld.Get (i, s2);
        for (int j=s2; j>s1; j--)
            for (int i=t1; i<=t2; i++)
                aufknuepfung.feld.Set (i, j, aufknuepfung.feld.Get (i, j-1));
        for (int i=t1; i<=t2; i++)
            aufknuepfung.feld.Set (i, s1, data[i-t1]);

        RecalcGewebe();
        CalcRangeSchuesse();
        CalcRangeKette();
        UpdateRapport();

        delete[] data;

        Invalidate();
        SetModified();

        dbw3_assert (undo!=0);
        undo->Snapshot();
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::AufRollDownClick(TObject *Sender)
{
    if (ViewSchlagpatrone->Checked) return;

    int s1 = GetFirstSchaft();
    int s2 = GetLastSchaft();
    if (s2<s1) return;

    int t1 = GetFirstTritt();
    int t2 = GetLastTritt();
    if (t2<t1) return;

    try {
        char* data = new char[t2-t1+1];

        for (int i=t1; i<=t2; i++)
            data[i-t1] = aufknuepfung.feld.Get (i, s1);
        for (int j=s1; j<s2; j++)
            for (int i=t1; i<=t2; i++)
                aufknuepfung.feld.Set (i, j, aufknuepfung.feld.Get (i, j+1));
        for (int i=t1; i<=t2; i++)
            aufknuepfung.feld.Set (i, s2, data[i-t1]);

        RecalcGewebe();
        CalcRangeSchuesse();
        CalcRangeKette();
        UpdateRapport();

        delete[] data;

        Invalidate();
        SetModified();

        dbw3_assert (undo!=0);
        undo->Snapshot();
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::AufRollLeftClick(TObject *Sender)
{
    if (ViewSchlagpatrone->Checked) return;

    int s1 = GetFirstSchaft();
    int s2 = GetLastSchaft();
    if (s2<s1) return;

    int t1 = GetFirstTritt();
    int t2 = GetLastTritt();
    if (t2<t1) return;

    try {
        char* data = new char[s2-s1+1];

        for (int j=s1; j<=s2; j++)
            data[j-s1] = aufknuepfung.feld.Get (t1, j);
        for (int i=t1; i<t2; i++)
            for (int j=s1; j<=s2; j++)
                aufknuepfung.feld.Set (i, j, aufknuepfung.feld.Get (i+1, j));
        for (int j=s1; j<=s2; j++)
            aufknuepfung.feld.Set (t2, j, data[j-s1]);

        RecalcGewebe();
        CalcRangeSchuesse();
        CalcRangeKette();
        UpdateRapport();

        delete[] data;

        Invalidate();
        SetModified();

        dbw3_assert (undo!=0);
        undo->Snapshot();
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::AufRollRightClick(TObject *Sender)
{
    if (ViewSchlagpatrone->Checked) return;

    int s1 = GetFirstSchaft();
    int s2 = GetLastSchaft();
    if (s2<s1) return;

    int t1 = GetFirstTritt();
    int t2 = GetLastTritt();
    if (t2<t1) return;

    try {
        char* data = new char[s2-s1+1];

        for (int j=s1; j<=s2; j++)
            data[j-s1] = aufknuepfung.feld.Get (t2, j);
        for (int i=t2; i>t1; i--)
            for (int j=s1; j<=s2; j++)
                aufknuepfung.feld.Set (i, j, aufknuepfung.feld.Get (i-1, j));
        for (int j=s1; j<=s2; j++)
            aufknuepfung.feld.Set (t1, j, data[j-s1]);

        RecalcGewebe();
        CalcRangeSchuesse();
        CalcRangeKette();
        UpdateRapport();

        delete[] data;

        Invalidate();
        SetModified();

        dbw3_assert (undo!=0);
        undo->Snapshot();
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::AufSteigungIncClick(TObject *Sender)
{
    if (ViewSchlagpatrone->Checked) return;

    int s1 = GetFirstSchaft();
    int s2 = GetLastSchaft();
    if (s2<s1) return;

    int t1 = GetFirstTritt();
    int t2 = GetLastTritt();
    if (t2<t1) return;

    IncrementSteigung (t1, s1, t2, s2, AUFKNUEPFUNG);

    RecalcGewebe();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();

    Invalidate();
    SetModified();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::AufSteigungDecClick(TObject *Sender)
{
    if (ViewSchlagpatrone->Checked) return;

    int s1 = GetFirstSchaft();
    int s2 = GetLastSchaft();
    if (s2<s1) return;

    int t1 = GetFirstTritt();
    int t2 = GetLastTritt();
    if (t2<t1) return;

    DecrementSteigung (t1, s1, t2, s2, AUFKNUEPFUNG);

    RecalcGewebe();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();

    Invalidate();
    SetModified();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::MinimizeAufknuepfung()
{
    if (ViewSchlagpatrone->Checked) return;

    // Leere Schäfte killen
    for (int j=0; j<Data->MAXY1; j++)
        if (einzughandler->IsEmptySchaft (j)) {
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
