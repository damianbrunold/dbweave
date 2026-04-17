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
#include <mem.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "dbw3_strings.h"
#include "blockmuster_form.h"
#include "undoredo.h"
#include "datamodule.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SwitchRange (int _range)
{
    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid() && selection.feld==GEWEBE) {
        bool changed = false;
        bool empty = true;
        for (int i=selection.begin.i; i<=selection.end.i; i++)
            for (int j=selection.begin.j; j<=selection.end.j; j++) {
                char c = gewebe.feld.Get(i,j);
                if (c>0) {
                    gewebe.feld.Set (i, j, char(_range));
                    if (c!=_range) changed = true;
                    empty = false;
                }
            }
        if (empty) {
            for (int i=selection.begin.i; i<=selection.end.i; i++)
                for (int j=selection.begin.j; j<=selection.end.j; j++)
                    gewebe.feld.Set (i, j, char(_range));
            changed = true;
        }
        if (changed) {
            RecalcAll();
            CalcRangeSchuesse();
            CalcRangeKette();
            CalcRapport();
            UpdateStatusBar();
            Invalidate();
            SetModified();
            if (undo) undo->Snapshot();
        }
    } else if (selection.Valid() && selection.feld==AUFKNUEPFUNG) {
        bool changed = false;
        bool empty = true;
        for (int i=selection.begin.i; i<=selection.end.i; i++)
            for (int j=selection.begin.j; j<=selection.end.j; j++) {
                char c = aufknuepfung.feld.Get(i,j);
                if (c>0) {
                    aufknuepfung.feld.Set (i, j, char(_range));
                    if (c!=_range) changed = true;
                    empty = false;
                }
            }
        if (empty) {
            for (int i=selection.begin.i; i<=selection.end.i; i++)
                for (int j=selection.begin.j; j<=selection.end.j; j++)
                    aufknuepfung.feld.Set (i, j, char(_range));
            changed = true;
        }
        if (changed) {
            RecalcGewebe();
//            RecalcAll(); // Nötig weil u. U. Änderungen...
            CalcRangeSchuesse();
            CalcRangeKette();
            CalcRapport();
            UpdateStatusBar();
            Invalidate();
            SetModified();
            if (undo) undo->Snapshot();
        }
    } else if (ViewSchlagpatrone->Checked && selection.Valid() && selection.feld==TRITTFOLGE) {
        bool changed = false;
        bool empty = true;
        for (int i=selection.begin.i; i<=selection.end.i; i++)
            for (int j=selection.begin.j; j<=selection.end.j; j++) {
                char c = trittfolge.feld.Get(i,j);
                if (c>0) {
                    trittfolge.feld.Set (i, j, char(_range));
                    if (c!=_range) changed = true;
                    empty = false;
                }
            }
        if (empty) {
            for (int i=selection.begin.i; i<=selection.end.i; i++)
                for (int j=selection.begin.j; j<=selection.end.j; j++)
                    trittfolge.feld.Set (i, j, char(_range));
            changed = true;
        }
        if (changed) {
            RecalcGewebe();
//            RecalcAll(); // Nötig weil u. U. Änderungen...
            CalcRangeSchuesse();
            CalcRangeKette();
            CalcRapport();
            UpdateStatusBar();
            Invalidate();
            SetModified();
            if (undo) undo->Snapshot();
        }
    }
    switch (_range) {
        case 1: Range1Click(this); break;
        case 2: Range2Click(this); break;
        case 3: Range3Click(this); break;
        case 4: Range4Click(this); break;
        case 5: Range5Click(this); break;
        case 6: Range6Click(this); break;
        case 7: Range7Click(this); break;
        case 8: Range8Click(this); break;
        case 9: Range9Click(this); break;
    }
    selection = savesel;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Range1Click(TObject *Sender)
{
    Range1->Checked = true;
    PopupRange1->Checked = true;
    currentrange = 1;
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Range2Click(TObject *Sender)
{
    Range2->Checked = true;
    PopupRange2->Checked = true;
    currentrange = 2;
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Range3Click(TObject *Sender)
{
    Range3->Checked = true;
    PopupRange3->Checked = true;
    currentrange = 3;
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Range4Click(TObject *Sender)
{
    Range4->Checked = true;
    PopupRange4->Checked = true;
    currentrange = 4;
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Range5Click(TObject *Sender)
{
    Range5->Checked = true;
    PopupRange5->Checked = true;
    currentrange = 5;
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Range6Click(TObject *Sender)
{
    Range6->Checked = true;
    PopupRange6->Checked = true;
    currentrange = 6;
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Range7Click(TObject *Sender)
{
    Range7->Checked = true;
    PopupRange7->Checked = true;
    currentrange = 7;
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Range8Click(TObject *Sender)
{
    Range8->Checked = true;
    PopupRange8->Checked = true;
    currentrange = 8;
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Range9Click(TObject *Sender)
{
    Range9->Checked = true;
    PopupRange9->Checked = true;
    currentrange = 9;
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RangeAushebungClick(TObject *Sender)
{
    RangeAushebung->Checked = true;
    PopupRangeLiftout->Checked = true;
    currentrange = AUSHEBUNG;
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RangeAnbindungClick(TObject *Sender)
{
    RangeAnbindung->Checked = true;
    PopupRangeBinding->Checked = true;
    currentrange = ANBINDUNG;
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RangeAbbindungClick(TObject *Sender)
{
    RangeAbbindung->Checked = true;
    PopupRangeUnbinding->Checked = true;
    currentrange = ABBINDUNG;
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RangePatternsClick(TObject *Sender)
{
    dbw3_assert(cursorhandler);
    cursorhandler->DisableCursor();
    try {
        dbw3_assert(bereichundo);
        TBlockmusterForm* pFrm = new TBlockmusterForm(this, *bereichundo, &bereichmuster, currentbm, cursorhandler, SUBST_RANGE, false);
        if (pFrm->ShowModal()==mrOk) {
            int x = pFrm->mx+1;
            int y = pFrm->my+1;

            // Auf Schlagpatrone umschalten
            ViewSchlagpatrone->Checked = true;
            trittfolge.einzeltritt = false;

            BereicheFillPattern (x, y);

            CalcRange();
            CalcRapport();
            RecalcAll();

            Invalidate();
            dbw3_assert (undo!=0);
            undo->Snapshot();
        }
        delete pFrm;
    } catch (...) {
    }
    SetModified();
    cursorhandler->EnableCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::BereicheFillPattern (int _x, int _y)
{
    int i0;
    int i1;
    int j0;
    int j1;

    RANGE savesel = selection;
    selection.Normalize();
    if (selection.Valid()) {
        i0 = selection.begin.i;
        i1 = selection.end.i;
        j0 = selection.begin.j;
        j1 = selection.end.j;
    } else {
        i0 = kette.a;
        i1 = kette.b;
        j0 = schuesse.a;
        j1 = schuesse.b;
    }
    selection = savesel;

    for (int i=i0; i<=i1; i++)
        for (int j=j0; j<=j1; j++) {
            char bindung = gewebe.feld.Get (i, j);
            dbw3_assert (bindung>=0 && bindung<10);
            int x = bereichmuster[bindung].SizeX();
            int y = bereichmuster[bindung].SizeY();
            gewebe.feld.Set (i, j, bereichmuster[bindung].Get (i%x, j%y));
        }
}
/*-----------------------------------------------------------------*/

