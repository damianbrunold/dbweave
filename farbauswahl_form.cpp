/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File farbauswahl_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#include <math.h>
#include <stdio.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "farbauswahl_form.h"
#include "assert.h"
#include "colors.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TChooseHSVForm::TChooseHSVForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();

    try {
        bmpHue = new Graphics::TBitmap;
        bmpHue->Width = pbFarbkreis->Width;
        bmpHue->Height = pbFarbkreis->Height;
    } catch (...) {
        bmpHue = 0;
    }

    try {
        bmpSatVal = new Graphics::TBitmap;
        bmpSatVal->Width = pbSatur->Width;
        bmpSatVal->Height = pbSatur->Height;
    } catch (...) {
        bmpSatVal = 0;
    }
}
/*-----------------------------------------------------------------*/
__fastcall TChooseHSVForm::~TChooseHSVForm()
{
    delete bmpHue;
    delete bmpSatVal;
}
/*-----------------------------------------------------------------*/
COLORREF __fastcall TChooseHSVForm::GetSelectedColor()
{
    float h = hue;
    float s = float(1)/255*sat;
    float v = float(1)/255*val;
    int r, g, b;
    HSV2RGB (h, s, v, r, g, b);
    dbw3_assert(r<=255); dbw3_assert(r>=0);
    dbw3_assert(g<=255); dbw3_assert(g>=0);
    dbw3_assert(b<=255); dbw3_assert(b>=0);
    return RGB(r, g, b);
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseHSVForm::SelectColor (COLORREF _color)
{
    int r = GetRValue(_color);
    int g = GetGValue(_color);
    int b = GetBValue(_color);
    float h;
    float s;
    float v;
    RGB2HSV (r, g, b, h, s, v);
    hue = short(h);
    sat = short(255.0*s);
    val = short(255.0*v);
    tbFarbton->Position = 360-hue;
    tbSaettigung->Position = sat;
    tbHelligkeit->Position = val;
    UpdateValues();
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseHSVForm::UpdateValues(bool _sat/*=true*/, bool _val/*=true*/)
{
    // Hue
    HueVal->Caption = IntToStr(tbFarbton->Position)+"°";

    // Sat
    if (_sat) {
        sat = short(tbSaettigung->Position);
        char sz[10];
        sprintf (sz, "%.3f", 1.0*sat/255);
        SatVal->Caption = sz;
    }

    // Val
    if (_val) {
        val = short(tbHelligkeit->Position);
        char sz[10];
        sprintf (sz, "%.3f", 1.0*val/255);
        ValVal->Caption = sz;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseHSVForm::pbFarbkreisMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    int dy = Y-pbFarbkreis->Height/2;
    int dx = X-pbFarbkreis->Width/2;
    if (dx==0) dx = 1;
    float phi = atan2(double(dy), double(dx));
    if (phi<0.0) phi += 2.0*3.14159265;
    hue = short(360.0*phi/(2.0*3.14159265));
    tbFarbton->Position = 360-hue;
    UpdateValues();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseHSVForm::pbFarbkreisPaint(TObject *Sender)
{
    TCanvas* pCanvas = bmpHue->Canvas;
    pCanvas->Brush->Color = Color;
    pCanvas->Pen->Color = Color;
    pCanvas->Rectangle (0, 0, bmpHue->Width, bmpHue->Height);
    const short slices = 100;
    const float dphi = 2.0*3.14159265/slices/2;
    short save_sat = sat;
    short save_val = val;
    short save_hue = hue;
//    sat = val = 255;
    for (short i=0; i<=slices; i++) {
        hue = short(360*i/slices);
        float phi = 2.0*3.14159265*i/slices;
        int x1 = pbFarbkreis->Width/2 + int(cos(phi-dphi+2.0*3.141592)*pbFarbkreis->Width/2);
        int y1 = pbFarbkreis->Height/2 + int(sin(phi-dphi+2.0*3.141592)*pbFarbkreis->Height/2);
        int x2 = pbFarbkreis->Width/2 + int(cos(phi+dphi)*pbFarbkreis->Width/2);
        int y2 = pbFarbkreis->Height/2 + int(sin(phi+dphi)*pbFarbkreis->Height/2);
        COLORREF col = GetSelectedColor();
        pCanvas->Brush->Color = TColor(col);
        pCanvas->Pen->Color = TColor(col);
        pCanvas->Pie (0, 0, pbFarbkreis->Width, pbFarbkreis->Height, x2, y2, x1, y1);
    }
    sat = save_sat;
    val = save_val;
    hue = save_hue;

    float phi = 2.0*3.14159265/360.0*float(hue);
    int x1 = pbFarbkreis->Width/2 + int(cos(phi)*pbFarbkreis->Width/2);
    int y1 = pbFarbkreis->Height/2 + int(sin(phi)*pbFarbkreis->Height/2);
    int x2 = pbFarbkreis->Width/2 + int(cos(phi)*(pbFarbkreis->Width/2-10));
    int y2 = pbFarbkreis->Height/2 + int(sin(phi)*(pbFarbkreis->Height/2-10));
    pCanvas->Pen->Color = val>120 ? clBlack : clWhite;
    pCanvas->MoveTo (x1, y1);
    pCanvas->LineTo (x2, y2);

    pbFarbkreis->Canvas->Draw (0, 0, bmpHue);
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseHSVForm::pbSaturMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    const short xpieces = 13;
    const short ypieces = 21;
    int dx = pbSatur->Width/xpieces;
    int dy = pbSatur->Height/ypieces;
    sat = short(255*X/(xpieces*dx));
    val = short(255*(ypieces*dy-Y)/(ypieces*dy));
    tbHelligkeit->Position = val;
    tbSaettigung->Position = sat;
    UpdateValues();
    InvalidateRanges();
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseHSVForm::pbSaturPaint(TObject *Sender)
{
    TCanvas* pCanvas = bmpSatVal->Canvas;
    const short xpieces = 13;
    const short ypieces = 21;
    int dx = pbSatur->Width/xpieces;
    int dy = pbSatur->Height/ypieces;
    short save_sat = sat;
    short save_val = val;
    short save_hue = hue;
    for (short i=0; i<xpieces; i++) {
        sat = short(255*i/xpieces);
        if (sat==0) sat = 1;
        for (short j=0; j<ypieces; j++) {
            val = short(255*j/ypieces);
            COLORREF col = GetSelectedColor();
            pCanvas->Brush->Color = TColor(col);
            pCanvas->Pen->Color = TColor(col);
            int x = i*dx;
            int y = ypieces*dy - j*dy;
            pCanvas->Rectangle (x, y-dx, x+dx, y);
        }
    }
    sat = save_sat;
    val = save_val;
    hue = save_hue;

    int x = xpieces*dx*sat/255;
    int y = ypieces*dy - ypieces*dy*val/255;
    pCanvas->Pen->Color = val>120 ? clBlack : clWhite;
    pCanvas->Arc (x-3, y-3, x+3, y+3, 0, 0, 0, 0);

    pbSatur->Canvas->Draw (0, 0, bmpSatVal);
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseHSVForm::pbMusterPaint(TObject *Sender)
{
    TColor col = TColor(GetSelectedColor());
    pbMuster->Canvas->Brush->Color = col;
    pbMuster->Canvas->Pen->Color = col;
    pbMuster->Canvas->Rectangle (0, 0, pbMuster->Width, pbMuster->Height);
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseHSVForm::tbFarbtonChange(TObject *Sender)
{
    hue = short(360-tbFarbton->Position);
    tbFarbton->Hint = IntToStr(360-hue);
    UpdateValues();
    InvalidateRanges();
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseHSVForm::tbSaettigungChange(TObject *Sender)
{
    sat = short(tbSaettigung->Position);
    char sz[10];
    sprintf (sz, "%.3f", 1.0*sat/255);
    tbSaettigung->Hint = sz;
    UpdateValues(true, false);
    InvalidateRanges();
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseHSVForm::tbHelligkeitChange(TObject *Sender)
{
    val = short(tbHelligkeit->Position);
    char sz[10];
    sprintf (sz, "%.3f", 1.0*val/255);
    tbHelligkeit->Hint = sz;
    UpdateValues(false, true);
    InvalidateRanges();
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseHSVForm::InvalidateRanges()
{
    pbFarbkreis->Invalidate();
    pbSatur->Invalidate();
    pbMuster->Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TChooseHSVForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Colors HSV", "")
    LANG_C_H(this, GE, "Farbauswahl HSV", "")
    LANG_C_H(lFarbton, EN, "Color hues", "")
    LANG_C_H(lFarbton, GE, "Farbtonkreis", "")
    LANG_C_H(lSaettigung, EN, "Saturation and Value", "")
    LANG_C_H(lSaettigung, GE, "Sättigung und Helligkeit", "")
    LANG_C_H(lFarbmuster, EN, "Color sample", "")
    LANG_C_H(lFarbmuster, GE, "Farbmuster", "")
    LANG_C_H(l2Farbton, EN, "&Hue:", "")
    LANG_C_H(l2Farbton, GE, "&Farbton:", "")
    LANG_C_H(l2Saettigung, EN, "&Saturation:", "")
    LANG_C_H(l2Saettigung, GE, "&Sättigung:", "")
    LANG_C_H(l2Helligkeit, EN, "&Value:", "")
    LANG_C_H(l2Helligkeit, GE, "&Helligkeit:", "")
    LANG_C_H(bOK, EN, "OK", "")
    LANG_C_H(bOK, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/

