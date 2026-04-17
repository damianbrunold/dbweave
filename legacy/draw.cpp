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
#include "datamodule.h"
#include "felddef.h"
#include "palette.h"
#include "rangecolors.h"
/*-----------------------------------------------------------------*/
#define GETPALCOL(col) \
    ((TColor)Data->palette->GetColor(col))
/*-----------------------------------------------------------------*/
// W8006 TColor wird mit const int initialisiert
#pragma warn -8006
/*-----------------------------------------------------------------*/
// Diese Funktionen sind sehr high-level. Das heisst, man muss nur
// die relativen Koordinaten des Punktes übergeben und den Rest macht
// die Funktion. Beim Gewebe inklusive Rapport und Range.
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawEinzug (int _i, int _j, TColor _col/*=clBlack*/)
{
    int i = _i;
    if (righttoleft) {
        i = einzug.pos.width/einzug.gw - i - 1;
    }

    int j = _j;
    if (toptobottom) {
        j = einzug.pos.height/einzug.gh - j - 1;
    }

    int x = einzug.pos.x0 + i*einzug.gw;
    int y = einzug.pos.y0 + einzug.pos.height - (j+1)*einzug.gh;

    if (einzug.feld.Get(scroll_x1+_i)-1==(scroll_y1+_j))
        PaintCell (Canvas, einzug.darstellung, x, y, x+einzug.gw, y+einzug.gh, _col, false, scroll_y1+_j);
    else
        ClearCell (Canvas, x, y, x+einzug.gw, y+einzug.gh);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawEinzugRahmen (int _i, int _j)
{
    int i = _i;
    if (righttoleft) {
        i = einzug.pos.width/einzug.gw - i - 1;
    }

    int j = _j;
    if (toptobottom) {
        j = einzug.pos.height/einzug.gh - j - 1;
    }

    int x = einzug.pos.x0 + i*einzug.gw;
    int y = einzug.pos.y0 + einzug.pos.height - (j+1)*einzug.gh;
    int xx = x + einzug.gw;
    int yy = y + einzug.gh;

    // Rahmen inkl. betonte Linien zeichnen
    Canvas->Pen->Color = clBtnShadow;
    Canvas->MoveTo (x, y);
    Canvas->LineTo (xx, y);
    Canvas->MoveTo (x, y);
    Canvas->LineTo (x, yy);

    Canvas->Pen->Color = strongclr;
    if (einzug.pos.strongline_x!=0 && _i>0) {
        if (righttoleft) {
            if (((_i+scroll_x1) % einzug.pos.strongline_x)==0 && _i!=0) {
                Canvas->MoveTo (xx, y);
                Canvas->LineTo (xx, yy);
            }
            if (((_i+1+scroll_x1) % einzug.pos.strongline_x)==0 && _i+1<einzug.pos.width/einzug.gw) {
                Canvas->MoveTo (x, y);
                Canvas->LineTo (x, yy);
            }
        } else {
            if (((_i+scroll_x1) % einzug.pos.strongline_x)==0 && _i!=0) {
                Canvas->MoveTo (x, y);
                Canvas->LineTo (x, yy);
            }
            if (((_i+1+scroll_x1) % einzug.pos.strongline_x)==0 && _i+1<einzug.pos.width/einzug.gw) {
                Canvas->MoveTo (xx, y);
                Canvas->LineTo (xx, yy);
            }
        }
    }
    if (einzug.pos.strongline_y!=0 && _j>0) {
        if (toptobottom) {
            if (((_j+scroll_y1+1) % einzug.pos.strongline_y)==0 && _j+1<einzug.pos.height/einzug.gh) {
                Canvas->MoveTo (x, yy);
                Canvas->LineTo (xx, yy);
            }
            if (((_j+scroll_y1) % einzug.pos.strongline_y)==0 && _j!=0) {
                Canvas->MoveTo (x, y);
                Canvas->LineTo (xx, y);
            }
        } else {
            if (((_j+scroll_y1+1) % einzug.pos.strongline_y)==0 && _j+1<einzug.pos.height/einzug.gh) {
                Canvas->MoveTo (x, y);
                Canvas->LineTo (xx, y);
            }
            if (((_j+scroll_y1) % einzug.pos.strongline_y)==0 && _j!=0) {
                Canvas->MoveTo (x, yy);
                Canvas->LineTo (xx, yy);
            }
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawAufknuepfung (int _i, int _j, int _pegplan/*=false*/, TColor _col/*=clBlack*/)
{
    // Bei Schlagpatronenansicht wird nichts gezeichnet!
    if (ViewSchlagpatrone->Checked && aufknuepfung.pegplanstyle && !toptobottom) return;

    int x = aufknuepfung.pos.x0 + _i*aufknuepfung.gw;
    int y;
    if (toptobottom) y = aufknuepfung.pos.y0 + _j*aufknuepfung.gh;
    else y = aufknuepfung.pos.y0 + aufknuepfung.pos.height - (_j+1)*aufknuepfung.gh;

    if (!ViewSchlagpatrone->Checked) {
        char s = aufknuepfung.feld.Get(scroll_x2+_i,scroll_y1+_j);
        if (s==AUSHEBUNG) {
            if (darst_aushebung!=AUSGEFUELLT)
                PaintCell (Canvas, darst_aushebung, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh, _col);
            else if (_col==clBlack)
                PaintCell (Canvas, darst_aushebung, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh, GetRangeColor(s));
            else
                PaintCell (Canvas, darst_aushebung, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh, _col);
        } else if (s==ANBINDUNG) {
            if (darst_aushebung!=AUSGEFUELLT)
                PaintCell (Canvas, darst_anbindung, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh, _col);
            else if (_col==clBlack)
                PaintCell (Canvas, darst_anbindung, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh, GetRangeColor(s));
            else
                PaintCell (Canvas, darst_anbindung, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh, _col);
        } else if (s==ABBINDUNG) {
            if (darst_aushebung!=AUSGEFUELLT)
                PaintCell (Canvas, darst_abbindung, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh, _col);
            else if (_col==clBlack)
                PaintCell (Canvas, darst_abbindung, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh, GetRangeColor(s));
            else
                PaintCell (Canvas, darst_abbindung, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh, _col);
        } else if (_col!=clBlack) {
            PaintCell (Canvas, aufknuepfung.darstellung, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh, _col, false, scroll_y1+_j);
        } else if (s>0) {
            PaintCell (Canvas, aufknuepfung.darstellung, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh, GetRangeColor(s), false, scroll_y1+_j);
        } else
            ClearCell (Canvas, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh);
    } else {
        if (aufknuepfung.feld.Get(scroll_x2+_i,scroll_y1+_j)>0)
            PaintCell (Canvas, /*aufknuepfung.darstellung*/AUSGEFUELLT, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh, clDkGray, false, -1);
        else
            ClearCell (Canvas, x, y, x+aufknuepfung.gw, y+aufknuepfung.gh);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawAufknuepfungRahmen (int _i, int _j)
{
    int j = _j;
    if (toptobottom) {
        j = einzug.pos.height/einzug.gh - j - 1;
    }

    int x = aufknuepfung.pos.x0 + _i*aufknuepfung.gw;
    int y = aufknuepfung.pos.y0 + aufknuepfung.pos.height - (j+1)*aufknuepfung.gh;
    int xx = x + aufknuepfung.gw;
    int yy = y + aufknuepfung.gh;

    // Rahmen inkl. betonte Linien zeichnen
    Canvas->Pen->Color = clBtnShadow;
    Canvas->MoveTo (x, y);
    Canvas->LineTo (xx, y);
    Canvas->MoveTo (x, y);
    Canvas->LineTo (x, yy);

    Canvas->Pen->Color = strongclr;
    if (aufknuepfung.pos.strongline_x!=0 && _i>0) {
        if (((_i+scroll_x2) % aufknuepfung.pos.strongline_x)==0 && _i!=0) {
            Canvas->MoveTo (x, y);
            Canvas->LineTo (x, yy);
        }
        if (((_i+1+scroll_x2) % aufknuepfung.pos.strongline_x)==0 && _i+1<aufknuepfung.pos.width/aufknuepfung.gw) {
            Canvas->MoveTo (xx, y);
            Canvas->LineTo (xx, yy);
        }
    }
    if (aufknuepfung.pos.strongline_y!=0 && _j>0) {
        if (toptobottom) {
            if (((_j+scroll_y1+1) % aufknuepfung.pos.strongline_y)==0 && _j+1<aufknuepfung.pos.height/aufknuepfung.gh) {
                Canvas->MoveTo (x, yy);
                Canvas->LineTo (xx, yy);
            }
            if (((_j+scroll_y1) % aufknuepfung.pos.strongline_y)==0 && _j!=0) {
                Canvas->MoveTo (x, y);
                Canvas->LineTo (xx, y);
            }
        } else {
            if (((_j+scroll_y1+1) % aufknuepfung.pos.strongline_y)==0 && _j+1<aufknuepfung.pos.height/aufknuepfung.gh) {
                Canvas->MoveTo (x, y);
                Canvas->LineTo (xx, y);
            }
            if (((_j+scroll_y1) % aufknuepfung.pos.strongline_y)==0 && _j!=0) {
                Canvas->MoveTo (x, yy);
                Canvas->LineTo (xx, yy);
            }
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawTrittfolge (int _i, int _j, TColor _col/*=clBlack*/)
{
    int x = trittfolge.pos.x0 + _i*trittfolge.gw;
    int y = trittfolge.pos.y0 + trittfolge.pos.height - (_j+1)*trittfolge.gh;

    char s = trittfolge.feld.Get(scroll_x2+_i,scroll_y2+_j);
    if (s==AUSHEBUNG) {
        if (ViewSchlagpatrone->Checked) {
            if (darst_aushebung!=AUSGEFUELLT)
                PaintCell (Canvas, darst_aushebung, x, y, x+trittfolge.gw, y+trittfolge.gh, _col);
            else if (_col==clBlack)
                PaintCell (Canvas, darst_aushebung, x, y, x+trittfolge.gw, y+trittfolge.gh, GetRangeColor(s));
            else
                PaintCell (Canvas, darst_aushebung, x, y, x+trittfolge.gw, y+trittfolge.gh, _col);
        } else
            PaintCell (Canvas, trittfolge.darstellung, x, y, x+trittfolge.gw, y+trittfolge.gh, _col);
    } else if (s==ANBINDUNG) {
        if (ViewSchlagpatrone->Checked) {
            if (darst_aushebung!=AUSGEFUELLT)
                PaintCell (Canvas, darst_anbindung, x, y, x+trittfolge.gw, y+trittfolge.gh, _col);
            else if (_col==clBlack)
                PaintCell (Canvas, darst_anbindung, x, y, x+trittfolge.gw, y+trittfolge.gh, GetRangeColor(s));
            else
                PaintCell (Canvas, darst_anbindung, x, y, x+trittfolge.gw, y+trittfolge.gh, _col);
        } else
            PaintCell (Canvas, trittfolge.darstellung, x, y, x+trittfolge.gw, y+trittfolge.gh, _col);
    } else if (s==ABBINDUNG) {
        if (ViewSchlagpatrone->Checked) {
            if (darst_aushebung!=AUSGEFUELLT)
                PaintCell (Canvas, darst_abbindung, x, y, x+trittfolge.gw, y+trittfolge.gh, _col);
            else if (_col==clBlack)
                PaintCell (Canvas, darst_abbindung, x, y, x+trittfolge.gw, y+trittfolge.gh, GetRangeColor(s));
            else
                PaintCell (Canvas, darst_abbindung, x, y, x+trittfolge.gw, y+trittfolge.gh, _col);
        } else
            PaintCell (Canvas, trittfolge.darstellung, x, y, x+trittfolge.gw, y+trittfolge.gh, _col);
    } else if (_col!=clBlack) {
        if (ViewSchlagpatrone->Checked) {
            PaintCell (Canvas, schlagpatronendarstellung, x, y, x+trittfolge.gw, y+trittfolge.gh, _col, false, scroll_x2+_i);
        } else
            PaintCell (Canvas, trittfolge.darstellung, x, y, x+trittfolge.gw, y+trittfolge.gh, _col);
    } else if (s>0) {
        if (ViewSchlagpatrone->Checked) {
            PaintCell (Canvas, schlagpatronendarstellung, x, y, x+trittfolge.gw, y+trittfolge.gh, GetRangeColor(s), false, scroll_x2+_i);
        } else
            PaintCell (Canvas, trittfolge.darstellung, x, y, x+trittfolge.gw, y+trittfolge.gh, _col);
    } else
        ClearCell (Canvas, x, y, x+trittfolge.gw, y+trittfolge.gh);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawTrittfolgeRahmen (int _i, int _j)
{
    int x = trittfolge.pos.x0 + _i*trittfolge.gw;
    int y = trittfolge.pos.y0 + trittfolge.pos.height - (_j+1)*trittfolge.gh;
    int xx = x + trittfolge.gw;
    int yy = y + trittfolge.gh;

    // Rahmen inkl. betonte Linien zeichnen
    Canvas->Pen->Color = clBtnShadow;
    Canvas->MoveTo (x, y);
    Canvas->LineTo (xx, y);
    Canvas->MoveTo (x, y);
    Canvas->LineTo (x, yy);

    Canvas->Pen->Color = strongclr;
    if (trittfolge.pos.strongline_x!=0 && _i>0) {
        if (((_i+scroll_x2) % trittfolge.pos.strongline_x)==0 && _i!=0) {
            Canvas->MoveTo (x, y);
            Canvas->LineTo (x, yy);
        }
        if (((_i+1+scroll_x2) % trittfolge.pos.strongline_x)==0 && _i+1<trittfolge.pos.width/trittfolge.gw) {
            Canvas->MoveTo (xx, y);
            Canvas->LineTo (xx, yy);
        }
    }
    if (trittfolge.pos.strongline_y!=0 && _j>0) {
        if (((_j+scroll_y2+1) % trittfolge.pos.strongline_y)==0 && _j+1<trittfolge.pos.height/trittfolge.gh) {
            Canvas->MoveTo (x, y);
            Canvas->LineTo (xx, y);
        }
        if (((_j+scroll_y2) % trittfolge.pos.strongline_y)==0 && _j!=0) {
            Canvas->MoveTo (x, yy);
            Canvas->LineTo (xx, yy);
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawGewebeRahmen (int _i, int _j)
{
    int i = _i;
    if (righttoleft) {
        i = gewebe.pos.width/gewebe.gw - i - 1;
    }

    int x = gewebe.pos.x0 + i*gewebe.gw;
    int y = gewebe.pos.y0 + gewebe.pos.height - (_j+1)*gewebe.gh;
    int xx = x + gewebe.gw;
    int yy = y + gewebe.gh;
    DrawGewebeRahmen (_i, _j, x, y, xx, yy);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawGewebeRahmen (int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    // Rahmen inkl. betonte Linien zeichnen
    Canvas->Pen->Color = clBtnShadow;
    Canvas->MoveTo (_x, _y);
    Canvas->LineTo (_xx, _y);
    Canvas->MoveTo (_x, _y);
    Canvas->LineTo (_x, _yy);

    Canvas->Pen->Color = strongclr;
    if (righttoleft) {
        if (gewebe.pos.strongline_x!=0) {
            if (((_i+scroll_x1) % gewebe.pos.strongline_x)==0 && _i!=0) {
                Canvas->MoveTo (_xx, _y);
                Canvas->LineTo (_xx, _yy);
            }
            if (((_i+1+scroll_x1) % gewebe.pos.strongline_x)==0 && _i+1<gewebe.pos.width/gewebe.gw) {
                Canvas->MoveTo (_x, _y);
                Canvas->LineTo (_x, _yy);
            }
        }
    } else {
        if (gewebe.pos.strongline_x!=0) {
            if (((_i+scroll_x1) % gewebe.pos.strongline_x)==0 && _i!=0) {
                Canvas->MoveTo (_x, _y);
                Canvas->LineTo (_x, _yy);
            }
            if (((_i+1+scroll_x1) % gewebe.pos.strongline_x)==0 && _i+1<gewebe.pos.width/gewebe.gw) {
                Canvas->MoveTo (_xx, _y);
                Canvas->LineTo (_xx, _yy);
            }
        }
    }
    if (gewebe.pos.strongline_y!=0) {
        if (((_j+scroll_y2+1) % gewebe.pos.strongline_y)==0 && _j+1<gewebe.pos.height/gewebe.gh) {
            Canvas->MoveTo (_x, _y);
            Canvas->LineTo (_xx, _y);
        }
        if (((_j+scroll_y2) % gewebe.pos.strongline_y)==0 && _j!=0) {
            Canvas->MoveTo (_x, _yy);
            Canvas->LineTo (_xx, _yy);
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawGewebe (int _i, int _j, TColor _col/*=clBlack*/)
{
    int i = _i;
    if (righttoleft) {
        i = gewebe.pos.width/gewebe.gw - i - 1;
    }

    int x = gewebe.pos.x0 + i*gewebe.gw;
    int y = gewebe.pos.y0 + gewebe.pos.height - (_j+1)*gewebe.gh;

    if ((IsEmptyEinzug(scroll_x1+_i) || IsEmptyTrittfolge(scroll_y2+_j))) {
        Canvas->Pen->Color = clBtnFace;
        Canvas->Brush->Color = Canvas->Pen->Color;
        Canvas->Rectangle (x+1, y+1, x+gewebe.gw, y+gewebe.gh);
        return;
    }

    if (!Inverserepeat->Checked) {
        if (!GewebeNone->Checked && RappViewRapport->Checked && IsInRapport (_i+scroll_x1, _j+scroll_y2))
            DrawGewebeRapport (_i, _j, x, y, x+gewebe.gw, y+gewebe.gh);
        else if (GewebeNormal->Checked)
            DrawGewebeNormal (_i, _j, x, y, x+gewebe.gw, y+gewebe.gh, _col);
        else if (GewebeFarbeffekt->Checked)
            DrawGewebeFarbeffekt (_i, _j, x, y, x+gewebe.gw, y+gewebe.gh);
        else if (GewebeSimulation->Checked)
            DrawGewebeSimulation (_i, _j, x, y, x+gewebe.gw, y+gewebe.gh);
        else if (GewebeNone->Checked)
            DrawGewebeRahmen (_i, _j);
        else
            dbw3_assert(false);
    } else {
        if (!GewebeNone->Checked && RappViewRapport->Checked && !IsInRapport (_i+scroll_x1, _j+scroll_y2))
            DrawGewebeRapport (_i, _j, x, y, x+gewebe.gw, y+gewebe.gh);
        else if (GewebeNormal->Checked)
            DrawGewebeNormal (_i, _j, x, y, x+gewebe.gw, y+gewebe.gh, _col);
        else if (GewebeFarbeffekt->Checked)
            DrawGewebeFarbeffekt (_i, _j, x, y, x+gewebe.gw, y+gewebe.gh);
        else if (GewebeSimulation->Checked)
            DrawGewebeSimulation (_i, _j, x, y, x+gewebe.gw, y+gewebe.gh);
        else if (GewebeNone->Checked)
            DrawGewebeRahmen (_i, _j);
        else
            dbw3_assert(false);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawGewebeNormal (int _i, int _j, int _x, int _y, int _xx, int _yy, TColor _col/*=clBlack*/)
{
    int range = gewebe.feld.Get (scroll_x1+_i, scroll_y2+_j);
    if (range==AUSHEBUNG) {
        if (darst_aushebung!=AUSGEFUELLT) {
            PaintCell (Canvas, darst_aushebung, _x, _y, _xx, _yy, _col);
        } else if (_col==clBlack)
            PaintCell (Canvas, darst_aushebung, _x, _y, _xx, _yy, GetRangeColor(range));
        else
            PaintCell (Canvas, darst_aushebung, _x, _y, _xx, _yy, _col);
    } else if (range==ANBINDUNG) {
        if (darst_aushebung!=AUSGEFUELLT) {
            Canvas->Pen->Color = clBtnFace;
            Canvas->Brush->Color = col_anbindung;
            Canvas->Rectangle (_x+1, _y+1, _xx, _yy);
            PaintCell (Canvas, darst_anbindung, _x, _y, _xx, _yy, _col, true);
        } else if (_col==clBlack)
            PaintCell (Canvas, darst_anbindung, _x, _y, _xx, _yy, GetRangeColor(range));
        else
            PaintCell (Canvas, darst_anbindung, _x, _y, _xx, _yy, _col);
    } else if (range==ABBINDUNG) {
        if (darst_aushebung!=AUSGEFUELLT) {
            Canvas->Pen->Color = clBtnFace;
            Canvas->Brush->Color = col_abbindung;
            Canvas->Rectangle (_x+1, _y+1, _xx, _yy);
            PaintCell (Canvas, darst_abbindung, _x, _y, _xx, _yy, _col, true);
        } else if (_col==clBlack)
            PaintCell (Canvas, darst_abbindung, _x, _y, _xx, _yy, GetRangeColor(range));
        else
            PaintCell (Canvas, darst_abbindung, _x, _y, _xx, _yy, _col);
    } else if (_col!=clBlack) {
        Canvas->Brush->Color = _col;
        Canvas->Pen->Color = clBtnFace;
        Canvas->Rectangle (_x+1, _y+1, _xx, _yy);
    } else if (range>0) {
        // Farbe je nach Bereich wählen
        Canvas->Brush->Color = GetRangeColor(range);
        Canvas->Pen->Color = clBtnFace;
        Canvas->Rectangle (_x+1, _y+1, _xx, _yy);
    } else {
        Canvas->Brush->Color = clBtnFace;
        Canvas->Pen->Color = clBtnFace;
        Canvas->Rectangle (_x+1, _y+1, _xx, _yy);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawGewebeRapport (int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    char s = gewebe.feld.Get(scroll_x1+_i,scroll_y2+_j);
    if (s!=ABBINDUNG && s>0) {
        Canvas->Brush->Color = clRed;
        Canvas->Pen->Color = clBtnFace;
        Canvas->Rectangle (_x+1, _y+1, _xx, _yy);
    } else {
        Canvas->Pen->Color = clBtnFace;
        Canvas->Brush->Color = Canvas->Pen->Color;
        Canvas->Rectangle (_x+1, _y+1, _xx, _yy);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawGewebeFarbeffekt (int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    char s = gewebe.feld.Get(scroll_x1+_i,scroll_y2+_j);
    bool drawhebung = s>0 && s!=ABBINDUNG;
    if (sinkingshed) drawhebung = !drawhebung;
    if (drawhebung) {
        Canvas->Pen->Color = GETPALCOL(kettfarben.feld.Get(scroll_x1+_i));
        Canvas->Brush->Color = Canvas->Pen->Color;
    } else {
        Canvas->Pen->Color = GETPALCOL(schussfarben.feld.Get(scroll_y2+_j));
        Canvas->Brush->Color = Canvas->Pen->Color;
    }
    Canvas->Rectangle (_x, _y, _xx, _yy);
    if (fewithraster) DrawGewebeRahmen (_i, _j, _x, _y, _xx, _yy);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawGewebeSimulation (int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    int dw = gewebe.gw/5;
    int dh = gewebe.gh/5;
    if (faktor_kette==1.0 && faktor_schuss==1.0)
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
    if (dw==0 && gewebe.gw>2) dw = 1;
    if (dh==0 && gewebe.gh>2) dh = 1;

    Canvas->Brush->Color = clBtnFace;
    Canvas->Pen->Color = clBtnFace;
    Canvas->Rectangle (_x, _y, _xx, _yy);

    char s = gewebe.feld.Get(scroll_x1+_i,scroll_y2+_j);
    bool drawhebung = s>0 && s!=ABBINDUNG;
    if (sinkingshed) drawhebung = !drawhebung;
    if (drawhebung) {
        Canvas->Pen->Color = GETPALCOL(kettfarben.feld.Get(scroll_x1+_i));
        Canvas->Brush->Color = Canvas->Pen->Color;
        Canvas->Rectangle (_x+dw, _y, _xx-dw, _yy);
        Canvas->Pen->Color = GETPALCOL(schussfarben.feld.Get(scroll_y2+_j));
        Canvas->Brush->Color = Canvas->Pen->Color;
        Canvas->Rectangle (_x, _y+dh, _x+dw, _yy-dh);
        Canvas->Rectangle (_xx, _y+dh, _xx-dw, _yy-dh);
        // Schatten
        Canvas->Pen->Color = clBlack;
        Canvas->MoveTo (_xx-dw, _y+dh);
        Canvas->LineTo (_xx-dw, _yy-dh);
    } else {
        Canvas->Pen->Color = GETPALCOL(schussfarben.feld.Get(scroll_y2+_j));
        Canvas->Brush->Color = Canvas->Pen->Color;
        Canvas->Rectangle (_x, _y+dh, _xx, _yy-dh);
        Canvas->Pen->Color = GETPALCOL(kettfarben.feld.Get(scroll_x1+_i));
        Canvas->Brush->Color = Canvas->Pen->Color;
        Canvas->Rectangle (_x+dw, _y, _xx-dw, _y+dh);
        Canvas->Rectangle (_x+dw, _yy-dh, _xx-dw, _yy);
        // Schatten
        Canvas->Pen->Color = clBlack;
        Canvas->MoveTo (_x+dw, _yy-dh);
        Canvas->LineTo (_xx-dw, _yy-dh);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawGewebeSchuss (int _j)
{
    for (int i=0; i<gewebe.pos.width/gewebe.gw; i++) {
        DrawGewebe (i, _j);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawGewebeKette (int _i)
{
    for (int j=0; j<gewebe.pos.height/gewebe.gh; j++) {
        DrawGewebe (_i, j);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DeleteGewebeSchuss (int _j)
{
    for (int i=0; i<gewebe.pos.width/gewebe.gw; i++) {
        DrawGewebeRahmen (i, _j);
        DrawGewebe (i, _j);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DeleteGewebeKette (int _i)
{
    for (int j=0; j<gewebe.pos.height/gewebe.gh; j++) {
        DrawGewebeRahmen (_i, j);
        DrawGewebe (_i, j);
    }
}
/*-----------------------------------------------------------------*/
