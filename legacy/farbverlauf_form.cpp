/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File farbverlauf_form.cpp
 * Copyright (C) 1998-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#include <mem.h>
#include <math.h>
/*-----------------------------------------------------------------*/
#include "dbw3_form.h"
#include "assert.h"
#include "colors.h"
#include "palette.h"
#include "farbverlauf_form.h"
#include "farbauswahl_form.h"
#include "farbauswahl1_form.h"
#include "farbauswahl2_form.h"
#include "datamodule.h"
#include "dbw3_strings.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
COLORREF g_startfarbe = RGB(0, 0, 255);
COLORREF g_endfarbe = RGB(255, 255, 0);
/*-----------------------------------------------------------------*/
__fastcall TFarbverlaufForm::TFarbverlaufForm(TDBWFRM* _frm, TComponent* Owner)
: TForm(Owner), frm(_frm)
{
    LoadLanguage();

    startcolor = g_startfarbe;
    endcolor = g_endfarbe;
    for (int i=0; i<MAX_PAL_ENTRY; i++)
        table[i] = RGB(255,255,255);
    count = 0;
    nodividers->Checked = true;
    CreateFarbverlauf();
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::pbStartfarbeClick(TObject *Sender)
{
    bStartcolorClick (this);
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::pbEndfarbeClick(TObject *Sender)
{
    bEndcolorClick (this);
}
/*-----------------------------------------------------------------*/
COLORREF __fastcall TFarbverlaufForm::SelectColorRGB (COLORREF _col)
{
    try {
        TChooseRGBForm* frm = new TChooseRGBForm(this);
        frm->SelectColor (_col);
        if (frm->ShowModal()==mrOk) {
            _col = frm->GetSelectedColor();
        }
        delete frm;
    } catch (...) {
    }
    return _col;
}
/*-----------------------------------------------------------------*/
COLORREF __fastcall TFarbverlaufForm::SelectColorHSV (COLORREF _col)
{
    try {
        TChooseHSVForm* frm = new TChooseHSVForm(this);
        frm->SelectColor (_col);
        if (frm->ShowModal()==mrOk) {
            _col = frm->GetSelectedColor();
        }
        delete frm;
    } catch (...) {
    }
    return _col;
}
/*-----------------------------------------------------------------*/
COLORREF __fastcall TFarbverlaufForm::SelectColorPalette (COLORREF _col)
{
    try {
        TChoosePaletteForm* frm = new TChoosePaletteForm(this);
        frm->SelectColor (_col);
        if (frm->ShowModal()==mrOk) {
            _col = frm->GetSelectedColor();
        }
        delete frm;
    } catch (...) {
    }
    return _col;
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::bStartcolorClick(TObject *Sender)
{
    start = true;
    int caption = GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYBORDER);
    int border = GetSystemMetrics(SM_CXBORDER);
    PopupMenuFarbwahl->Popup(Left+border+bStartcolor->Left, Top+caption+bStartcolor->Top+bStartcolor->Height);
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::bEndcolorClick(TObject *Sender)
{
    start = false;
    int caption = GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYBORDER);
    int border = GetSystemMetrics(SM_CXBORDER);
    PopupMenuFarbwahl->Popup(Left+border+bEndcolor->Left, Top+caption+bEndcolor->Top+bEndcolor->Height);
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::ChooseColorHSVClick(TObject *Sender)
{
    if (start) startcolor = SelectColorHSV(startcolor);
    else endcolor = SelectColorHSV(endcolor);
    CreateFarbverlauf();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::ChooseColorRGBClick(TObject *Sender)
{
    if (start) startcolor = SelectColorRGB(startcolor);
    else endcolor = SelectColorRGB(endcolor);
    CreateFarbverlauf();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::ChooseColorPaletteClick(TObject *Sender)
{
    if (start) startcolor = SelectColorPalette(startcolor);
    else endcolor = SelectColorPalette(endcolor);
    CreateFarbverlauf();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::ChooseColorOtherClick(TObject *Sender)
{
    if (start) startcolor = endcolor;
    else endcolor = startcolor;
    CreateFarbverlauf();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::pbFarbverlaufClick(TObject *Sender)
{
    //
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::pbStartfarbePaint(TObject *Sender)
{
    pbStartfarbe->Canvas->Brush->Color = (TColor)startcolor;
    pbStartfarbe->Canvas->Pen->Color = clBlack;
    pbStartfarbe->Canvas->Rectangle (0, 0, pbStartfarbe->Width, pbStartfarbe->Height);
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::pbEndfarbePaint(TObject *Sender)
{
    pbEndfarbe->Canvas->Brush->Color = (TColor)endcolor;
    pbEndfarbe->Canvas->Pen->Color = clBlack;
    pbEndfarbe->Canvas->Rectangle (0, 0, pbEndfarbe->Width, pbEndfarbe->Height);
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::pbFarbverlaufPaint(TObject *Sender)
{
    int xx = pbFarbverlauf->Width;
    int dx = xx/(count+2);
    pbFarbverlauf->Canvas->Brush->Color = (TColor)startcolor;
    pbFarbverlauf->Canvas->Pen->Color = nodividers->Checked ? (TColor)startcolor : clBlack;
    pbFarbverlauf->Canvas->Rectangle (0, 0, dx+1, pbFarbverlauf->Height);
    for (int i=0; i<count; i++) {
        pbFarbverlauf->Canvas->Brush->Color = (TColor)table[i];
        pbFarbverlauf->Canvas->Pen->Color = nodividers->Checked ? (TColor)table[i] : clBlack;
        pbFarbverlauf->Canvas->Rectangle ((i+1)*dx, 0, (i+2)*dx+1, pbFarbverlauf->Height);
    }
    pbFarbverlauf->Canvas->Brush->Color = (TColor)endcolor;
    pbFarbverlauf->Canvas->Pen->Color = nodividers->Checked ? (TColor)endcolor : clBlack;
    pbFarbverlauf->Canvas->Rectangle ((count+1)*dx, 0, (count+2)*dx+1, pbFarbverlauf->Height);
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::CreateFarbverlauf()
{
    try {
        if (Abstufungen->Text=="") return;
        int abstufungen;
        try { abstufungen = StrToInt(Abstufungen->Text); } catch (...) { abstufungen = 20; }
        if (abstufungen<1 || abstufungen>150) {
            Application->MessageBox (MAXCOLORSTEPS.c_str(), APP_TITLE, MB_OK);
            return;
        }
        if (ModelRGB->Checked) FarbverlaufRGB (abstufungen);
        else FarbverlaufHSV (abstufungen);
    } catch (...) {
        count = 0;
    }

}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::FarbverlaufRGB (int _abstufungen)
{
    int d = 1;
    double m = double(/*Gewichtung->Max+Gewichtung->Min-*/Gewichtung->Position)/(Gewichtung->Max+Gewichtung->Min);
    count = 0;
    for (int i=0; i<_abstufungen; i++) {
        double x = double(i+1)/(_abstufungen+1);
        double t;
        if (m>0.5) t = (2*d-4*m)*x*x + (4*m-d)*x;
        else if (m>=0.25 && m<=0.5) t = (2*d-4*m)*x*x + (4*m-d)*x;
        else if (m>=0.125 && m<0.25) t = (2*d-8*m)*x*x*x + (8*m-d)*x*x;
        else dbw3_assert(false);
        // Zwischenfarbe interpolieren im RGB-Farbraum
        // Rot
        int r1 = GetRValue(startcolor);
        int rd = GetRValue(endcolor)-GetRValue(startcolor);
        int r = r1 + int(double(rd)*t);
        // Grün
        int g1 = GetGValue(startcolor);
        int gd = GetGValue(endcolor)-GetGValue(startcolor);
        int g = g1 + int(double(gd)*t);
        // Blau
        int b1 = GetBValue(startcolor);
        int bd = GetBValue(endcolor)-GetBValue(startcolor);
        int b = b1 + int(double(bd)*t);
        // Neue Farbe:
        table[count++] = RGB(r,g,b);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::FarbverlaufHSV (int _abstufungen)
{
    int d = 1;
    double m = double(/*Gewichtung->Max+Gewichtung->Min-*/Gewichtung->Position)/(Gewichtung->Max+Gewichtung->Min);
    count = 0;
    for (int i=0; i<_abstufungen; i++) {
        double x = double(i+1)/(_abstufungen+1);
        double t;
        if (m>0.5) t = (2*d-4*m)*x*x + (4*m-d)*x;
        else if (m>=0.25 && m<=0.5) t = (2*d-4*m)*x*x + (4*m-d)*x;
        else if (m>=0.125 && m<0.25) t = (2*d-8*m)*x*x*x + (8*m-d)*x*x;
        else dbw3_assert(false);
        // Zwischenfarbe interpolieren im HSV-Farbraum
        float h1, s1, v1, h2, s2, v2;
        RGB2HSV (GetRValue(startcolor), GetGValue(startcolor), GetBValue(startcolor), h1, s1, v1);
        RGB2HSV (GetRValue(endcolor), GetGValue(endcolor), GetBValue(endcolor), h2, s2, v2);
        // Hue
        float h = h1 + (h2-h1)*t;
        // Saturation
        float s = s1 + (s2-s1)*t;
        // Value
        float v = v1 + (v2-v1)*t;
        // Neue Farbe:
        int r, g, b;
        HSV2RGB (h, s, v, r, g, b);
        table[count++] = RGB(r,g,b);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::AbstufungenChange(TObject *Sender)
{
    CreateFarbverlauf();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::nodividersClick(TObject *Sender)
{
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::bOkClick(TObject *Sender)
{
    if (count!=0) {
        // Ist Index gültig?
        int index = 0;
        if (Position->Text!="") {
            try { index = StrToInt(Position->Text); } catch (...) { index = 0; }
        }
        if (index<1 && index>MAX_PAL_ENTRY) {
            Application->MessageBox (PALETTEIDXRANGE.c_str(), APP_TITLE, MB_OK);
            return;
        }
        // Daten in palette speichern
        Data->palette->SetColor (index-1, startcolor);
        Data->palette->SetColor (index-1+count+1, endcolor);
        for (int i=index; i<min(index+count, MAX_PAL_ENTRY); i++)
            Data->palette->SetColor (i, table[i-index]);
        frm->Invalidate();
        g_startfarbe = startcolor;
        g_endfarbe = endcolor;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::GewichtungChange(TObject *Sender)
{
    CreateFarbverlauf();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::ModelRGBClick(TObject *Sender)
{
    CreateFarbverlauf();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::ModelHSVClick(TObject *Sender)
{
    CreateFarbverlauf();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFarbverlaufForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Create color blending", "")
    LANG_C_H(this, GE, "Farbverlauf erstellen", "")
    LANG_C_H(lStufen, EN, "S&teps:", "")
    LANG_C_H(lStufen, GE, "&Abstufungen:", "")
    LANG_C_H(lPosition, EN, "&Palette index:", "")
    LANG_C_H(lPosition, GE, "&Palettenindex:", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
    LANG_C_H(nodividers, EN, "N&o Dividers", "")
    LANG_C_H(nodividers, GE, "&Ohne Unterteilung", "")
    LANG_C_H(labGewichtung, EN, "&Weighting:", "")
    LANG_C_H(labGewichtung, GE, "&Gewichtung:", "")
    LANG_C_H(gbColormodel, EN, "Color model for Interpolation", "")
    LANG_C_H(gbColormodel, GE, "Farbmodell für Interpolation", "")
    LANG_C_H(bStartcolor, EN, "&Start color", "")
    LANG_C_H(bStartcolor, GE, "&Startfarbe", "")
    LANG_C_H(bEndcolor, EN, "&End color", "")
    LANG_C_H(bEndcolor, GE, "&Endfarbe", "")
    LANG_C_H(ChooseColorHSV, EN, "Choose color with &HSV model", "")
    LANG_C_H(ChooseColorHSV, GE, "Farbe mit &HSV-Modell wählen", "")
    LANG_C_H(ChooseColorRGB, EN, "Choose color with &RGB model", "")
    LANG_C_H(ChooseColorRGB, GE, "Farbe mit &RGB-Modell wählen", "")
    LANG_C_H(ChooseColorPalette, EN, "Choose color from &palette", "")
    LANG_C_H(ChooseColorPalette, GE, "Farbe aus &Palette wählen", "")
    LANG_C_H(ChooseColorOther, EN, "&Copy other color", "")
    LANG_C_H(ChooseColorOther, GE, "&Andere Farbe kopieren", "")
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FarbverlaufClick(TObject *Sender)
{
    try {
        TFarbverlaufForm* frm = new TFarbverlaufForm (this, this);
        frm->ShowModal();
        delete frm;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/

