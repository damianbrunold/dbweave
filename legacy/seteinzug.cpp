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
void __fastcall TDBWFRM::SetEinzug (int _i, int _j)
{
    if (ViewOnlyGewebe->Checked) {
        MessageBeep (MB_OK);
        return;
    }

    DoSetEinzug (_i, _j);

    // ggf. Punkt replizieren!
    if (RappViewRapport->Checked && (_i+scroll_x1)>=rapport.kr.a && (_i+scroll_x1)<=rapport.kr.b) {
        int i = _i+scroll_x1;

        if (i>kette.a) {
            while (i>kette.a) i -= rapport.kr.count();
            i += rapport.kr.count();
        }

        while (i<=kette.b) {
            if (i!=_i+scroll_x1) {
                // Einzug kopieren
                short neweinzug = einzug.feld.Get(scroll_x1+_i);
                einzug.feld.Set (i, neweinzug);

                // Kettfaden kopieren
                for (int jj=0; jj<Data->MAXY2; jj++)
                    gewebe.feld.Set (i, jj, gewebe.feld.Get (_i+scroll_x1, jj));

                // Neuzeichnen falls sichtbar
                if (i>=scroll_x1 && i<scroll_x1+einzug.pos.width/einzug.gw) {
                    // Einzug neuzeichnen
                    for (int jj=scroll_y1; jj<scroll_y1+einzug.pos.height/einzug.gh; jj++)
                        DrawEinzug (i-scroll_x1, jj-scroll_y1);

                    // Kettfaden neuzeichnen
                    if (neweinzug!=0) DrawGewebeKette (i-scroll_x1);
                    else DeleteGewebeKette (i-scroll_x1);
                }
            }
            i += rapport.kr.count();
        }
        RecalcFreieSchaefte();
    }

    // Falls Einzug fixiert, dann Daten updaten
//    if (EzEinzugFixiert->Checked)
//        UpdateEinzugFixiert();

    UpdateRapport();

    SetModified();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DoSetEinzug (int _i, int _j)
{
    // Einzug neu setzen
    short oldeinzug = einzug.feld.Get(scroll_x1+_i);
    short neweinzug = (short)(scroll_y1+_j+1);
    if (oldeinzug==neweinzug) neweinzug = 0;
    einzug.feld.Set(scroll_x1+_i, neweinzug);
    if (neweinzug!=0)
        freieschaefte[neweinzug-1] = false;
    if (oldeinzug!=0) {
        freieschaefte[oldeinzug-1] = true;
        for (int i=0; i<Data->MAXX1; i++)
            if (einzug.feld.Get(i)==oldeinzug) {
                freieschaefte[oldeinzug-1] = false;
                break;
            }
    }

    // Kettfaden neuberechnen
    int i, j, k;
    for (j=0; j<Data->MAXY2; j++) gewebe.feld.Set (scroll_x1+_i, j, 0);
    for (i=0; i<Data->MAXX2; i++) {
        char s = aufknuepfung.feld.Get (i, neweinzug-1);
        if (s>0)
            for (k=0; k<Data->MAXY2; k++)
                if (trittfolge.feld.Get (i, k)>0)
                    gewebe.feld.Set (scroll_x1+_i, k, s);
    }

    // Belegter Bereich nachführen
//    UpdateRange (_i+scroll_x1, -1, neweinzug!=0);
    CalcRangeKette();
    CalcRangeSchuesse();

	// Neuzeichnen nur wenn im sichtbaren Bereich
    if (_i<0 || _i>=einzug.pos.width/einzug.gw) return;

    // Altes Einzugfeld löschen und neues zeichnen
    if (oldeinzug!=0  && oldeinzug-1>=scroll_y1 && oldeinzug-1<scroll_y1+einzug.pos.height/einzug.gh)
        DrawEinzug (_i, oldeinzug-1-scroll_y1);

    // Neues Einzugfeld zeichnen
    if (neweinzug!=0 && neweinzug-1>=scroll_y1 && neweinzug-1<scroll_y1+einzug.pos.height/einzug.gh) {
        DrawEinzug (_i, neweinzug-1-scroll_y1);
    }

    // Kettfaden neuzeichnen
    if (neweinzug!=0) DrawGewebeKette (_i);
    else DeleteGewebeKette (_i);
}
/*-----------------------------------------------------------------*/
