/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File farbauswahl2_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "palette.h"
#include "datamodule.h"
#include "farbauswahl2_form.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TChoosePaletteForm::TChoosePaletteForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();

    DRAWSIZE = pbColors->Width / 16;
    if (DRAWSIZE > pbColors->Height/16) DRAWSIZE = pbColors->Height/16;

    ClientWidth = pbColors->Width + 8 + DRAWSIZE;
    ClientHeight = bOk->Top + bOk->Height + 12;

    index = 0;
    ActiveControl = DummyEdit;
}
/*-----------------------------------------------------------------*/
void __fastcall TChoosePaletteForm::_RawDrawCursor (bool _erase/*=false*/)
{
    int x = (index/GRIDSIZE)*DRAWSIZE;
    int y = (index%GRIDSIZE)*DRAWSIZE;
    pbColors->Canvas->Pen->Color = _erase ? (ActiveControl==DummyEdit ? clBlack : clDkGray)
                                          : clWhite;
    pbColors->Canvas->MoveTo (x, y);
    pbColors->Canvas->LineTo (x+DRAWSIZE, y);
    pbColors->Canvas->LineTo (x+DRAWSIZE, y+DRAWSIZE);
    pbColors->Canvas->LineTo (x, y+DRAWSIZE);
    pbColors->Canvas->LineTo (x, y);
}
/*-----------------------------------------------------------------*/
void __fastcall TChoosePaletteForm::DrawCursor()
{
    _RawDrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TChoosePaletteForm::EraseCursor()
{
    _RawDrawCursor (true);
}
/*-----------------------------------------------------------------*/
void __fastcall TChoosePaletteForm::SetCursor (int _scrx, int _scry)
{
    int i = _scrx/DRAWSIZE;
    int j = _scry/DRAWSIZE;
    if (i*GRIDSIZE+j >= MAX_PAL_ENTRY) return;
    index = i*GRIDSIZE+j;
}
/*-----------------------------------------------------------------*/
void __fastcall TChoosePaletteForm::DummyEditKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
    int x = index/GRIDSIZE;
    int y = index%GRIDSIZE;

    switch (Key) {
        case VK_LEFT:
            if (x>0) --x;
            break;

        case VK_RIGHT:
            if (x<15) ++x;
            break;

        case VK_UP:
            if (y>0) --y;
            else if (x>0) { y=15; --x; }
            break;

        case VK_DOWN:
            if (y<15) ++y;
            else if (x<15) { y=0; ++x; }
            break;
    }

    int newcol = x*GRIDSIZE+y;
    if (newcol!=index && newcol<MAX_PAL_ENTRY) {
        EraseCursor();
        index = x*GRIDSIZE+y;
        DrawCursor();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TChoosePaletteForm::pbColorsPaint(TObject *Sender)
{
    pbColors->Canvas->Pen->Color = clBlack;
    for (int i=0; i<GRIDSIZE; i++)
        for (int j=0; j<GRIDSIZE; j++) {
            if (i*GRIDSIZE+j>=MAX_PAL_ENTRY) goto g_exit;
            pbColors->Canvas->Brush->Color = TColor(Data->palette->GetColor(i*GRIDSIZE+j));
            pbColors->Canvas->Rectangle (i*DRAWSIZE, j*DRAWSIZE, (i+1)*DRAWSIZE+1, (j+1)*DRAWSIZE+1);
        }

g_exit:
    DrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TChoosePaletteForm::bOkClick(TObject *Sender)
{
    //
}
/*-----------------------------------------------------------------*/
COLORREF __fastcall TChoosePaletteForm::GetSelectedColor()
{
    return Data->palette->GetColor (index);
}
/*-----------------------------------------------------------------*/
int __fastcall TChoosePaletteForm::GetSelectedIndex()
{
    return index;
}
/*-----------------------------------------------------------------*/
void __fastcall TChoosePaletteForm::SelectColor (COLORREF _color)
{
    EraseCursor();
    for (int i=0; i<MAX_PAL_ENTRY; i++)
        if (_color==Data->palette->GetColor(i)) {
            index = i;
            DrawCursor();
            return;
        }
    index = 0;
    DrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TChoosePaletteForm::DummyEditEnter(TObject *Sender)
{
    // Cursor aktiv darstellen
    DrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TChoosePaletteForm::DummyEditExit(TObject *Sender)
{
    // Cursor inaktiv darstellen
    DrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TChoosePaletteForm::pbColorsMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (Button==mbLeft) {
        EraseCursor();
        SetCursor(X, Y);
        DrawCursor();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TChoosePaletteForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Colors Palette", "")
    LANG_C_H(this, GE, "Farbauswahl Palette", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/

