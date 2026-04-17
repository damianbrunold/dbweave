/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File farbpalette_form.cpp
 * Copyright (C) 1998-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "farbpalette_form.h"
#include "palette.h"
#include "dbw3_form.h"
#include "datamodule.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
bool palette_created = false;
TFarbPalette *FarbPalette;
extern PACKAGE TDBWFRM *DBWFRM;
/*-----------------------------------------------------------------*/
#define GETPALCOL(col) \
    ((TColor)Data->palette->GetColor(col))
/*-----------------------------------------------------------------*/
#define GRIDSIZE 16
/*-----------------------------------------------------------------*/
__fastcall TFarbPalette::TFarbPalette(TComponent* Owner)
: TForm(Owner)
{
    palette_created = true;
    ReloadLanguage();

    DRAWSIZE = ClientWidth/16;
    if (DRAWSIZE > ClientHeight/16) DRAWSIZE = ClientHeight/16;
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbPalette::UpdatePalette()
{
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbPalette::_RawDrawCursor (bool _erase/*=false*/)
{
    int x = (Data->color/GRIDSIZE)*DRAWSIZE;
    int y = (Data->color%GRIDSIZE)*DRAWSIZE;
    Canvas->Pen->Color = _erase ? clBlack : clWhite;
    Canvas->MoveTo (x, y);
    Canvas->LineTo (x+DRAWSIZE, y);
    Canvas->LineTo (x+DRAWSIZE, y+DRAWSIZE);
    Canvas->LineTo (x, y+DRAWSIZE);
    Canvas->LineTo (x, y);
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbPalette::DrawCursor()
{
    _RawDrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbPalette::EraseCursor()
{
    _RawDrawCursor (true);
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbPalette::SetCursor (int _scrx, int _scry)
{
    int i = _scrx/DRAWSIZE;
    int j = _scry/DRAWSIZE;
    if (i*GRIDSIZE+j >= MAX_PAL_ENTRY) return;
    Data->color = (unsigned char)(i*GRIDSIZE+j);
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbPalette::FormPaint(TObject *Sender)
{
    Canvas->Pen->Color = clBlack;
    for (int i=0; i<GRIDSIZE; i++)
        for (int j=0; j<GRIDSIZE; j++) {
            if (i*GRIDSIZE+j>=MAX_PAL_ENTRY) goto g_exit;
            Canvas->Brush->Color = GETPALCOL(i*GRIDSIZE+j);
            Canvas->Rectangle (i*DRAWSIZE, j*DRAWSIZE, (i+1)*DRAWSIZE+1, (j+1)*DRAWSIZE+1);
        }

g_exit:
    DrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbPalette::FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (Button==mbLeft) {
        EraseCursor();
        SetCursor(X, Y);
        DrawCursor();
        DBWFRM->Statusbar->Invalidate();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbPalette::FormClose(TObject *Sender, TCloseAction &Action)
{
    DBWFRM->ViewFarbpalette->Checked = false;
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbPalette::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
    int x = Data->color/GRIDSIZE;
    int y = Data->color%GRIDSIZE;

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
    if (newcol!=Data->color && newcol<MAX_PAL_ENTRY) {
        EraseCursor();
        Data->color = (unsigned char)(x*GRIDSIZE+y);
        DrawCursor();
        DBWFRM->Statusbar->Invalidate();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbPalette::FormKeyPress(TObject *Sender, char &Key)
{
    if (Key==VK_ESCAPE) {
        ::PostMessage (Handle, WM_CLOSE, 0, 0L);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbPalette::ReloadLanguage()
{
    if (!palette_created) return;
    LANG_C_H(this, EN, "Color palette", "")
    LANG_C_H(this, GE, "Farbpalette", "")
}
/*-----------------------------------------------------------------*/

