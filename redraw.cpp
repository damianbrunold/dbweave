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

#include <vcl\vcl.h>
#include <mem.h>
#pragma hdrstop

#include "assert.h"
#include "dbw3_form.h"
#include "felddef.h"
#include "dbw3_strings.h"
#include "datamodule.h"
#include "palette.h"

#define GETPALCOL(col) \
    ((TColor)Data->palette->GetColor(col))

bool NeedRedraw (TCanvas* _canvas, GRIDPOS _gridpos)
{
    RECT gridrc;
    RECT rc;

    gridrc.left   = _gridpos.x0;
    gridrc.right  = _gridpos.x0 + _gridpos.width;
    gridrc.top    = _gridpos.y0;
    gridrc.bottom = _gridpos.y0 + _gridpos.height+1;

    return IntersectRect (&rc, &((RECT)_canvas->ClipRect), &gridrc)!=0;
}

void __fastcall TDBWFRM::FormPaint(TObject *Sender)
{
    if (!bInitialized) return;

    CursorTimer->Enabled = false;

    DrawDividers();

    if (ViewBlatteinzug->Checked && NeedRedraw (Canvas, blatteinzug.pos))
        DrawBlatteinzug();

    if (ViewFarbe->Checked) {
        if (NeedRedraw (Canvas, kettfarben.pos)) DrawKettfarben();
        if (NeedRedraw (Canvas, schussfarben.pos)) DrawSchussfarben();
    }

    if (ViewEinzug->Checked && NeedRedraw (Canvas, einzug.pos))
        DrawEinzug();

    if (ViewEinzug->Checked && ViewTrittfolge->Checked)
        if (NeedRedraw (Canvas, aufknuepfung.pos)) DrawAufknuepfung();

    if (ViewTrittfolge->Checked && NeedRedraw (Canvas, trittfolge.pos))
        DrawTrittfolge();

    if (NeedRedraw (Canvas, gewebe.pos))
        DrawGewebe();

    if (ViewHlines->Checked) {
        DrawHlineBars();
        DrawHilfslinien();
    }
    
    if (RappViewRapport->Checked) DrawRapport();
    DrawSelection();
    DrawCursor();

    UpdateStatusBar();

    CursorTimer->Enabled = true;
}

void __fastcall TDBWFRM::DrawDividers()
{
    if (GetDeviceCaps(Canvas->Handle, BITSPIXEL)>=16) { // Mehr als 256 Farben
        int r = GetRValue(GetSysColor(COLOR_BTNFACE))*110/100;
        int g = GetGValue(GetSysColor(COLOR_BTNFACE))*110/100;
        int b = GetBValue(GetSysColor(COLOR_BTNFACE))*110/100;
        if (r>255) r=255;
        if (g>255) g=255;
        if (b>255) b=255;
        Canvas->Pen->Color = TColor(RGB(r,g,b));
        Canvas->Brush->Color = Canvas->Pen->Color;
    } else {
        Canvas->Pen->Color = clBtnFace;
        Canvas->Brush->Color = Canvas->Pen->Color;
    }

    int x  = 0;
    int xx = ClientWidth;
    int y  = Toolbar->Height;
    int yy = ClientHeight;

    int yy1 = yy - Statusbar->Height - sb_horz1->Height;
    int xx1 = xx - sb_vert1->Width;

    // Horizontal
    if (!einzugunten) {
        // Hilfslinien
        if (ViewHlines->Checked) {
            int a = x;
            int b = hlinehorz1.y0+hlinehorz1.height+1;
            int c = xx;
            int d;
            if (ViewFarbe->Checked) d = kettfarben.pos.y0;
            else if (ViewEinzug->Checked) d = einzug.pos.y0;
            else if (ViewBlatteinzug->Checked) d = blatteinzug.pos.y0;
            else d = gewebe.pos.y0;
            Canvas->Rectangle (a, b, c, d);
            a = hlinehorz2.x0+hlinehorz2.width+1;
            b = hlinehorz1.y0;
            c = xx;
            d = hlinehorz1.y0+hlinehorz1.height+1;
            Canvas->Rectangle (a, b, c, d);
        }

        // Kettfarben
        if (ViewFarbe->Checked) {
            int a = x;
            int b = kettfarben.pos.y0+kettfarben.pos.height+1;
            int c = xx;
            int d;
            if (ViewEinzug->Checked) d = einzug.pos.y0;
            else if (ViewBlatteinzug->Checked) d = blatteinzug.pos.y0;
            else d = gewebe.pos.y0;
            Canvas->Rectangle (a, b, c, d);
            a = kettfarben.pos.x0+kettfarben.pos.width+1;
            b = kettfarben.pos.y0;
            c = xx;
            d = kettfarben.pos.y0+kettfarben.pos.height+1;
            Canvas->Rectangle (a, b, c, d);
        }

        // Einzug
        if (ViewEinzug->Checked) {
            int a = x;
            int b = einzug.pos.y0+einzug.pos.height+1;
            int c = xx;
            int d;
            if (ViewBlatteinzug->Checked) d = blatteinzug.pos.y0;
            else d = gewebe.pos.y0;
            Canvas->Rectangle (a, b, c, d);
        }
        // Blatteinzug
        if (ViewBlatteinzug->Checked) {
            int a = x;
            int b = blatteinzug.pos.y0+blatteinzug.pos.height+1;
            int c = xx;
            int d = gewebe.pos.y0;
            Canvas->Rectangle (a, b, c, d);
            a = blatteinzug.pos.x0+blatteinzug.pos.width+1;
            b = blatteinzug.pos.y0;
            c = xx;
            d = blatteinzug.pos.y0+blatteinzug.pos.height+1;
            Canvas->Rectangle (a, b, c, d);
        }
        // Gewebe
        int a = x;
        int b = gewebe.pos.y0+gewebe.pos.height+1;
        int c = xx;
        int d = yy1;
        Canvas->Rectangle (a, b, c, d);
    } else {
        // Hilfslinien
        if (ViewHlines->Checked) {
            int a = x;
            int b = hlinehorz1.y0+hlinehorz1.height+1;
            int c = xx;
            int d = gewebe.pos.y0;
            Canvas->Rectangle (a, b, c, d);
            a = hlinehorz2.x0+hlinehorz2.width+1;
            b = hlinehorz1.y0;
            c = xx;
            d = hlinehorz1.y0+hlinehorz1.height+1;
            Canvas->Rectangle (a, b, c, d);
        }

        // Gewebe
        int a = x;
        int b = gewebe.pos.y0+gewebe.pos.height+1;
        int c = xx;
        int d;
        if (ViewBlatteinzug->Checked) d = blatteinzug.pos.y0;
        else if (ViewEinzug->Checked) d = einzug.pos.y0;
        else if (ViewFarbe->Checked) d = kettfarben.pos.y0;
        else d = yy1;
        Canvas->Rectangle (a, b, c, d);

        // Blatteinzug
        if (ViewBlatteinzug->Checked) {
            int a = x;
            int b = blatteinzug.pos.y0+blatteinzug.pos.height+1;
            int c = xx;
            int d;
            if (ViewEinzug->Checked) d = einzug.pos.y0;
            else if (ViewFarbe->Checked) d = kettfarben.pos.y0;
            else d = yy1;
            Canvas->Rectangle (a, b, c, d);
            a = blatteinzug.pos.x0+blatteinzug.pos.width+1;
            b = blatteinzug.pos.y0;
            c = xx;
            d = blatteinzug.pos.y0+blatteinzug.pos.height+1;
            Canvas->Rectangle (a, b, c, d);
        }

        // Einzug
        if (ViewEinzug->Checked) {
            int a = x;
            int b = einzug.pos.y0+einzug.pos.height+1;
            int c = xx;
            int d;
            if (ViewFarbe->Checked) d = kettfarben.pos.y0;
            else d = yy1;
            Canvas->Rectangle (a, b, c, d);
        }

        // Kettfarben
        if (ViewFarbe->Checked) {
            int a = x;
            int b = kettfarben.pos.y0+kettfarben.pos.height+1;
            int c = xx;
            int d = yy1;
            Canvas->Rectangle (a, b, c, d);
            a = kettfarben.pos.x0+kettfarben.pos.width+1;
            b = kettfarben.pos.y0;
            c = xx;
            d = kettfarben.pos.y0+kettfarben.pos.height+1;
            Canvas->Rectangle (a, b, c, d);
        }
    }

    // Vertikal
    // Gewebe
    int a = gewebe.pos.x0+gewebe.pos.width+1;
    int b = y;
    int c;
    int d = yy;
    if (ViewTrittfolge->Checked) c = trittfolge.pos.x0;
    else if (ViewFarbe->Checked) c = schussfarben.pos.x0;
    else if (ViewHlines->Checked) c = hlinevert1.x0;
    else c = xx1;
    Canvas->Rectangle (a, b, c, d);

    // Trittfolge
    if (ViewTrittfolge->Checked) {
        int a = trittfolge.pos.x0+trittfolge.pos.width+1;
        int b = y;
        int c;
        int d = yy;
        if (ViewFarbe->Checked) c = schussfarben.pos.x0;
        else if (ViewHlines->Checked) c = hlinevert1.x0;
        else c = xx1;
        Canvas->Rectangle (a, b, c, d);
    }

    // Schussfarben
    if (ViewFarbe->Checked) {
        int a = schussfarben.pos.x0+schussfarben.pos.width+1;
        int b = y;
        int c;
        int d = yy;
        if (ViewHlines->Checked) c = hlinevert1.x0;
        else c = xx1;
        Canvas->Rectangle (a, b, c, d);
        a = schussfarben.pos.x0;
        b = schussfarben.pos.y0+schussfarben.pos.height+1;
        c = schussfarben.pos.x0+schussfarben.pos.width+1;
        d = yy;
        Canvas->Rectangle (a, b, c, d);
        if (!einzugunten) {
            a = schussfarben.pos.x0;
            b = y;
            c = schussfarben.pos.x0+schussfarben.pos.width+1;
            d = schussfarben.pos.y0+1;
            Canvas->Rectangle (a, b, c, d);
        }
    }

    // Hilfslinien
    if (ViewHlines->Checked) {
        int a = schussfarben.pos.x0+schussfarben.pos.width+1;
        int b = y;
        int c = xx1;
        int d = yy;
        Canvas->Rectangle (a, b, c, d);
        a = hlinevert2.x0;
        b = hlinevert2.y0+hlinevert2.height+1;
        c = hlinevert2.x0+hlinevert2.width+1;
        d = yy;
        Canvas->Rectangle (a, b, c, d);
    }

    // Zwischen den Scrollbars
    Canvas->Rectangle (xx1, yy1, xx, yy);

    // Eventuell hat es links noch Platz!
    if (gewebe.pos.x0>0) Canvas->Rectangle (x, y, gewebe.pos.x0, yy);
}

void __fastcall TDBWFRM::DrawBlatteinzug()
{
    int x, i;
    int x0, x1, inc;
    if (righttoleft) {
        x0 = blatteinzug.pos.x0+blatteinzug.pos.width-blatteinzug.gw;
        x1 = blatteinzug.pos.x0;
        inc = blatteinzug.gw;
        for (x=x0, i=scroll_x1; x>=x1; x-=inc, i++) {
            Canvas->Pen->Color = clBtnShadow;
            Canvas->Brush->Color = Canvas->Pen->Color;
            Canvas->MoveTo (x, blatteinzug.pos.y0);
            Canvas->LineTo (x, blatteinzug.pos.y0+blatteinzug.pos.height);
            if (blatteinzug.feld.Get(i)==0) {
                Canvas->Pen->Color = clBlack;
                Canvas->Brush->Color = Canvas->Pen->Color;
                Canvas->Rectangle (x, blatteinzug.pos.y0+blatteinzug.pos.height/2, x+blatteinzug.gw, blatteinzug.pos.y0+blatteinzug.pos.height);
                Canvas->Pen->Color = clBtnFace;
                Canvas->Brush->Color = Canvas->Pen->Color;
                Canvas->Rectangle (x+1, blatteinzug.pos.y0, x+blatteinzug.gw, blatteinzug.pos.y0+blatteinzug.pos.height/2);
            } else {
                Canvas->Pen->Color = clBlack;
                Canvas->Brush->Color = Canvas->Pen->Color;
                Canvas->Rectangle (x, blatteinzug.pos.y0, x+blatteinzug.gw, blatteinzug.pos.y0+blatteinzug.pos.height/2);
                Canvas->Pen->Color = clBtnFace;
                Canvas->Brush->Color = Canvas->Pen->Color;
                Canvas->Rectangle (x+1, blatteinzug.pos.y0+blatteinzug.pos.height/2, x+blatteinzug.gw, blatteinzug.pos.y0+blatteinzug.pos.height);
            }
            Canvas->Pen->Color = clBtnShadow;
            Canvas->MoveTo (x, blatteinzug.pos.y0+1);
            Canvas->LineTo (x, blatteinzug.pos.y0+blatteinzug.pos.height);
        }
    } else {
        x0 = blatteinzug.pos.x0;
        x1 = blatteinzug.pos.x0+blatteinzug.pos.width;
        inc = blatteinzug.gw;
        for (x=x0, i=scroll_x1; x<x1; x+=inc, i++) {
            Canvas->Pen->Color = clBtnShadow;
            Canvas->Brush->Color = Canvas->Pen->Color;
            Canvas->MoveTo (x, blatteinzug.pos.y0);
            Canvas->LineTo (x, blatteinzug.pos.y0+blatteinzug.pos.height);
            if (blatteinzug.feld.Get(i)==0) {
                Canvas->Pen->Color = clBlack;
                Canvas->Brush->Color = Canvas->Pen->Color;
                Canvas->Rectangle (x, blatteinzug.pos.y0+blatteinzug.pos.height/2, x+blatteinzug.gw, blatteinzug.pos.y0+blatteinzug.pos.height);
                Canvas->Pen->Color = clBtnFace;
                Canvas->Brush->Color = Canvas->Pen->Color;
                Canvas->Rectangle (x+1, blatteinzug.pos.y0, x+blatteinzug.gw, blatteinzug.pos.y0+blatteinzug.pos.height/2);
            } else {
                Canvas->Pen->Color = clBlack;
                Canvas->Brush->Color = Canvas->Pen->Color;
                Canvas->Rectangle (x, blatteinzug.pos.y0, x+blatteinzug.gw, blatteinzug.pos.y0+blatteinzug.pos.height/2);
                Canvas->Pen->Color = clBtnFace;
                Canvas->Brush->Color = Canvas->Pen->Color;
                Canvas->Rectangle (x+1, blatteinzug.pos.y0+blatteinzug.pos.height/2, x+blatteinzug.gw, blatteinzug.pos.y0+blatteinzug.pos.height);
            }
            Canvas->Pen->Color = clBtnShadow;
            Canvas->MoveTo (x, blatteinzug.pos.y0+1);
            Canvas->LineTo (x, blatteinzug.pos.y0+blatteinzug.pos.height);
        }
    }
    Canvas->Pen->Color = clBtnShadow;
    Canvas->Brush->Color = Canvas->Pen->Color;
    Canvas->MoveTo (blatteinzug.pos.x0, blatteinzug.pos.y0);
    Canvas->LineTo (blatteinzug.pos.x0+blatteinzug.pos.width, blatteinzug.pos.y0);
    Canvas->LineTo (blatteinzug.pos.x0+blatteinzug.pos.width, blatteinzug.pos.y0+blatteinzug.pos.height);
    Canvas->LineTo (blatteinzug.pos.x0, blatteinzug.pos.y0+blatteinzug.pos.height);
    Canvas->LineTo (blatteinzug.pos.x0, blatteinzug.pos.y0);
}

void __fastcall TDBWFRM::DrawKettfarben()
{
    Canvas->Pen->Color = clBtnShadow;
    int x, i;
    if (righttoleft) {
        for (x=kettfarben.pos.x0+kettfarben.pos.width-kettfarben.gw, i=scroll_x1; x>=kettfarben.pos.x0; x-=kettfarben.gw, i++) {
            Canvas->MoveTo (x, kettfarben.pos.y0);
            Canvas->LineTo (x, kettfarben.pos.y0+kettfarben.pos.height);
            Canvas->Brush->Color = GETPALCOL(kettfarben.feld.Get(i));
            Canvas->Rectangle (x, kettfarben.pos.y0, x+kettfarben.gw+1, kettfarben.pos.y0+kettfarben.pos.height+1);
        }
    } else {
        for (x=kettfarben.pos.x0, i=scroll_x1; x<kettfarben.pos.x0+kettfarben.pos.width; x+=kettfarben.gw, i++) {
            Canvas->MoveTo (x, kettfarben.pos.y0);
            Canvas->LineTo (x, kettfarben.pos.y0+kettfarben.pos.height);
            Canvas->Brush->Color = GETPALCOL(kettfarben.feld.Get(i));
            Canvas->Rectangle (x, kettfarben.pos.y0, x+kettfarben.gw+1, kettfarben.pos.y0+kettfarben.pos.height+1);
        }
    }
    Canvas->MoveTo (kettfarben.pos.x0, kettfarben.pos.y0);
    Canvas->LineTo (kettfarben.pos.x0+kettfarben.pos.width, kettfarben.pos.y0);
    Canvas->LineTo (kettfarben.pos.x0+kettfarben.pos.width, kettfarben.pos.y0+kettfarben.pos.height);
    Canvas->LineTo (kettfarben.pos.x0, kettfarben.pos.y0+kettfarben.pos.height);
    Canvas->LineTo (kettfarben.pos.x0, kettfarben.pos.y0);
}

void __fastcall TDBWFRM::DrawEinzug()
{
    int x, y, i, j;

    if (ViewOnlyGewebe->Checked) {
        Canvas->Pen->Color = clBtnShadow;
        Canvas->Brush->Color = clBtnFace;
        Canvas->Rectangle (einzug.pos.x0, einzug.pos.y0, einzug.pos.x0+einzug.pos.width+1, einzug.pos.y0+einzug.pos.height+1);
    }

    // Rahmen
    Canvas->Pen->Color = clBtnShadow;
    Canvas->Brush->Color = Canvas->Pen->Color;
    for (x=einzug.pos.x0; x<einzug.pos.x0+einzug.pos.width; x+=einzug.gw) {
        Canvas->Pen->Color = clBtnShadow;
        Canvas->MoveTo (x, einzug.pos.y0);
        Canvas->LineTo (x, einzug.pos.y0+einzug.pos.height);
    }
    for (y=einzug.pos.y0+einzug.pos.height; y>einzug.pos.y0; y-=einzug.gh) {
        Canvas->Pen->Color = clBtnShadow;
        Canvas->MoveTo (einzug.pos.x0, y);
        Canvas->LineTo (einzug.pos.x0+einzug.pos.width, y);
    }
    if (righttoleft) {
        for (x=einzug.pos.x0+einzug.pos.width-einzug.gw, i=scroll_x1; x>=einzug.pos.x0; x-=einzug.gw) {
            if (einzug.pos.strongline_x!=0 && (++i%einzug.pos.strongline_x)==0) {
                Canvas->Pen->Color = strongclr;
                Canvas->MoveTo (x, einzug.pos.y0);
                Canvas->LineTo (x, einzug.pos.y0+einzug.pos.height);
            }
        }
    } else {
        for (x=einzug.pos.x0, i=scroll_x1; x<einzug.pos.x0+einzug.pos.width; x+=einzug.gw) {
            if (einzug.pos.strongline_x!=0 && (i++%einzug.pos.strongline_x)==0) {
                Canvas->Pen->Color = strongclr;
                Canvas->MoveTo (x, einzug.pos.y0);
                Canvas->LineTo (x, einzug.pos.y0+einzug.pos.height);
            }
        }
    }
    if (toptobottom) {
        for (y=einzug.pos.y0, i=scroll_y1; y<einzug.pos.y0+einzug.pos.height; y+=einzug.gh) {
            if (einzug.pos.strongline_y!=0 && (i++%einzug.pos.strongline_y)==0) {
                Canvas->Pen->Color = strongclr;
                Canvas->MoveTo (einzug.pos.x0, y);
                Canvas->LineTo (einzug.pos.x0+einzug.pos.width, y);
            }
        }
    } else {
        for (y=einzug.pos.y0+einzug.pos.height, i=scroll_y1; y>einzug.pos.y0; y-=einzug.gh) {
            if (einzug.pos.strongline_y!=0 && (i++%einzug.pos.strongline_y)==0) {
                Canvas->Pen->Color = strongclr;
                Canvas->MoveTo (einzug.pos.x0, y);
                Canvas->LineTo (einzug.pos.x0+einzug.pos.width, y);
            }
        }
    }
    Canvas->Pen->Color = clBtnShadow;
    Canvas->MoveTo (einzug.pos.x0, einzug.pos.y0);
    Canvas->LineTo (einzug.pos.x0+einzug.pos.width, einzug.pos.y0);
    Canvas->LineTo (einzug.pos.x0+einzug.pos.width, einzug.pos.y0+einzug.pos.height);
    Canvas->LineTo (einzug.pos.x0, einzug.pos.y0+einzug.pos.height);
    Canvas->LineTo (einzug.pos.x0, einzug.pos.y0);

    if (ViewOnlyGewebe->Checked)
        return;

    // Inhalt
    int ii = einzug.pos.width/einzug.gw;
    int jj = einzug.pos.height/einzug.gh;
    for (i=0; i<ii; i++)
        for (j=0; j<jj; j++)
            DrawEinzug (i, j);

#ifdef _DEBUG
    // Kennzeichnung für leere Schäfte
    Canvas->Pen->Color = clBlack;
    for (int j=scroll_y1; j<scroll_y1+einzug.pos.height/einzug.gh; j++)
        if (freieschaefte[j]) {
            if (toptobottom) {
                Canvas->MoveTo (einzug.pos.x0+einzug.pos.width, einzug.pos.y0+(j-scroll_y1+1)*einzug.gh);
                Canvas->LineTo (einzug.pos.x0+einzug.pos.width, einzug.pos.y0+(j-scroll_y1)*einzug.gh);
            } else {
                Canvas->MoveTo (einzug.pos.x0+einzug.pos.width, einzug.pos.y0+einzug.pos.height-(j-scroll_y1+1)*einzug.gh);
                Canvas->LineTo (einzug.pos.x0+einzug.pos.width, einzug.pos.y0+einzug.pos.height-(j-scroll_y1)*einzug.gh);
            }
        }
#endif
}

void __fastcall TDBWFRM::DrawAufknuepfung()
{
    int x, y, i, j;

    if (ViewOnlyGewebe->Checked) {
        Canvas->Pen->Color = clBtnShadow;
        Canvas->Brush->Color = clBtnFace;
        Canvas->Rectangle (aufknuepfung.pos.x0, aufknuepfung.pos.y0, aufknuepfung.pos.x0+aufknuepfung.pos.width+1, aufknuepfung.pos.y0+aufknuepfung.pos.height+1);
    }

    // Rahmen
    Canvas->Pen->Color = clBtnShadow;
    Canvas->Brush->Color = Canvas->Pen->Color;
    int x0 = aufknuepfung.pos.x0;
    int y0 = aufknuepfung.pos.y0;
    if (!ViewSchlagpatrone->Checked || !aufknuepfung.pegplanstyle || toptobottom) {
        for (x=x0; x<x0+aufknuepfung.pos.width; x+=aufknuepfung.gw) {
            Canvas->Pen->Color = clBtnShadow;
            Canvas->MoveTo (x, y0);
            Canvas->LineTo (x, y0+aufknuepfung.pos.height);
        }
        for (y=y0+aufknuepfung.pos.height; y>y0; y-=aufknuepfung.gh) {
            Canvas->Pen->Color = clBtnShadow;
            Canvas->MoveTo (x0, y);
            Canvas->LineTo (x0+aufknuepfung.pos.width, y);
        }
        for (x=x0, i=scroll_x2; x<x0+aufknuepfung.pos.width; x+=aufknuepfung.gw) {
            if (aufknuepfung.pos.strongline_x!=0 && (i++%aufknuepfung.pos.strongline_x)==0) {
                Canvas->Pen->Color = strongclr;
                Canvas->MoveTo (x, y0);
                Canvas->LineTo (x, y0+aufknuepfung.pos.height);
            }
        }
        if (toptobottom) {
            for (y=y0, i=scroll_y1; y<y0+aufknuepfung.pos.height; y+=aufknuepfung.gh) {
                if (aufknuepfung.pos.strongline_y!=0 && (i++%aufknuepfung.pos.strongline_y)==0) {
                    Canvas->Pen->Color = strongclr;
                    Canvas->MoveTo (x0, y);
                    Canvas->LineTo (x0+aufknuepfung.pos.width, y);
                }
            }
        } else {
            for (y=y0+aufknuepfung.pos.height, i=scroll_y1; y>y0; y-=aufknuepfung.gh) {
                if (aufknuepfung.pos.strongline_y!=0 && (i++%aufknuepfung.pos.strongline_y)==0) {
                    Canvas->Pen->Color = strongclr;
                    Canvas->MoveTo (x0, y);
                    Canvas->LineTo (x0+aufknuepfung.pos.width, y);
                }
            }
        }
    }
    Canvas->Pen->Color = clBtnShadow;
    Canvas->MoveTo (x0, y0);
    Canvas->LineTo (x0+aufknuepfung.pos.width, y0);
    Canvas->LineTo (x0+aufknuepfung.pos.width, y0+aufknuepfung.pos.height);
    Canvas->LineTo (x0, y0+aufknuepfung.pos.height);
    Canvas->LineTo (x0, y0);

    // Inhalt
    int ii = aufknuepfung.pos.width/aufknuepfung.gw;
    int jj = aufknuepfung.pos.height/aufknuepfung.gh;
    if (!ViewSchlagpatrone->Checked) {
        if (ViewOnlyGewebe->Checked)
            return;
        for (i=0; i<ii; i++)
            for (j=0; j<jj; j++)
                DrawAufknuepfung (i, j);
    } else if (!aufknuepfung.pegplanstyle) {
        for (i=0; i<ii; i++)
            for (j=0; j<jj; j++)
                DrawAufknuepfung (i, j, true);
    } else if (!einzugunten) {
        if (!toptobottom) {
            Canvas->Brush->Color = clBtnFace;
            Canvas->Pen->Color = clBtnShadow;
            Canvas->Rectangle (aufknuepfung.pos.x0, aufknuepfung.pos.y0, aufknuepfung.pos.x0+aufknuepfung.pos.width+1, aufknuepfung.pos.y0+aufknuepfung.pos.height+1);
            for (int i=scroll_x2; i<scroll_x2+aufknuepfung.pos.width/aufknuepfung.gw; i++) {
                Canvas->MoveTo (aufknuepfung.pos.x0+(i-scroll_x2)*aufknuepfung.gw+aufknuepfung.gw/2, aufknuepfung.pos.y0+aufknuepfung.pos.height);
                int y = aufknuepfung.pos.y0+aufknuepfung.pos.height-(i-scroll_y1)*aufknuepfung.gh-aufknuepfung.gh/2;
                if (y<aufknuepfung.pos.y0) y = aufknuepfung.pos.y0;
                if (y>aufknuepfung.pos.y0+aufknuepfung.pos.height) y = aufknuepfung.pos.y0+aufknuepfung.pos.height;
                Canvas->LineTo (aufknuepfung.pos.x0+(i-scroll_x2)*aufknuepfung.gw+aufknuepfung.gw/2, y-1);
            }
            for (int j=scroll_y1; j<scroll_y1+aufknuepfung.pos.height/aufknuepfung.gh; j++) {
                Canvas->MoveTo (aufknuepfung.pos.x0, aufknuepfung.pos.y0+aufknuepfung.pos.height-(j-scroll_y1)*aufknuepfung.gh-aufknuepfung.gh/2);
                int x = aufknuepfung.pos.x0+(j-scroll_x2)*aufknuepfung.gw+aufknuepfung.gw/2;
                if (x<aufknuepfung.pos.x0) x = aufknuepfung.pos.x0;
                if (x>aufknuepfung.pos.x0+aufknuepfung.pos.width) x = aufknuepfung.pos.x0+aufknuepfung.pos.width;
                Canvas->LineTo (x, aufknuepfung.pos.y0+aufknuepfung.pos.height-(j-scroll_y1)*aufknuepfung.gh-aufknuepfung.gh/2);
            }
        } else {
            for (i=0; i<ii; i++)
                for (j=0; j<jj; j++)
                    DrawAufknuepfung (i, j, true);
        }
    } else {
        if (!toptobottom) {
            Canvas->Brush->Color = clBtnFace;
            Canvas->Pen->Color = clBtnShadow;
            Canvas->Rectangle (aufknuepfung.pos.x0, aufknuepfung.pos.y0, aufknuepfung.pos.x0+aufknuepfung.pos.width+1, aufknuepfung.pos.y0+aufknuepfung.pos.height+1);
            for (int i=scroll_x2; i<scroll_x2+aufknuepfung.pos.width/aufknuepfung.gw; i++) {
                Canvas->MoveTo (aufknuepfung.pos.x0+(i-scroll_x2)*aufknuepfung.gw+aufknuepfung.gw/2, aufknuepfung.pos.y0);
                int y = aufknuepfung.pos.y0+(i+scroll_y1)*aufknuepfung.gh+aufknuepfung.gh/2;
                if (y<aufknuepfung.pos.y0) y = aufknuepfung.pos.y0;
                if (y>aufknuepfung.pos.y0+aufknuepfung.pos.height) y = aufknuepfung.pos.y0+aufknuepfung.pos.height;
                Canvas->LineTo (aufknuepfung.pos.x0+(i-scroll_x2)*aufknuepfung.gw+aufknuepfung.gw/2, y-1);
            }
            for (int j=scroll_y1; j<scroll_y1+aufknuepfung.pos.height/aufknuepfung.gh; j++) {
                Canvas->MoveTo (aufknuepfung.pos.x0, aufknuepfung.pos.y0+(j-scroll_y1)*aufknuepfung.gh+aufknuepfung.gh/2);
                int x = (j-scroll_y1)*aufknuepfung.gw;
                x -= scroll_y1*aufknuepfung.gw;
                x -= scroll_x2*aufknuepfung.gw;
                x += aufknuepfung.gw/2;
                x += aufknuepfung.pos.x0 + 1;
                if (x<aufknuepfung.pos.x0) x = aufknuepfung.pos.x0;
                if (x>aufknuepfung.pos.x0+aufknuepfung.pos.width) x = aufknuepfung.pos.x0+aufknuepfung.pos.width;
                Canvas->LineTo (x, aufknuepfung.pos.y0+(j-scroll_y1)*aufknuepfung.gh+aufknuepfung.gh/2);
            }
        } else {
            for (i=0; i<ii; i++)
                for (j=0; j<jj; j++)
                    DrawAufknuepfung (i, j, true);
        }
    }
}

void __fastcall TDBWFRM::DrawTrittfolge()
{
    int x, y, i, j;

    if (ViewOnlyGewebe->Checked) {
        Canvas->Pen->Color = clBtnShadow;
        Canvas->Brush->Color = clBtnFace;
        Canvas->Rectangle (trittfolge.pos.x0, trittfolge.pos.y0, trittfolge.pos.x0+trittfolge.pos.width+1, trittfolge.pos.y0+trittfolge.pos.height+1);
    }

    // Rahmen
    Canvas->Pen->Color = clBtnShadow;
    Canvas->Brush->Color = Canvas->Pen->Color;
    int x0 = trittfolge.pos.x0;
    int y0 = trittfolge.pos.y0;
    for (x=x0; x<x0+trittfolge.pos.width; x+=trittfolge.gw) {
        Canvas->Pen->Color = clBtnShadow;
        Canvas->MoveTo (x, y0);
        Canvas->LineTo (x, y0+trittfolge.pos.height);
    }
    for (y=y0+trittfolge.pos.height; y>y0; y-=trittfolge.gh) {
        Canvas->Pen->Color = clBtnShadow;
        Canvas->MoveTo (x0, y);
        Canvas->LineTo (x0+trittfolge.pos.width, y);
    }
    for (x=x0, i=scroll_x2; x<x0+trittfolge.pos.width; x+=trittfolge.gw) {
        if (trittfolge.pos.strongline_x!=0 && (i++%trittfolge.pos.strongline_x)==0) {
            Canvas->Pen->Color = strongclr;
            Canvas->MoveTo (x, y0);
            Canvas->LineTo (x, y0+trittfolge.pos.height);
        }
    }
    for (y=y0+trittfolge.pos.height, i=scroll_y2; y>y0; y-=trittfolge.gh) {
        if (trittfolge.pos.strongline_y!=0 && (i++%trittfolge.pos.strongline_y)==0) {
            Canvas->Pen->Color = strongclr;
            Canvas->MoveTo (x0, y);
            Canvas->LineTo (x0+trittfolge.pos.width, y);
        }
    }
    Canvas->Pen->Color = clBtnShadow;
    Canvas->MoveTo (x0, y0);
    Canvas->LineTo (x0+trittfolge.pos.width, y0);
    Canvas->LineTo (x0+trittfolge.pos.width, y0+trittfolge.pos.height);
    Canvas->LineTo (x0, y0+trittfolge.pos.height);
    Canvas->LineTo (x0, y0);

    if (ViewOnlyGewebe->Checked)
        return;

    // Inhalt
    int ii = trittfolge.pos.width/trittfolge.gw;
    int jj = trittfolge.pos.height/trittfolge.gh;
    for (i=0; i<ii; i++)
        for (j=0; j<jj; j++)
            DrawTrittfolge (i, j);

    // Markierung für IsEmpty
#ifdef _DEBUG
    Canvas->Pen->Color = clBlack;
    for (j=0; j<trittfolge.pos.height/trittfolge.gh; j++) {
        if (trittfolge.isempty.Get(scroll_y2+j)) {
            Canvas->MoveTo (x0, y0+trittfolge.pos.height-(j+1)*trittfolge.gh);
            Canvas->LineTo (x0, y0+trittfolge.pos.height-j*trittfolge.gh);
        }
    }
#endif

#ifdef _DEBUG
    // Kennzeichnung für leere Tritte
    Canvas->Pen->Color = clBlack;
    for (int i=scroll_x2; i<scroll_x2+trittfolge.pos.width/trittfolge.gw; i++)
        if (freietritte[i]) {
            Canvas->MoveTo (trittfolge.pos.x0+(i-scroll_x2)*trittfolge.gw, trittfolge.pos.y0);
            Canvas->LineTo (trittfolge.pos.x0+(i-scroll_x2+1)*trittfolge.gw, trittfolge.pos.y0);
        }
#endif
}

void __fastcall TDBWFRM::DrawGewebe()
{
    if (GewebeNone->Checked) {
        Canvas->Pen->Color = clBtnShadow;
        Canvas->Brush->Color = clBtnFace;
        Canvas->Rectangle (gewebe.pos.x0, gewebe.pos.y0, gewebe.pos.x0+gewebe.pos.width+1, gewebe.pos.y0+gewebe.pos.height+1);
    }

    int x, y, i, j;
    int x0 = gewebe.pos.x0;
    int y0 = gewebe.pos.y0;

    // Rahmen
    Canvas->Pen->Color = clBtnShadow;
    for (x=x0; x<x0+gewebe.pos.width; x+=gewebe.gw) {
        Canvas->Pen->Color = clBtnShadow;
        Canvas->MoveTo (x, y0);
        Canvas->LineTo (x, y0+gewebe.pos.height);
    }
    for (y=y0+gewebe.pos.height; y>y0; y-=gewebe.gh) {
        Canvas->Pen->Color = clBtnShadow;
        Canvas->MoveTo (x0, y);
        Canvas->LineTo (x0+gewebe.pos.width, y);
    }
    if (righttoleft) {
        for (x=x0+gewebe.pos.width-gewebe.gw, i=scroll_x1; x>=x0; x-=gewebe.gw) {
            if (gewebe.pos.strongline_x!=0 && (++i%gewebe.pos.strongline_x)==0) {
                Canvas->Pen->Color = strongclr;
                Canvas->MoveTo (x, y0);
                Canvas->LineTo (x, y0+gewebe.pos.height);
            }
        }
    } else {
        for (x=x0, i=scroll_x1; x<x0+gewebe.pos.width; x+=gewebe.gw) {
            if (gewebe.pos.strongline_x!=0 && (i++%gewebe.pos.strongline_x)==0) {
                Canvas->Pen->Color = strongclr;
                Canvas->MoveTo (x, y0);
                Canvas->LineTo (x, y0+gewebe.pos.height);
            }
        }
    }
    for (y=y0+gewebe.pos.height, i=scroll_y2; y>y0; y-=gewebe.gh) {
        if (gewebe.pos.strongline_y!=0 && (i++%gewebe.pos.strongline_y)==0) {
            Canvas->Pen->Color = strongclr;
            Canvas->MoveTo (x0, y);
            Canvas->LineTo (x0+gewebe.pos.width, y);
        }
    }
    Canvas->Pen->Color = clBtnShadow;
    Canvas->MoveTo (x0, y0);
    Canvas->LineTo (x0+gewebe.pos.width, y0);
    Canvas->LineTo (x0+gewebe.pos.width, y0+gewebe.pos.height);
    Canvas->LineTo (x0, y0+gewebe.pos.height);
    Canvas->LineTo (x0, y0);

    // Wenn Gewebe nicht dargestellt werden soll: Abbrechen
    if (GewebeNone->Checked)
        return;

    // Inhalt
    for (i=0; i<gewebe.pos.width/gewebe.gw; i++)
        for (j=0; j<gewebe.pos.height/gewebe.gh; j++)
            DrawGewebe (i, j);
}

void __fastcall TDBWFRM::DrawSchussfarben()
{
    Canvas->Pen->Color = clBtnShadow;
    int y, j;
    for (y=schussfarben.pos.y0+schussfarben.pos.height-schussfarben.gh, j=scroll_y2; y>schussfarben.pos.y0-schussfarben.gh; y-=schussfarben.gh, j++) {
        Canvas->MoveTo (schussfarben.pos.x0, y+schussfarben.gh);
        Canvas->LineTo (schussfarben.pos.x0+schussfarben.pos.width, y+schussfarben.gh);
        Canvas->Brush->Color = GETPALCOL(schussfarben.feld.Get(j));
        Canvas->Rectangle (schussfarben.pos.x0, y, schussfarben.pos.x0+schussfarben.pos.width+1, y+schussfarben.gh+1);
    }
    Canvas->MoveTo (schussfarben.pos.x0, schussfarben.pos.y0);
    Canvas->LineTo (schussfarben.pos.x0+schussfarben.pos.width, schussfarben.pos.y0);
    Canvas->LineTo (schussfarben.pos.x0+schussfarben.pos.width, schussfarben.pos.y0+schussfarben.pos.height);
    Canvas->LineTo (schussfarben.pos.x0, schussfarben.pos.y0+schussfarben.pos.height);
    Canvas->LineTo (schussfarben.pos.x0, schussfarben.pos.y0);
}

void __fastcall TDBWFRM::RedrawGewebe (int _i, int _j)
{
    if (_i<scroll_x1 || _i>=scroll_x1+gewebe.pos.width/gewebe.gw ||
        _j<scroll_y2 || _j>=scroll_y2+gewebe.pos.height/gewebe.gh)
        return;
    DrawGewebe (_i-scroll_x1, _j-scroll_y2);
}

void __fastcall TDBWFRM::ClearGewebe (int _i, int _j)
{
    if (_i<scroll_x1 || _i>=scroll_x1+gewebe.pos.width/gewebe.gw ||
        _j<scroll_y2 || _j>=scroll_y2+gewebe.pos.height/gewebe.gh)
        return;
    DrawGewebe (_i-scroll_x1, _j-scroll_y2);
}

void __fastcall TDBWFRM::RedrawAufknuepfung (int _i, int _j)
{
    // Sichtbar?
    if (_i<scroll_x2 || _i>=scroll_x2+aufknuepfung.pos.width/aufknuepfung.gw ||
        _j<scroll_y1 || _j>=scroll_y1+aufknuepfung.pos.height/aufknuepfung.gh)
        return;
    DrawAufknuepfung (_i-scroll_x2, _j-scroll_y1);
}

void __fastcall TDBWFRM::_ClearEinzug()
{
    int maxi = scroll_x1+einzug.pos.width/einzug.gw;
    int maxj = scroll_y1+einzug.pos.height/einzug.gh;
    for (int i=scroll_x1; i<maxi; i++) {
        short s = einzug.feld.Get(i);
        if (s!=0 && s-1>=scroll_y1 && s-1<maxj)
            DrawEinzug (i, s-1, clBtnFace);
    }
}

void __fastcall TDBWFRM::_DrawEinzug()
{
    int maxi = scroll_x1+einzug.pos.width/einzug.gw;
    int maxj = scroll_y1+einzug.pos.height/einzug.gh;
    for (int i=scroll_x1; i<maxi; i++) {
        short s = einzug.feld.Get(i);
        if (s!=0 && s-1>=scroll_y1 && s-1<maxj)
            DrawEinzug (i, s-1);
    }
}

void __fastcall TDBWFRM::_ClearAufknuepfung()
{
    if (ViewSchlagpatrone->Checked) return;

    for (int i=scroll_x2; i<scroll_x2+aufknuepfung.pos.width/aufknuepfung.gw; i++)
        for (int j=scroll_y1; j<scroll_y1+aufknuepfung.pos.height/aufknuepfung.gh; j++) {
            char s = aufknuepfung.feld.Get (i, j);
            if (s>0) DrawAufknuepfung (i, j, aufknuepfung.pegplanstyle, clBtnFace);
        }
}

void __fastcall TDBWFRM::_DrawAufknuepfung()
{
    if (ViewSchlagpatrone->Checked) return;

    for (int i=scroll_x2; i<scroll_x2+aufknuepfung.pos.width/aufknuepfung.gw; i++)
        for (int j=scroll_y1; j<scroll_y1+aufknuepfung.pos.height/aufknuepfung.gh; j++) {
            char s = aufknuepfung.feld.Get (i, j);
            if (s>0) DrawAufknuepfung (i, j);
        }
}

void __fastcall TDBWFRM::_ClearSchlagpatrone()
{
    if (!ViewSchlagpatrone->Checked) return;

    for (int i=scroll_x2; i<scroll_x2+trittfolge.pos.width/trittfolge.gw; i++)
        for (int j=scroll_y2; j<scroll_y2+trittfolge.pos.height/trittfolge.gh; j++) {
            char s = trittfolge.feld.Get (i, j);
            if (s>0) DrawTrittfolge (i, j, clBtnFace);
        }
}

void __fastcall TDBWFRM::_DrawSchlagpatrone()
{
    if (!ViewSchlagpatrone->Checked) return;

    for (int i=scroll_x2; i<scroll_x2+trittfolge.pos.width/trittfolge.gw; i++)
        for (int j=scroll_y2; j<scroll_y2+trittfolge.pos.height/trittfolge.gh; j++) {
            char s = trittfolge.feld.Get (i, j);
            if (s>0) DrawTrittfolge (i, j);
        }
}


