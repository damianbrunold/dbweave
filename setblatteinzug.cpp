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
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SetBlatteinzug (int _i)
{
    DoSetBlatteinzug (_i);

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DoSetBlatteinzug (int _i)
{
    // Feld setzen
    blatteinzug.feld.Set (_i+scroll_x1, !blatteinzug.feld.Get(_i+scroll_x1));

    // Feld neuzeichnen
    int x;
    if (righttoleft) x = blatteinzug.pos.x0+blatteinzug.pos.width - (_i+1)*blatteinzug.gw;
    else x = blatteinzug.pos.x0+_i*blatteinzug.gw;
    if (blatteinzug.feld.Get(_i+scroll_x1)==0) {
        Canvas->Pen->Color = clBtnShadow;
        Canvas->Brush->Color = clBtnFace;
        Canvas->Rectangle (x, blatteinzug.pos.y0, x+blatteinzug.gw+1, blatteinzug.pos.y0+blatteinzug.pos.height+1);
        Canvas->Pen->Color = clBlack;
        Canvas->Brush->Color = clBlack;
        Canvas->Rectangle (x, blatteinzug.pos.y0+blatteinzug.pos.height/2, x+blatteinzug.gw+1, blatteinzug.pos.y0+blatteinzug.pos.height);
    } else {
        Canvas->Pen->Color = clBtnShadow;
        Canvas->Brush->Color = clBtnFace;
        Canvas->Rectangle (x, blatteinzug.pos.y0, x+blatteinzug.gw+1, blatteinzug.pos.y0+blatteinzug.pos.height+1);
        Canvas->Pen->Color = clBlack;
        Canvas->Brush->Color = clBlack;
        Canvas->Rectangle (x, blatteinzug.pos.y0+1, x+blatteinzug.gw+1, blatteinzug.pos.y0+blatteinzug.pos.height/2);
    }
    Canvas->Pen->Color = clBtnShadow;
    Canvas->MoveTo (x, blatteinzug.pos.y0+1);
    Canvas->LineTo (x, blatteinzug.pos.y0+blatteinzug.pos.height);

    SetModified();
}
/*-----------------------------------------------------------------*/

