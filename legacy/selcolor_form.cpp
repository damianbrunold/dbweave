/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File selcolor_form.cpp
 * Copyright (C) 1998-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#include <stdio.h>
#include <math.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "dbw3_form.h"
#include "colors.h"
#include "datamodule.h"
#include "palette.h"
#include "farbauswahl_form.h"
#include "farbauswahl1_form.h"
#include "selcolor_form.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
#define GETPALCOL(col) \
    ((TColor)Data->palette->GetColor(col))
/*-----------------------------------------------------------------*/
#define GRIDSIZE 17
/*-----------------------------------------------------------------*/
__fastcall TSelColorForm::TSelColorForm(int _selcolor, TComponent* Owner)
: TForm(Owner), selcolor(_selcolor)
{
    doedit = false;
    LoadLanguage();

    // Sicherheitskopie der Palette anlegen
    oldpal = new COLORREF[MAX_PAL_ENTRY];
    memcpy (oldpal, Data->palette->data, sizeof(COLORREF)*MAX_PAL_ENTRY);
	UpdateValues();
}
/*-----------------------------------------------------------------*/
__fastcall TSelColorForm::~TSelColorForm()
{
    delete[] oldpal;
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::UpdateValues()
{
	COLORREF clr = Data->palette->GetColor(selcolor);
	float h, s, v;
	RGB2HSV (GetRValue(clr), GetGValue(clr), GetBValue(clr), h, s, v);
	if (h!=UNDEFINED) {
		char buff[10];
		sprintf (buff, "%0.f°", h); Hue->Caption = buff;
		sprintf (buff, "%.3f", s);  Sat->Caption = buff;
		sprintf (buff, "%.3f", v);  Val->Caption = buff;
	} else {
        if (GetRValue(clr)==0 && GetGValue(clr)==0 && GetBValue(clr)==0) {
            Hue->Caption = "0";
            Sat->Caption = "0";
            Val->Caption = "0";
        } else if (GetRValue(clr)==255 && GetGValue(clr)==255 && GetBValue(clr)==255) {
            Hue->Caption = "0";
            Sat->Caption = "0";
            Val->Caption = "1";
        } else {
            Hue->Caption = "?";
            Sat->Caption = "?";
            Val->Caption = "?";
        }
    }

    Red->Caption = IntToStr (GetRValue(clr));
    Green->Caption = IntToStr (GetGValue(clr));
    Blue->Caption = IntToStr (GetBValue(clr));

    Idx->Caption = IntToStr (selcolor+1);
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::pb_palettePaint(TObject *Sender)
{
	TCanvas* canvas = pb_palette->Canvas;

    canvas->Pen->Color = clBlack;
    for (int j=0; j<13; j++) {
	    for (int i=0; i<19; i++) {
            if (i+j*19>=MAX_PAL_ENTRY) goto g_exit;
            canvas->Brush->Color = GETPALCOL(i+j*19);
            canvas->Rectangle (i*GRIDSIZE, j*GRIDSIZE, (i+1)*GRIDSIZE+1, (j+1)*GRIDSIZE+1);
        }
	}

g_exit:
    DrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::pb_paletteMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    EraseCursor();
    SetXY (X/GRIDSIZE, Y/GRIDSIZE);
    DrawCursor();
	UpdateValues();

    if (doedit) {
        doedit = false;
        EditHSVClick(this);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::pb_paletteDblClick(TObject *Sender)
{
    doedit = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DefineColorsClick(TObject *Sender)
{
    TSelColorForm* frm = 0;
    try {
        frm = new TSelColorForm(Data->color, this);
        frm->ShowModal();
        Invalidate();
        Statusbar->Invalidate();
        delete frm;
    } catch (...) {
        if (frm) delete frm;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::EditRGBClick(TObject *Sender)
{
	COLORREF clr = Data->palette->GetColor(selcolor);
    try {
        TChooseRGBForm* frm = new TChooseRGBForm(this);
        frm->SelectColor (clr);
        if (frm->ShowModal()==mrOk) {
            clr = frm->GetSelectedColor();
        }
        delete frm;
    } catch (...) {
    }
    Data->palette->SetColor (selcolor, clr);
    UpdateValues();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::EditHSVClick(TObject *Sender)
{
	COLORREF clr = Data->palette->GetColor(selcolor);
    try {
        TChooseHSVForm* frm = new TChooseHSVForm(this);
        frm->SelectColor (clr);
        if (frm->ShowModal()==mrOk) {
            clr = frm->GetSelectedColor();
        }
        delete frm;
    } catch (...) {
    }
    Data->palette->SetColor (selcolor, clr);
    UpdateValues();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::RevertChangesClick(TObject *Sender)
{
    // Zurücksetzen aller Änderungen!
    for (int i=0; i<MAX_PAL_ENTRY; i++) {
        COLORREF col = oldpal[i];
        Data->palette->SetColor (i, col);
    }
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::CloseItClick(TObject *Sender)
{
    ModalResult = mrOk;
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::FormKeyPress(TObject *Sender, char &Key)
{
    if (Key==VK_ESCAPE) {
        ModalResult = mrCancel;
        return;
    }
}
/*-----------------------------------------------------------------*/
int __fastcall TSelColorForm::GetX()
{
    return selcolor % 19;
}
/*-----------------------------------------------------------------*/
int __fastcall TSelColorForm::GetY()
{
    return selcolor / 19;
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::SetXY (int _x, int _y)
{
    int idx = _y*19 + _x;
    if (idx<MAX_PAL_ENTRY) selcolor = idx;
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::MoveLeft (bool _bigskip/*=false*/)
{
    EraseCursor();
    int x = GetX();
    int y = GetY();
    if (_bigskip) x -= 5;
    else x--;
    if (x<0) {
        if (y>0) {
            y--;
            x = 19-1;
        } else
            x = 0;
    }
    SetXY (x, y);
    DrawCursor();
    UpdateValues();
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::MoveRight (bool _bigskip/*=false*/)
{
    EraseCursor();
    int x = GetX();
    int y = GetY();
    if (_bigskip) x += 5;
    else x++;
    if (x>=19) {
        if (y<13-1) {
            y++;
            x = 0;
        } else
            x = 19-1;
    }
    SetXY (x, y);
    DrawCursor();
    UpdateValues();
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::MoveUp (bool _bigskip/*=false*/)
{
    EraseCursor();
    int x = GetX();
    int y = GetY();
    if (_bigskip) y -= 5;
    else y--;
    if (y<0) {
        if (x>0) {
            x--;
            y = 13-1;
            if (x+y*19>=MAX_PAL_ENTRY) y--;
        } else
            y=0;
    }
    SetXY (x, y);
    DrawCursor();
    UpdateValues();
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::MoveDown (bool _bigskip/*=false*/)
{
    EraseCursor();
    int x = GetX();
    int y = GetY();
    if (_bigskip) y += 5;
    else y++;
    if (y>=13) {
        if (x<19) {
            x++;
            y = 0;
        } else
            y = 13-1;
    } else if (x+y*19>=MAX_PAL_ENTRY && x<19) {
        x++;
        y = 0;
    }
    SetXY (x, y);
    DrawCursor();
    UpdateValues();
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::RawDrawCursor (bool _draw/*=true*/)
{
    TCanvas* canvas = pb_palette->Canvas;
	canvas->Pen->Color = _draw ? clWhite : clBlack;
	int x = GetX() * GRIDSIZE;
	int y = GetY() * GRIDSIZE;
	canvas->MoveTo (x, y);
	canvas->LineTo (x+GRIDSIZE, y);
	canvas->LineTo (x+GRIDSIZE, y+GRIDSIZE);
	canvas->LineTo (x, y+GRIDSIZE);
	canvas->LineTo (x, y);
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::DrawCursor()
{
    RawDrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::EraseCursor()
{
    RawDrawCursor(false);
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
    if (Key==VK_LEFT) {
        MoveLeft (Shift.Contains(ssCtrl));
    } else if (Key==VK_RIGHT) {
        MoveRight (Shift.Contains(ssCtrl));
    } else if (Key==VK_UP) {
        MoveUp (Shift.Contains(ssCtrl));
    } else if (Key==VK_DOWN) {
        MoveDown (Shift.Contains(ssCtrl));
    } else if (Key==VK_RETURN || Key==' ') {
        if (Shift.Contains(ssCtrl)) EditRGBClick(this);
        else EditHSVClick (this);
    } else if (!Shift.Contains(ssAlt)) {
        if (Key=='r' || Key=='R') EditRGBClick(this);
        else if (Key=='h' || Key=='H') EditHSVClick(this);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSelColorForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Color definition", "")
    LANG_C_H(this, GE, "Farben definieren", "")
    LANG_C_H(labHue, EN, "Hue:", "")
    LANG_C_H(labHue, GE, "Farbton:", "")
    LANG_C_H(labSat, EN, "Saturation:", "")
    LANG_C_H(labSat, GE, "Sättigung:", "")
    LANG_C_H(labVal, EN, "Value:", "")
    LANG_C_H(labVal, GE, "Helligkeit:", "")
    LANG_C_H(labRed, EN, "Red:", "")
    LANG_C_H(labRed, GE, "Rot:", "")
    LANG_C_H(labGreen, EN, "Green:", "")
    LANG_C_H(labGreen, GE, "Grün:", "")
    LANG_C_H(labBlue, EN, "Blue:", "")
    LANG_C_H(labBlue, GE, "Blau:", "")
    LANG_C_H(Farben1, EN, "&Colors", "")
    LANG_C_H(Farben1, GE, "&Farben", "")
    LANG_C_H(EditRGB, EN, "Edit &RGB...", "")
    LANG_C_H(EditRGB, GE, "Bearbeiten &RGB...", "")
    LANG_C_H(EditHSV, EN, "Edit &HSV...", "")
    LANG_C_H(EditHSV, GE, "Bearbeiten &HSV...", "")
    LANG_C_H(RevertChanges, EN, "Re&vert changes", "")
    LANG_C_H(RevertChanges, GE, "Ä&nderungen verwerfen", "")
    LANG_C_H(CloseIt, EN, "&Close", "")
    LANG_C_H(CloseIt, GE, "&Schliessen", "")
}
/*-----------------------------------------------------------------*/

