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
#include <math.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "datamodule.h"
#include "felddef.h"
#include "cursor.h"
#include "undoredo.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawDragTool (int _i, int _j, int _i1, int _j1)
{
    int i = _i;
    int i1 = _i1;

    if (righttoleft) {
        i = gewebe.pos.width/gewebe.gw - _i - 1;
        i1 = gewebe.pos.width/gewebe.gw - _i1 - 1;
    }

    switch (tool) {
        case TOOL_LINE: {
            TPenMode save = Canvas->Pen->Mode;
            Canvas->Pen->Color = clBlack;
            Canvas->Pen->Mode = pmNot;
            Canvas->MoveTo (gewebe.pos.x0+i*gewebe.gw+gewebe.gw/2,
                            gewebe.pos.y0+gewebe.pos.height-_j*gewebe.gh-gewebe.gh/2);
            Canvas->LineTo (gewebe.pos.x0+i1*gewebe.gw+gewebe.gw/2,
                            gewebe.pos.y0+gewebe.pos.height-_j1*gewebe.gh-gewebe.gh/2);
            Canvas->Pen->Mode = save;
            break;
        }
        case TOOL_RECTANGLE:
        case TOOL_FILLEDRECTANGLE: {
            TPenMode save = Canvas->Pen->Mode;
            Canvas->Pen->Color = clBlack;
            Canvas->Pen->Mode = pmNot;
            int x = gewebe.pos.x0+i*gewebe.gw+gewebe.gw/2;
            int y = gewebe.pos.y0+gewebe.pos.height-_j*gewebe.gh-gewebe.gh/2;
            int x1 = gewebe.pos.x0+i1*gewebe.gw+gewebe.gw/2;
            int y1 = gewebe.pos.y0+gewebe.pos.height-_j1*gewebe.gh-gewebe.gh/2;
            if (x>x1) { int temp = x; x = x1; x1 = temp; }
            if (y>y1) { int temp = y; y = y1; y1 = temp; }
            x -= 2;
            y -= 2;
            x1 += 2;
            y1 += 2;
            Canvas->MoveTo (x, y);
            Canvas->LineTo (x, y1);
            Canvas->LineTo (x1, y1);
            Canvas->LineTo (x1, y);
            Canvas->LineTo (x, y);
            Canvas->Pen->Mode = save;
            break;
        }
        case TOOL_ELLIPSE:
        case TOOL_FILLEDELLIPSE: {
            TPenMode save = Canvas->Pen->Mode;
            Canvas->Pen->Color = clBlack;
            Canvas->Pen->Mode = pmNot;
            int x = gewebe.pos.x0+i*gewebe.gw+gewebe.gw/2;
            int y = gewebe.pos.y0+gewebe.pos.height-_j*gewebe.gh-gewebe.gh/2;
            int x1 = gewebe.pos.x0+i1*gewebe.gw+gewebe.gw/2;
            int y1 = gewebe.pos.y0+gewebe.pos.height-_j1*gewebe.gh-gewebe.gh/2;
            if (x>x1) { int temp = x; x = x1; x1 = temp; }
            if (y>y1) { int temp = y; y = y1; y1 = temp; }
            x -= 2;
            y -= 2;
            x1 += 2;
            y1 += 2;
            Canvas->Arc (x, y, x1, y1, x, y, x, y);
            Canvas->Pen->Mode = save;
            break;
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DeleteDragTool (int _i, int _j, int _i1, int _j1)
{
    DrawDragTool (_i, _j, _i1, _j1);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawTool (int _i, int _j, int _i1, int _j1)
{
    switch (tool) {
        case TOOL_LINE:
            DrawToolLine (_i, _j, _i1, _j1);
            break;

        case TOOL_RECTANGLE:
            DrawToolRectangle (_i, _j, _i1, _j1, false);
            break;

        case TOOL_FILLEDRECTANGLE:
            DrawToolRectangle (_i, _j, _i1, _j1, true);
            break;

        case TOOL_ELLIPSE:
            DrawToolEllipse (_i, _j, _i1, _j1, false);
            break;

        case TOOL_FILLEDELLIPSE:
            DrawToolEllipse (_i, _j, _i1, _j1, true);
            break;
    }
    RecalcAll();
    CalcRangeSchuesse();
    CalcRangeKette();
    CalcRapport();
    Invalidate();
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
static void __fastcall DrawLine (FeldGewebe& _gewebe, int _i, int _j, int _i1, int _j1, char _currentrange)
{
    dbw3_assert (_i<_i1);

    int tx = _i;
    _i1 -= tx;
    _i -= tx;

    int ty = _j;
    _j1 -= ty;
    _j -= ty;

    int dx, dy, incrE, incrNE, d, x, y;
    dx = _i1 - _i;
    dy = _j1 - _j;

    bool spiegelnv = false;
    if (dy<0) {
        _j1 = -_j1;
        dy = _j1 - _j;
        spiegelnv = true;
    }

    bool spiegelnd = false;
    if (abs(dy)>dx) {
        int temp = _i1;
        _i1 = _j1;
        _j1 = temp;
        spiegelnd = true;
    }

    dx = _i1 - _i;
    dy = _j1 - _j;

    d = dy*2 - dx;

    incrE = dy*2;
    incrNE = (dy-dx)*2;

    x = _i;
    y = _j;

    if (spiegelnv) _gewebe.feld.Set (tx+x, ty-y, _currentrange);
    else _gewebe.feld.Set (tx+x, ty+y, _currentrange);

    while (x<_i1) {
        if (d<=0) {
            d += incrE;
            x++;
        } else {
            d += incrNE;
            x++;
            y++;
        }
        if (!spiegelnd) {
            if (!spiegelnv) _gewebe.feld.Set (tx+x, ty+y, _currentrange);
            else _gewebe.feld.Set (tx+x, ty-y, _currentrange);
        } else  {
            if (!spiegelnv) _gewebe.feld.Set (tx+y, ty+x, _currentrange);
            else _gewebe.feld.Set (tx+y, ty-x, _currentrange);
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawToolLine (int _i, int _j, int _i1, int _j1)
{
    if (_i==_i1) {
        // Senkrechte Linie
        if (_j>_j1) { int temp = _j; _j = _j1; _j1 = temp; }
        for (int j=scroll_y2+_j; j<=scroll_y2+_j1; j++)
            gewebe.feld.Set (scroll_x1+_i, j, currentrange);
    } else if (_j==_j1) {
        // Waagrechte Linie
        if (_i>_i1) { int temp = _i; _i = _i1; _i1 = temp; }
        for (int i=scroll_x1+_i; i<=scroll_x1+_i1; i++)
            gewebe.feld.Set (i, scroll_y2+_j, currentrange);
    } else if (abs(_i1-_i)==abs(_j1-_j)) {
        // Diagonale
        if (_i>_i1) {
            int temp = _i; _i = _i1; _i1 = temp;
            temp = _j; _j = _j1; _j1 = temp; 
        }
        if (_j<_j1) {
            for (int i=_i; i<=_i1; i++)
                gewebe.feld.Set (scroll_x1+i, scroll_y2+_j+i-_i, currentrange);
        } else {
            for (int i=_i; i<=_i1; i++)
                gewebe.feld.Set (scroll_x1+i, scroll_y2+_j-i+_i, currentrange);
        }
    } else {
        if (_i>_i1) {
            int temp = _i; _i = _i1; _i1 = temp;
            temp = _j; _j = _j1; _j1 = temp;
        }
        DrawLine (gewebe, scroll_x1+_i, scroll_y2+_j, scroll_x1+_i1, scroll_y2+_j1, currentrange);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawToolRectangle (int _i, int _j, int _i1, int _j1, bool _filled)
{
    if (_i>_i1) { int temp = _i; _i = _i1; _i1 = temp; }
    if (_j>_j1) { int temp = _j; _j = _j1; _j1 = temp; }
    if (_filled) {
        for (int i=scroll_x1+_i; i<=scroll_x1+_i1; i++)
            for (int j=scroll_y2+_j; j<=scroll_y2+_j1; j++)
                gewebe.feld.Set (i, j, currentrange);
    } else {
        for (int i=scroll_x1+_i; i<=scroll_x1+_i1; i++) {
            gewebe.feld.Set (i, scroll_y2+_j, currentrange);
            gewebe.feld.Set (i, scroll_y2+_j1, currentrange);
        }
        for (int j=scroll_y2+_j; j<=scroll_y2+_j1; j++) {
            gewebe.feld.Set (scroll_x1+_i, j, currentrange);
            gewebe.feld.Set (scroll_x1+_i1, j, currentrange);
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawToolEllipse (int _i, int _j, int _i1, int _j1, bool _filled)
{
    if (_i>_i1) { int temp = _i; _i = _i1; _i1 = temp; }
    if (_j>_j1) { int temp = _j; _j = _j1; _j1 = temp; }
    int i = _i1 - _i;
    int j = _j1 - _j;

    try {
        // Ich zeichne der Einfachheit halber die
        // Ellipse in ein offscreen-Bitmap und lese
        // dann die gesetzten Pixel aus.
        // So muss ich nicht selber die Ellipse zeichnen!
        Graphics::TBitmap* bmp = new Graphics::TBitmap;
        int mult = _filled ? 13 : 1;
        bmp->Width = (i+1)*mult;
        bmp->Height = (j+1)*mult;
        bmp->Canvas->Pen->Color = clWhite;
        bmp->Canvas->Brush->Color = clWhite;
        bmp->Canvas->Rectangle (0, 0, (i+1)*mult, (j+1)*mult);
        bmp->Canvas->Pen->Color = clBlack;
        bmp->Canvas->Brush->Color = clBlack;
        if (!_filled) bmp->Canvas->Arc (0, 0, i*mult+1, j*mult+1, 0, 0, 0, 0);
        else bmp->Canvas->Ellipse (0, 0, (i+1)*mult+1, (j+1)*mult+1);
        for (int x=0; x<=i; x++)
            for (int y=0; y<=j; y++)
                if (bmp->Canvas->Pixels[x*mult+mult/2][y*mult+mult/2]==clBlack)
                    gewebe.feld.Set (scroll_x1+_i+x, scroll_y2+_j+y, currentrange);
        delete bmp;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
