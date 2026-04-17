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
#include "hilfslinien.h"
/*-----------------------------------------------------------------*/
#define INITIALMAX 20
/*-----------------------------------------------------------------*/
__fastcall Hilfslinien::Hilfslinien()
{
    list = NULL;
    max = 0;
    last = -1;
    Reallocate (INITIALMAX);
}
/*-----------------------------------------------------------------*/
__fastcall Hilfslinien::~Hilfslinien()
{
    delete[] list;
}
/*-----------------------------------------------------------------*/
void __fastcall Hilfslinien::Reallocate (int _newmax)
{
    if (_newmax<=max) return;

    try {
        Hilfslinie* newlist = new Hilfslinie[_newmax];

        if (list && last>=0) {
            for (int i=0; i<=last && i<max; i++) newlist[i] = list[i];
        }

        delete[] list;
        list = newlist;
        max = _newmax;

    } catch (...) {
        list = NULL;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall Hilfslinien::SetData (Hilfslinie* _list, int _count)
{
    try {
        delete list;
        list = _list;
        max = _count;
        last = _count-1;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
bool __fastcall Hilfslinien::Add (HLTYP _typ, HLFELD _feld, int _pos)
{
    dbw3_assert (list);
    if (last>=max-1) Reallocate (max+10);
    if (last>=max-1) return false;

    last++;
    list[last].typ = _typ;
    list[last].feld = _feld;
    list[last].pos = _pos;

    return true;
}
/*-----------------------------------------------------------------*/
void __fastcall Hilfslinien::Update (int _index, HLFELD _feld, int _pos)
{
    dbw3_assert (list);
    dbw3_assert (_index<=last && _index<max);
    if (_index>last) return;
    list[_index].feld = _feld;
    list[_index].pos = _pos;
}
/*-----------------------------------------------------------------*/
void __fastcall Hilfslinien::Delete (int _index)
{
    dbw3_assert (list);
    dbw3_assert (_index<=last && _index<max);
    if (_index>last) return;
    for (int i=_index+1; i<=last; i++)
        list[i-1] = list[i];
    last--;
}
/*-----------------------------------------------------------------*/
void __fastcall Hilfslinien::Delete (Hilfslinie* _hline)
{
    dbw3_assert (_hline);
    if (!_hline) return;
    for (int i=0; i<=last && i<max; i++)
        if (list[i].typ==_hline->typ && list[i].feld==_hline->feld && list[i].pos==_hline->pos) {
            Delete (i);
            break;
        }
}
/*-----------------------------------------------------------------*/
void __fastcall Hilfslinien::DeleteAll()
{
    last = -1;
}
/*-----------------------------------------------------------------*/
int __fastcall Hilfslinien::GetCount() const
{
    return last+1;
}
/*-----------------------------------------------------------------*/
Hilfslinie* __fastcall Hilfslinien::GetLine (int _index)
{
    dbw3_assert (list);
    dbw3_assert (_index<=last && _index<max);
    if (_index>last) return NULL;
    return &list[_index];
}
/*-----------------------------------------------------------------*/
Hilfslinie* __fastcall Hilfslinien::GetLine (HLTYP _typ, HLFELD _feld, int _pos)
{
    if (!list) return NULL;
    for (int i=0; i<=last && i<max; i++)
        if (list[i].typ==_typ && list[i].feld==_feld && list[i].pos==_pos)
            return &list[i];
    return NULL;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawHilfslinien()
{
    int count = hlines.GetCount();
    for (int i=0; i<count; i++) {
        Hilfslinie* line = hlines.GetLine(i);
        dbw3_assert(line);
        if (line) DrawHilfslinie (line);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawHilfslinie (Hilfslinie* _hline)
{
    _DrawHilfslinie (_hline, true);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DeleteHilfslinie (Hilfslinie* _hline)
{
    _DrawHilfslinie (_hline, false);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::_DrawHilfslinie (Hilfslinie* _hline, bool _draw)
{
    switch (_hline->typ) {
        case HL_HORZ:
            if (_hline->feld==HL_TOP) {
                if (!ViewEinzug->Checked)
                    break;
                if (_hline->pos<=scroll_y1 || _hline->pos>=scroll_y1+hvisible)
                    break;
                Canvas->Pen->Color = _draw ? HLINECOLOR : clDkGray;
                int y;
                if (toptobottom) {
                    y = einzug.pos.y0+(_hline->pos-scroll_y1)*einzug.gh;
                } else {
                    y = einzug.pos.y0+einzug.pos.height-(_hline->pos-scroll_y1)*einzug.gh;
                }
                Canvas->MoveTo (einzug.pos.x0+1, y);
                Canvas->LineTo (einzug.pos.x0+einzug.pos.width, y);
                if (ViewTrittfolge->Checked && !ViewSchlagpatrone->Checked) {
                    Canvas->MoveTo (aufknuepfung.pos.x0+1, y);
                    Canvas->LineTo (aufknuepfung.pos.x0+aufknuepfung.pos.width, y);
                }
                Canvas->Pen->Color = _draw ? HLINECOLOR : clBtnFace;
                Canvas->MoveTo (hlinevert1.x0+5, y);
                Canvas->LineTo (hlinevert1.x0+hlinevert1.width, y);
            } else {
                if (_hline->pos<=scroll_y2 || _hline->pos>=scroll_y2+gewebe.pos.height/gewebe.gh)
                    break;
                Canvas->Pen->Color = _draw ? HLINECOLOR : clDkGray;
                int y = gewebe.pos.y0+gewebe.pos.height-(_hline->pos-scroll_y2)*gewebe.gh;
                if (GewebeNormal->Checked || GewebeNone->Checked) {
                    Canvas->MoveTo (gewebe.pos.x0+1, y);
                    Canvas->LineTo (gewebe.pos.x0+gewebe.pos.width, y);
                }
                if (ViewTrittfolge->Checked) {
                    Canvas->MoveTo (trittfolge.pos.x0+1, y);
                    Canvas->LineTo (trittfolge.pos.x0+trittfolge.pos.width, y);
                }
                Canvas->Pen->Color = _draw ? HLINECOLOR : clBtnFace;
                Canvas->MoveTo (hlinevert2.x0+5, y);
                Canvas->LineTo (hlinevert2.x0+hlinevert2.width, y);
            }
            break;
        case HL_VERT:
            if (_hline->feld==HL_LEFT) {
                if (_hline->pos<=scroll_x1 || _hline->pos>=scroll_x1+gewebe.pos.width/gewebe.gw)
                    break;
                Canvas->Pen->Color = _draw ? HLINECOLOR : clDkGray;
                int x;
                if (righttoleft) {
                    x = gewebe.pos.x0+gewebe.pos.width-(_hline->pos-scroll_x1)*gewebe.gw;
                } else {
                    x = gewebe.pos.x0+(_hline->pos-scroll_x1)*gewebe.gw;
                }
                if (GewebeNormal->Checked || GewebeNone->Checked) {
                    Canvas->MoveTo (x, gewebe.pos.y0+1);
                    Canvas->LineTo (x, gewebe.pos.y0+gewebe.pos.height);
                }
                if (ViewEinzug->Checked) {
                    Canvas->MoveTo (x, einzug.pos.y0+1);
                    Canvas->LineTo (x, einzug.pos.y0+einzug.pos.height);
                }
                Canvas->Pen->Color = _draw ? HLINECOLOR : clBtnFace;
                Canvas->MoveTo (x, hlinehorz1.y0+1);
                Canvas->LineTo (x, hlinehorz1.y0+hlinehorz1.height-4);
            } else {
                if (!ViewTrittfolge->Checked)
                    break;
                if (_hline->pos<=scroll_x2 || _hline->pos>=scroll_x2+wvisible)
                    break;
                Canvas->Pen->Color = _draw ? HLINECOLOR : clDkGray;
                int x = trittfolge.pos.x0+(_hline->pos-scroll_x2)*trittfolge.gw;
                Canvas->MoveTo (x, trittfolge.pos.y0+1);
                Canvas->LineTo (x, trittfolge.pos.y0+trittfolge.pos.height);
                if (ViewEinzug->Checked && !ViewSchlagpatrone->Checked) {
                    Canvas->MoveTo (x, aufknuepfung.pos.y0+1);
                    Canvas->LineTo (x, aufknuepfung.pos.y0+aufknuepfung.pos.height);
                }
                Canvas->Pen->Color = _draw ? HLINECOLOR : clBtnFace;
                Canvas->MoveTo (x, hlinehorz2.y0+1);
                Canvas->LineTo (x, hlinehorz2.y0+hlinehorz2.height-4);
            }
            break;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawHlineBars()
{
    Canvas->Brush->Color = clBtnFace;
    Canvas->Pen->Color = clDkGray;
    Canvas->Rectangle (hlinehorz1.x0, hlinehorz1.y0, hlinehorz1.x0+hlinehorz1.width+1, hlinehorz1.y0+hlinehorz1.height+1);
    Canvas->Rectangle (hlinehorz2.x0, hlinehorz2.y0, hlinehorz2.x0+hlinehorz2.width+1, hlinehorz2.y0+hlinehorz2.height+1);
    Canvas->Rectangle (hlinevert1.x0, hlinevert1.y0, hlinevert1.x0+hlinevert1.width+1, hlinevert1.y0+hlinevert1.height+1);
    Canvas->Rectangle (hlinevert2.x0, hlinevert2.y0, hlinevert2.x0+hlinevert2.width+1, hlinevert2.y0+hlinevert2.height+1);

    for (int i=hlinehorz1.x0; i<hlinehorz1.x0+hlinehorz1.width; i+=hlinehorz1.gw) {
        Canvas->MoveTo (i, hlinehorz1.y0+hlinehorz1.height-1);
        Canvas->LineTo (i, hlinehorz1.y0+hlinehorz1.height-5);
    }

    for (int i=hlinehorz2.x0; i<hlinehorz2.x0+hlinehorz2.width; i+=hlinehorz2.gw) {
        Canvas->MoveTo (i, hlinehorz2.y0+hlinehorz2.height-1);
        Canvas->LineTo (i, hlinehorz2.y0+hlinehorz2.height-5);
    }

    for (int j=hlinevert1.y0; j<hlinevert1.y0+hlinevert1.height; j+=hlinevert1.gh) {
        Canvas->MoveTo (hlinevert1.x0+1, j);
        Canvas->LineTo (hlinevert1.x0+5, j);
    }

    for (int j=hlinevert2.y0; j<hlinevert2.y0+hlinevert2.height; j+=hlinevert2.gh) {
        Canvas->MoveTo (hlinevert2.x0+1, j);
        Canvas->LineTo (hlinevert2.x0+5, j);
    }

    // Die eigentlichen Hilfslinien (auch das was in den
    // Hilfslinienbars sichtbar ist!) werden in
    // DrawHilfslinien gezeichnet
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ViewHlinesClick(TObject *Sender)
{
    ViewHlines->Checked = !ViewHlines->Checked;
    SetModified();
    RecalcDimensions();
    Invalidate();
}
/*-----------------------------------------------------------------*/
