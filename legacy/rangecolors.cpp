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
#include "dbw3_base.h"
/*-----------------------------------------------------------------*/
// Diese Farben sollten sich nicht mit anderweitig benötigten
// Farben überschneiden. Beispiele sind logischerweise Schwarz,
// dann aber auch Rot (wegen Rapport) etc.
TColor rangecolortable[13] =
    { clBlack, clBlue, clFuchsia, clMaroon, clNavy,
      clPurple, clOlive, clYellow, clGreen,
      clDkGray, clDkGray, clWhite, clWhite };
/*-----------------------------------------------------------------*/
TColor col_anbindung = clGreen;
TColor col_abbindung = clYellow;
/*-----------------------------------------------------------------*/
TColor GetRangeColor(int _s)
{
    if (_s==AUSHEBUNG) return rangecolortable[9];
    if (_s==ANBINDUNG) return rangecolortable[10];
    if (_s==ABBINDUNG) return rangecolortable[11];
    if (_s>0 && _s<=9) return rangecolortable[_s-1];
    return clBlack;
}
/*-----------------------------------------------------------------*/
// Initialisiert die Farben entsprechend der Grafikhardware
// des Computers. Falls nur 256 Farben dann Standard-Rangecolors
// belassen. Sonst (nonpalettized!) schönere Farben einstellen.
// Das ist eh bei 95% aller heutigen PC so...
void __fastcall InitRangeColors (TCanvas* _canvas)
{
    if (GetDeviceCaps(_canvas->Handle, BITSPIXEL)>=16) { // Mehr als 256 Farben
        rangecolortable[0] = TColor(RGB(0,0,0));
        rangecolortable[1] = TColor(RGB(50,50,255));
        rangecolortable[2] = TColor(RGB(128,0,0));
        rangecolortable[3] = TColor(RGB(0,140,255));
        rangecolortable[4] = TColor(RGB(56, 56, 56));
        rangecolortable[5] = TColor(RGB(0,194,78));
        rangecolortable[6] = TColor(RGB(255,123,0));
        rangecolortable[7] = TColor(RGB(255,210,0));
        rangecolortable[8] = TColor(RGB(0,87,0));
        rangecolortable[9] = TColor(RGB(128,128,128));
        rangecolortable[10] = TColor(RGB(128,128,128));
        rangecolortable[11] = TColor(RGB(255,255,255));
        rangecolortable[12] = TColor(RGB(255,255,255));

        col_anbindung = TColor(RGB(72,160,72));
        col_abbindung = TColor(RGB(255,255,110));
    }
}
/*-----------------------------------------------------------------*/
