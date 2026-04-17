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
#include <vcl\printers.hpp>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "felddef.h"
#include "datamodule.h"
#include "palette.h"
#include "print.h"
#include "rangecolors.h"
/*-----------------------------------------------------------------*/
#define XSCALE(x) \
    ((int)(x_scale*(x)))
/*-----------------------------------------------------------------*/
#define YSCALE(y) \
    ((int)(y_scale*(y)))
/*-----------------------------------------------------------------*/
#define SETSTRONGWIDTH \
    int oldwidth = canvas->Pen->Width; \
    canvas->Pen->Width = XSCALE(GetDeviceCaps(Printer()->Handle, LOGPIXELSX)*25/2540); \
    int pw = canvas->Pen->Width/2; 
/*-----------------------------------------------------------------*/
#define RESETSTRONGWIDTH \
    canvas->Pen->Width = oldwidth;
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrPaintCell (DARSTELLUNG _darstellung, int _x, int _y, int _xx, int _yy, TColor _col/*=clBlack*/, int _number/*=-1*/)
{
	int width = canvas->Pen->Width;
	int penwidth = 0;
	if (_darstellung!=AUSGEFUELLT) {
		penwidth = GetDeviceCaps (Printer()->Handle, LOGPIXELSX)*5/254; // 0.5 mm Strichstärke
		penwidth = penwidth*mainfrm->zoom[mainfrm->currentzoom]/mainfrm->zoom[3]; // Skalieren
		canvas->Pen->Width = XSCALE(penwidth);
	}

    canvas->Pen->Color = _col;
    canvas->Brush->Color = _col;

	int dx = penwidth;
	int dy = penwidth;

    switch (_darstellung) {
        case AUSGEFUELLT:
            canvas->Rectangle (XSCALE(_x+dx), YSCALE(_y+dy), XSCALE(_xx-dx+1), YSCALE(_yy-dy+1));
            break;

        case STRICH:
            canvas->MoveTo (XSCALE((_x+_xx)/2), YSCALE(_yy-dy));
            canvas->LineTo (XSCALE((_x+_xx)/2), YSCALE(_y+dy+1));
            canvas->MoveTo (XSCALE((_x+_xx)/2+1), YSCALE(_yy-dy));
            canvas->LineTo (XSCALE((_x+_xx)/2+1), YSCALE(_y+dy+1));
            break;

        case KREUZ:
            canvas->MoveTo (XSCALE(_x+dx), YSCALE(_yy-dy));
            canvas->LineTo (XSCALE(_xx-dx), YSCALE(_y+dy));
            canvas->MoveTo (XSCALE(_x+dx), YSCALE(_y+dy));
            canvas->LineTo (XSCALE(_xx-dx), YSCALE(_yy-dy));
            break;

        case PUNKT: {
            int x = (_x+_xx)/2;
            int y = (_y+_yy)/2;
            canvas->MoveTo (XSCALE(x), YSCALE(y));
			canvas->LineTo (XSCALE(x+1), YSCALE(y));
            canvas->MoveTo (XSCALE(x), YSCALE(y+1));
			canvas->LineTo (XSCALE(x+1), YSCALE(y+1));
            break;
        }
        case KREIS:
            canvas->Arc (XSCALE(_x+dx), YSCALE(_y+dy), XSCALE(_xx-dx+1), YSCALE(_yy-dy+1), XSCALE(_x), YSCALE(_y), XSCALE(_x), YSCALE(_y));
            break;

        case STEIGEND:
            canvas->MoveTo (XSCALE(_x+dx), YSCALE(_yy-dy));
            canvas->LineTo (XSCALE(_xx-dx), YSCALE(_y+dy));
            break;

        case FALLEND:
            canvas->MoveTo (XSCALE(_x+dx), YSCALE(_y+dy));
            canvas->LineTo (XSCALE(_xx-dx), YSCALE(_yy-dy));
            break;

        case SMALLKREIS:
            if (canvas->Pen->Width>2) canvas->Pen->Width = canvas->Pen->Width/2;
            dx = dx + (_xx-_x)/10;
            dy = dy + (_yy-_y)/10;
            canvas->Arc (XSCALE(_x+dx), YSCALE(_y+dy), XSCALE(_xx-dx+1), YSCALE(_yy-dy+1), XSCALE(_x), YSCALE(_y), XSCALE(_x), YSCALE(_y));
            break;

        case SMALLKREUZ:
            if (canvas->Pen->Width>2) canvas->Pen->Width = canvas->Pen->Width/2;
            dx = dx + (_xx-_x)/10;
            dy = dy + (_yy-_y)/10;
            canvas->MoveTo (XSCALE(_x+dx), YSCALE(_yy-dy));
            canvas->LineTo (XSCALE(_xx-dx), YSCALE(_y+dy));
            canvas->MoveTo (XSCALE(_x+dx), YSCALE(_y+dy));
            canvas->LineTo (XSCALE(_xx-dx), YSCALE(_yy-dy));
            break;

        case NUMBER:
            if (_number>=0) {
                canvas->Brush->Color = clWhite;
                int oldflags = canvas->TextFlags;
                int oldsize = canvas->Font->Height;
                canvas->TextFlags = canvas->TextFlags & ~ETO_OPAQUE;
                canvas->Font->Color = _col;
                canvas->Font->Height = -einzug.gh*8/10;
                String nr = IntToStr(_number+1);
                TSize sz = canvas->TextExtent(nr);
                int xx = 0; if (_xx-_x>sz.cx) xx = (_xx-_x-sz.cx)/2;
                int yy = 0; if (_yy-_y>sz.cy) yy = (_yy-_y-sz.cy)/2;
                canvas->TextRect (TRect(XSCALE(_x+dx), YSCALE(_y+dy), XSCALE(_xx-dx), YSCALE(_yy-dy)), XSCALE(_x+xx), YSCALE(_y+yy), nr);
                canvas->TextFlags = oldflags;
                canvas->Font->Height = oldsize;
            } else {
                canvas->Rectangle (XSCALE(_x+dx), YSCALE(_y+dy), XSCALE(_xx-dx+1), YSCALE(_yy-dy+1));
            }
            break;
    }

	canvas->Pen->Width = width;
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintGewebeNone (int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    // Rahmen
    canvas->Pen->Color = clBlack;
    canvas->MoveTo (XSCALE(_x),  YSCALE(_y));
    canvas->LineTo (XSCALE(_xx), YSCALE(_y));
    canvas->LineTo (XSCALE(_xx), YSCALE(_yy));
    canvas->LineTo (XSCALE(_x),  YSCALE(_yy));
    canvas->LineTo (XSCALE(_x),  YSCALE(_y));
    // 'Strongline'
    SETSTRONGWIDTH
    if (mainfrm->righttoleft) {
        if (_i!=0 && mainfrm->gewebe.pos.strongline_x!=0 && ((_i+kette.a)%mainfrm->gewebe.pos.strongline_x)==0) {
            canvas->MoveTo (XSCALE(_xx), YSCALE(_y+pw));
            canvas->LineTo (XSCALE(_xx), YSCALE(_yy-pw));
        }
    } else {
        if (_i!=0 && mainfrm->gewebe.pos.strongline_x!=0 && ((_i+kette.a)%mainfrm->gewebe.pos.strongline_x)==0) {
            canvas->MoveTo (XSCALE(_x), YSCALE(_y+pw));
            canvas->LineTo (XSCALE(_x), YSCALE(_yy-pw));
        }
    }
    if (_j!=0 && mainfrm->gewebe.pos.strongline_y!=0 && ((_j+schuesse.a)%mainfrm->gewebe.pos.strongline_y)==0) {
        canvas->MoveTo (XSCALE(_x+pw), YSCALE(_yy));
        canvas->LineTo (XSCALE(_xx-pw), YSCALE(_yy));
    }
	RESETSTRONGWIDTH
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintGewebeNormal (int _i, int _j, int _x, int _y, int _xx, int _yy)
{
	int dx = 0;
	int dy = 0;

    // Feld zeichnen
    char s = mainfrm->gewebe.feld.Get(_i+kette.a, _j+schuesse.a);
    if (s==AUSHEBUNG) {
        PrPaintCell (mainfrm->darst_aushebung, _x, _y, _xx, _yy);
    } else if (s==ANBINDUNG) {
        PrPaintCell (mainfrm->darst_anbindung, _x, _y, _xx, _yy);
    } else if (s==ABBINDUNG) {
        PrPaintCell (mainfrm->darst_abbindung, _x, _y, _xx, _yy);
    } else if (s>0) {
        canvas->Brush->Color = GetRangeColor(s);
        canvas->Pen->Color = GetRangeColor(s);
        canvas->Rectangle (XSCALE(_x+dx), YSCALE(_y+dy), XSCALE(_xx-dx+1), YSCALE(_yy-dy+1));
    } else {
        canvas->Brush->Color = clWhite;
        canvas->Pen->Color = clBlack;
        canvas->Rectangle (XSCALE(_x+dx), YSCALE(_y+dy), XSCALE(_xx-dx+1), YSCALE(_yy-dy+1));
    }
    // 'Strongline'
    SETSTRONGWIDTH
    if (mainfrm->righttoleft) {
        if (_i!=0 && mainfrm->gewebe.pos.strongline_x!=0 && ((_i+kette.a)%mainfrm->gewebe.pos.strongline_x)==0) {
            canvas->MoveTo (XSCALE(_xx), YSCALE(_y+pw));
            canvas->LineTo (XSCALE(_xx), YSCALE(_yy-pw));
        }
    } else {
        if (_i!=0 && mainfrm->gewebe.pos.strongline_x!=0 && ((_i+kette.a)%mainfrm->gewebe.pos.strongline_x)==0) {
            canvas->MoveTo (XSCALE(_x), YSCALE(_y+pw));
            canvas->LineTo (XSCALE(_x), YSCALE(_yy-pw));
        }
    }
    if (_j!=0 && mainfrm->gewebe.pos.strongline_y!=0 && ((_j+schuesse.a)%mainfrm->gewebe.pos.strongline_y)==0) {
        canvas->MoveTo (XSCALE(_x+pw), YSCALE(_yy));
        canvas->LineTo (XSCALE(_xx-pw), YSCALE(_yy));
    }
	RESETSTRONGWIDTH
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintGewebeRapport (int _i, int _j, int _x, int _y, int _xx, int _yy)
{
	int dx = 0;
	int dy = 0;

    // Feld zeichnen
    char s = mainfrm->gewebe.feld.Get(_i+kette.a,_j+schuesse.a);
    if (s>0 && s!=ABBINDUNG) {
        if (!blackandwhite) canvas->Brush->Color = clRed;
        else canvas->Brush->Color = clGray;
        canvas->Pen->Color = clBlack;
        canvas->Rectangle (XSCALE(_x+dx), YSCALE(_y+dy), XSCALE(_xx-dx+1), YSCALE(_yy-dy+1));
    } else {
        canvas->Brush->Color = clWhite;
        canvas->Pen->Color = clBlack;
        canvas->Rectangle (XSCALE(_x+dx), YSCALE(_y+dy), XSCALE(_xx-dx+1), YSCALE(_yy-dy+1));
    }
    // 'Strongline'
    SETSTRONGWIDTH
    if (mainfrm->righttoleft) {
        if (_i!=0 && mainfrm->gewebe.pos.strongline_x!=0 && ((_i+kette.a)%mainfrm->gewebe.pos.strongline_x)==0) {
            canvas->MoveTo (XSCALE(_xx), YSCALE(_y+pw));
            canvas->LineTo (XSCALE(_xx), YSCALE(_yy-pw));
        }
    } else {
        if (_i!=0 && mainfrm->gewebe.pos.strongline_x!=0 && ((_i+kette.a)%mainfrm->gewebe.pos.strongline_x)==0) {
            canvas->MoveTo (XSCALE(_x), YSCALE(_y+pw));
            canvas->LineTo (XSCALE(_x), YSCALE(_yy-pw));
        }
    }
    if (_j!=0 && mainfrm->gewebe.pos.strongline_y!=0 && ((_j+schuesse.a)%mainfrm->gewebe.pos.strongline_y)==0) {
        canvas->MoveTo (XSCALE(_x+pw), YSCALE(_yy));
        canvas->LineTo (XSCALE(_xx-pw), YSCALE(_yy));
    }
	RESETSTRONGWIDTH
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintGewebeFarbeffekt (int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    char s = mainfrm->gewebe.feld.Get(_i+kette.a,_j+schuesse.a);
    bool drawhebung = s>0 && s!=ABBINDUNG;
    if (mainfrm->sinkingshed) drawhebung = !drawhebung;
    if (drawhebung) {
        canvas->Brush->Color = (TColor)Data->palette->GetColor(mainfrm->kettfarben.feld.Get(_i+kette.a));
    } else {
        canvas->Brush->Color = (TColor)Data->palette->GetColor(mainfrm->schussfarben.feld.Get(_j+schuesse.a));
    }
	TRect rc;
	rc.Left = XSCALE(_x);
	rc.Top = YSCALE(_y);
	rc.Right = XSCALE(_xx+1);
	rc.Bottom = YSCALE(_yy+1);
    canvas->FillRect (rc);
    if (mainfrm->fewithraster)
        PrintGewebeNone (_i, _j, _x, _y, _xx, _yy);
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintGewebeSimulation (int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    int dw = abs(_xx-_x)/4;
    int dh = abs(_yy-_y)/4;
    if (dw==0 && abs(_xx-_x)>2) dw = 1;
    if (dh==0 && abs(_yy-_y)>2) dh = 1;

    char s = mainfrm->gewebe.feld.Get(_i+kette.a,_j+schuesse.a);
    bool drawhebung = s>0 && s!=ABBINDUNG;
    if (mainfrm->sinkingshed) drawhebung = !drawhebung;
    if (drawhebung) {
        canvas->Brush->Color = (TColor)Data->palette->GetColor(mainfrm->kettfarben.feld.Get(_i+kette.a));
        canvas->Pen->Color = (TColor)Data->palette->GetColor(mainfrm->kettfarben.feld.Get(_i+kette.a));
        canvas->Rectangle (XSCALE(_x+dw), YSCALE(_y), XSCALE(_xx-dw), YSCALE(_yy));
        canvas->Brush->Color = (TColor)Data->palette->GetColor(mainfrm->schussfarben.feld.Get(_j+schuesse.a));
        canvas->Pen->Color = (TColor)Data->palette->GetColor(mainfrm->schussfarben.feld.Get(_j+schuesse.a));
        canvas->Rectangle (XSCALE(_x), YSCALE(_y+dh), XSCALE(_x+dw), YSCALE(_yy-dh));
        canvas->Rectangle (XSCALE(_xx), YSCALE(_y+dh), XSCALE(_xx-dw), YSCALE(_yy-dh));
        // Schatten
		int oldwidth = canvas->Pen->Width;
		canvas->Pen->Width = dw>1 ? dw/3 : 1;
        canvas->Pen->Color = clBlack;
        canvas->MoveTo (XSCALE(_xx-dw), YSCALE(_y+dh));
        canvas->LineTo (XSCALE(_xx-dw), YSCALE(_yy-dh));
		canvas->Pen->Width = oldwidth;
    } else {
        canvas->Brush->Color = (TColor)Data->palette->GetColor(mainfrm->schussfarben.feld.Get(_j+schuesse.a));
        canvas->Pen->Color = (TColor)Data->palette->GetColor(mainfrm->schussfarben.feld.Get(_j+schuesse.a));
        canvas->Rectangle (XSCALE(_x), YSCALE(_y+dh), XSCALE(_xx), YSCALE(_yy-dh));
        canvas->Brush->Color = (TColor)Data->palette->GetColor(mainfrm->kettfarben.feld.Get(_i+kette.a));
        canvas->Pen->Color = (TColor)Data->palette->GetColor(mainfrm->kettfarben.feld.Get(_i+kette.a));
        canvas->Rectangle (XSCALE(_x+dw), YSCALE(_y), XSCALE(_xx-dw), YSCALE(_y+dh));
        canvas->Rectangle (XSCALE(_x+dw), YSCALE(_yy-dh), XSCALE(_xx-dw), YSCALE(_yy));
        // Schatten
		int oldwidth = canvas->Pen->Width;
		canvas->Pen->Width = dh>1 ? dh/3 : 1;
        canvas->Pen->Color = clBlack;
        canvas->MoveTo (XSCALE(_x+dw), YSCALE(_yy-dh));
        canvas->LineTo (XSCALE(_xx-dw), YSCALE(_yy-dh));
		canvas->Pen->Width = oldwidth;
    }
}
/*-----------------------------------------------------------------*/

