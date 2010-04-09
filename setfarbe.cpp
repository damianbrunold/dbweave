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
#include "felddef.h"
#include "dbw3_strings.h"
#include "datamodule.h"
#include "undoredo.h"
#include "palette.h"
/*-----------------------------------------------------------------*/
#define GETPALCOL(col) \
    ((TColor)Data->palette->GetColor(col))
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SetKettfarben (int _i)
{
    DoSetKettfarben (_i);

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DoSetKettfarben (int _i)
{
    kettfarben.feld.Set (_i+scroll_x1, Data->color);

    // Feld neuzeichnen
    Canvas->Pen->Color = clBtnShadow;
    Canvas->Brush->Color = GETPALCOL(kettfarben.feld.Get(_i+scroll_x1));
    if (righttoleft) {
        Canvas->Rectangle (kettfarben.pos.x0+kettfarben.pos.width-(_i+1)*kettfarben.gw, kettfarben.pos.y0, kettfarben.pos.x0+kettfarben.pos.width-_i*kettfarben.gw+1, kettfarben.pos.y0+kettfarben.pos.height+1);
    } else {
        Canvas->Rectangle (kettfarben.pos.x0+_i*kettfarben.gw, kettfarben.pos.y0, kettfarben.pos.x0+(_i+1)*kettfarben.gw+1, kettfarben.pos.y0+kettfarben.pos.height+1);
    }

    // Kettfaden neuzeichen falls FE oder GS
    if (GewebeFarbeffekt->Checked || GewebeSimulation->Checked)
        DrawGewebeKette (_i);

    SetModified();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SetSchussfarben (int _j)
{
    DoSetSchussfarben (_j);

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DoSetSchussfarben (int _j)
{
    schussfarben.feld.Set (_j+scroll_y2, Data->color);

    // Feld neuzeichnen
    Canvas->Pen->Color = clBtnShadow;
    Canvas->Brush->Color = GETPALCOL(schussfarben.feld.Get(_j+scroll_y2));
    Canvas->Rectangle (schussfarben.pos.x0, schussfarben.pos.y0+schussfarben.pos.height-(_j+1)*schussfarben.gh, schussfarben.pos.x0+schussfarben.pos.width+1, schussfarben.pos.y0+schussfarben.pos.height-_j*schussfarben.gh+1);

    // Schussfaden
    if (GewebeFarbeffekt->Checked || GewebeSimulation->Checked)
        DrawGewebeSchuss (_j);

    SetModified();
}
/*-----------------------------------------------------------------*/
