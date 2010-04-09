/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File farbauswahl1_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "farbauswahl1_form.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TChooseRGBForm::TChooseRGBForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
}
/*-----------------------------------------------------------------*/
COLORREF __fastcall TChooseRGBForm::GetSelectedColor()
{
    dbw3_assert (red>=0 && red<=255);
    dbw3_assert (green>=0 && green<=255);
    dbw3_assert (blue>=0 && blue<=255);
    return RGB(red, green, blue);
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseRGBForm::SelectColor (COLORREF _color)
{
    red = GetRValue (_color);
    green = GetGValue (_color);
    blue = GetBValue (_color);
    tbRed->Position = red;
    tbGreen->Position = green;
    tbBlue->Position = blue;
    UpdateValues();
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseRGBForm::UpdateValues()
{
    RedVal->Caption = IntToStr (red);
    GreenVal->Caption = IntToStr (green);
    BlueVal->Caption = IntToStr (blue);
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseRGBForm::pbRedPaint(TObject *Sender)
{
    const int step = 3;
    for (int i=0; i<pbRed->Height; i+=step) {
        int r = 255*i/pbRed->Height;
        pbRed->Canvas->Brush->Color = TColor(RGB(r, 0, 0));
        pbRed->Canvas->Pen->Color = TColor(RGB(r, 0, 0));
        pbRed->Canvas->Rectangle (0, i, pbRed->Width, i+step);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseRGBForm::pbGreenPaint(TObject *Sender)
{
    const int step = 3;
    for (int i=0; i<pbGreen->Height; i+=step) {
        int g = 255*i/pbGreen->Height;
        pbGreen->Canvas->Brush->Color = TColor(RGB(0, g, 0));
        pbGreen->Canvas->Pen->Color = TColor(RGB(0, g, 0));
        pbGreen->Canvas->Rectangle (0, i, pbGreen->Width, i+step);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseRGBForm::pbBluePaint(TObject *Sender)
{
    const int step = 3;
    for (int i=0; i<pbBlue->Height; i+=step) {
        int b = 255*i/pbBlue->Height;
        pbBlue->Canvas->Brush->Color = TColor(RGB(0, 0, b));
        pbBlue->Canvas->Pen->Color = TColor(RGB(0, 0, b));
        pbBlue->Canvas->Rectangle (0, i, pbBlue->Width, i+step);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseRGBForm::pbFarbmusterPaint(TObject *Sender)
{
    pbFarbmuster->Canvas->Brush->Color = TColor(RGB(red, green, blue));
    pbFarbmuster->Canvas->Pen->Color = TColor(RGB(red, green, blue));
    pbFarbmuster->Canvas->Rectangle (0, 0, pbFarbmuster->Width, pbFarbmuster->Height);
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseRGBForm::tbRedChange(TObject *Sender)
{
    red = tbRed->Position;
    tbRed->Hint = IntToStr (tbRed->Position);
    UpdateValues();
    pbFarbmuster->Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseRGBForm::tbGreenChange(TObject *Sender)
{
    green = tbGreen->Position;
    tbGreen->Hint = IntToStr (tbGreen->Position);
    UpdateValues();
    pbFarbmuster->Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseRGBForm::tbBlueChange(TObject *Sender)
{
    blue = tbBlue->Position;
    tbBlue->Hint = IntToStr (tbBlue->Position);
    UpdateValues();
    pbFarbmuster->Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseRGBForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Colors RGB", "")
    LANG_C_H(this, GE, "Farbauswahl RGB", "")
    LANG_C_H(lRot, EN, "&Red:", "")
    LANG_C_H(lRot, GE, "&Rot:", "")
    LANG_C_H(lGruen, EN, "&Green:", "")
    LANG_C_H(lGruen, GE, "&Grün:", "")
    LANG_C_H(lBlau, EN, "&Blue:", "")
    LANG_C_H(lBlau, GE, "&Blau:", "")
    LANG_C_H(lMuster, EN, "Color sample", "")
    LANG_C_H(lMuster, GE, "Farbmuster", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/

