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
bool __fastcall TDBWFRM::KettfadenEqual (int _a, int _b)
{
    for (int j=0; j<Data->MAXY2; j++) {
        char s1 = gewebe.feld.Get (_a, j);
        char s2 = gewebe.feld.Get (_b, j);
        if ((s1<=0 && s2>0) || (s1>0 && s2<=0)) return false;
        if (s1>0 && s2>0 && s1!=s2) return false;
    }
    return true;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SetGewebe (int _i, int _j, bool _set, int _range)
{
    if (GewebeNone->Checked) {
        MessageBeep (MB_OK);
        return;
    }

    TCursor oldcursor = Cursor;
    Cursor = crHourGlass;

    // Punkt setzen
    DoSetGewebe (_i, _j, _set, _range);

    // ggf. Punkt replizieren!
    if (RappViewRapport->Checked && IsInRapport(_i+scroll_x1, _j+scroll_y2)) {
        int i0 = _i+scroll_x1;
        int j0 = _j+scroll_y2;

        if (i0>kette.a) {
            while (i0>kette.a) i0 -= rapport.kr.count();
            if (i0<kette.a) i0 += rapport.kr.count();
        }

        if (j0>schuesse.a) {
            while (j0>schuesse.a) j0 -= rapport.sr.count();
            if (j0<schuesse.a) j0 += rapport.sr.count();
        }

        int i = i0;
        while (i<=kette.b) {
            int j = j0;
            while (j<=schuesse.b) {
                if (i!=_i+scroll_x1 || j!=_j+scroll_y2) {
                    // Gewebepunkt toggeln
                    if (!_set) ToggleGewebe (i, j);
                    else gewebe.feld.Set (i, j, char(_range));

                    // Einzug kopieren
                    einzug.feld.Set (i, einzug.feld.Get (_i+scroll_x1));

                    // Tritt kopieren
                    for (int ii=0; ii<Data->MAXX2; ii++)
                        trittfolge.feld.Set (ii, j, trittfolge.feld.Get (ii, _j+scroll_y2));
                    trittfolge.isempty.Set (j, trittfolge.isempty.Get (_j+scroll_y2));

                    // Neuzeichnen falls sichtbar
                    if (i>=scroll_x1 && i<scroll_x1+gewebe.pos.width/gewebe.gw &&
                        j>=scroll_y2 && j<scroll_y2+gewebe.pos.height/gewebe.gh)
                    {
                        // Feld neuzeichnen
                        DrawGewebe (i-scroll_x1, j-scroll_y2);

                        // Einzug neuzeichnen
                        for (int jj=scroll_y1; jj<scroll_y1+einzug.pos.height/einzug.gh; jj++)
                            DrawEinzug (i-scroll_x1, jj-scroll_y1);

                        // Tritt neuzeichnen
                        for (int ii=scroll_x2; ii<scroll_x2+trittfolge.pos.width/trittfolge.gw; ii++)
                            DrawTrittfolge (ii-scroll_x2, j-scroll_y2);
                    }
                }
                j += rapport.sr.count();
            }
            i += rapport.kr.count();
        }
        RecalcFreieSchaefte();
        RecalcFreieTritte();
    }

    RearrangeSchaefte();

    if (!ViewSchlagpatrone->Checked) {
        EliminateEmptyTritt();
        RearrangeTritte();
    }

    UpdateRapport();

    Cursor = oldcursor;
    SetModified();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DoSetGewebe (int _i, int _j, bool _set, int _range)
{
    int i, j, k;
    bool wasemptyeinzug = IsEmptyEinzug (_i+scroll_x1);
    bool wasemptytrittfolge = IsEmptyTrittfolge (_j+scroll_y2);

    // Feld toggeln
    char oldstate = gewebe.feld.Get (_i+scroll_x1, _j+scroll_y2);
    if (!_set) gewebe.feld.Set (_i+scroll_x1, _j+scroll_y2, char(oldstate<=0 ? currentrange : -oldstate));
    else gewebe.feld.Set (_i+scroll_x1, _j+scroll_y2, char(_range));
    if (!_set) UpdateRange (_i+scroll_x1, _j+scroll_y2, oldstate<=0);
    else UpdateRange (_i+scroll_x1, _j+scroll_y2, _range>0);

    // Ist Neuzeichnen oder Berechnen erforderlich
    if (_i<0 || _i>=gewebe.pos.width/gewebe.gw ||
        _j<0 || _j>=gewebe.pos.height/gewebe.gh)
        return;

    // Feld löschen (wird später neugezeichnet)
    ClearGewebe (_i+scroll_x1, _j+scroll_y2);

    // Einzug löschen
    int oldeinzug = einzug.feld.Get (_i+scroll_x1);
    einzug.feld.Set (_i+scroll_x1, 0);
    if (ViewEinzug->Checked) {
        if (oldeinzug!=0 && oldeinzug-1>=scroll_y1 && oldeinzug-1<scroll_y1+einzug.pos.height/einzug.gh) {
            DrawEinzug (_i, oldeinzug-1-scroll_y1);
        }
    }
    if (oldeinzug!=0) {
        freieschaefte[oldeinzug-1] = true;
        for (int i=0; i<Data->MAXX1; i++)
            if (einzug.feld.Get(i)==oldeinzug) {
                freieschaefte[oldeinzug-1] = false;
                break;
            }
    }

    // Bestimmen, ob Einzug berechnet werden muss
    // dies ist nicht der Fall, wenn der Kettfaden nur
    // Senkungen aufweist.
    bool bNeedEinzugRecalc = false;
    for (j=0; j<Data->MAXY2; j++) {
        char s = gewebe.feld.Get (_i+scroll_x1, j);
        if (s>0) {
            bNeedEinzugRecalc = true;
            break;
        }
    }

    // Einzug neuberechnen
    short neweinzug = 0;
    if (bNeedEinzugRecalc) {
        // Schaft ermitteln... (Fixeinzug ist im rearrangeschaefte!)
        bool bEqualCol = false;
        for (i=0; i<Data->MAXX1; i++) {
            if (i==_i+scroll_x1) continue;
            bEqualCol = true;
            for (j=0; j<Data->MAXY2; j++) {
                if (IsEmptyEinzug(i) || (gewebe.feld.Get(i, j)!=gewebe.feld.Get(_i+scroll_x1, j))) {
                    bEqualCol = false;
                    break;
                }
            }
            if (bEqualCol) break;
        }
        if (bEqualCol) neweinzug = einzug.feld.Get (i);
        else neweinzug = GetFreeEinzug();
        // ...und entsprechend setzen
        einzug.feld.Set (_i+scroll_x1, neweinzug);
        if (neweinzug>0) freieschaefte[neweinzug-1] = false;
        if (!bEqualCol && ViewSchlagpatrone->Checked) {
            for (int j=0; j<Data->MAXY2; j++) {
                char s = gewebe.feld.Get(_i+scroll_x1, j);
                if (s>0) {
                    if (neweinzug!=0) {
                        trittfolge.feld.Set (neweinzug-1, j, s);
                        dbw3_assert(neweinzug-1<Data->MAXX2);
                        freietritte[neweinzug-1] = false;
                        int k = neweinzug-1-scroll_x2;
                        if (j>=scroll_y2 && j<scroll_y2+trittfolge.pos.height/trittfolge.gh &&
                            k>=0 && k<trittfolge.pos.width/trittfolge.gw)
                            DrawTrittfolge (neweinzug-1-scroll_x2, j-scroll_y2);
                    }
                }
            }
        }
    }

    // Falls Schaft leer: Aufknuepfung anpassen!
    if (oldeinzug!=0) {
        if (freieschaefte[oldeinzug-1]) {
            if (!ViewSchlagpatrone->Checked) {
                for (i=0; i<Data->MAXX2; i++) {
                    aufknuepfung.feld.Set (i, oldeinzug-1, 0);
                    RedrawAufknuepfung (i, oldeinzug-1);
                }
            } else {
                for (j=0; j<Data->MAXY2; j++) {
                    trittfolge.feld.Set (oldeinzug-1, j, 0);
                    RecalcTrittfolgeEmpty (j);
                    if (j>=scroll_y2 && j<scroll_y2+trittfolge.pos.height/trittfolge.gh)
                        DrawTrittfolge (oldeinzug-1, j);
                }
                dbw3_assert(oldeinzug-1<Data->MAXX2);
                freietritte[oldeinzug-1] = true;
            }
        }
    }

    // Einzug neuzeichnen
    if (ViewEinzug->Checked) {
        if (neweinzug!=0 && neweinzug-1>=scroll_y1 && neweinzug-1<scroll_y1+einzug.pos.height/einzug.gh) {
            DrawEinzug (_i, neweinzug-1-scroll_y1);
        }
    }

    // Trittfolge löschen
    for (i=0; i<Data->MAXX2; i++) {
        char s = trittfolge.feld.Get (i, _j+scroll_y2);
        if (s>0) {
            trittfolge.feld.Set (i, _j+scroll_y2, 0);
            bool bEmptyTritt = true;
            for (j=0; j<Data->MAXY2; j++)
                if (trittfolge.feld.Get (i, j)>0) {
                    bEmptyTritt = false;
                    break;
                }
            if (bEmptyTritt && !ViewSchlagpatrone->Checked)
                for (j=0; j<Data->MAXY1; j++) {
                    aufknuepfung.feld.Set (i, j, 0);
                    RedrawAufknuepfung (i, j);
                }
            if (bEmptyTritt) freietritte[i] = true;
        }
    }
    if (ViewTrittfolge->Checked) {
        for (i=scroll_x2; i<scroll_x2+trittfolge.pos.width/trittfolge.gw; i++) {
            DrawTrittfolge (i-scroll_x2, _j);
        }
    }

    // Bestimmen, ob Trittfolge berechnet werden muss
    // dies ist nicht der Fall, wenn der Schussfaden nur
    // Hebungen aufweist.
    bool bNeedTrittfolgeRecalc = false;
    for (i=0; i<Data->MAXX1; i++) {
        char s = gewebe.feld.Get (i, _j+scroll_y2);
        if (s>0) {
            bNeedTrittfolgeRecalc = true;
            break;
        }
    }

    // Trittfolge neuberechnen
    if (bNeedTrittfolgeRecalc) {
        if (!ViewSchlagpatrone->Checked) {
            bool bEqualRow = false;
            for (j=0; j<Data->MAXY2; j++) {
                if (j==_j+scroll_y2) continue;
                bEqualRow = true;
                if (IsEmptyTrittfolge(j)) bEqualRow = false;
                else {
                    for (i=0; i<Data->MAXX1; i++) {
                        if (gewebe.feld.Get(i, j)!=gewebe.feld.Get(i, _j+scroll_y2)) {
                            bEqualRow = false;
                            break;
                        }
                    }
                }
                if (bEqualRow) break;
            }
            if (bEqualRow) CopyTritt (j, _j+scroll_y2);
            else {
                short freetritt = GetFreeTritt();
                if (freetritt!=-1) {
                    trittfolge.feld.Set (freetritt, _j+scroll_y2, 1);
                    dbw3_assert(freetritt<Data->MAXX2);
                    freietritte[freetritt] = false;
                }
            }
        } else {
            for (int i=0; i<Data->MAXX1; i++) {
                char c = gewebe.feld.Get (i, _j+scroll_y2);
                if (c>0) {
                    int jj = einzug.feld.Get(i);
                    if (jj!=0) {
                        trittfolge.feld.Set (jj-1, _j+scroll_y2, c);
                        dbw3_assert (jj-1<Data->MAXX2);
                        freietritte[jj-1] = false;
                        trittfolge.isempty.Set (_j+scroll_y2, false);
                    }
                }
            }
        }
    }

    // Tritte neu zeichnen
    if (ViewTrittfolge->Checked) {
        for (int i=scroll_x2; i<scroll_x2+trittfolge.pos.width/trittfolge.gw; i++) {
            DrawTrittfolge (i-scroll_x2, _j);
        }
    }

    RecalcTrittfolgeEmpty (_j+scroll_y2);

    // Aufknuepfung nachführen
    if (!ViewSchlagpatrone->Checked) {
        // Schussfaden
        for (i=0; i<Data->MAXX1; i++)
            if (einzug.feld.Get(i))
                for (k=0; k<Data->MAXX2; k++)
                    if (trittfolge.feld.Get (k, _j+scroll_y2)>0) {
                        aufknuepfung.feld.Set (k, einzug.feld.Get(i)-1, gewebe.feld.Get(i, _j+scroll_y2));
                        RedrawAufknuepfung (k, einzug.feld.Get(i)-1);
                    }
        // Kettfaden
        if (einzug.feld.Get(_i+scroll_x1))
            for (j=0; j<Data->MAXY2; j++)
                for (k=0; k<Data->MAXX2; k++)
                    if (trittfolge.feld.Get (k, j)>0) {
                        aufknuepfung.feld.Set (k, einzug.feld.Get(_i+scroll_x1)-1, gewebe.feld.Get(_i+scroll_x1, j));
                        RedrawAufknuepfung (k, einzug.feld.Get(_i+scroll_x1)-1);
                    }
    }

    // Wenn neuer Faden: Senkungen auch zeichnen
    if (wasemptyeinzug!=IsEmptyEinzug (_i+scroll_x1)) {
        for (j=scroll_y2; j<scroll_y2+gewebe.pos.height/gewebe.gh; j++) {
            if (IsEmptyTrittfolge (j)) continue;
            DrawGewebe (_i, j-scroll_y2);
            if (IsEmptyEinzug(_i+scroll_x1)) DrawGewebeRahmen (_i, j-scroll_y2);
        }
    }

    // Wenn neuer Schuss: Senkungen auch zeichnen
    if (wasemptytrittfolge!=IsEmptyTrittfolge (_j+scroll_y2)) {
        for (i=scroll_x1; i<scroll_x1+gewebe.pos.width/gewebe.gw; i++) {
            if (IsEmptyEinzug (i)) continue;
            DrawGewebe (i-scroll_x1, _j);
            if (IsEmptyTrittfolge(_j+scroll_y2)) DrawGewebeRahmen (i-scroll_x1, _j);
        }
    }
}
/*-----------------------------------------------------------------*/
short __fastcall TDBWFRM::GetFreeEinzug()
{
    for (int i=0; i<Data->MAXY1; i++)
        if (freieschaefte[i]==true) return short(i+1);
    short free = short(Data->MAXY1);
    ExtendSchaefte();
    dbw3_assert (freieschaefte[free]==true);
    return free;
}
/*-----------------------------------------------------------------*/
short __fastcall TDBWFRM::GetFreeTritt()
{
    for (short i=0; i<Data->MAXX2; i++)
        if (freietritte[i]==true) return i;
    short free = short(Data->MAXX2);
    ExtendTritte();
    dbw3_assert (freietritte[free]==true);
    return free;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::CopyTritt (int _von, int _nach)
{
    for (int i=0; i<Data->MAXX2; i++)
        trittfolge.feld.Set (i, _nach, trittfolge.feld.Get (i, _von));
}
/*-----------------------------------------------------------------*/

