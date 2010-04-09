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
#include "farbpalette_form.h"
#include "felddef.h"
#include "palette.h"
#include "dbw3_strings.h"
#include "language.h"
#include "rangecolors.h"
/*-----------------------------------------------------------------*/
#define GETPALCOL(col) \
    ((TColor)Data->palette->GetColor(col))
/*-----------------------------------------------------------------*/
#define STB_RANGE LANG_STR("Range ", "Bereich ")
#define STB_RANGEAUSHEBUNG LANG_STR("Lift out", "Aushebung")
#define STB_RANGEANBINDUNG LANG_STR("Binding", "Anbindung")
#define STB_RANGEABBINDUNG LANG_STR("Unbinding", "Abbindung")
#define STB_BINDUNG       LANG_STR("Pattern", "Bindung")
#define STB_EINZUG        LANG_STR("Threading", "Einzug")
#define STB_TRITTFOLGE    LANG_STR("Treadling", "Trittfolge")
#define STB_AUFKNUEPFUNG  LANG_STR("Tie-Up", "Aufknüpfung")
#define STB_SCHLAGPATRONE LANG_STR("Pegplan", "Schlagpatrone")
#define STB_BLATTEINZUG   LANG_STR("Reed treading", "Blatteinzug")
#define STB_KETTFARBEN    LANG_STR("Warp colors", "Kettfarben")
#define STB_SCHUSSFARBEN  LANG_STR("Weft colors", "Schussfarben")
#define STB_SPALTE LANG_STR("  Col ", "  Spalte ")
#define STB_ZEILE LANG_STR("  Row ", "  Zeile ")
#define STB_SIZE LANG_STR("Size ", "Grösse ")
#define STB_RAPPORT LANG_STR("Repeat ", "Rapport ")
#define STB_RAPPORT1 LANG_STR("Repeat* ", "Rapport* ")
#define STB_SELECTION LANG_STR("Selection  Size ", "Selektion  Grösse ")
#define STB_TEMPLRAPPORT LANG_STR("Size 999x999 Repeat 999x999", "Grösse 999x999 Rapport 999x999")
#define STB_TEMPLPOS LANG_STR("Reed threading  Col 999  Row 999", "Aufknüpfung  Spalte 999  Zeile 999")
#define STB_TEMPLRANGE LANG_STR("Unbinding", "Bereich 9")
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::UpdateStatusBar()
{
    INPUTPOS kbd;
    int x, y, sx, sy;
    String line;

    if (currentrange>0 && currentrange<=9) line = String(STB_RANGE) + IntToStr(currentrange);
    else if (currentrange==AUSHEBUNG) line = STB_RANGEAUSHEBUNG;
    else if (currentrange==ANBINDUNG) line = STB_RANGEANBINDUNG;
    else if (currentrange==ABBINDUNG) line = STB_RANGEABBINDUNG;
    Statusbar->Panels->Items[1]->Text = line;

    switch (kbd_field) {
        case GEWEBE:
            line = STB_BINDUNG;
            x = gewebe.kbd.i+1;
            y = gewebe.kbd.j+1;
            sx = scroll_x1;
            sy = scroll_y2;
            break;

        case EINZUG:
            line = STB_EINZUG;
            x = einzug.kbd.i+1;
            y = einzug.kbd.j+1;
            sx = scroll_x1;
            sy = scroll_y1;
            break;

        case AUFKNUEPFUNG:
            line = STB_AUFKNUEPFUNG;
            x = aufknuepfung.kbd.i+1;
            y = aufknuepfung.kbd.j+1;
            sx = scroll_x2;
            sy = scroll_y1;
            break;

        case TRITTFOLGE:
			if (!ViewSchlagpatrone->Checked) line = STB_TRITTFOLGE;
			else line = STB_SCHLAGPATRONE;
            x = trittfolge.kbd.i+1;
            y = trittfolge.kbd.j+1;
            sx = scroll_x2;
            sy = scroll_y2;
            break;

        case BLATTEINZUG:
            line = STB_BLATTEINZUG;
            x = blatteinzug.kbd.i+1;
            y = -1;
            sx = scroll_x1;
            break;

        case KETTFARBEN:
            line = STB_KETTFARBEN;
            x = kettfarben.kbd.i+1;
            y = -1;
            sx = scroll_x1;
            break;

        case SCHUSSFARBEN:
            line = STB_SCHUSSFARBEN;
            x = -1;
            y = schussfarben.kbd.j+1;
            sy = scroll_y2;
            break;
    }

    if (x!=-1) {
        line += STB_SPALTE;
        line += IntToStr (x+sx);
    }

    if (y!=-1) {
        line += STB_ZEILE;
        line += IntToStr (y+sy);
    }

    Statusbar->Panels->Items[3]->Text = line;

    line = "";

    if (kette.a!=-1 || schuesse.a!=-1) {
        line = line + STB_SIZE;
        if (kette.a!=-1) line = line + IntToStr(kette.b-kette.a+1);
        else line = line + "0";
        line = line + "x";
        if (schuesse.a!=-1) line = line + IntToStr(schuesse.b-schuesse.a+1);
        else line = line + "0";
        line = line + "  ";
    }

    if (rapport.kr.b!=-1 || rapport.sr.b!=-1) {
        if (!rapport.overridden) line = line + STB_RAPPORT;
        else line = line + STB_RAPPORT1;
        if (rapport.kr.b!=-1) line = line + IntToStr(rapport.kr.b-rapport.kr.a+1)+"x";
        else line = line + "0x";
        if (rapport.sr.b!=-1) line = line + IntToStr(rapport.sr.b-rapport.sr.a+1);
        else line = line + "0";
    }

    Statusbar->Panels->Items[4]->Text = line;

    // Falls eine Selektion: Grösse angeben
    Statusbar->Panels->Items[0]->Text = "";
    if (!bSelectionCleared) {
        RANGE sel = selection;
        sel.Normalize();
        if (sel.Valid()) {
            int dx = sel.end.i-sel.begin.i+1;
            int dy = sel.end.j-sel.begin.j+1;
            if (dx>1 || dy>1) {
                line = STB_SELECTION;
                line = line + IntToStr(dx);
                line = line + "x";
                line = line + IntToStr(dy);
                Statusbar->Panels->Items[0]->Text = line;
            }
        }
    }

    // Platz für alle!
    RecalcStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RecalcStatusBar()
{
	const int colwidth = Statusbar->ClientHeight;

    int rapwidth = Statusbar->Canvas->TextWidth (STB_TEMPLRAPPORT)+80;
    int poswidth = Statusbar->Canvas->TextWidth (STB_TEMPLPOS);
    int berwidth = Statusbar->Canvas->TextWidth (STB_TEMPLRANGE)+20;

    Statusbar->Panels->Items[0]->Width = Statusbar->ClientWidth - poswidth - colwidth - rapwidth;
	Statusbar->Panels->Items[1]->Width = berwidth;
	Statusbar->Panels->Items[2]->Width = colwidth;
    Statusbar->Panels->Items[3]->Width = poswidth;
    Statusbar->Panels->Items[4]->Width = rapwidth;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::OnDrawStatusBar (TStatusBar* StatusBar,
										  TStatusPanel* Panel,
										  const Windows::TRect &Rect)
{
	if (Panel==Statusbar->Panels->Items[2]) { // Aktive Farbe
		TCanvas* canvas = Statusbar->Canvas;
		canvas->Pen->Color = clBtnFace;
		canvas->Brush->Color = GETPALCOL(Data->color);
		canvas->Rectangle (Rect.Left, Rect.Top, Rect.Right, Rect.Bottom);
	} else if (Panel==Statusbar->Panels->Items[1]) { // Aktiver Bereich
		TCanvas* canvas = Statusbar->Canvas;
        if (currentrange>1 && currentrange<=9) {
            canvas->Brush->Color = GetRangeColor(currentrange);
    		canvas->Pen->Color = canvas->Brush->Color;
            canvas->Rectangle (Rect.Left, Rect.Top, Rect.Right, Rect.Bottom);
            canvas->Font->Color = clWhite;
            canvas->TextFlags = canvas->TextFlags & ~ETO_OPAQUE;
            canvas->TextOut(Rect.Left+4, Rect.Top, Statusbar->Panels->Items[1]->Text);
        } else {
            canvas->Brush->Color = clBtnFace;
    		canvas->Pen->Color = clBtnFace;
            canvas->Rectangle (Rect.Left, Rect.Top, Rect.Right, Rect.Bottom);
            canvas->Font->Color = clBlack;
            canvas->TextFlags = canvas->TextFlags & ~ETO_OPAQUE;
            canvas->TextOut(Rect.Left+4, Rect.Top, Statusbar->Panels->Items[1]->Text);
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DisplayHint (TObject* Sender)
{
    Statusbar->Panels->Items[0]->Text = GetLongHint(Application->Hint);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::StatusbarDblClick(TObject *Sender)
{
    //
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::StatusbarClick(TObject *Sender)
{
    POINT pt;
    GetCursorPos (&pt);
    Statusbar->ScreenToClient (pt);
    int x1, x2;
    // Check auf Range
    x1 = Statusbar->Panels->Items[0]->Width;
    x2 = x1 + Statusbar->Panels->Items[1]->Width;
    if (pt.x>x1 && pt.x<x2) {
        if (rangepopupactive) return;
        rangepopupactive = true;
        PopupRange->Popup (x1, Statusbar->Top-13*GetSystemMetrics(SM_CYMENU)+15);
        rangepopupactive = false;
        return;
    }
    // Check auf Palette
    x1 = x2;
    x2 = x1 + Statusbar->Panels->Items[2]->Width;
    if (pt.x>x1 && pt.x<x2) {
        if (ViewFarbpalette->Checked==true) {
            FarbPalette->Visible = false;
            ViewFarbpalette->Checked = false;
        } else {
            FarbPalette->Visible = true;
            ViewFarbpalette->Checked = true;
        }
        return;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::PopupRangePopup(TObject *Sender)
{
    //
}
/*-----------------------------------------------------------------*/
