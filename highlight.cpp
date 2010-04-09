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
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "datamodule.h"
#include "cursor.h"
/*-----------------------------------------------------------------*/
#define HIGHLIGHTCOLOR clRed
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::_DrawHighlight (TColor _col)
{
    if (kbd_field==GEWEBE) {
        int i = gewebe.kbd.i + scroll_x1;
        int j = gewebe.kbd.j + scroll_y2;
        short ez = einzug.feld.Get(i);
        if (ez>scroll_y1 && ez<=einzug.pos.height/einzug.gh) {
            DrawEinzug (i-scroll_x1, ez-1-scroll_y1, _col);
            if (!ViewSchlagpatrone->Checked) {
                for (int ii=scroll_x2; ii<scroll_x2+trittfolge.pos.width/trittfolge.gw; ii++) {
                    char s = trittfolge.feld.Get(ii, j);
                    if (s>0) {
                        char s1 = gewebe.feld.Get(i, j);
                        if (s1==0 || aufknuepfung.feld.Get(ii, ez-1)>0) {
                            DrawTrittfolge (ii-scroll_x2, j-scroll_y2, _col);
                            DrawAufknuepfung (ii-scroll_x2, ez-1-scroll_y1, aufknuepfung.pegplanstyle, _col);
                        }
                    }
                }
            } else {
                if (ez-1>=scroll_x2 && ez-1<scroll_x2+trittfolge.pos.width/trittfolge.gw)
                    DrawTrittfolge (ez-1-scroll_x2, j-scroll_y2, _col);
            }
        }
    } else if (kbd_field==AUFKNUEPFUNG) {
        int i = aufknuepfung.kbd.i + scroll_x2;
        int j = aufknuepfung.kbd.j + scroll_y1;
        for (int ii=scroll_x1; ii<scroll_x1+einzug.pos.width/einzug.gw; ii++) {
            if (einzug.feld.Get(ii)==j+1) {
                DrawEinzug (ii-scroll_x1, j-scroll_y1, _col);
                if (GewebeNormal->Checked)
                    for (int jj=scroll_y2; jj<scroll_y2+trittfolge.pos.height/trittfolge.gh; jj++)
                        if (trittfolge.feld.Get(i, jj)>0) {
                            DrawTrittfolge (i-scroll_x2, jj-scroll_y2, _col);
                            DrawGewebe (ii-scroll_x1, jj-scroll_y2, _col);
                        }
            }
        }
    } else if (kbd_field==TRITTFOLGE) {
        if (ViewSchlagpatrone->Checked) {
            for (int ii=scroll_x1; ii<scroll_x1+gewebe.pos.width/gewebe.gw; ii++)
                if (einzug.feld.Get(ii)==trittfolge.kbd.i+scroll_x2+1)
                    DrawGewebe (ii-scroll_x1, trittfolge.kbd.j, _col);
            if (ViewEinzug->Checked) {
                for (int ii=scroll_x1; ii<scroll_x1+einzug.pos.width/einzug.gw; ii++)
                    if (einzug.feld.Get(ii)==trittfolge.kbd.i+scroll_x2+1)
                        DrawEinzug (ii-scroll_x1, trittfolge.kbd.i, _col);
            }
        } else {
            for (int ii=scroll_x1; ii<scroll_x1+gewebe.pos.width/gewebe.gw; ii++)
                if (gewebe.feld.Get (ii, trittfolge.kbd.j+scroll_y2)>0)
                    DrawGewebe (ii-scroll_x1, trittfolge.kbd.j, _col);
            for (int jj=scroll_y1; jj<scroll_y1+aufknuepfung.pos.height/aufknuepfung.gh; jj++)
                if (aufknuepfung.feld.Get(trittfolge.kbd.i+scroll_x2, jj)>0)
                    DrawAufknuepfung (trittfolge.kbd.i, jj-scroll_y1, aufknuepfung.pegplanstyle, _col);
        }
    } else if (kbd_field==EINZUG) {
        if (GewebeNormal->Checked) {
            for (int jj = scroll_y2; jj<scroll_y2+gewebe.pos.height/gewebe.gh; jj++)
                if (gewebe.feld.Get (einzug.kbd.i+scroll_x1, jj)>0)
                    DrawGewebe (einzug.kbd.i, jj, _col);
        }
        if (!ViewSchlagpatrone->Checked) {
            for (int ii=scroll_x2; ii<scroll_x2+aufknuepfung.pos.width/aufknuepfung.gw; ii++)
                if (aufknuepfung.feld.Get(ii, einzug.kbd.j+scroll_y1)>0)
                    DrawAufknuepfung (ii-scroll_x2, einzug.kbd.j, aufknuepfung.pegplanstyle, _col);
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawHighlight()
{
    _DrawHighlight (HIGHLIGHTCOLOR);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ClearHighlight()
{
    _DrawHighlight (clBlack);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBHighlightMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    DrawHighlight();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBHighlightMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    ClearHighlight();
}
/*-----------------------------------------------------------------*/
