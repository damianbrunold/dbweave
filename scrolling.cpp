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
#include "dbw3_form.h"
#include "einzug.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::sb_vert1Scroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos)
{
    int oldy1;
    if (toptobottom) {
        oldy1 = scroll_y1;
        scroll_y1 = ScrollPos;
    } else {
        oldy1 = scroll_y1;
        scroll_y1 = sb_vert1->Max - ScrollPos;
    }

    if (oldy1==scroll_y1) return;

    InvalidateFeld (einzug.pos);
    InvalidateFeld (aufknuepfung.pos);
    InvalidateFeld (hlinevert1);
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::sb_vert2Scroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos)
{
    int oldy2 = scroll_y2;
    scroll_y2 = sb_vert2->Max - ScrollPos;

    if (oldy2==scroll_y2) return;

    InvalidateFeld (gewebe.pos);
    InvalidateFeld (trittfolge.pos);
    InvalidateFeld (schussfarben.pos);
    InvalidateFeld (hlinevert2);
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::sb_horz1Scroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos)
{
    int oldx1;
    if (righttoleft) {
        oldx1 = scroll_x1;
        scroll_x1 = sb_horz1->Max - ScrollPos;
    } else {
        oldx1 = scroll_x1;
        scroll_x1 = ScrollPos;
    }
    if (oldx1==scroll_x1) return;
    InvalidateFeld (blatteinzug.pos);
    InvalidateFeld (kettfarben.pos);
    InvalidateFeld (einzug.pos);
    InvalidateFeld (gewebe.pos);
    InvalidateFeld (hlinehorz1);
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::sb_horz2Scroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos)
{
    int oldx2 = scroll_x2;
    scroll_x2 = ScrollPos;

    if (oldx2==scroll_x2) return;

    InvalidateFeld (aufknuepfung.pos);
    InvalidateFeld (trittfolge.pos);
    InvalidateFeld (hlinehorz2);
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
