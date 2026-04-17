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
#include "undoredo.h"
#include "palette.h"
#include "farbauswahl2_form.h"
/*-----------------------------------------------------------------*/
int __fastcall TDBWFRM::SelectColorIndex (int _index)
{
    int idx = 1;
    try {
        TChoosePaletteForm* frm = new TChoosePaletteForm(this);
        frm->SelectColor (Data->palette->GetColor(_index));
        if (frm->ShowModal()==mrOk) {
            idx = frm->GetSelectedIndex();
        } else
            idx = -1;
        delete frm;
    } catch (...) {
    }
    return idx;
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::SelectColor (COLORREF& _col)
{
    bool success = false;
    try {
        TChoosePaletteForm* frm = new TChoosePaletteForm(this);
        if (frm->ShowModal()==mrOk) {
            _col = frm->GetSelectedColor();
            success = true;
        }
        delete frm;
    } catch (...) {
    }
    return success;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SetKettfarbeClick(TObject *Sender)
{
    int col = SelectColorIndex(Data->color);
    if (col==-1) return;
    for (int i=0; i<Data->MAXX1; i++)
        kettfarben.feld.Set (i, char(col));
    Data->color = (unsigned char)col;
    Statusbar->Invalidate();
    Invalidate();
    SetModified();
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SetSchussfarbeClick(TObject *Sender)
{
    int col = SelectColorIndex(Data->color);
    if (col==-1) return;
    for (int i=0; i<Data->MAXY2; i++)
        schussfarben.feld.Set (i, char(col));
    Data->color = (unsigned char)col;
    Statusbar->Invalidate();
    Invalidate();
    SetModified();
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ReplaceColorClick(TObject *Sender)
{
    int col;
    if (kbd_field==KETTFARBEN) {
        col = kettfarben.feld.Get (scroll_x1+kettfarben.kbd.i);
        int newcol = SelectColorIndex(col);
        if (newcol==-1) return;
        for (int i=0; i<Data->MAXX1; i++)
            if (col==kettfarben.feld.Get(i))
                kettfarben.feld.Set (i, char(newcol));
    } else if (kbd_field==SCHUSSFARBEN) {
        col = schussfarben.feld.Get (scroll_y2+schussfarben.kbd.j);
        int newcol = SelectColorIndex(col);
        if (newcol==-1) return;
        for (int i=0; i<Data->MAXY2; i++)
            if (col==schussfarben.feld.Get(i))
                schussfarben.feld.Set (i, char(newcol));
    } else return;

    Data->color = (unsigned char)col;
    Statusbar->Invalidate();
    Invalidate();
    SetModified();
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SwitchColorsClick(TObject *Sender)
{
    int maxi = Data->MAXX1>Data->MAXY2 ? Data->MAXY2 : Data->MAXX1;

    for (int i=0; i<maxi; i++) {
        char col = kettfarben.feld.Get (i);
        kettfarben.feld.Set (i, schussfarben.feld.Get(i));
        schussfarben.feld.Set (i, col);
    }

    Invalidate();
    SetModified();
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/

