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
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SetTrittfolge (int _i, int _j, bool _set, int _range)
{
    if (ViewOnlyGewebe->Checked) {
        MessageBeep (MB_OK);
        return;
    }

    DoSetTrittfolge (_i, _j, _set, _range);

    // ggf. Punkt replizieren!
    if (RappViewRapport->Checked && (_j+scroll_y2)>=rapport.sr.a && (_j+scroll_y2)<=rapport.sr.b) {
        int j = _j+scroll_y2;

        if (j>schuesse.a) {
            while (j>schuesse.a) j -= rapport.sr.count();
            j += rapport.sr.count();
        }

        while (j<=schuesse.b) {
            if (j!=_j+scroll_y2) {
                // Tritt kopieren
                for (int ii=0; ii<Data->MAXX2; ii++)
                    trittfolge.feld.Set (ii, j, trittfolge.feld.Get (ii, _j+scroll_y2));
                trittfolge.isempty.Set (j, trittfolge.isempty.Get (_j+scroll_y2));

                // Schussfaden kopieren
                for (int ii=0; ii<Data->MAXX1; ii++)
                    gewebe.feld.Set (ii, j, gewebe.feld.Get (ii, _j+scroll_y2));

                // Neuzeichnen falls sichtbar
                if (j>=scroll_y2 && j<scroll_y2+trittfolge.pos.height/trittfolge.gw) {
                    // Tritt neuzeichnen
                    for (int ii=scroll_x1; ii<scroll_x1+trittfolge.pos.width/trittfolge.gw; ii++)
                        DrawTrittfolge (ii-scroll_x1, j-scroll_y2);

                    // Schussfaden neuzeichnen
                    if (!trittfolge.isempty.Get(j)) DrawGewebeSchuss (j-scroll_y2);
                    else DeleteGewebeSchuss (j-scroll_y2);
                }
            }
            j += rapport.sr.count();
        }
        RecalcFreieTritte();
    }

    UpdateRapport();

    SetModified();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DoSetTrittfolge (int _i, int _j, bool _set, int _range)
{
    // Tritt neu setzen
    if (ViewSchlagpatrone->Checked) {
        char oldstate = trittfolge.feld.Get(scroll_x2+_i,scroll_y2+_j);
        if (!_set) trittfolge.feld.Set (scroll_x2+_i, scroll_y2+_j, char(oldstate<=0 ? currentrange : -oldstate));
        else trittfolge.feld.Set (scroll_x2+_i, scroll_y2+_j, char(_range));
        if ((!_set && oldstate<0) || (_set && _range!=0)) {
            dbw3_assert (scroll_x2+_i<Data->MAXX2);
            freietritte[scroll_x2+_i] = false;
        } else {
            dbw3_assert (scroll_x2+_i<Data->MAXX2);
            freietritte[scroll_x2+_i] = true;
            for (int j=0; j<Data->MAXY2; j++)
                if (trittfolge.feld.Get (scroll_x1+_i, j)>0) {
                    freietritte[scroll_x2+_i] = false;
                    break;
                }
        }
    } else if (!trittfolge.einzeltritt) {
        char oldstate = trittfolge.feld.Get(scroll_x2+_i,scroll_y2+_j);
        trittfolge.feld.Set (scroll_x2+_i, scroll_y2+_j, char(oldstate<=0 ? 1 : -oldstate));
        if (oldstate<0) {
            dbw3_assert (scroll_x2+_i<Data->MAXX2);
            freietritte[scroll_x2+_i] = false;
        } else {
            dbw3_assert (scroll_x2+_i<Data->MAXX2);
            freietritte[scroll_x2+_i] = true;
            for (int j=0; j<Data->MAXY2; j++)
                if (trittfolge.feld.Get (scroll_x1+_i, j)>0) {
                    freietritte[scroll_x2+_i] = false;
                    break;
                }
        }
    } else {
        bool bSet = trittfolge.feld.Get (scroll_x2+_i, scroll_y2+_j);
        for (int i=0; i<Data->MAXX2; i++) trittfolge.feld.Set (i, scroll_y2+_j, 0);
        trittfolge.feld.Set (scroll_x2+_i, scroll_y2+_j, bSet ? (char)0 : (char)1);
        RecalcFreieTritte();
    }

    RecalcTrittfolgeEmpty (_j+scroll_y2);

    // Faden neu berechnen
    int i, j, k;
    for (i=0; i<Data->MAXX1; i++) gewebe.feld.Set (i, _j+scroll_y2, 0);
    for (i=0; i<Data->MAXX2; i++) {
        char t = trittfolge.feld.Get(i, _j+scroll_y2);
        if (t>0)
            for (j=0; j<Data->MAXY1; j++) {
                char s = aufknuepfung.feld.Get (i, j);
                if (s>0)
                    for (k=0; k<Data->MAXX1; k++)
                        if (einzug.feld.Get (k)==j+1)
                            gewebe.feld.Set (k, _j+scroll_y2, ViewSchlagpatrone->Checked ? t : s);
            }
    }

    // Belegter Bereich nachführen
//    UpdateRange (-1, _j+scroll_y2, !IsEmptyTrittfolge(_j+scroll_y2));
    CalcRangeKette();
    CalcRangeSchuesse();

	// Neuzeichnen nur wenn im sichtbaren Bereich
    if (_j<0 || _j>=trittfolge.pos.height/trittfolge.gh) return;

    // Tritte neuzeichnen
    for (i=scroll_x2; i<scroll_x2+trittfolge.pos.width/trittfolge.gw; i++) {
        DrawTrittfolge (i-scroll_x2, _j);
    }

    // Faden neu zeichnen
    if (!trittfolge.isempty.Get(_j+scroll_y2)) DrawGewebeSchuss (_j);
    else DeleteGewebeSchuss (_j);
}
/*-----------------------------------------------------------------*/
