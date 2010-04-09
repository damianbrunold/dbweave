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
#include "datamodule.h"
#include "felddef.h"
/*-----------------------------------------------------------------*/
// CalcRangeSchuesse: Berechnet den belegten Schussbereich
void __fastcall TDBWFRM::CalcRangeSchuesse()
{
    schuesse.a = schuesse.b = -1;
    bool first = true;
    for (int j=0; j<Data->MAXY2; j++) {
        if (!IsEmptySchuss (j)) {
            if (first) { schuesse.a = j; first = false; }
            else schuesse.b = j;
        }
    }
    if (schuesse.a!=-1 && schuesse.b==-1) schuesse.b = schuesse.a;
}
/*-----------------------------------------------------------------*/
// CalcRangeKette: Berechnet den belegten Kettbereich
void __fastcall TDBWFRM::CalcRangeKette()
{
    kette.a = kette.b = -1;
    bool first = true;
    for (int i=0; i<Data->MAXX1; i++) {
        if (!IsEmptyKette (i)) {
            if (first) { kette.a = i; first = false; }
            else kette.b = i;
        }
    }
    if (kette.a!=-1 && kette.b==-1) kette.b = kette.a;
}
/*-----------------------------------------------------------------*/
// CalcRange: Berechnet den belegten Kett- und Schussbereich
void __fastcall TDBWFRM::CalcRange()
{
    CalcRangeKette();
    CalcRangeSchuesse();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::UpdateRange (int _i, int _j, bool _set)
{
    // _i==-1 nur Schuesse updaten
    // _j==-1 nur Kette updaten

    SZ oldkette = kette;
    SZ oldschuesse = schuesse;

    // Kette nachführen
    if (_i!=-1) {
        if (_set) {
            if (_i<kette.a) kette.a = _i;
            else if (_i>kette.b) kette.b = _i;
            if (kette.a==-1) kette.a = kette.b;
            if (kette.b==-1) kette.b = kette.a;
        } else {
            if (_i==kette.a) {
                for (int i=kette.a; i<=kette.b; i++)
                    if (!IsEmptyKetteNurGewebe(i)) {
                        kette.a = i;
                        goto g_k_weiter;
                    }
                kette.a = kette.b = -1;
            } else if (_i==kette.b) {
                for (int i=kette.b; i>=kette.a; i--)
                    if (!IsEmptyKetteNurGewebe(i)) {
                        kette.b = i;
                        goto g_k_weiter;
                    }
                kette.a = kette.b = -1;
            }
g_k_weiter:
        }
    }

    // Schüsse nachführen
    if (_j!=-1) {
        if (_set) {
            if (_j<schuesse.a) schuesse.a = _j;
            else if (_j>schuesse.b) schuesse.b = _j;
            if (schuesse.a==-1) schuesse.a = schuesse.b;
            if (schuesse.b==-1) schuesse.b = schuesse.a;
        } else {
            if (_j==schuesse.a) {
                for (int j=schuesse.a; j<=schuesse.b; j++)
                    if (!IsEmptySchussNurGewebe(j)) {
                        schuesse.a = j;
                        goto g_s_weiter;
                    }
                schuesse.a = schuesse.b = -1;
            } else if (_j==schuesse.b) {
                for (int j=schuesse.b; j>=schuesse.a; j--)
                    if (!IsEmptySchussNurGewebe(j)) {
                        schuesse.b = j;
                        goto g_s_weiter;
                    }
                schuesse.a = schuesse.b = -1;
            }
g_s_weiter:
        }
    }

    if (kette.a!=oldkette.a || kette.b!=oldkette.b ||
        schuesse.a!=oldschuesse.a || schuesse.b!=oldschuesse.b)
    {
        UpdateStatusBar();
    }
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::IsEmptySchuss (int _j)
{
    // Hier muss auch die Trittfolge abgecheckt werden,
    // Weil sonst SetAufknuepfung nicht mehr funktioniert,
    // da u. U. die range zu klein wird...
    if (!trittfolge.isempty.Get(_j)) return false;
    for (int i=0; i<Data->MAXX1; i++)
        if (gewebe.feld.Get (i, _j)>0) return false;
    return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::IsEmptyKette (int _i)
{
    // Hier muss auch der Einzug abgecheckt werden,
    // Weil sonst SetAufknuepfung nicht mehr funktioniert,
    // da u. U. die range zu klein wird...
    if (einzug.feld.Get(_i)!=0) return false;
    for (int j=0; j<Data->MAXY2; j++)
        if (gewebe.feld.Get (_i, j)>0) return false;
    return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::IsEmptySchussNurGewebe (int _j)
{
    for (int i=0; i<Data->MAXX1; i++)
        if (gewebe.feld.Get (i, _j)>0) return false;
    return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::IsEmptyKetteNurGewebe (int _i)
{
    for (int j=0; j<Data->MAXY2; j++)
        if (gewebe.feld.Get (_i, j)>0) return false;
    return true;
}
/*-----------------------------------------------------------------*/

