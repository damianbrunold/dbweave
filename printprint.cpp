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
#include "printcancel_form.h"
#include "hilfslinien.h"
#include "rangecolors.h"
/*-----------------------------------------------------------------*/
#define CHECKCANCEL \
    Application->ProcessMessages(); \
    if (canceldlg && !cancelfrm->Visible) return;
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
void __fastcall PrPrinter::PrintHilfslinien()
{
    if (!mainfrm->ViewHlines->Checked) return;

    // Hilfslinien drucken
    int count = mainfrm->hlines.GetCount();
    for (int i=0; i<count; i++) {
        Hilfslinie* line = mainfrm->hlines.GetLine(i);
        dbw3_assert(line);
        if (line) PrintHilfslinie (line);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintHilfslinie (Hilfslinie* _hline)
{
    switch (_hline->typ) {
        case HL_HORZ:
            if (_hline->feld==HL_TOP) {
                if (!einzug.visible)
                    break;
                if (_hline->pos<=schaefte.a || _hline->pos>=schaefte.b)
                    break;
                canvas->Pen->Color = blackandwhite ? clBlack : HLINECOLOR;
                int y;
                if (mainfrm->toptobottom) y = einzug.y0+(_hline->pos-schaefte.a)*einzug.gh;
                else y = einzug.y0+einzug.height-(_hline->pos-schaefte.a)*einzug.gh;
                SETSTRONGWIDTH
                canvas->MoveTo (XSCALE(einzug.x0+pw), YSCALE(y));
                canvas->LineTo (XSCALE(einzug.x0+einzug.width-pw), YSCALE(y));
                if (trittfolge.visible) {
                    canvas->MoveTo (XSCALE(aufknuepfung.x0+pw), YSCALE(y));
                    canvas->LineTo (XSCALE(aufknuepfung.x0+aufknuepfung.width-pw), YSCALE(y));
                }
                RESETSTRONGWIDTH
            } else {
                if (_hline->pos<=schuesse.a || _hline->pos>=schuesse.b)
                    break;
                canvas->Pen->Color = blackandwhite ? clBlack : HLINECOLOR;
                int y = gewebe.y0+gewebe.height-(_hline->pos-schuesse.a)*gewebe.gh;
                SETSTRONGWIDTH
                if (mainfrm->GewebeNormal->Checked || mainfrm->GewebeNone->Checked) {
                    canvas->MoveTo (XSCALE(gewebe.x0+pw), YSCALE(y));
                    canvas->LineTo (XSCALE(gewebe.x0+gewebe.width-pw), YSCALE(y));
                }
                if (trittfolge.visible) {
                    canvas->MoveTo (XSCALE(trittfolge.x0+pw), YSCALE(y));
                    canvas->LineTo (XSCALE(trittfolge.x0+trittfolge.width-pw), YSCALE(y));
                }
                RESETSTRONGWIDTH
            }
            break;
        case HL_VERT:
            if (_hline->feld==HL_LEFT) {
                bool rtl = mainfrm->righttoleft;
                if (_hline->pos<=kette.a || _hline->pos>=kette.b)
                    break;
                canvas->Pen->Color = blackandwhite ? clBlack : HLINECOLOR;
                int x;
                if (!rtl) x = gewebe.x0+(_hline->pos-kette.a)*gewebe.gw;
                else x = gewebe.x0+gewebe.width - (_hline->pos-kette.a)*gewebe.gw;
                SETSTRONGWIDTH
                if (mainfrm->GewebeNormal->Checked || mainfrm->GewebeNone->Checked) {
                    canvas->MoveTo (XSCALE(x), YSCALE(gewebe.y0+pw));
                    canvas->LineTo (XSCALE(x), YSCALE(gewebe.y0+gewebe.height-pw));
                }
                if (einzug.visible) {
                    canvas->MoveTo (XSCALE(x), YSCALE(einzug.y0+pw));
                    canvas->LineTo (XSCALE(x), YSCALE(einzug.y0+einzug.height-pw));
                }
                RESETSTRONGWIDTH
            } else {
                if (!trittfolge.visible)
                    break;
                if (_hline->pos<=tritte.a || _hline->pos>=tritte.b)
                    break;
                canvas->Pen->Color = blackandwhite ? clBlack : HLINECOLOR;
                int x = trittfolge.x0+(_hline->pos-tritte.a)*trittfolge.gw;
                SETSTRONGWIDTH
                canvas->MoveTo (XSCALE(x), YSCALE(trittfolge.y0+pw));
                canvas->LineTo (XSCALE(x), YSCALE(trittfolge.y0+trittfolge.height-pw));
                if (einzug.visible) {
                    canvas->MoveTo (XSCALE(x), YSCALE(aufknuepfung.y0+pw));
                    canvas->LineTo (XSCALE(x), YSCALE(aufknuepfung.y0+aufknuepfung.height-pw));
                }
                RESETSTRONGWIDTH
            }
            break;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintKettfarben()
{
    if (!kettfarben.visible) return;

    int x0 = kettfarben.x0;
    int y0 = kettfarben.y0;
    bool rtl = mainfrm->righttoleft;
    for (int i=kette.a; i<=kette.b; i++) {
        canvas->Pen->Color = clBlack;
        canvas->Brush->Color = (TColor)Data->palette->GetColor(mainfrm->kettfarben.feld.Get(i));
        int x;
        if (!rtl) x = x0+(i-kette.a)*kettfarben.gw;
        else x = x0 + (kette.b-i)*kettfarben.gw;
        canvas->Rectangle (XSCALE(x), YSCALE(y0), XSCALE(x+kettfarben.gw+1), YSCALE(y0+kettfarben.height+1));
        CHECKCANCEL
    }
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintEinzug()
{
    if (!einzug.visible) return;

    int x0 = einzug.x0;
    int y0 = einzug.y0 + einzug.height;
    bool rtl = mainfrm->righttoleft;
    bool ttb = mainfrm->toptobottom;
    for (int i=0; i<kette.count(); i++) {
        for (int j=0; j<schaefte.count(); j++) {
            int ii;
            if (!rtl) ii = i;
            else ii = kette.count()-1-i;
            int jj;
            if (!ttb) jj = j;
            else jj = schaefte.count()-1-j;
            // Rahmen
            canvas->Pen->Color = clBlack;
            canvas->MoveTo (XSCALE(x0+ii*einzug.gw), YSCALE(y0-jj*einzug.gh));
            canvas->LineTo (XSCALE(x0+(ii+1)*einzug.gw), YSCALE(y0-jj*einzug.gh));
            canvas->LineTo (XSCALE(x0+(ii+1)*einzug.gw), YSCALE(y0-(jj+1)*einzug.gh));
            canvas->LineTo (XSCALE(x0+ii*einzug.gw), YSCALE(y0-(jj+1)*einzug.gh));
            canvas->LineTo (XSCALE(x0+ii*einzug.gw), YSCALE(y0-jj*einzug.gh));
            if (!mainfrm->ViewOnlyGewebe->Checked) {
                // Inhalt
                if (mainfrm->einzug.feld.Get (i+kette.a)==j+schaefte.a+1) {
                    PrPaintCell (mainfrm->einzug.darstellung,
                                x0+ii*einzug.gw, y0-(jj+1)*einzug.gh,
                                x0+(ii+1)*einzug.gw, y0-jj*einzug.gh,
								clBlack,
                                j+schaefte.a);
                }
            }
        }
        CHECKCANCEL
    }
    // 'Strongline'
    canvas->Pen->Color = clBlack;
    SETSTRONGWIDTH
    for (int i=0; i<kette.count(); i++) {
        int ii;
        if (!rtl) ii = i;
        else ii = kette.count()-1-i;
        if (mainfrm->righttoleft) {
            if (i!=0 && mainfrm->einzug.pos.strongline_x!=0 && ((i+kette.a)%mainfrm->einzug.pos.strongline_x)==0) {
                canvas->MoveTo (XSCALE(x0+(ii+1)*einzug.gw), YSCALE(einzug.y0+pw));
                canvas->LineTo (XSCALE(x0+(ii+1)*einzug.gw), YSCALE(einzug.y0+einzug.height-pw));
            }
        } else {
            if (i!=0 && mainfrm->einzug.pos.strongline_x!=0 && ((i+kette.a)%mainfrm->einzug.pos.strongline_x)==0) {
                canvas->MoveTo (XSCALE(x0+ii*einzug.gw), YSCALE(einzug.y0+pw));
                canvas->LineTo (XSCALE(x0+ii*einzug.gw), YSCALE(einzug.y0+einzug.height-pw));
            }
        }
    }
    for (int j=0; j<schaefte.count(); j++) {
        int jj;
        if (!ttb) jj = j;
        else jj = schaefte.count()-1-j;
        if (mainfrm->toptobottom) {
            if (j!=0 && mainfrm->einzug.pos.strongline_y!=0 && ((j+schaefte.a)%mainfrm->einzug.pos.strongline_y)==0) {
                canvas->MoveTo (XSCALE(einzug.x0+pw), YSCALE(y0-(jj+1)*einzug.gh));
                canvas->LineTo (XSCALE(einzug.x0+einzug.width-pw), YSCALE(y0-(jj+1)*einzug.gh));
            }
        } else {
            if (j!=0 && mainfrm->einzug.pos.strongline_y!=0 && ((j+schaefte.a)%mainfrm->einzug.pos.strongline_y)==0) {
                canvas->MoveTo (XSCALE(einzug.x0+pw), YSCALE(y0-jj*einzug.gh));
                canvas->LineTo (XSCALE(einzug.x0+einzug.width-pw), YSCALE(y0-jj*einzug.gh));
            }
        }
    }
    RESETSTRONGWIDTH
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintBlatteinzug()
{
    if (!blatteinzug.visible) return;

    int x0 = blatteinzug.x0;
    int y0 = blatteinzug.y0;
    bool rtl = mainfrm->righttoleft;
    for (int i=kette.a; i<=kette.b; i++) {
        canvas->Pen->Color = clBlack;
        canvas->Brush->Color = mainfrm->blatteinzug.feld.Get(i) ? clBlack : clWhite;
        int x;
        if (!rtl) x = x0+(i-kette.a)*blatteinzug.gw;
        else x = x0 + (kette.b-i)*blatteinzug.gw;
        canvas->Rectangle (XSCALE(x), YSCALE(y0), XSCALE(x+blatteinzug.gw+1), YSCALE(y0+blatteinzug.height/2+1));
        canvas->Brush->Color = mainfrm->blatteinzug.feld.Get(i) ? clWhite : clBlack;
        canvas->Rectangle (XSCALE(x), YSCALE(y0+blatteinzug.height/2-1), XSCALE(x+blatteinzug.gw+1), YSCALE(y0+blatteinzug.height+1));
        CHECKCANCEL
    }
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintGewebe()
{
    if (!gewebe.visible) return;

    int x0 = gewebe.x0;
    int y0 = gewebe.y0 + gewebe.height;
    canvas->Pen->Color = clBlack;
    bool rtl = mainfrm->righttoleft;
    for (int i=0; i<kette.count(); i++) {
        for (int j=0; j<schuesse.count(); j++) {
            int ii;
            if (!rtl) ii = i;
            else ii = kette.count()-1-i;
            // Rahmen
            if (mainfrm->IsEmptyTrittfolge (j+schuesse.a) || mainfrm->IsEmptyEinzug (i+kette.a) || mainfrm->GewebeNormal->Checked) {
                canvas->Pen->Color = clBlack;
                canvas->MoveTo (XSCALE(x0+ii*gewebe.gw), YSCALE(y0-j*gewebe.gh));
                canvas->LineTo (XSCALE(x0+(ii+1)*gewebe.gw), YSCALE(y0-j*gewebe.gh));
                canvas->LineTo (XSCALE(x0+(ii+1)*gewebe.gw), YSCALE(y0-(j+1)*gewebe.gh));
                canvas->LineTo (XSCALE(x0+ii*gewebe.gw), YSCALE(y0-(j+1)*gewebe.gh));
                canvas->LineTo (XSCALE(x0+ii*gewebe.gw), YSCALE(y0-j*gewebe.gh));
            }
            // Inhalt
            if (!mainfrm->IsEmptyEinzug(i+kette.a) && !mainfrm->IsEmptyTrittfolge(j+schuesse.a)) {
                if (!mainfrm->Inverserepeat->Checked) {
                    if (!mainfrm->GewebeNone->Checked && mainfrm->RappViewRapport->Checked && mainfrm->IsInRapport (i+kette.a, j+schuesse.a))
                        PrintGewebeRapport (i, j, x0+ii*gewebe.gw, y0-(j+1)*gewebe.gh, x0+(ii+1)*gewebe.gw, y0-j*gewebe.gh);
                    else if (mainfrm->GewebeNormal->Checked)
                        PrintGewebeNormal (i, j, x0+ii*gewebe.gw, y0-(j+1)*gewebe.gh, x0+(ii+1)*gewebe.gw, y0-j*gewebe.gh);
                    else if (mainfrm->GewebeFarbeffekt->Checked)
                        PrintGewebeFarbeffekt (i, j, x0+ii*gewebe.gw, y0-(j+1)*gewebe.gh, x0+(ii+1)*gewebe.gw, y0-j*gewebe.gh);
                    else if (mainfrm->GewebeSimulation->Checked)
                        PrintGewebeSimulation (i, j, x0+ii*gewebe.gw, y0-(j+1)*gewebe.gh, x0+(ii+1)*gewebe.gw, y0-j*gewebe.gh+1);
                    else if (mainfrm->GewebeNone->Checked)
                        PrintGewebeNone (i, j, x0+ii*gewebe.gw, y0-(j+1)*gewebe.gh, x0+(ii+1)*gewebe.gw, y0-j*gewebe.gh);
                    else
                        dbw3_assert(false);
                } else {
                    if (!mainfrm->GewebeNone->Checked && mainfrm->RappViewRapport->Checked && !mainfrm->IsInRapport (i+kette.a, j+schuesse.a))
                        PrintGewebeRapport (i, j, x0+ii*gewebe.gw, y0-(j+1)*gewebe.gh, x0+(ii+1)*gewebe.gw, y0-j*gewebe.gh);
                    else if (mainfrm->GewebeNormal->Checked)
                        PrintGewebeNormal (i, j, x0+ii*gewebe.gw, y0-(j+1)*gewebe.gh, x0+(ii+1)*gewebe.gw, y0-j*gewebe.gh);
                    else if (mainfrm->GewebeFarbeffekt->Checked)
                        PrintGewebeFarbeffekt (i, j, x0+ii*gewebe.gw, y0-(j+1)*gewebe.gh, x0+(ii+1)*gewebe.gw, y0-j*gewebe.gh);
                    else if (mainfrm->GewebeSimulation->Checked)
                        PrintGewebeSimulation (i, j, x0+ii*gewebe.gw, y0-(j+1)*gewebe.gh, x0+(ii+1)*gewebe.gw, y0-j*gewebe.gh+1);
                    else if (mainfrm->GewebeNone->Checked)
                        PrintGewebeNone (i, j, x0+ii*gewebe.gw, y0-(j+1)*gewebe.gh, x0+(ii+1)*gewebe.gw, y0-j*gewebe.gh);
                    else
                        dbw3_assert(false);
                }
            }
        }
        CHECKCANCEL
    }

    // 'Strongline'
    if (mainfrm->GewebeNone->Checked || mainfrm->GewebeNormal->Checked) {
        SETSTRONGWIDTH
        for (int i=0; i<kette.count(); i++) {
            int ii;
            if (!rtl) ii = i;
            else ii = kette.count()-1-i;
            if (mainfrm->righttoleft) {
                if (i!=0 && mainfrm->gewebe.pos.strongline_x!=0 && ((i+kette.a)%mainfrm->gewebe.pos.strongline_x)==0) {
                    canvas->MoveTo (XSCALE(x0+(ii+1)*gewebe.gw), YSCALE(gewebe.y0+pw));
                    canvas->LineTo (XSCALE(x0+(ii+1)*gewebe.gw), YSCALE(gewebe.y0+gewebe.height-pw));
                }
            } else {
                if (i!=0 && mainfrm->gewebe.pos.strongline_x!=0 && ((i+kette.a)%mainfrm->gewebe.pos.strongline_x)==0) {
                    canvas->MoveTo (XSCALE(x0+ii*gewebe.gw), YSCALE(gewebe.y0+pw));
                    canvas->LineTo (XSCALE(x0+ii*gewebe.gw), YSCALE(gewebe.y0+gewebe.height-pw));
                }
            }
        }
        for (int j=0; j<schuesse.count(); j++) {
            if (j!=0 && mainfrm->gewebe.pos.strongline_y!=0 && ((j+schuesse.a)%mainfrm->gewebe.pos.strongline_y)==0) {
                canvas->MoveTo (XSCALE(gewebe.x0+pw), YSCALE(y0-j*gewebe.gh));
                canvas->LineTo (XSCALE(gewebe.x0+gewebe.width-pw), YSCALE(y0-j*gewebe.gh));
            }
        }
        RESETSTRONGWIDTH
    }

    // Rahmen rundherum
    canvas->Pen->Color = clBlack;
    canvas->MoveTo (XSCALE(gewebe.x0), YSCALE(gewebe.y0));
    canvas->LineTo (XSCALE(gewebe.x0+kette.count()*gewebe.gw),
                    YSCALE(gewebe.y0));
    canvas->LineTo (XSCALE(gewebe.x0+kette.count()*gewebe.gw),
                    YSCALE(gewebe.y0+schuesse.count()*gewebe.gh));
    canvas->LineTo (XSCALE(gewebe.x0),
                    YSCALE(gewebe.y0+schuesse.count()*gewebe.gh));
    canvas->LineTo (XSCALE(gewebe.x0), YSCALE(gewebe.y0));
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintAufknuepfung()
{
    if (!aufknuepfung.visible) return;

    int x0 = aufknuepfung.x0;
    int y0 = aufknuepfung.y0 + aufknuepfung.height;
    if (!mainfrm->ViewSchlagpatrone->Checked ||
        !mainfrm->aufknuepfung.pegplanstyle ||
        mainfrm->einzugunten)
    {
        // 'Normale' Darstellung mit Diagonalen gefüllten Feldern
        for (int i=0; i<tritte.count(); i++) {
            for (int j=0; j<schaefte.count(); j++) {
                int jj;
                if (!mainfrm->toptobottom) jj = j;
                else jj = schaefte.count()-1-j;
                // Rahmen
                canvas->Pen->Color = clBlack;
                canvas->MoveTo (XSCALE(x0+i*aufknuepfung.gw), YSCALE(y0-jj*aufknuepfung.gh));
                canvas->LineTo (XSCALE(x0+(i+1)*aufknuepfung.gw), YSCALE(y0-jj*aufknuepfung.gh));
                canvas->LineTo (XSCALE(x0+(i+1)*aufknuepfung.gw), YSCALE(y0-(jj+1)*aufknuepfung.gh));
                canvas->LineTo (XSCALE(x0+i*aufknuepfung.gw), YSCALE(y0-(jj+1)*aufknuepfung.gh));
                canvas->LineTo (XSCALE(x0+i*aufknuepfung.gw), YSCALE(y0-jj*aufknuepfung.gw));
                if (!mainfrm->ViewOnlyGewebe->Checked) {
                    // Inhalt
                    char s = mainfrm->aufknuepfung.feld.Get (i+tritte.a, j+schaefte.a);
                    if (s==AUSHEBUNG) {
                        PrPaintCell (mainfrm->darst_aushebung,
                                    x0+i*aufknuepfung.gw, y0-(jj+1)*aufknuepfung.gh,
                                    x0+(i+1)*aufknuepfung.gw, y0-jj*aufknuepfung.gh);
                    } else if (s==ANBINDUNG) {
                        PrPaintCell (mainfrm->darst_anbindung,
                                    x0+i*aufknuepfung.gw, y0-(jj+1)*aufknuepfung.gh,
                                    x0+(i+1)*aufknuepfung.gw, y0-jj*aufknuepfung.gh);
                    } else if (s==ABBINDUNG) {
                        PrPaintCell (mainfrm->darst_abbindung,
                                    x0+i*aufknuepfung.gw, y0-(jj+1)*aufknuepfung.gh,
                                    x0+(i+1)*aufknuepfung.gw, y0-jj*aufknuepfung.gh);
                    } else if (s>0) {
                        if (mainfrm->ViewSchlagpatrone->Checked) {
                            PrPaintCell (mainfrm->aufknuepfung.darstellung,
                                        x0+i*aufknuepfung.gw, y0-(jj+1)*aufknuepfung.gh,
                                        x0+(i+1)*aufknuepfung.gw, y0-jj*aufknuepfung.gh);
                        } else {
                            PrPaintCell (mainfrm->aufknuepfung.darstellung,
                                        x0+i*aufknuepfung.gw, y0-(jj+1)*aufknuepfung.gh,
                                        x0+(i+1)*aufknuepfung.gw, y0-jj*aufknuepfung.gh,
										GetRangeColor(s),
                                        j+schaefte.a);
                        }
                    }
                }
            }
            CHECKCANCEL
        }
        // 'Strongline'
        canvas->Pen->Color = clBlack;
        SETSTRONGWIDTH
        for (int i=0; i<tritte.count(); i++) {
            if (i!=0 && mainfrm->aufknuepfung.pos.strongline_x!=0 && ((i+tritte.a)%mainfrm->aufknuepfung.pos.strongline_x)==0) {
                canvas->MoveTo (XSCALE(x0+i*aufknuepfung.gw), YSCALE(aufknuepfung.y0+pw));
                canvas->LineTo (XSCALE(x0+i*aufknuepfung.gw), YSCALE(aufknuepfung.y0+aufknuepfung.height-pw));
            }
        }
        for (int j=0; j<schaefte.count(); j++) {
            int jj;
            if (!mainfrm->toptobottom) jj = j;
            else jj = schaefte.count()-1-j;
            if (mainfrm->toptobottom) {
                if (j!=0 && mainfrm->aufknuepfung.pos.strongline_y!=0 && ((j+schaefte.a)%mainfrm->aufknuepfung.pos.strongline_y)==0) {
                    canvas->MoveTo (XSCALE(aufknuepfung.x0+pw), YSCALE(y0-(jj+1)*aufknuepfung.gh));
                    canvas->LineTo (XSCALE(aufknuepfung.x0+aufknuepfung.width-pw), YSCALE(y0-(jj+1)*aufknuepfung.gh));
                }
            } else {
                if (j!=0 && mainfrm->aufknuepfung.pos.strongline_y!=0 && ((j+schaefte.a)%mainfrm->aufknuepfung.pos.strongline_y)==0) {
                    canvas->MoveTo (XSCALE(aufknuepfung.x0+pw), YSCALE(y0-jj*aufknuepfung.gh));
                    canvas->LineTo (XSCALE(aufknuepfung.x0+aufknuepfung.width-pw), YSCALE(y0-jj*aufknuepfung.gh));
                }
            }
        }
        RESETSTRONGWIDTH
    } else {
        // Spezielle Darstellung mit gewinkelten Strichen
        canvas->Pen->Color = clBlack;
        canvas->MoveTo (XSCALE(x0), YSCALE(y0));
        canvas->LineTo (XSCALE(x0+aufknuepfung.width), YSCALE(y0));
        canvas->LineTo (XSCALE(x0+aufknuepfung.width), YSCALE(aufknuepfung.y0));
        canvas->LineTo (XSCALE(x0), YSCALE(aufknuepfung.y0));
        canvas->LineTo (XSCALE(x0), YSCALE(y0));
        for (int i=0; i<tritte.count(); i++) {
            canvas->MoveTo (XSCALE(x0+i*aufknuepfung.gw+aufknuepfung.gw/2), YSCALE(y0));
            int y = y0-i*aufknuepfung.gh-aufknuepfung.gh/2;
            if (y<aufknuepfung.y0) y = aufknuepfung.y0;
            if (y>y0) y = y0;
            canvas->LineTo (XSCALE(x0+i*aufknuepfung.gw+aufknuepfung.gw/2), YSCALE(y-1));
            CHECKCANCEL
        }
        for (int j=0; j<schaefte.count(); j++) {
            canvas->MoveTo (XSCALE(x0), YSCALE(y0-j*aufknuepfung.gh-aufknuepfung.gh/2));
            int x = x0+j*aufknuepfung.gw+aufknuepfung.gw/2;
            if (x<x0) x = x0;
            if (x>x0+aufknuepfung.width) x = x0+aufknuepfung.width;
            canvas->LineTo (XSCALE(x), YSCALE(y0-j*aufknuepfung.gh-aufknuepfung.gh/2));
            CHECKCANCEL
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintTrittfolge()
{
    if (!trittfolge.visible) return;

    DARSTELLUNG darst = mainfrm->trittfolge.darstellung;
    if (mainfrm->ViewSchlagpatrone->Checked) darst = mainfrm->schlagpatronendarstellung;

    int x0 = trittfolge.x0;
    int y0 = trittfolge.y0 + trittfolge.height;
    for (int i=0; i<tritte.count(); i++) {
        for (int j=0; j<schuesse.count(); j++) {
            // Rahmen
            canvas->Pen->Color = clBlack;
            canvas->MoveTo (XSCALE(x0+i*trittfolge.gw), YSCALE(y0-j*trittfolge.gh));
            canvas->LineTo (XSCALE(x0+(i+1)*trittfolge.gw), YSCALE(y0-j*trittfolge.gh));
            canvas->LineTo (XSCALE(x0+(i+1)*trittfolge.gw), YSCALE(y0-(j+1)*trittfolge.gh));
            canvas->LineTo (XSCALE(x0+i*trittfolge.gw), YSCALE(y0-(j+1)*trittfolge.gh));
            canvas->LineTo (XSCALE(x0+i*trittfolge.gw), YSCALE(y0-j*trittfolge.gw));
            if (!mainfrm->ViewOnlyGewebe->Checked) {
                // Inhalt
                char s = mainfrm->trittfolge.feld.Get (i+tritte.a, j+schuesse.a);
                if (s==AUSHEBUNG) {
                    PrPaintCell (mainfrm->darst_aushebung,
                                x0+i*trittfolge.gw, y0-(j+1)*trittfolge.gh,
                                x0+(i+1)*trittfolge.gw, y0-j*trittfolge.gh);
                } else if (s==ANBINDUNG) {
                    PrPaintCell (mainfrm->darst_anbindung,
                                x0+i*trittfolge.gw, y0-(j+1)*trittfolge.gh,
                                x0+(i+1)*trittfolge.gw, y0-j*trittfolge.gh);
                } else if (s==ABBINDUNG) {
                    PrPaintCell (mainfrm->darst_abbindung,
                                x0+i*trittfolge.gw, y0-(j+1)*trittfolge.gh,
                                x0+(i+1)*trittfolge.gw, y0-j*trittfolge.gh);
                } else if (s>0) {
                    PrPaintCell (darst,
                                x0+i*trittfolge.gw, y0-(j+1)*trittfolge.gh,
                                x0+(i+1)*trittfolge.gw, y0-j*trittfolge.gh,
								GetRangeColor(s),
								i);
                }
            }
        }
        CHECKCANCEL
    }
    // 'Strongline'
    canvas->Pen->Color = clBlack;
    SETSTRONGWIDTH
    for (int i=0; i<tritte.count(); i++) {
        if (i!=0 && mainfrm->trittfolge.pos.strongline_x!=0 && ((i+tritte.a)%mainfrm->trittfolge.pos.strongline_x)==0) {
            canvas->MoveTo (XSCALE(x0+i*trittfolge.gw), YSCALE(trittfolge.y0+pw));
            canvas->LineTo (XSCALE(x0+i*trittfolge.gw), YSCALE(trittfolge.y0+trittfolge.height-pw));
        }
    }
    for (int j=0; j<schuesse.count(); j++) {
        if (j!=0 && mainfrm->trittfolge.pos.strongline_y!=0 && ((j+schuesse.a)%mainfrm->trittfolge.pos.strongline_y)==0) {
            canvas->MoveTo (XSCALE(trittfolge.x0+pw), YSCALE(y0-j*trittfolge.gh));
            canvas->LineTo (XSCALE(trittfolge.x0+trittfolge.width-pw), YSCALE(y0-j*trittfolge.gh));
        }
    }
    RESETSTRONGWIDTH
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintSchussfarben()
{
    if (!schussfarben.visible) return;

    int x0 = schussfarben.x0;
    int y0 = schussfarben.y0 + schussfarben.height;
    for (int j=schuesse.a; j<=schuesse.b; j++) {
        canvas->Pen->Color = clBlack;
        canvas->Brush->Color = (TColor)Data->palette->GetColor(mainfrm->schussfarben.feld.Get(j));
        canvas->Rectangle (XSCALE(x0), YSCALE(y0-(j-schuesse.a+1)*schussfarben.gh), XSCALE(x0+schussfarben.width+1), YSCALE(y0-(j-schuesse.a)*schussfarben.gh+1));
        CHECKCANCEL
    }
}
/*-----------------------------------------------------------------*/

