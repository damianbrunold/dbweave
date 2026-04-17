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
void __fastcall TDBWFRM::UpdateDeleteMenu()
{
    DeleteSchaft->Enabled = kbd_field==EINZUG || kbd_field==AUFKNUEPFUNG;
    DeleteTritt->Enabled = kbd_field==TRITTFOLGE || kbd_field==AUFKNUEPFUNG;
    DeleteKette->Enabled = kbd_field==EINZUG || kbd_field==GEWEBE;
    DeleteSchuss->Enabled = kbd_field==TRITTFOLGE || kbd_field==GEWEBE;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DeleteSchaftClick(TObject *Sender)
{
    dbw3_assert(kbd_field==EINZUG || kbd_field==AUFKNUEPFUNG);

    // Aktuellen Schaft ermitteln
    int currentpos;
    if (kbd_field==EINZUG) currentpos = einzug.kbd.j;
    else currentpos = gewebe.kbd.j;
    if (currentpos==-1) return;

    // Einzug löschen
    for (int i=0; i<Data->MAXX1; i++) {
        if (einzug.feld.Get(i)==currentpos+1)
            einzug.feld.Set (i, 0);
    }

    // Aufknuepfung/Trittfolge löschen
    if (!ViewSchlagpatrone->Checked) {
        for (int i=0; i<Data->MAXX2; i++)
            aufknuepfung.feld.Set (i, currentpos, 0);
    } else {
        for (int j=0; j<Data->MAXY2; j++) {
            trittfolge.feld.Set (currentpos, j, 0);
            trittfolge.isempty.Set (j, true);
            for (int i=0; i<Data->MAXX2; i++)
                if (trittfolge.feld.Get(i, j)>0) {
                    trittfolge.isempty.Set (j, false);
                    break;
                }
        }
    }

    RecalcGewebe();

    EliminateEmptySchaft();

    CalcRangeKette();
    CalcRangeSchuesse();
    CalcRapport();

    Invalidate();
    SetModified();
    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DeleteTrittClick(TObject *Sender)
{
    dbw3_assert(kbd_field==TRITTFOLGE || kbd_field==AUFKNUEPFUNG);

    // Aktuellen Tritt ermitteln
    int currentpos;
    if (kbd_field==TRITTFOLGE) currentpos = trittfolge.kbd.i;
    else currentpos = gewebe.kbd.i;
    if (currentpos==-1) return;

    // Tritt löschen
    for (int j=0; j<Data->MAXY2; j++) {
        trittfolge.feld.Set (currentpos, j, 0);
        trittfolge.isempty.Set (j, true);
        for (int i=0; i<Data->MAXX2; i++)
            if (trittfolge.feld.Get(i, j)>0) {
                trittfolge.isempty.Set (j, false);
                break;
            }
    }

    // Aufknuepfung/Einzug löschen
    if (!ViewSchlagpatrone->Checked) {
        for (int j=0; j<Data->MAXY1; j++)
            aufknuepfung.feld.Set (currentpos, j, 0);
    } else {
        for (int i=0; i<Data->MAXX1; i++)
            if (einzug.feld.Get(i)==currentpos+1)
                einzug.feld.Set (i, 0);
    }

    RecalcGewebe();

    if (!ViewSchlagpatrone->Checked) EliminateEmptyTritt();

    CalcRangeKette();
    CalcRangeSchuesse();
    CalcRapport();

    Invalidate();
    SetModified();
    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DeleteKettfaden (int _i)
{
    // Alles eins nach links kopieren im Einzug
    for (int i=_i+1; i<Data->MAXX1; i++)
        einzug.feld.Set (i-1, einzug.feld.Get(i));

    // Alles eins nach links kopieren im Gewebe
    for (int i=_i+1; i<Data->MAXX1; i++) {
        for (int j=0; j<Data->MAXY2; j++)
            gewebe.feld.Set (i-1, j, gewebe.feld.Get(i, j));
    }

    // Äusserster Faden initialisieren
    for (int j=0; j<Data->MAXY2; j++)
        gewebe.feld.Set(Data->MAXX1-1, j, 0);
    einzug.feld.Set(Data->MAXX1-1, 0);

    RecalcFreieSchaefte();
    CalcRangeKette();
    CalcRapport();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DeleteSchussfaden (int _j)
{
    // Alles eins nach unten kopieren in der Trittfolge
    for (int j=_j+1; j<Data->MAXY2; j++) {
        for (int i=0; i<Data->MAXX2; i++)
            trittfolge.feld.Set (i, j-1, trittfolge.feld.Get(i, j));
        trittfolge.isempty.Set (j-1, trittfolge.isempty.Get(j));
    }

    // Alles eins nach unten kopieren im Gewebe
    for (int j=_j+1; j<Data->MAXY2; j++)
        for (int i=0; i<Data->MAXX1; i++)
            gewebe.feld.Set (i, j-1, gewebe.feld.Get(i, j));

    // Oberster Faden initialisieren
    for (int i=0; i<Data->MAXX1; i++)
        gewebe.feld.Set(i, Data->MAXY2-1, 0);
    for (int i=0; i<Data->MAXX2; i++)
        trittfolge.feld.Set (i, Data->MAXY2-1, 0);
    trittfolge.isempty.Set (Data->MAXY2-1, true);

    RecalcFreieTritte();
    CalcRangeSchuesse();
    CalcRapport();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DeleteKetteClick(TObject *Sender)
{
    dbw3_assert(kbd_field==EINZUG || kbd_field==GEWEBE);

    if (selection.Valid() && (selection.feld==GEWEBE || selection.feld==EINZUG)) {
        for (int i=selection.begin.i; i<=selection.end.i; i++)
            DeleteKettfaden (selection.begin.i);
    } else {
        int currentpos;
        if (kbd_field==EINZUG) currentpos = einzug.kbd.i;
        else currentpos = gewebe.kbd.i;
        if (currentpos==-1) return;
        DeleteKettfaden (currentpos);
    }

    Invalidate();
    SetModified();

    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DeleteSchussClick(TObject *Sender)
{
    dbw3_assert(kbd_field==TRITTFOLGE || kbd_field==GEWEBE);

    if (selection.Valid() && (selection.feld==GEWEBE || selection.feld==TRITTFOLGE)) {
        for (int j=selection.begin.j; j<=selection.end.j; j++)
            DeleteSchussfaden (selection.begin.j);
    } else {
        int currentpos;
        if (kbd_field==TRITTFOLGE) currentpos = trittfolge.kbd.j;
        else currentpos = gewebe.kbd.j;
        if (currentpos==-1) return;
        DeleteSchussfaden (currentpos);
    }

    Invalidate();
    SetModified();

    dbw3_assert(undo);
    undo->Snapshot();
}
/*----------------------------------------------------------------*/

