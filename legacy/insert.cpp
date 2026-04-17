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
#include "datamodule.h"
#include "dbw3_form.h"
#include "einzug.h"
#include "cursor.h"
#include "undoredo.h"
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::_IsEmptySchaft (int _j)
{
    for (int i=0; i<Data->MAXX1; i++)
        if (einzug.feld.Get(i)==_j+1) {
            return false;
        }
    if (!ViewSchlagpatrone->Checked) {
        for (int i=0; i<Data->MAXX2; i++)
            if (aufknuepfung.feld.Get(i, _j)>0) {
                return false;
            }
    } else {
        for (int j=0; j<Data->MAXY2; j++)
            if (trittfolge.feld.Get(_j, j)>0) {
                return false;
            }
    }
    return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::_IsEmptyTritt (int _i)
{
    for (int j=0; j<Data->MAXY2; j++)
        if (trittfolge.feld.Get(_i, j)>0) {
            return false;
        }
    if (!ViewSchlagpatrone->Checked) {
        for (int j=0; j<Data->MAXY1; j++)
            if (aufknuepfung.feld.Get(_i, j)>0) {
                return false;
            }
    } else {
        for (int i=0; i<Data->MAXX1; i++)
            if (einzug.feld.Get(i)==_i+1) {
                return false;
            }
    }
    return true;
}
/*-----------------------------------------------------------------*/
int __fastcall TDBWFRM::GetEmptySchaft()
{
    int lastempty = -1;
    signed int j = Data->MAXY1-1;
    while (j>=0 && _IsEmptySchaft(j)) {
        lastempty = j;
        --j;
    }
    return lastempty;
}
/*-----------------------------------------------------------------*/
int __fastcall TDBWFRM::GetEmptyTritt()
{
    int lastempty = -1;
    signed int i = Data->MAXX2-1;
    while (i>=0 && _IsEmptyTritt(i)) {
        lastempty = i;
        --i;
    }
    return lastempty;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::UpdateInsertMenu()
{
    InsertSchaft->Enabled = kbd_field==EINZUG || kbd_field==AUFKNUEPFUNG;
    InsertTritt->Enabled = kbd_field==TRITTFOLGE || kbd_field==AUFKNUEPFUNG;
    InsertKette->Enabled = kbd_field==EINZUG || kbd_field==GEWEBE;
    InsertSchuss->Enabled = kbd_field==TRITTFOLGE || kbd_field==GEWEBE;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::InsertSchaftClick(TObject *Sender)
{
    dbw3_assert(kbd_field==EINZUG || kbd_field==AUFKNUEPFUNG);

    // Aktuellen Schaft ermitteln
    int currentschaft;
    if (kbd_field==EINZUG) currentschaft = scroll_y1+einzug.kbd.j;
    else currentschaft = scroll_y1+aufknuepfung.kbd.j;
    if (currentschaft==-1) return;

    // Vertauschen mit erstem leeren Schaft
    int emptyschaft = GetEmptySchaft();
    if (emptyschaft==-1) {
        ::MessageBeep(MB_OK);
        return;
    }
    dbw3_assert(einzughandler);
    for (int i=emptyschaft; i>currentschaft; i--) {
        einzughandler->SwitchSchaefte (i, i-1);
    }

    SetModified();
    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::InsertTrittClick(TObject *Sender)
{
    dbw3_assert(kbd_field==TRITTFOLGE || kbd_field==AUFKNUEPFUNG);

    // Aktuellen Tritt ermitteln
    int currenttritt;
    if (kbd_field==TRITTFOLGE) currenttritt = scroll_x2+trittfolge.kbd.i;
    else currenttritt = scroll_x2+aufknuepfung.kbd.i;
    if (currenttritt==-1) return;

    // Vertauschen mit erstem leeren Tritt
    int emptytritt = GetEmptyTritt();
    if (emptytritt==-1) {
        ::MessageBeep(MB_OK);
        return;
    }
    for (int i=emptytritt; i>currenttritt; i--) {
        SwitchTritte (i, i-1);
    }

    SetModified();
    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::InsertKettfaden (int _i)
{
    // Alles eins nach rechts kopieren im Einzug
    for (int i=Data->MAXX1-1; i>_i; i--)
        einzug.feld.Set (i, einzug.feld.Get(i-1));

    // Alles eins nach rechts kopieren im Gewebe
    for (int i=Data->MAXX1-1; i>_i; i--) {
        for (int j=0; j<Data->MAXY2; j++)
            gewebe.feld.Set (i, j, gewebe.feld.Get(i-1, j));
    }

    // Alles eins nach rechts kopieren in den Kettfarben
    for (int i=Data->MAXX1-1; i>_i; i--)
        kettfarben.feld.Set (i, kettfarben.feld.Get (i-1));
    kettfarben.feld.Set (_i, Data->color);

    // Neuen Faden initialisieren
    for (int j=0; j<Data->MAXY2; j++)
        gewebe.feld.Set(_i, j, 0);
    einzug.feld.Set(_i, 0);

    CalcRangeKette();
    CalcRapport();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::InsertSchussfaden (int _j)
{
    // Alles eins nach oben kopieren in der Trittfolge
    for (int j=Data->MAXY2-1; j>_j; j--) {
        for (int i=0; i<Data->MAXX2; i++)
            trittfolge.feld.Set (i, j, trittfolge.feld.Get(i, j-1));
        trittfolge.isempty.Set (j, trittfolge.isempty.Get(j-1));
    }

    // Alles eins nach oben kopieren im Gewebe
    for (int j=Data->MAXY2-1; j>_j; j--)
        for (int i=0; i<Data->MAXX1; i++)
            gewebe.feld.Set (i, j, gewebe.feld.Get(i, j-1));

    // Alles eins nach oben kopieren in den Schussfarben
    for (int j=Data->MAXY2-1; j>_j; j--)
        schussfarben.feld.Set (j, schussfarben.feld.Get (j-1));
    schussfarben.feld.Set (_j, Data->color);

    // Neuen Faden initialisieren
    for (int i=0; i<Data->MAXX1; i++)
        gewebe.feld.Set(i, _j, 0);
    for (int i=0; i<Data->MAXX2; i++)
        trittfolge.feld.Set (i, _j, 0);
    trittfolge.isempty.Set (_j, true);

    CalcRangeSchuesse();
    CalcRapport();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::InsertKetteClick(TObject *Sender)
{
    dbw3_assert(kbd_field==EINZUG || kbd_field==GEWEBE);

    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid() && (selection.feld==GEWEBE || selection.feld==EINZUG)) {
        for (int i=selection.begin.i; i<=selection.end.i; i++)
            InsertKettfaden (i);
    } else {
        int currentpos;
        if (kbd_field==EINZUG) currentpos = einzug.kbd.i;
        else currentpos = gewebe.kbd.i;
        if (currentpos==-1) return;
        InsertKettfaden (scroll_x1+currentpos);
    }

    Invalidate();
    SetModified();

    selection = savesel;

    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::InsertSchussClick(TObject *Sender)
{
    dbw3_assert(kbd_field==TRITTFOLGE || kbd_field==GEWEBE);

    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid() && (selection.feld==GEWEBE || selection.feld==TRITTFOLGE)) {
        for (int j=selection.begin.j; j<=selection.end.j; j++)
            InsertSchussfaden (j);
    } else {
        int currentpos;
        if (kbd_field==TRITTFOLGE) currentpos = trittfolge.kbd.j;
        else currentpos = gewebe.kbd.j;
        if (currentpos==-1) return;
        InsertSchussfaden (scroll_y2+currentpos);
    }
    Invalidate();
    SetModified();

    selection = savesel;

    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/

