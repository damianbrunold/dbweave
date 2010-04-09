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
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "steuerung_form.h"
#include "dbw3_form.h"
#include "assert.h"
#include "datamodule.h"
#include "palette.h"
#include "rangecolors.h"
/*-----------------------------------------------------------------*/
#define GETPALCOL(col) \
    ((TColor)Data->palette->GetColor(col))
/*-----------------------------------------------------------------*/
#define COL_VALID    clBlue
#define COL_LAST     clBlue
#define COL_INVALID  clRed
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::CalcSizes()
{
    gridsize = zoom[currentzoom];

    top = toolbar->Height;
    bottom = ClientHeight - statusbar->Height - 1;
    left = 0;
    right = ClientWidth - scrollbar->Width;

    height = bottom - top;
    dx = gridsize;
    klammerwidth = MAXKLAMMERN*11;
    maxi = (right-left - dx - klammerwidth) / gridsize;
    maxj = (height-1) / gridsize;
    CalcTritte();
    if (tr==0) tr = 8;
    if (maxi<=tr) tr = 0;
    x1 = maxi - tr - 1;
    if (x1<0) x1 = 0;

    int dy = height - maxj*gridsize;
    bottom -= dy;
    height = bottom - top;

    scrollbar->PageSize = maxj;
    scrollbar->SmallChange = 1;
    scrollbar->LargeChange = TScrollBarInc(scrollbar->PageSize / 2);

    if (scrollbar->Position!=Data->MAXY2-maxj-scrolly) {
        scrollbar->Position = Data->MAXY2-maxj-scrolly;
        UpdateScrollbar();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::CalcTritte()
{
    tr = 0;
    if (schlagpatrone) {
        for (signed int i=Data->MAXX2-1; i>=0; i--)
            for (int j=0; j<Data->MAXY2; j++)
                if (trittfolge->feld.Get(i, j)>0) {
                    tr = i+1;
                    return;
                }
    } else {
        int maxj = min(Data->MAXY1-1, Data->MAXX2-1);
        for (signed int j=maxj; j>=0; j--)
            for (int i=0; i<Data->MAXX2; i++)
                if (aufknuepfung->feld.Get(i, j)>0) {
                    tr = j+1;
                    return;
                }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::FormPaint(TObject *Sender)
{
    CalcSizes();
    DrawGrid();
    DrawData();
    DrawKlammern();
    DrawSelection();
    UpdateStatusbar();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::DrawGrid()
{
    Canvas->Pen->Color = clDkGray;

    // Gewebe
    for (int i=0; i<=x1; i++) {
        Canvas->MoveTo (left+i*gridsize, bottom-1);
        Canvas->LineTo (left+i*gridsize, bottom-1-maxj*gridsize);
    }
    for (int j=0; j<=maxj; j++) {
        Canvas->MoveTo (left, bottom - j*gridsize);
        Canvas->LineTo (left+x1*gridsize+1, bottom - j*gridsize);
    }

    // Schlagpatrone
    for (int i=x1+1; i<=x1+1+tr; i++) {
        Canvas->MoveTo (left+i*gridsize, bottom);
        Canvas->LineTo (left+i*gridsize, bottom-maxj*gridsize);
    }
    for (int j=0; j<=maxj; j++) {
        Canvas->MoveTo (left+(x1+1)*gridsize, bottom - j*gridsize);
        Canvas->LineTo (left+(x1+1+tr)*gridsize+1, bottom - j*gridsize);
    }

    // Klammern
    Canvas->Pen->Color = clDkGray;
    for (int i=0; i<=MAXKLAMMERN; i++) {
        Canvas->MoveTo (left+maxi*gridsize+dx+i*11, bottom);
        Canvas->LineTo (left+maxi*gridsize+dx+i*11, bottom-maxj*gridsize);
    }
    Canvas->MoveTo (left+dx+maxi*gridsize, bottom);
    Canvas->LineTo (left+dx+maxi*gridsize+MAXKLAMMERN*11+1, bottom);
    Canvas->MoveTo (left+dx+maxi*gridsize, bottom - maxj*gridsize);
    Canvas->LineTo (left+dx+maxi*gridsize+MAXKLAMMERN*11+1, bottom - maxj*gridsize);

    // Rapport
    Canvas->Pen->Color = clBlack;
    if (rapportx>0) {
        int i = rapportx;
        while (i<x1) {
            Canvas->MoveTo (left+i*gridsize, top+1);
            Canvas->LineTo (left+i*gridsize, bottom);
            i += rapportx;
        }
    }
    if (rapporty>0) {
        int j = rapporty - (scrolly % rapporty);
        while (j<maxj) {
            Canvas->MoveTo (left+1, bottom - j*gridsize);
            Canvas->LineTo (left+x1*gridsize, bottom - j*gridsize);
            j += rapporty;
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::DrawData()
{
    dbw3_assert(einzug);
    dbw3_assert(aufknuepfung);
    dbw3_assert(trittfolge);
    dbw3_assert(gewebe);
    dbw3_assert(kettfarben);
    dbw3_assert(schussfarben);

    int mj = min(maxj, gewebe->feld.SizeY()-scrolly);

    // Gewebe
    Canvas->Brush->Color = clBlack;
    Canvas->Pen->Color = clBlack;
    for (int i=0; i<x1; i++)
        for (int j=scrolly; j<scrolly+mj; j++)
            DrawGewebe (i, j);

    // Schlagpatrone
    Canvas->Brush->Color = clBlack;
    Canvas->Pen->Color = clBlack;
    if (schlagpatrone) {
        for (int i=0; i<tr; i++) {
            for (int j=scrolly; j<scrolly+mj; j++) {
                char s = trittfolge->feld.Get(i, j);
                if (s==AUSHEBUNG) {
                    PaintCell (Canvas,
                        DBWFRM->darst_aushebung,
                        left+x1*gridsize+dx+i*gridsize,
                        bottom-(j-scrolly+1)*gridsize,
                        left+x1*gridsize+dx+(i+1)*gridsize,
                        bottom-(j-scrolly)*gridsize);
                } else if (s==ANBINDUNG) {
                    PaintCell (Canvas,
                        DBWFRM->darst_anbindung,
                        left+x1*gridsize+dx+i*gridsize,
                        bottom-(j-scrolly+1)*gridsize,
                        left+x1*gridsize+dx+(i+1)*gridsize,
                        bottom-(j-scrolly)*gridsize);
                } else if (s==ABBINDUNG) {
                    PaintCell (Canvas,
                        DBWFRM->darst_abbindung,
                        left+x1*gridsize+dx+i*gridsize,
                        bottom-(j-scrolly+1)*gridsize,
                        left+x1*gridsize+dx+(i+1)*gridsize,
                        bottom-(j-scrolly)*gridsize);
                } else if (s>0) {
                    PaintCell (Canvas,
                        schlagpatronendarstellung,
                        left+x1*gridsize+dx+i*gridsize,
                        bottom-(j-scrolly+1)*gridsize,
                        left+x1*gridsize+dx+(i+1)*gridsize,
                        bottom-(j-scrolly)*gridsize);
                }
            }
        }
    } else {
        for (int j=scrolly; j<scrolly+mj; j++) {
            for (int i=0; i</*tr*/trittfolge->feld.SizeX(); i++) {
                if (trittfolge->feld.Get(i, j)>0) {
                    for (int k=0; k<min(tr,aufknuepfung->feld.SizeY()); k++) {
                        char s = aufknuepfung->feld.Get(i, k);
                        if (s==AUSHEBUNG) {
                            PaintCell (Canvas,
                                DBWFRM->darst_aushebung,
                                left+x1*gridsize+dx+k*gridsize,
                                bottom-(j-scrolly+1)*gridsize,
                                left+x1*gridsize+dx+(k+1)*gridsize,
                                bottom-(j-scrolly)*gridsize);
                        } else if (s==ANBINDUNG) {
                            PaintCell (Canvas,
                                DBWFRM->darst_anbindung,
                                left+x1*gridsize+dx+k*gridsize,
                                bottom-(j-scrolly+1)*gridsize,
                                left+x1*gridsize+dx+(k+1)*gridsize,
                                bottom-(j-scrolly)*gridsize);
                        } else if (s==ABBINDUNG) {
                            PaintCell (Canvas,
                                DBWFRM->darst_abbindung,
                                left+x1*gridsize+dx+k*gridsize,
                                bottom-(j-scrolly+1)*gridsize,
                                left+x1*gridsize+dx+(k+1)*gridsize,
                                bottom-(j-scrolly)*gridsize);
                        } else if (s>0) {
                            PaintCell (Canvas,
                                schlagpatronendarstellung,
                                left+x1*gridsize+dx+k*gridsize,
                                bottom-(j-scrolly+1)*gridsize,
                                left+x1*gridsize+dx+(k+1)*gridsize,
                                bottom-(j-scrolly)*gridsize);
                        }
                    }
                }
            }
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::DrawGewebe (int _i, int _j)
{
    dbw3_assert(_i<x1);

    if (_j<scrolly) return;
    if (_j>=scrolly+maxj) return;

    int x  = left + _i*gridsize;
    int xx = x + gridsize;
    int y  = bottom - (_j-scrolly+1)*gridsize;
    int yy = y + gridsize;

    if (einzug->feld.Get(_i)==0 || trittfolge->isempty.Get(_j)==true)
        return;

    if (ViewPatrone->Checked) DrawGewebeNormal (_i, _j, x, y, xx, yy);
    else if (ViewFarbeffekt->Checked) DrawGewebeFarbeffekt (_i, _j, x, y, xx, yy);
    else if (ViewGewebesimulation->Checked) DrawGewebeSimulation (_i, _j, x, y, xx, yy);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::DrawGewebeNormal (int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    char s = gewebe->feld.Get(_i, _j);
    if (s==AUSHEBUNG) {
        Canvas->Brush->Color = clBtnFace;
        Canvas->Pen->Color = clBtnFace;
        Canvas->Rectangle (_x+1, _y+1, _xx, _yy);
        PaintCell (Canvas, DBWFRM->darst_aushebung, _x, _y, _xx, _yy);
    } else if (s==ANBINDUNG) {
        Canvas->Brush->Color = clBtnFace;
        Canvas->Pen->Color = clBtnFace;
        Canvas->Rectangle (_x+1, _y+1, _xx, _yy);
        PaintCell (Canvas, DBWFRM->darst_anbindung, _x, _y, _xx, _yy);
    } else if (s==ABBINDUNG) {
        Canvas->Brush->Color = clBtnFace;
        Canvas->Pen->Color = clBtnFace;
        Canvas->Rectangle (_x+1, _y+1, _xx, _yy);
        PaintCell (Canvas, DBWFRM->darst_abbindung, _x, _y, _xx, _yy);
    } else if (s>0) {
        Canvas->Brush->Color = GetRangeColor(s);
        Canvas->Pen->Color = clBtnFace;
        Canvas->Rectangle (_x+1, _y+1, _xx, _yy);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::DrawGewebeFarbeffekt (int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    char s = gewebe->feld.Get(_i,_j);
    bool hebung = s>0 && s!=ABBINDUNG;
    if (DBWFRM->sinkingshed) hebung = !hebung;
    if (hebung) {
        Canvas->Brush->Color = GETPALCOL(kettfarben->feld.Get(_i));
        Canvas->Pen->Color = GETPALCOL(kettfarben->feld.Get(_i));
    } else {
        Canvas->Brush->Color = GETPALCOL(schussfarben->feld.Get(_j));
        Canvas->Pen->Color = GETPALCOL(schussfarben->feld.Get(_j));
    }
    Canvas->Rectangle (_x, _y, _xx, _yy);
    if (fewithraster) {
        Canvas->Pen->Color = clDkGray;
        Canvas->MoveTo (_x, _y);
        Canvas->LineTo (_x, _yy);
        Canvas->LineTo (_xx, _yy);
        Canvas->LineTo (_xx, _y);
        Canvas->LineTo (_x, _y);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::DrawGewebeSimulation (int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    int dw = gridsize/5;
    int dh = gridsize/5;
    if (dw==0 && gridsize>2) dw = 1;
    if (dh==0 && gridsize>2) dh = 1;

    Canvas->Brush->Color = clBtnFace;
    Canvas->Pen->Color = clBtnFace;
    Canvas->Rectangle (_x, _y, _xx, _yy);

    char s = gewebe->feld.Get(_i,_j);
    bool hebung = s>0 && s!=ABBINDUNG;
    if (DBWFRM->sinkingshed) hebung = !hebung;
    if (hebung) {
        Canvas->Brush->Color = GETPALCOL(kettfarben->feld.Get(_i));
        Canvas->Pen->Color = GETPALCOL(kettfarben->feld.Get(_i));
        Canvas->Rectangle (_x+dw, _y, _xx-dw, _yy);
        Canvas->Brush->Color = GETPALCOL(schussfarben->feld.Get(_j));
        Canvas->Pen->Color = GETPALCOL(schussfarben->feld.Get(_j));
        Canvas->Rectangle (_x, _y+dh, _x+dw, _yy-dh);
        Canvas->Rectangle (_xx, _y+dh, _xx-dw, _yy-dh);
        // Schatten
        Canvas->Pen->Color = clBlack;
        Canvas->MoveTo (_xx-dw, _y+dh);
        Canvas->LineTo (_xx-dw, _yy-dh);
    } else {
        Canvas->Brush->Color = GETPALCOL(schussfarben->feld.Get(_j));
        Canvas->Pen->Color = GETPALCOL(schussfarben->feld.Get(_j));
        Canvas->Rectangle (_x, _y+dh, _xx, _yy-dh);
        Canvas->Brush->Color = GETPALCOL(kettfarben->feld.Get(_i));
        Canvas->Pen->Color = GETPALCOL(kettfarben->feld.Get(_i));
        Canvas->Rectangle (_x+dw, _y, _xx-dw, _y+dh);
        Canvas->Rectangle (_x+dw, _yy-dh, _xx-dw, _yy);
        // Schatten
        Canvas->Pen->Color = clBlack;
        Canvas->MoveTo (_x+dw, _yy-dh);
        Canvas->LineTo (_xx-dw, _yy-dh);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::DrawKlammern()
{
    for (int i=0; i<MAXKLAMMERN; i++)
        DrawKlammer(i);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::DrawKlammer (int i)
{
    TRect rc;
    rc.Top = bottom-maxj*gridsize+1;
    rc.Bottom = bottom;
    rc.Left = left+(x1+tr+1)*gridsize+dx+i*11+1;
    rc.Right = left+(x1+tr+1)*gridsize+dx+(i+1)*11;
    Canvas->Brush->Color = Color;
    Canvas->FillRect (rc);

    if (!schussselected && current_klammer==i) Canvas->Pen->Color = clRed;
    else Canvas->Pen->Color = clBlack;
    _DrawKlammer (i);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::_DrawKlammer(int i)
{
    bool clip_top = false;
    bool clip_bottom = false;

    int j1 = klammern[i].first - scrolly;
    int j2 = klammern[i].last  - scrolly;

    if (j1<0) {
        clip_bottom = true;
        j1 = 0;
    } else if (j1>maxj) {
        clip_bottom = true;
        j1 = maxj;
    }

    if (j2<0) {
        clip_top = true;
        j2 = 0;
    } else if (j2>maxj) {
        clip_top = true;
        j2 = maxj;
    }

    // Senkrechter Strich
    if (j1<maxj && j2>0) {
        Canvas->MoveTo (left+(x1+tr+1)*gridsize+dx+i*11+8,
                        bottom-j1*gridsize-1);
        Canvas->LineTo (left+(x1+tr+1)*gridsize+dx+i*11+8,
                        bottom-(j2+1)*gridsize);
    }

    // Unterer waagrechter Strich
    if (!clip_bottom) {
        Canvas->MoveTo (left+(x1+tr+1)*gridsize+dx+i*11+8,
                        bottom-j1*gridsize-1);
        Canvas->LineTo (left+(x1+tr+1)*gridsize+dx+i*11+2,
                        bottom-j1*gridsize-1);
    }

    // Oberer waagrechter Strich
    if (!clip_top) {
        Canvas->MoveTo (left+(x1+tr+1)*gridsize+dx+i*11+8,
                        bottom-(j2+1)*gridsize+1);
        Canvas->LineTo (left+(x1+tr+1)*gridsize+dx+i*11+2,
                        bottom-(j2+1)*gridsize+1);
    }

    // Anzahl Repetitionen
    if (j1<maxj && j2>0) {
        char rep[3];
        itoa (klammern[i].repetitions, rep, 10);
        TRect rc;
        rc.Top = bottom-(j2+1)*gridsize;
        rc.Bottom = bottom-j1*gridsize;
        rc.Left = left+(x1+tr+1)*gridsize+dx+i*11+1;
        rc.Right = left+(x1+tr+1)*gridsize+dx+(i+1)*11-1;
        int dy = Canvas->TextHeight(rep);
        TBrushStyle save = Canvas->Brush->Style;
        Canvas->Brush->Style = bsClear;
        Canvas->TextRect (rc, rc.Left, (rc.Top+rc.Bottom-dy)/2, rep);
        Canvas->Brush->Style = save;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::_DrawSelection()
{
    if (schussselected) {
        _DrawPositionSelected();
    } else _DrawKlammerSelected();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::DrawSelection()
{
    Canvas->Pen->Color = clRed;
    Canvas->Brush->Style = bsClear;
    if (schussselected) {
        if (IsValidWeavePosition()) Canvas->Pen->Color = COL_VALID;
        else Canvas->Pen->Color = COL_INVALID;
    }
    _DrawSelection();
    if (!schussselected) {
        Canvas->Pen->Color = clDkGray;
        _DrawPositionSelected();
    } else {
        if (weave_klammer>=0) _DrawKlammer(weave_klammer);
    }
    Canvas->Brush->Style = bsSolid;
    Canvas->Pen->Color = clDkGray;
    DrawLastPos();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::ClearSelection()
{
    if (schussselected) Canvas->Pen->Color = clDkGray;
    else Canvas->Pen->Color = clBlack;
    Canvas->Brush->Style = bsClear;
    if (!schussselected) _DrawPositionSelected();
    _DrawSelection();
    if (schussselected) {
        Canvas->Pen->Color = clBlack;
        if (weave_klammer>=0) _DrawKlammer(weave_klammer);
    }
    Canvas->Brush->Style = bsSolid;
    ClearLastPos();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::DrawPositionSelection()
{
    if (IsValidWeavePosition()) Canvas->Pen->Color = COL_VALID;
    else Canvas->Pen->Color = COL_INVALID;
    Canvas->Brush->Style = bsClear;
    _DrawPositionSelected();
    if (weave_klammer>=0) _DrawKlammer(weave_klammer);
    Canvas->Brush->Style = bsSolid;
    Canvas->Pen->Color = clDkGray;
    DrawLastPos();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::ClearPositionSelection()
{
    Canvas->Pen->Color = clDkGray;
    Canvas->Brush->Style = bsClear;
    _DrawPositionSelected();
    Canvas->Pen->Color = clBlack;
    if (weave_klammer>=0) _DrawKlammer(weave_klammer);
    Canvas->Brush->Style = bsSolid;
    ClearLastPos();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::DrawLastPos()
{
    Canvas->Pen->Color = COL_LAST;
    _DrawLastPos();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::ClearLastPos()
{
    Canvas->Pen->Color = clBtnFace;
    _DrawLastPos();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::_DrawLastPos()
{
    // Letzter Schuss (zwischen Gewebe und Schlagpatrone)
    int xx1 = left + (x1+1)*gridsize - 2;
    int xx2 = left + (x1+1)*gridsize - dx + 2;
    int yy1 = bottom - (last_position-scrolly+1)*gridsize;
    int yy2 = bottom - (last_position-scrolly)*gridsize + 1;
    Canvas->MoveTo (xx1, (yy1+yy2)/2);
    Canvas->LineTo (xx2, yy1);
    Canvas->LineTo (xx2, yy2);
    Canvas->LineTo (xx1, (yy1+yy2)/2);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::DrawKlammerSelection()
{
    Canvas->Pen->Color = clRed;
    Canvas->Brush->Style = bsClear;
    _DrawKlammerSelected();
    Canvas->Brush->Style = bsSolid;
    Canvas->Pen->Color = clDkGray;
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::ClearKlammerSelection()
{
    Canvas->Pen->Color = Color;
    Canvas->Brush->Style = bsClear;
    TRect rc;
    rc.Top = bottom-maxj*gridsize+1;
    rc.Bottom = bottom;
    rc.Left = left+(x1+tr+1)*gridsize+dx+current_klammer*11+1;
    rc.Right = left+(x1+tr+1)*gridsize+dx+(current_klammer+1)*11;
    Canvas->Brush->Color = Color;
    Canvas->FillRect (rc);
    Canvas->Brush->Style = bsSolid;
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::_DrawPositionSelected()
{
    // Aktueller Tritt (=Schlagpatronenzeile)
    Canvas->Rectangle (left+(x1+1)*gridsize,
                       bottom-(weave_position-scrolly+1)*gridsize,
                       left+(x1+1+tr)*gridsize+1,
                       bottom-(weave_position-scrolly)*gridsize+1);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::_DrawKlammerSelected()
{
    // Akutelle Klammer
    _DrawKlammer (current_klammer);
}
/*-----------------------------------------------------------------*/
