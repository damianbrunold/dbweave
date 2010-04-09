/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File importbmp_form.cpp
 * Copyright (C) 1999-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#include <stdlib.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "importbmp_form.h"
#include "rangecolors.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TImportBmpForm::TImportBmpForm(TComponent* Owner, String _filename)
: TForm(Owner)
{
    LoadLanguage();
    try {
        Picture = new TPicture;
        Picture->LoadFromFile (_filename);
        Bitmap->Picture = Picture;
    } catch (...) {
    }
    needrepaint = false;
    data = NULL;
    BuildColorTable();
}
/*-----------------------------------------------------------------*/
__fastcall TImportBmpForm::~TImportBmpForm()
{
    Bitmap->Picture = NULL;
    delete Picture;
    delete[] data;
}
/*-----------------------------------------------------------------*/
void __fastcall TImportBmpForm::ResolutionChange(TObject *Sender)
{
    needrepaint = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TImportBmpForm::PreviewPaint(TObject *Sender)
{
    // Höhe und Breite Bitmap bestimmen
    int width = Picture->Width;
    int height = Picture->Height;

    // Durch Auflösung ergibt Anzahl Felder Preview
    int res = atoi (Resolution->Text.c_str());
    if (res==0) return;
    int dx = width / res;
    int dy = height / res;

    // Doppelschleife durch Felder
    for (int i=0; i<dx; i++)
        for (int j=0; j<dy; j++) {
            int xx = (i*2+1)*res/2;
            int yy = (j*2+1)*res/2;
            TColor col = GetRangeCol(Picture->Bitmap->Canvas->Pixels[xx][yy]);
            Preview->Canvas->Pen->Color = res<3 ? col : clDkGray;
            Preview->Canvas->Brush->Color = col;
            Preview->Canvas->Rectangle (i*res, j*res, (i+1)*res+1, (j+1)*res+1);
        }

    needrepaint = false;
}
/*-----------------------------------------------------------------*/
void __fastcall TImportBmpForm::lazytimerTimer(TObject *Sender)
{
    if (needrepaint) Preview->Repaint();
}
/*-----------------------------------------------------------------*/
void __fastcall TImportBmpForm::FormActivate(TObject *Sender)
{
    lazytimer->Enabled = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TImportBmpForm::FormDeactivate(TObject *Sender)
{
    lazytimer->Enabled = false;
}
/*-----------------------------------------------------------------*/
void __fastcall TImportBmpForm::bOkClick(TObject *Sender)
{
    // Daten bereitstellen

    // Höhe und Breite Bitmap bestimmen
    int width = Picture->Width;
    int height = Picture->Height;

    // Durch Auflösung ergibt Anzahl Felder Preview
    int res = atoi (Resolution->Text.c_str());
    if (res==0) return;
    x = width / res;
    y = height / res;

    // Feld anlegen
    data = new char[x*y];

    // Doppelschleife durch Felder
    for (int i=0; i<x; i++)
        for (int j=y-1; j>=0; j--) {
            int xx = (i*res + (i+1)*res) / 2;
            int yy = (j*res + (j+1)*res) / 2;
            data[i+(y-1-j)*x] = GetRange(Picture->Bitmap->Canvas->Pixels[xx][yy]);
        }

    ModalResult = mrOk;
}
/*-----------------------------------------------------------------*/
void __fastcall TImportBmpForm::BuildColorTable()
{
    used = 0;
    bkground = Picture->Bitmap->Canvas->Pixels[0][0];
    for (int i=0; i<Picture->Width; i++)
        for (int j=0; j<Picture->Height; j++) {
            TColor col = Picture->Bitmap->Canvas->Pixels[i][j];
            if (col!=bkground) {
                bool found = false;
                for (int k=0; k<used; k++)
                    if (colors[k]==col) {
                        found = true;
                        break;
                    }
                if (!found && used<=9)
                    colors[used++] = col;
                if (used>9) goto g_exit;
            }
        }
g_exit:
    ;
}
/*-----------------------------------------------------------------*/
TColor __fastcall TImportBmpForm::GetRangeCol (TColor _col)
{
    int range = GetRange (_col);
    if (range<1) return clWhite;
    else if (range<10) return GetRangeColor(range);
    else return clWhite;
}
/*-----------------------------------------------------------------*/
char __fastcall TImportBmpForm::GetRange (TColor _col)
{
    for (char i=0; i<used; i++)
        if (colors[i]==_col) return char(i+1);
    return 0;
}
/*-----------------------------------------------------------------*/
void __fastcall TImportBmpForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Import picture", "")
    LANG_C_H(this, GE, "Bild importieren", "")
    LANG_C_H(lResolution, EN, "&Resolution:", "")
    LANG_C_H(lResolution, GE, "&Auflösung:", "")
    LANG_C_H(lBitmap, EN, "Picture template:", "")
    LANG_C_H(lBitmap, GE, "Bildvorlage:", "")
    LANG_C_H(lPreview, EN, "Preview:", "")
    LANG_C_H(lPreview, GE, "Vorschau:", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/

