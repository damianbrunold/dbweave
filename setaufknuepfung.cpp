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
#include "felddef.h"
#include "dbw3_strings.h"
#include "datamodule.h"
#include "undoredo.h"
#include "einzug.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SetAufknuepfung (int _i, int _j, bool _set, int _range)
{
    if (ViewOnlyGewebe->Checked) {
        MessageBeep (MB_OK);
        return;
    }

    DoSetAufknuepfung (_i, _j, _set, _range);

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DoSetAufknuepfung (int _i, int _j, bool _set, int _range)
{
    int i, j, k, l;
    if (ViewSchlagpatrone->Checked) {
        MessageBeep (MB_OK);
        return;
    }

    // Punkt toggeln und neuzeichnen
    if (!_set) ToggleAufknuepfung (scroll_x2+_i, scroll_y1+_j);
    else aufknuepfung.feld.Set (scroll_x2+_i, scroll_y1+_j, char(_range));
    RedrawAufknuepfung (scroll_x2+_i, scroll_y1+_j);

    // Muss überhaupt neuberechnet werden?!
    if (einzughandler->IsEmptySchaft (scroll_y1+_j) || IsEmptyTritt (scroll_x2+_i)) {
        SetModified();
        return;
    }

    // Zeilen und Spalten löschen
    for (i=0; i<Data->MAXX1; i++)
        if (einzug.feld.Get (i)==scroll_y1+_j+1)
            for (j=0; j<Data->MAXY2; j++) {
                char s = gewebe.feld.Get (i, j);
                if (s>0) {
                    gewebe.feld.Set (i, j, 0);
                    RedrawGewebe (i, j);
                }
            }
    for (j=0; j<Data->MAXY2; j++)
        if (trittfolge.feld.Get (scroll_x2+_i, j)>0)
            for (i=0; i<Data->MAXX1; i++) {
                char s = gewebe.feld.Get (i, j);
                if (s>0) {
                    gewebe.feld.Set (i, j, 0);
                    RedrawGewebe (i, j);
                }
            }

    // Neuberechnen und -zeichnen
    for (i=0; i<Data->MAXX2; i++)
        for (l=0; l<Data->MAXY1; l++) {
            char s = aufknuepfung.feld.Get (i, l);
            if (s>0) {
                for (j=0; j<Data->MAXY2; j++)
                    if (trittfolge.feld.Get (i, j)>0)
                        for (k=0; k<Data->MAXX1; k++)
                            if (einzug.feld.Get(k)==l+1)
                                if (gewebe.feld.Get(k, j)<=0) {
                                    gewebe.feld.Set (k, j, s);
                                    RedrawGewebe (k, j);
                                }
            }
        }

    CalcRange();
    UpdateRapport();

    SetModified();
}
/*-----------------------------------------------------------------*/
