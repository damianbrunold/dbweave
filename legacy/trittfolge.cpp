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
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "felddef.h"
#include "datamodule.h"
#include "undoredo.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ClearTrittfolgeClick(TObject *Sender)
{
    for (int j=0; j<Data->MAXY2; j++) {
        trittfolge.isempty.Set (j, true);
        for (int i=0; i<Data->MAXX2; i++)
            trittfolge.feld.Set (i, j, 0);
        for (int i=0; i<Data->MAXX1; i++)
            gewebe.feld.Set (i, j, 0);
    }

    CalcRangeSchuesse();
    CalcRangeKette();
    RecalcFreieTritte();
    UpdateRapport();
    Invalidate();
    SetModified();

    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::TfSpiegelnClick(TObject *Sender)
{
    TfBelassen->Checked = true;
    for (int j=schuesse.a; j<=(schuesse.b-schuesse.a)/2; j++) {
        bool isempty = trittfolge.isempty.Get (j);
        trittfolge.isempty.Set (j, trittfolge.isempty.Get(schuesse.b-(j-schuesse.a)));
        trittfolge.isempty.Set (schuesse.b-(j-schuesse.a), isempty);        
        for (int i=0; i<Data->MAXX2; i++) {
            char s = trittfolge.feld.Get (i, j);
            trittfolge.feld.Set (i, j, trittfolge.feld.Get(i, schuesse.b-(j-schuesse.a)));
            trittfolge.feld.Set (i, schuesse.b-(j-schuesse.a), s);
        }
        for (int i=kette.a; i<=kette.b; i++) {
            char s = gewebe.feld.Get (i, j);
            gewebe.feld.Set (i, j, gewebe.feld.Get (i, schuesse.b-(j-schuesse.a)));
            gewebe.feld.Set (i, schuesse.b-(j-schuesse.a), s);
        }
    }

    UpdateRapport();
    Invalidate();
    SetModified();

    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::TfBelassenClick(TObject *Sender)
{
    TfBelassen->Checked = true;
    savedtrittfolgenstyle = TfBelassen;
    // Tritte nicht neu anordnen!
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::TfMinimalZClick(TObject *Sender)
{
    TfMinimalZ->Checked = true;
    RearrangeTritte();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::TfMinimalSClick(TObject *Sender)
{
    TfMinimalS->Checked = true;
    RearrangeTritte();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::TfGeradeZClick(TObject *Sender)
{
    TfGeradeZ->Checked = true;
    RearrangeTritte();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::TfGeradeSClick(TObject *Sender)
{
    TfGeradeS->Checked = true;
    RearrangeTritte();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::TfGesprungenClick(TObject *Sender)
{
    TfGesprungen->Checked = true;
    RearrangeTritte();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::IsEmptyTritt (int _i)
{
    return freietritte[_i];
}
/*-----------------------------------------------------------------*/
int __fastcall TDBWFRM::GetFirstNonemptyTritt (int _i)
{
    for (int i=_i; i<Data->MAXX2; i++)
        if (!IsEmptyTritt (i)) return i;
    return -1;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RedrawTritt (int _i)
{
    if (!ViewTrittfolge->Checked) return;
    if (_i<scroll_x2 || _i>=scroll_x2+trittfolge.pos.width/trittfolge.gw) return; // nicht sichtbar
    for (int j=0; j<trittfolge.pos.height/trittfolge.gh; j++)
        DrawTrittfolge (_i-scroll_x2, j);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RedrawAufknuepfungTritt (int _i)
{
    if (!ViewEinzug->Checked || !ViewTrittfolge->Checked) return;
    if (_i<scroll_x2 || _i>=scroll_x2+aufknuepfung.pos.width/aufknuepfung.gw) return; // nicht sichtbar
    for (int j=scroll_y1; j<scroll_y1+aufknuepfung.pos.height/aufknuepfung.gh; j++)
        DrawAufknuepfung (_i-scroll_x2, j-scroll_y1);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::MoveTritt (int _von, int _nach)
{
    int j;
    for (j=schuesse.a; j<=schuesse.b; j++) {
        trittfolge.feld.Set (_nach, j, trittfolge.feld.Get (_von, j));
        trittfolge.feld.Set (_von, j, 0);
    }
    for (j=0; j<Data->MAXY1; j++) {
        aufknuepfung.feld.Set (_nach, j, aufknuepfung.feld.Get (_von, j));
        aufknuepfung.feld.Set (_von, j, 0);
    }
    RedrawTritt (_von);
    RedrawTritt (_nach);
    RedrawAufknuepfungTritt (_von);
    RedrawAufknuepfungTritt (_nach);
    freietritte[_nach] = freietritte[_von];
    freietritte[_von] = true;
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::AufknuepfungsspalteEqual (int _i1, int _i2)
{
    bool nonempty = false;
    for (int j=0; j<Data->MAXY1; j++) {
        char s1 = aufknuepfung.feld.Get(_i1, j);
        char s2 = aufknuepfung.feld.Get(_i2, j);
        if (s1>0 || s2>0) nonempty = true;
        if (s1<=0 && s2>0) return false;
        if (s1>0 && s2<=0) return false;
        if (s1>0 && s2>0 && s1!=s2) return false;
    }
    return nonempty ? true : false;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::MergeTritte()
{
    bool found;

    if (ViewSchlagpatrone->Checked) return;

    do {
        found = false;
        // Identische Aufknüpfungsspalten suchen
        for (int i=0; i<Data->MAXX2; i++)
            for (int ii=0; ii<i; ii++)
                if (AufknuepfungsspalteEqual(i, ii)) {
                    // Tritte zusammenlegen
                    for (int j=schuesse.a; j<=schuesse.b; j++)
                        if (trittfolge.feld.Get(i, j)>0) {
                            trittfolge.feld.Set(ii, j, trittfolge.feld.Get(i, j));
                            trittfolge.feld.Set (i, j, 0);
                        }
                    RedrawTritt (i);
                    RedrawTritt (ii);
                    freietritte[i] = true;
                    // Aufknuepfung bereinigen
                    for (int j=0; j<Data->MAXY1; j++)
                        aufknuepfung.feld.Set (i, j, 0);
                    RedrawAufknuepfungTritt (i);

                    found = true;
                    continue; // Suche restarten
                }
    } while (found);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EliminateEmptyTritt()
{
    for (int i=0; i<Data->MAXX2; i++)
        if (IsEmptyTritt (i)) {
            int firstnonempty = GetFirstNonemptyTritt(i);
            if (firstnonempty==-1 || firstnonempty<i)
                return;
             MoveTritt (firstnonempty, i);
        }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SwitchTritte (int _a, int _b)
{
    int j;
    for (j=schuesse.a; j<=schuesse.b; j++) {
        char temp = trittfolge.feld.Get (_a, j);
        trittfolge.feld.Set (_a, j, trittfolge.feld.Get (_b, j));
        trittfolge.feld.Set (_b, j, temp);
    }
    for (j=0; j<Data->MAXY1; j++) {
        char temp = aufknuepfung.feld.Get (_a, j);
        aufknuepfung.feld.Set (_a, j, aufknuepfung.feld.Get (_b, j));
        aufknuepfung.feld.Set (_b, j, temp);
    }
    RedrawTritt (_a);
    RedrawTritt (_b);
    RedrawAufknuepfungTritt (_a);
    RedrawAufknuepfungTritt (_b);
    dbw3_assert (_a<Data->MAXX2);
    dbw3_assert (_b<Data->MAXX2);
    bool IsEmpty = freietritte[_a];
    freietritte[_a] = freietritte[_b];
    freietritte[_b] = IsEmpty;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RearrangeTritte()
{
    if (TfBelassen->Checked) return;
    if (ViewSchlagpatrone->Checked) return;

    if (schuesse.a==-1) return;

    int i1, i2, j, ii;

    // Ersten Tritt suchen
    for (ii=0; ii<Data->MAXX2; ii++)
        for (j=schuesse.a; j<=schuesse.b; j++)
            if (trittfolge.feld.Get (ii, j)>0) {
                i1 = ii;
                goto g_weiter;
            }

    // Nichts tun
    return;

g_weiter:
    // Letzten Tritt suchen
    for (ii=Data->MAXX2-1; ii>0; ii--)
        for (j=schuesse.a; j<=schuesse.b; j++)
            if (trittfolge.feld.Get (ii, j)>0) {
                i2 = ii;
                goto g_weiter2;
            }

    // Nichts tun
    return;

g_weiter2:

    MergeTritte();
    
    // Alle Tritte gemäss aktueller Einstellung arrangieren
    if (TfMinimalZ->Checked) {
        for (j=schuesse.a; j<=schuesse.b; j++)
            for (ii=i1; ii<=i2; ii++)
                if (trittfolge.feld.Get (ii, j)>0) {
                    if (ii>i1) SwitchTritte (i1, ii);
                    i1++;
                    if (i1>=i2) goto g_end;
                }
    } else if (TfMinimalS->Checked) {
        for (j=schuesse.a; j<=schuesse.b; j++)
            for (ii=i2; ii>=i1; ii--)
                if (trittfolge.feld.Get (ii, j)>0) {
                    if (ii<i2) SwitchTritte (i2, ii);
                    i2--;
                    if (i2<=i1) goto g_end;
                }
    } else if (TfGeradeZ->Checked) {
		//xxxx RTD
    } else if (TfGeradeS->Checked) {
		//xxxx RTD
    } else if (TfGesprungen->Checked) {
        int b = 0;
        for (j=schuesse.a; j<=schuesse.b; j++)
            for (ii=i1; ii<=i2; ii++)
                if (trittfolge.feld.Get (ii, j)>0) {
                    if ((b%2)==0) {
                        if (ii>i1 && ii<=i2) SwitchTritte (i1, ii);
                        i1++;
                        if (i1>=i2) goto g_end;
                    } else if ((b%2)==1) {
                        if (ii<i2 && ii>=i1) SwitchTritte (i2, ii);
                        i2--;
                        if (i2<=i1) goto g_end;
                    }
                    b = (b+1)%2;
                }
    }

g_end:
    // Schlussarbeiten...
    EliminateEmptyTritt();
    return;
 }
 /*-----------------------------------------------------------------*/

