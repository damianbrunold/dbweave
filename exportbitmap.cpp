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
#include "datamodule.h"
#include "palette.h"
#include "rangecolors.h"
/*-----------------------------------------------------------------*/
#define MAXDX 800
#define MAXDY 600
/*-----------------------------------------------------------------*/
#define GETPALCOL(col) \
    ((TColor)Data->palette->GetColor(col))
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DoExportBitmap (const String& _filename)
{
    try {
        Graphics::TBitmap* bmp = new Graphics::TBitmap;
        // Zellgrösse wie bei Bildschirm?!
        int gw = gridwidth;
        int gh = gridheight;

        // Ausmass der Bitmap berechnen und setzen
        int shafts = 0;
        if (!ViewEinzug->Checked) shafts = 0;
        else
            for (int ii=kette.a; ii<=kette.b; ii++) {
                int s = einzug.feld.Get (ii);
                if (s>shafts) shafts = s;
            }
        int treadles = 0;
        if (!ViewTrittfolge->Checked) treadles = 0;
        else
            for (int i=Data->MAXX2-1; i>=0; i--)
                for (int j=schuesse.a; j<=schuesse.b; j++)
                    if (trittfolge.feld.Get(i, j)!=0)
                        if (treadles<i+1) treadles = i+1;

        int sdy = shafts!=0 ? 1 : 0;
        int tdx = treadles!=0 ? 1 : 0;

        int dx = kette.count();
        int dy = schuesse.count();

        int x = gw*(dx+tdx+treadles);
        int y = gh*(dy+sdy+shafts);

        bool clr = ViewFarbe->Checked;
        int clry = clr ? 2 : 0;
        int clrx = clr ? 2 : 0;
        if (clr) {
            x += 2*gw;
            y += 2*gh;
        }

        bool blez = ViewBlatteinzug->Checked;
        int by = blez ? 2 : 0;
        if (blez) y += 2*gh;

        x++;
        y++;

        bmp->Width = x>MAXDX ? MAXDX : x;
        bmp->Height = y>MAXDY ? MAXDY : y;

        if (x>MAXDX) dx -= (x-MAXDX)/gw; // Falls zu gross, Gewebe einschränken
        if (y>MAXDY) dy -= (y-MAXDY)/gh;

        bool hl = ViewHlines->Checked;

        // Dividers abtönen
        bmp->Canvas->Pen->Color = TColor(RGB(228,228,228));
        bmp->Canvas->Brush->Color = bmp->Canvas->Pen->Color;
        if (clr) {
            if (!einzugunten) {
                bmp->Canvas->Rectangle (dx*gw+1, 0, (dx+tdx+treadles+clrx)*gw+1, gh+1);
                bmp->Canvas->Rectangle (0, gh+1, (dx+tdx+treadles+clrx)*gw+1, 2*gh);
            } else {
                bmp->Canvas->Rectangle (0, (dy+by+sdy+shafts)*gh+1, (dx+tdx+treadles+clrx)*gw+1, (dy+by+sdy+shafts+1)*gh);
                bmp->Canvas->Rectangle (dx*gw+1, (dy+by+sdy+shafts+1)*gh, (dx+tdx+treadles+clrx)*gw+1, (dy+by+sdy+shafts+2)*gh+1);
            }
        }
        if (shafts!=0) {
            if (!einzugunten) {
                bmp->Canvas->Rectangle (0, (clry+shafts)*gh+1, (dx+tdx+treadles+clrx)*gw+1, (clry+shafts+1)*gh);
            } else {
                bmp->Canvas->Rectangle (0, (dy+by)*gh+1, (dx+tdx+treadles+clrx)*gw+1, (dy+by+1)*gh);
            }
        }
        if (blez) {
            if (!einzugunten) {
                bmp->Canvas->Rectangle (0, (clry+shafts+sdy+1)*gh+1, (dx+tdx+treadles+clrx)*gw+1, (clry+shafts+sdy+2)*gh);
                bmp->Canvas->Rectangle (dx*gw, (clry+shafts+sdy)*gh, (dx+tdx+treadles+clrx)*gw+1, (clry+shafts+sdy+1)*gh+1);
            } else {
                bmp->Canvas->Rectangle (0, dy*gh+1, (dx+tdx+treadles+clrx)*gw+1, (dy+1)*gh);
                bmp->Canvas->Rectangle (dx*gw, (dy+1)*gh, (dx+tdx+treadles+clrx)*gw+1, (dy+2)*gh+1);
            }
        }
        if (treadles!=0) {
            bmp->Canvas->Rectangle (dx*gw+1, 0, (dx+1)*gw, (clry+shafts+sdy+by+dy)*gh+1);
        }
        if (clr) {
            bmp->Canvas->Rectangle ((dx+tdx+treadles)*gw+1, 0, (dx+tdx+treadles+1)*gw, (clry+shafts+sdy+by+dy)*gh+1);
            if (!einzugunten) {
                bmp->Canvas->Rectangle ((dx+tdx+treadles+1)*gw, 0, (dx+tdx+treadles+2)*gw+1, (clry+shafts+sdy+by)*gh+1);
            } else {
                bmp->Canvas->Rectangle ((dx+tdx+treadles+1)*gw, dy*gh+1, (dx+tdx+treadles+2)*gw+1, (clry+shafts+sdy+by+dy)*gh+1);
            }
        }

        // Blatteinzug zeichnen
        if (blez) {
            int x0 = 0;
            int y0 = (clry+shafts+sdy)*gh;
            if (einzugunten) y0 = (dy+1)*gh;
            // Gitter
            bmp->Canvas->Pen->Color = clDkGray;
            for (int i=0; i<=dx; i++) {
                bmp->Canvas->MoveTo (x0+i*gw, y0);
                bmp->Canvas->LineTo (x0+i*gw, y0+gh);
            }
            for (int j=0; j<=1; j++) {
                bmp->Canvas->MoveTo (x0, y0+j*gh);
                bmp->Canvas->LineTo (x0+dx*gw+1, y0+j*gh);
            }
            // Daten
            bmp->Canvas->Pen->Color = clBlack;
            bmp->Canvas->Brush->Color = bmp->Canvas->Pen->Color;
            for (int i=0; i<dx; i++) {
                if (blatteinzug.feld.Get(kette.a+i)) {
                    bmp->Canvas->Rectangle (x0+i*gw+1, y0+1, x0+(i+1)*gw, y0+gh/2+1);
                } else {
                    bmp->Canvas->Rectangle (x0+i*gw+1, y0+gh/2+1, x0+(i+1)*gw, y0+gh+1);
                }
            }
            // Rundherum
            bmp->Canvas->Pen->Color = clBlack;
            bmp->Canvas->MoveTo (x0, y0);
            bmp->Canvas->LineTo (x0+dx*gw, y0);
            bmp->Canvas->LineTo (x0+dx*gw, y0+gh);
            bmp->Canvas->LineTo (x0, y0+gh);
            bmp->Canvas->LineTo (x0, y0);
        }

        // Kettfarben zeichnen
        if (clr) {
            int x0 = 0;
            int y0 = 0;
            if (einzugunten) y0 = (dy+by+sdy+shafts+1)*gh;
            // Gitter
            bmp->Canvas->Pen->Color = clDkGray;
            for (int i=0; i<=dx; i++) {
                bmp->Canvas->MoveTo (x0+i*gw, y0);
                bmp->Canvas->LineTo (x0+i*gw, y0+gh);
            }
            for (int j=0; j<=1; j++) {
                bmp->Canvas->MoveTo (x0, y0+j*gh);
                bmp->Canvas->LineTo (x0+dx*gw+1, y0+j*gh);
            }
            // Daten
            for (int i=0; i<dx; i++) {
                bmp->Canvas->Pen->Color = GETPALCOL(kettfarben.feld.Get(kette.a+i));
                bmp->Canvas->Brush->Color = bmp->Canvas->Pen->Color;
                bmp->Canvas->Rectangle (x0+i*gw+1, y0+1, x0+(i+1)*gw, y0+gh+1);
            }
            // Rundherum
            bmp->Canvas->Pen->Color = clBlack;
            bmp->Canvas->MoveTo (x0, y0);
            bmp->Canvas->LineTo (x0+dx*gw, y0);
            bmp->Canvas->LineTo (x0+dx*gw, y0+gh);
            bmp->Canvas->LineTo (x0, y0+gh);
            bmp->Canvas->LineTo (x0, y0);
        }

        // Schussfarben zeichnen
        if (clr) {
            int x0 = (dx+tdx+treadles+1)*gw;
            int y0 = (clry+shafts+sdy+by)*gh;
            if (einzugunten) y0 = 0;
            // Gitter
            bmp->Canvas->Pen->Color = clDkGray;
            for (int i=0; i<=1; i++) {
                bmp->Canvas->MoveTo (x0+i*gw, y0);
                bmp->Canvas->LineTo (x0+i*gw, y0+dy*gh);
            }
            for (int j=0; j<=dy; j++) {
                bmp->Canvas->MoveTo (x0, y0+j*gh);
                bmp->Canvas->LineTo (x0+gw, y0+j*gh);
            }
            // Daten
            for (int j=0; j<dy; j++) {
                bmp->Canvas->Pen->Color = GETPALCOL(schussfarben.feld.Get(schuesse.a+j));
                bmp->Canvas->Brush->Color = bmp->Canvas->Pen->Color;
                bmp->Canvas->Rectangle (x0+1, y0+(dy-j-1)*gh+1, x0+gw+1, y0+(dy-j)*gh);
            }
            // Rundherum
            bmp->Canvas->Pen->Color = clBlack;
            bmp->Canvas->MoveTo (x0, y0);
            bmp->Canvas->LineTo (x0+gw, y0);
            bmp->Canvas->LineTo (x0+gw, y0+dy*gh);
            bmp->Canvas->LineTo (x0, y0+dy*gh);
            bmp->Canvas->LineTo (x0, y0);
        }

        // Einzug zeichnen
        if (shafts!=0) {
            int x0 = 0;
            int y0 = clry*gh;
            if (einzugunten) y0 = (dy+by+sdy)*gh;
            // Gitter
            bmp->Canvas->Pen->Color = clDkGray;
            for (int i=0; i<=dx; i++) {
                bmp->Canvas->MoveTo (x0+i*gw, y0);
                bmp->Canvas->LineTo (x0+i*gw, y0+shafts*gh);
            }
            for (int j=0; j<=shafts; j++) {
                bmp->Canvas->MoveTo (x0, y0+j*gh);
                bmp->Canvas->LineTo (x0+dx*gw+1, y0+j*gh);
            }
            // Schenien
            bmp->Canvas->Pen->Color = clBlack;
            int sx = einzug.pos.strongline_x;
            int sy = einzug.pos.strongline_y;
            for (int i=0; i<=dx; i++) {
                if (i%sx==0) {
                    bmp->Canvas->MoveTo (x0+i*gw, y0);
                    bmp->Canvas->LineTo (x0+i*gw, y0+shafts*gh);
                }
            }
            for (int j=0; j<=shafts; j++) {
                if (j%sy==0) {
                    bmp->Canvas->MoveTo (x0, y0+j*gh);
                    bmp->Canvas->LineTo (x0+dx*gw+1, y0+j*gh);
                }
            }
            // Daten
            if (!ViewOnlyGewebe->Checked) {
                for (int i=0; i<dx; i++) {
                    int s = einzug.feld.Get(kette.a+i);
                    if (s!=0) {
                        int x = x0+i*gw;
                        if (righttoleft) x = x0+(dx-i-1)*gw;
                        int y = y0+(shafts-s)*gh;
                        if (toptobottom) y = y0+(s-1)*gh;
                        PaintCell (bmp->Canvas, einzug.darstellung,
                            x, y, x+gw, y+gh, clBlack, true, s-1, clWhite);
                    }
                }
            }
            // Hilfslinien
            if (hl) {
                int count = hlines.GetCount();
                bmp->Canvas->Pen->Color = clBlue;
                for (int i=0; i<count; i++) {
                    Hilfslinie* line = hlines.GetLine(i);
                    if (!line) continue;
                    if (line->typ==HL_HORZ && line->feld==HL_TOP) {
                        if (line->pos>=shafts) continue;
                        int y = y0 + (shafts - line->pos)*gh;
                        if (toptobottom) y = y0 + line->pos*gh;
                        bmp->Canvas->MoveTo (x0, y);
                        bmp->Canvas->LineTo (x0+dx*gw, y);
                    } else if (line->typ==HL_VERT && line->feld==HL_LEFT) {
                        if (line->pos<=kette.a || line->pos>kette.b) continue;
                        int x = x0 + (line->pos-kette.a)*gw;
                        if (righttoleft) x = x0 + (dx-line->pos+kette.a)*gw;
                        bmp->Canvas->MoveTo (x, y0);
                        bmp->Canvas->LineTo (x, y0+shafts*gh);
                    }
                }
            }
            // Rapport
            bmp->Canvas->Pen->Color = clRed;
            if (RappViewRapport->Checked) {
                int x = x0 + (rapport.kr.b+1-kette.a)*gw;
                if (righttoleft) x = x0 + (dx-rapport.kr.b-1+kette.a)*gw;
                bmp->Canvas->MoveTo (x, y0);
                bmp->Canvas->LineTo (x, y0+shafts*gh);
            }
            // Rundherum
            bmp->Canvas->Pen->Color = clBlack;
            bmp->Canvas->MoveTo (x0, y0);
            bmp->Canvas->LineTo (x0+dx*gw, y0);
            bmp->Canvas->LineTo (x0+dx*gw, y0+shafts*gh);
            bmp->Canvas->LineTo (x0, y0+shafts*gh);
            bmp->Canvas->LineTo (x0, y0);
        }

        // Aufknüpfung zeichnen
        if (treadles!=0 && shafts!=0) {
            int x0 = (dx+tdx)*gw;
            int y0 = clry*gh;
            if (einzugunten) y0 = (dy+sdy+by)*gh;
            // Gitter
            bmp->Canvas->Pen->Color = clDkGray;
            for (int i=0; i<=treadles; i++) {
                bmp->Canvas->MoveTo (x0+i*gw, y0);
                bmp->Canvas->LineTo (x0+i*gw, y0+shafts*gh);
            }
            for (int j=0; j<=shafts; j++) {
                bmp->Canvas->MoveTo (x0, y0+j*gh);
                bmp->Canvas->LineTo (x0+treadles*gw+1, y0+j*gh);
            }
            // Schenien
            bmp->Canvas->Pen->Color = clBlack;
            int sx = aufknuepfung.pos.strongline_x;
            int sy = aufknuepfung.pos.strongline_y;
            for (int i=0; i<=treadles; i++) {
                if (i%sx==0) {
                    bmp->Canvas->MoveTo (x0+i*gw, y0);
                    bmp->Canvas->LineTo (x0+i*gw, y0+shafts*gh);
                }
            }
            for (int j=0; j<=shafts; j++) {
                if (j%sy==0) {
                    bmp->Canvas->MoveTo (x0, y0+j*gh);
                    bmp->Canvas->LineTo (x0+treadles*gw+1, y0+j*gh);
                }
            }
            // Daten
            if (ViewSchlagpatrone->Checked || !ViewOnlyGewebe->Checked) {
                DARSTELLUNG darst = aufknuepfung.darstellung;
                if (ViewSchlagpatrone->Checked) darst = AUSGEFUELLT;
                for (int j=0; j<shafts; j++)
                    for (int i=0; i<treadles; i++) {
                        char s = aufknuepfung.feld.Get(i, j);
                        if (s>0) {
                            int x = x0+i*gw;
                            int y = y0+(shafts-j-1)*gh;
                            if (toptobottom) y = y0+j*gh;
                            if (ViewSchlagpatrone->Checked) {
                                PaintCell (bmp->Canvas, darst, x, y, x+gw, y+gh, clDkGray, true);
                            } else if (s==AUSHEBUNG && darst_aushebung!=AUSGEFUELLT) {
                                PaintCell (bmp->Canvas, darst_aushebung, x, y, x+gw, y+gh, clBlack, true);
                            } else if (s==ANBINDUNG && darst_anbindung!=AUSGEFUELLT) {
                                PaintCell (bmp->Canvas, darst_anbindung, x, y, x+gw, y+gh, clBlack, true);
                            } else if (s==ABBINDUNG && darst_abbindung!=AUSGEFUELLT) {
                                PaintCell (bmp->Canvas, darst_abbindung, x, y, x+gw, y+gh, clBlack, true);
                            } else {
                                PaintCell (bmp->Canvas, darst, x, y, x+gw, y+gh, GetRangeColor(s), true);
                            }
                        }
                    }
            }
            // Hilfslinien
            if (hl && !ViewSchlagpatrone->Checked) {
                int count = hlines.GetCount();
                bmp->Canvas->Pen->Color = clBlue;
                for (int i=0; i<count; i++) {
                    Hilfslinie* line = hlines.GetLine(i);
                    if (!line) continue;
                    if (line->typ==HL_HORZ && line->feld==HL_TOP) {
                        if (line->pos>=shafts) continue;
                        int y = y0 + (shafts - line->pos)*gh;
                        if (toptobottom) y = y0 + line->pos*gh;
                        bmp->Canvas->MoveTo (x0, y);
                        bmp->Canvas->LineTo (x0+treadles*gw, y);
                    } else if (line->typ==HL_VERT && line->feld==HL_RIGHT) {
                        if (line->pos>=treadles) continue;
                        int x = x0 + line->pos*gw;
                        bmp->Canvas->MoveTo (x, y0);
                        bmp->Canvas->LineTo (x, y0+shafts*gh);
                    }
                }
            }
            // Rundherum
            bmp->Canvas->Pen->Color = clBlack;
            bmp->Canvas->MoveTo (x0, y0);
            bmp->Canvas->LineTo (x0+treadles*gw, y0);
            bmp->Canvas->LineTo (x0+treadles*gw, y0+shafts*gh);
            bmp->Canvas->LineTo (x0, y0+shafts*gh);
            bmp->Canvas->LineTo (x0, y0);
        }

        // Trittfolge zeichnen
        if (treadles!=0) {
            int x0 = (dx+tdx)*gw;
            int y0 = (clry+shafts+sdy+by)*gh;
            if (einzugunten) y0 = 0;
            // Gitter
            bmp->Canvas->Pen->Color = clDkGray;
            for (int i=0; i<=treadles; i++) {
                bmp->Canvas->MoveTo (x0+i*gw, y0);
                bmp->Canvas->LineTo (x0+i*gw, y0+dy*gh);
            }
            for (int j=0; j<=dy; j++) {
                bmp->Canvas->MoveTo (x0, y0+j*gh);
                bmp->Canvas->LineTo (x0+treadles*gw+1, y0+j*gh);
            }
            // Schenien
            bmp->Canvas->Pen->Color = clBlack;
            int sx = trittfolge.pos.strongline_x;
            int sy = trittfolge.pos.strongline_y;
            for (int i=0; i<=treadles; i++) {
                if (i%sx==0) {
                    bmp->Canvas->MoveTo (x0+i*gw, y0);
                   bmp->Canvas->LineTo (x0+i*gw, y0+dy*gh);
                }
            }
            for (int j=0; j<=dy; j++) {
                if (j%sy==0) {
                    bmp->Canvas->MoveTo (x0, y0+j*gh);
                    bmp->Canvas->LineTo (x0+treadles*gw+1, y0+j*gh);
                }
            }
            // Daten
            if (!ViewOnlyGewebe->Checked) {
                DARSTELLUNG darst = trittfolge.darstellung;
                if (ViewSchlagpatrone->Checked) darst = schlagpatronendarstellung;
                for (int j=0; j<dy; j++)
                    for (int i=0; i<treadles; i++) {
                        char s = trittfolge.feld.Get(i, schuesse.a+j);
                        if (s>0) {
                            int x = x0+i*gw;
                            int y = y0+(dy-j-1)*gh;
                            if (!ViewSchlagpatrone->Checked) {
                                PaintCell (bmp->Canvas, darst, x, y, x+gw, y+gh, clBlack, true);
                            } else if (s==AUSHEBUNG && darst_aushebung!=AUSGEFUELLT) {
                                PaintCell (bmp->Canvas, darst_aushebung, x, y, x+gw, y+gh, clBlack, true);
                            } else if (s==ANBINDUNG && darst_anbindung!=AUSGEFUELLT) {
                                PaintCell (bmp->Canvas, darst_anbindung, x, y, x+gw, y+gh, clBlack, true);
                            } else if (s==ABBINDUNG && darst_abbindung!=AUSGEFUELLT) {
                                PaintCell (bmp->Canvas, darst_abbindung, x, y, x+gw, y+gh, clBlack, true);
                            } else {
                                PaintCell (bmp->Canvas, darst, x, y, x+gw, y+gh, GetRangeColor(s), true);
                            }
                        }
                    }
            }
            // Hilfslinien
            if (hl) {
                int count = hlines.GetCount();
                bmp->Canvas->Pen->Color = clBlue;
                for (int i=0; i<count; i++) {
                    Hilfslinie* line = hlines.GetLine(i);
                    if (!line) continue;
                    if (line->typ==HL_HORZ && line->feld==HL_BOTTOM) {
                        if (line->pos<=schuesse.a || line->pos>schuesse.b) continue;
                        int y = y0 + (dy-line->pos+schuesse.a)*gh;
                        bmp->Canvas->MoveTo (x0, y);
                        bmp->Canvas->LineTo (x0+treadles*gw, y);
                    } else if (line->typ==HL_VERT && line->feld==HL_RIGHT) {
                        if (line->pos>=treadles) continue;
                        int x = x0 + line->pos*gw;
                        bmp->Canvas->MoveTo (x, y0);
                        bmp->Canvas->LineTo (x, y0+dy*gh);
                    }
                }
            }
            // Rapport
            bmp->Canvas->Pen->Color = clRed;
            if (RappViewRapport->Checked) {
                int y = y0 + (rapport.sr.b+1-schuesse.a)*gh;
                bmp->Canvas->MoveTo (x0, y);
                bmp->Canvas->LineTo (x0+treadles*gw, y);
            }
            // Rundherum
            bmp->Canvas->Pen->Color = clBlack;
            bmp->Canvas->MoveTo (x0, y0);
            bmp->Canvas->LineTo (x0+treadles*gw, y0);
            bmp->Canvas->LineTo (x0+treadles*gw, y0+dy*gh);
            bmp->Canvas->LineTo (x0, y0+dy*gh);
            bmp->Canvas->LineTo (x0, y0);
        }

        // Gewebe zeichnen
        int x0 = 0;
        int y0 = (clry+shafts+sdy+by)*gh;
        if (einzugunten) y0 = 0;
        // Gitter
        bmp->Canvas->Pen->Color = clDkGray;
        for (int i=0; i<=dx; i++) {
            bmp->Canvas->MoveTo (x0+i*gw, y0);
            bmp->Canvas->LineTo (x0+i*gw, y0+dy*gh);
        }
        for (int j=0; j<=dy; j++) {
            bmp->Canvas->MoveTo (x0, y0+j*gh);
            bmp->Canvas->LineTo (x0+dx*gw+1, y0+j*gh);
        }
        // Schenien
        bmp->Canvas->Pen->Color = clBlack;
        int sx = gewebe.pos.strongline_x;
        int sy = gewebe.pos.strongline_y;
        for (int i=0; i<=dx; i++) {
            if (i%sx==0) {
                bmp->Canvas->MoveTo (x0+i*gw, y0);
                bmp->Canvas->LineTo (x0+i*gw, y0+dy*gh);
            }
        }
        for (int j=0; j<=dy; j++) {
            if (j%sy==0) {
                bmp->Canvas->MoveTo (x0, y0+j*gh);
                bmp->Canvas->LineTo (x0+dx*gw+1, y0+j*gh);
            }
        }
        // Daten
        if (!GewebeNone->Checked) {
            for (int i=0; i<dx; i++)
                for (int j=0; j<dy; j++) {
                    char s = gewebe.feld.Get(i+kette.a, j+schuesse.a);
                    if (IsEmptyEinzug(i+kette.a))
                        continue;
                    if (IsEmptyTrittfolge(j+schuesse.a))
                        continue;
                    if (RappViewRapport->Checked &&
                        ((IsInRapport (i+kette.a, j+schuesse.a) && !Inverserepeat->Checked) ||
                         (!IsInRapport (i+kette.a, j+schuesse.a) && Inverserepeat->Checked)))
                    {
                        // Rapport
                        if (s!=ABBINDUNG && s>0) {
                            bmp->Canvas->Brush->Color = clRed;
                            bmp->Canvas->Pen->Color = clWhite;
                            bmp->Canvas->Rectangle (x0+i*gw+1, y0+(dy-j-1)*gh+1, x0+(i+1)*gw, y0+(dy-j)*gh);
                        } else {
                            // leer lassen
                        }
                    } else if (GewebeNormal->Checked) {
                        if (s==AUSHEBUNG && darst_aushebung!=AUSGEFUELLT) {
                            PaintCell (bmp->Canvas, darst_aushebung,
                                x0+i*gw, y0+(dy-j-1)*gh, x0+(i+1)*gw, y0+(dy-j)*gh,
                                clBlack, true);
                        } else if (s==ANBINDUNG && darst_anbindung!=AUSGEFUELLT) {
                            bmp->Canvas->Brush->Color = col_anbindung;
                            bmp->Canvas->Pen->Color = clWhite;
                            bmp->Canvas->Rectangle (x0+i*gw+1, y0+(dy-j-1)*gh+1, x0+(i+1)*gw, y0+(dy-j)*gh);
                            PaintCell (bmp->Canvas, darst_anbindung,
                                x0+i*gw, y0+(dy-j-1)*gh, x0+(i+1)*gw, y0+(dy-j)*gh,
                                clBlack, true);
                        } else if (s==ABBINDUNG && darst_abbindung!=AUSGEFUELLT) {
                            bmp->Canvas->Brush->Color = col_abbindung;
                            bmp->Canvas->Pen->Color = clWhite;
                            bmp->Canvas->Rectangle (x0+i*gw+1, y0+(dy-j-1)*gh+1, x0+(i+1)*gw, y0+(dy-j)*gh);
                            PaintCell (bmp->Canvas, darst_abbindung,
                                x0+i*gw, y0+(dy-j-1)*gh, x0+(i+1)*gw, y0+(dy-j)*gh,
                                clBlack, true);
                        } else if (s>0) {
                            bmp->Canvas->Brush->Color = GetRangeColor(s);
                            bmp->Canvas->Pen->Color = clWhite;
                            bmp->Canvas->Rectangle (x0+i*gw+1, y0+(dy-j-1)*gh+1, x0+(i+1)*gw, y0+(dy-j)*gh);
                        }
                    } else if (GewebeFarbeffekt->Checked) {
                        bool drawhebung = s>0 && s!=ABBINDUNG;
                        if (sinkingshed) drawhebung = !drawhebung;
                        if (drawhebung) {
                            bmp->Canvas->Pen->Color = GETPALCOL(kettfarben.feld.Get(i+kette.a));
                            bmp->Canvas->Brush->Color = bmp->Canvas->Pen->Color;
                        } else {
                            bmp->Canvas->Pen->Color = GETPALCOL(schussfarben.feld.Get(j+schuesse.a));
                            bmp->Canvas->Brush->Color = bmp->Canvas->Pen->Color;
                        }
                        if (fewithraster) bmp->Canvas->Rectangle (x0+i*gw+1, y0+(dy-j-1)*gh+1, x0+(i+1)*gw, y0+(dy-j)*gh);
                        else bmp->Canvas->Rectangle (x0+i*gw, y0+(dy-j-1)*gh, x0+(i+1)*gw, y0+(dy-j)*gh);
                    } else if (GewebeSimulation->Checked) {
                        int dw = gw/5;
                        int dh = gh/5;
                        switch (currentzoom) {
                            case 0: dw = dh = 1; break;
                            case 1: dw = dh = 2; break;
                            case 2: dw = dh = 2; break;
                            case 3: dw = dh = 2; break;
                            case 4: dw = dh = 3; break;
                            case 5: dw = dh = 3; break;
                            case 6: dw = dh = 3; break;
                            case 7: dw = dh = 3; break;
                            case 8: dw = dh = 4; break;
                            case 9: dw = dh = 4; break;
                        }
                        if (dw==0 && gw>2) dw = 1;
                        if (dh==0 && gh>2) dh = 1;

                        bool drawhebung = s>0 && s!=ABBINDUNG;
                        if (sinkingshed) drawhebung = !drawhebung;

                        int _x = x0+i*gw;
                        int _y = y0+(dy-j-1)*gh;
                        int _xx = _x + gw;
                        int _yy = _y + gh;

                        bmp->Canvas->Pen->Color = clWhite;
                        bmp->Canvas->Brush->Color = clWhite;
                        bmp->Canvas->Rectangle (_x, _y, _xx, _yy);

                        if (drawhebung) {
                            bmp->Canvas->Pen->Color = GETPALCOL(kettfarben.feld.Get(i+kette.a));
                            bmp->Canvas->Brush->Color = bmp->Canvas->Pen->Color;
                            bmp->Canvas->Rectangle (_x+dw, _y, _xx-dw, _yy);
                            bmp->Canvas->Pen->Color = GETPALCOL(schussfarben.feld.Get(j+schuesse.a));
                            bmp->Canvas->Brush->Color = bmp->Canvas->Pen->Color;
                            bmp->Canvas->Rectangle (_x, _y+dh, _x+dw, _yy-dh);
                            bmp->Canvas->Rectangle (_xx, _y+dh, _xx-dw, _yy-dh);
                            // Schatten
                            bmp->Canvas->Pen->Color = clBlack;
                            bmp->Canvas->MoveTo (_xx-dw, _y+dh);
                            bmp->Canvas->LineTo (_xx-dw, _yy-dh);
                        } else {
                            bmp->Canvas->Pen->Color = GETPALCOL(schussfarben.feld.Get(j+schuesse.a));
                            bmp->Canvas->Brush->Color = bmp->Canvas->Pen->Color;
                            bmp->Canvas->Rectangle (_x, _y+dh, _xx, _yy-dh);
                            bmp->Canvas->Pen->Color = GETPALCOL(kettfarben.feld.Get(i+kette.a));
                            bmp->Canvas->Brush->Color = bmp->Canvas->Pen->Color;
                            bmp->Canvas->Rectangle (_x+dw, _y, _xx-dw, _y+dh);
                            bmp->Canvas->Rectangle (_x+dw, _yy-dh, _xx-dw, _yy);
                            // Schatten
                            bmp->Canvas->Pen->Color = clBlack;
                            bmp->Canvas->MoveTo (_x+dw, _yy-dh);
                            bmp->Canvas->LineTo (_xx-dw, _yy-dh);
                        }
                    }
                }
        }
        // Hilfslinien
        if (hl) {
            int count = hlines.GetCount();
            bmp->Canvas->Pen->Color = clBlue;
            for (int i=0; i<count; i++) {
                Hilfslinie* line = hlines.GetLine(i);
                if (!line) continue;
                if (line->typ==HL_HORZ && line->feld==HL_BOTTOM) {
                    if (line->pos<=schuesse.a || line->pos>schuesse.b) continue;
                    int y = y0 + (dy-line->pos+schuesse.a)*gh;
                    bmp->Canvas->MoveTo (x0, y);
                    bmp->Canvas->LineTo (x0+dx*gw, y);
                } else if (line->typ==HL_VERT && line->feld==HL_LEFT) {
                    if (line->pos<=kette.a || line->pos>kette.b) continue;
                    int x = x0 + (line->pos-kette.a)*gw;
                    if (righttoleft) x = x0 + (dx-line->pos+kette.a)*gw;
                    bmp->Canvas->MoveTo (x, y0);
                    bmp->Canvas->LineTo (x, y0+dy*gh);
                }
            }
        }
        // Rundherum
        bmp->Canvas->Pen->Color = clBlack;
        bmp->Canvas->MoveTo (x0, y0);
        bmp->Canvas->LineTo (x0+dx*gw, y0);
        bmp->Canvas->LineTo (x0+dx*gw, y0+dy*gh);
        bmp->Canvas->LineTo (x0, y0+dy*gh);
        bmp->Canvas->LineTo (x0, y0);

        // Abspeichern...
        bmp->SaveToFile (_filename);
        delete bmp;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/

