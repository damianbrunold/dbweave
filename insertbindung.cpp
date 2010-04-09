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
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "dbw3_form.h"
#include "datamodule.h"
#include "undoredo.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::KoeperEinfuegen (int _h, int _s)
{
    if (kbd_field!=GEWEBE) return;

    // Einfügeposition bestimmen
    int posi = gewebe.kbd.i + scroll_x1;
    int posj = gewebe.kbd.j + scroll_y2;

    // Bereich löschen
//    for (int i=posi; i<posi+_h+_s; i++)
//        for (int j=posj; j<posj+_h+_s; j++)
//            if (i<Data->MAXX1 && j<Data->MAXY2)
//                gewebe.feld.Set (i, j, 0);

    // Köper einfügen
    int n = _h + _s;
    if (_h<=_s) {
        for (int i=0; i<n; i++)
            for (int j=i; j<i+_h; j++)
                if (posi+i<Data->MAXX1 && posj+(j%n)<Data->MAXY2)
                    gewebe.feld.Set (posi+i, posj+(j%n), currentrange);
    } else {
        for (int i=0; i<n; i++)
            for (int j=i+_s; j<i+n; j++)
                if (posi+i<Data->MAXX1 && posj+(j%n)<Data->MAXY2)
                    gewebe.feld.Set (posi+i, posj+(j%n), currentrange);
    }

    // Selektion setzen
    selection.begin.i = posi;
    selection.begin.j = posj;
    selection.end.i = selection.begin.i + _h+_s - 1;
    selection.end.j = selection.begin.j + _h+_s - 1;
    if (selection.end.i>=scroll_x1+gewebe.pos.width/gewebe.gw) selection.end.i = scroll_x1 + gewebe.pos.width/gewebe.gw - 1;
    if (selection.end.j>=scroll_y2+gewebe.pos.height/gewebe.gh) selection.end.j = scroll_y2 + gewebe.pos.height/gewebe.gh - 1;
    selection.feld = GEWEBE;

    // Abschlussarbeiten
    RecalcAll();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    Invalidate();
    SetModified();
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
static void __fastcall FSet (FeldGridChar& _m, int _i, int _j, char _s)
{
    if (_i<Data->MAXX1 && _j<Data->MAXY2)
        _m.Set (_i, _j, _s);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::AtlasEinfuegen (int _n)
{
    if (kbd_field!=GEWEBE) return;

    // Einfügeposition bestimmen
    int posi = gewebe.kbd.i + scroll_x1;
    int posj = gewebe.kbd.j + scroll_y2;

    // Bereich löschen
//    for (int i=posi; i<posi+_n; i++)
//        for (int j=posj; j<posj+_n; j++)
//            if (i<Data->MAXX1 && j<Data->MAXY2)
//                gewebe.feld.Set (i, j, 0);

    // Atlas einfügen
    char s = currentrange;
    FeldGridChar& m = gewebe.feld;
    switch (_n) {
        case 5:
            FSet (m, posi+0, posj+0, s);
            FSet (m, posi+1, posj+2, s);
            FSet (m, posi+2, posj+4, s);
            FSet (m, posi+3, posj+1, s);
            FSet (m, posi+4, posj+3, s);
            break;
        case 6:
            FSet (m, posi+0, posj+0, s);
            FSet (m, posi+1, posj+2, s);
            FSet (m, posi+2, posj+4, s);
            FSet (m, posi+3, posj+1, s);
            FSet (m, posi+4, posj+5, s);
            FSet (m, posi+5, posj+3, s);
            break;
        case 7:
            FSet (m, posi+0, posj+2, s);
            FSet (m, posi+1, posj+6, s);
            FSet (m, posi+2, posj+3, s);
            FSet (m, posi+3, posj+0, s);
            FSet (m, posi+4, posj+4, s);
            FSet (m, posi+5, posj+1, s);
            FSet (m, posi+6, posj+5, s);
            break;
        case 8:
            FSet (m, posi+0, posj+2, s);
            FSet (m, posi+1, posj+5, s);
            FSet (m, posi+2, posj+0, s);
            FSet (m, posi+3, posj+3, s);
            FSet (m, posi+4, posj+6, s);
            FSet (m, posi+5, posj+1, s);
            FSet (m, posi+6, posj+4, s);
            FSet (m, posi+7, posj+7, s);
            break;
        case 9:
            FSet (m, posi+0, posj+0, s);
            FSet (m, posi+1, posj+2, s);
            FSet (m, posi+2, posj+4, s);
            FSet (m, posi+3, posj+6, s);
            FSet (m, posi+4, posj+8, s);
            FSet (m, posi+5, posj+1, s);
            FSet (m, posi+6, posj+3, s);
            FSet (m, posi+7, posj+5, s);
            FSet (m, posi+8, posj+7, s);
            break;
        case 10:
            FSet (m, posi+0, posj+0, s);
            FSet (m, posi+1, posj+7, s);
            FSet (m, posi+2, posj+4, s);
            FSet (m, posi+3, posj+1, s);
            FSet (m, posi+4, posj+8, s);
            FSet (m, posi+5, posj+5, s);
            FSet (m, posi+6, posj+2, s);
            FSet (m, posi+7, posj+9, s);
            FSet (m, posi+8, posj+6, s);
            FSet (m, posi+9, posj+3, s);
            break;
    }

    // Selektion setzen
    selection.begin.i = posi;
    selection.begin.j = posj;
    selection.end.i = selection.begin.i + _n - 1;
    selection.end.j = selection.begin.j + _n - 1;
    if (selection.end.i>=scroll_x1+gewebe.pos.width/gewebe.gw) selection.end.i = scroll_x1 + gewebe.pos.width/gewebe.gw - 1;
    if (selection.end.j>=scroll_y2+gewebe.pos.height/gewebe.gh) selection.end.j = scroll_y2 + gewebe.pos.height/gewebe.gh - 1;
    selection.feld = GEWEBE;

    // Abschlussarbeiten
    RecalcAll();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    Invalidate();
    SetModified();
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Koeper2_2Click(TObject *Sender)
{
    KoeperEinfuegen (2, 2);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Koeper3_3Click(TObject *Sender)
{
    KoeperEinfuegen (3, 3);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Koeper4_4Click(TObject *Sender)
{
    KoeperEinfuegen (4, 4);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Koeper5_5Click(TObject *Sender)
{
    KoeperEinfuegen (5, 5);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Koeper2_1Click(TObject *Sender)
{
    KoeperEinfuegen (2, 1);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Koeper3_1Click(TObject *Sender)
{
    KoeperEinfuegen (3, 1);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Koeper4_1Click(TObject *Sender)
{
    KoeperEinfuegen (4, 1);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Koeper5_1Click(TObject *Sender)
{
    KoeperEinfuegen (5, 1);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Koeper3_2Click(TObject *Sender)
{
    KoeperEinfuegen (3, 2);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Koeper4_2Click(TObject *Sender)
{
    KoeperEinfuegen (4, 2);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Koeper5_2Click(TObject *Sender)
{
    KoeperEinfuegen (5, 2);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Koeper4_3Click(TObject *Sender)
{
    KoeperEinfuegen (4, 3);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Koeper5_3Click(TObject *Sender)
{
    KoeperEinfuegen (5, 3);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Atlas5Click(TObject *Sender)
{
    AtlasEinfuegen (5);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Atlas6Click(TObject *Sender)
{
    AtlasEinfuegen (6);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Atlas7Click(TObject *Sender)
{
    AtlasEinfuegen (7);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Atlas8Click(TObject *Sender)
{
    AtlasEinfuegen (8);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Atlas9Click(TObject *Sender)
{
    AtlasEinfuegen (9);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Atlas10Click(TObject *Sender)
{
    AtlasEinfuegen (10);
}
/*-----------------------------------------------------------------*/
