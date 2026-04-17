/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File overview_form.cpp
 * Copyright (C) 1999-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#include <vcl/printers.hpp>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "overview_form.h"
#include "dbw3_form.h"
#include "datamodule.h"
#include "palette.h"
#include "rangecolors.h"
#include "dbw3_strings.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
#define GETPALCOL(col) \
    ((TColor)Data->palette->GetColor(col))
/*-----------------------------------------------------------------*/
__fastcall TOverviewForm::TOverviewForm(TComponent* Owner, TDBWFRM* _frm)
: TForm(Owner), frm(_frm)
{
    LoadLanguage();

    int ddx, ddy;

    // Zoom dynamisch anhand Range ermitteln...
    if (frm->kette.count()!=0) ddx = ClientWidth / frm->kette.count() + 1;
    else ddx = 0;
    if (frm->schuesse.count()!=0) ddy = (ClientHeight-Toolbar->Height) / frm->schuesse.count() + 1;
    else ddy = 0;

    if (ddx<=0) ddx = 1;
    if (ddy<=0) ddy = 1;
    if (ddx>10) ddx = 10;
    if (ddy>10) ddy = 10;

    zoom = min (ddx, ddy);

    if (zoom==1) {
        SBZoomIn->Enabled = true;
        SBZoomOut->Enabled = false;
    }

    if (zoom==10) {
        SBZoomIn->Enabled = false;
        SBZoomOut->Enabled = true;
    }

    grid = true;
    SBGrid->Down = grid;
}
/*-----------------------------------------------------------------*/
void __fastcall TOverviewForm::FormPaint(TObject *Sender)
{
    if (frm->kette.a==-1 || frm->schuesse.a==-1) return;

    int gw = zoom;
    int gh = zoom;

    if (frm->faktor_kette>frm->faktor_schuss) gw = int(double(gw)*frm->faktor_kette/frm->faktor_schuss);
    else gh = int(double(gh)*frm->faktor_schuss/frm->faktor_kette);

    if (gw>frm->gewebe.gw || gh>frm->gewebe.gh) {
        gw = frm->gewebe.gw;
        gh = frm->gewebe.gh;
    }

    int mx = min(ClientWidth/gw, frm->kette.count());
    int my = min((ClientHeight-Toolbar->Height)/gh, frm->schuesse.count());

    if (frm->GewebeNormal->Checked) {
        for (int i=frm->kette.a; i<frm->kette.a+mx; i++)
            for (int j=frm->schuesse.a; j<frm->schuesse.a+my; j++){
                if (frm->gewebe.feld.Get(i, j)>0) {
                    Canvas->Pen->Color = GetRangeColor(frm->gewebe.feld.Get(i, j));
                    Canvas->Brush->Color = Canvas->Pen->Color;
                } else {
                    Canvas->Pen->Color = clBtnFace;
                    Canvas->Brush->Color = Canvas->Pen->Color;
                }
                int x;
                if (frm->righttoleft) x = ClientWidth - (i-frm->kette.a+1)*gw;
                else x = (i-frm->kette.a)*gw;
                int y = ClientHeight - (j-frm->schuesse.a+1)*gh;
                if (gw>1 || gh>1) {
                    if (grid && gw>4 && gh>4) {
                        Canvas->Rectangle (x+2, y+2, x+gw-1, y+gh-1);
                        Canvas->Pen->Color = clDkGray;
                        Canvas->MoveTo (x, y);
                        Canvas->LineTo (x, y+gh);
                        Canvas->LineTo (x+gw, y+gh);
                        Canvas->LineTo (x+gw, y);
                        Canvas->LineTo (x, y);
                    } else if (grid && gw>2 && gh>2) {
                        Canvas->Rectangle (x+1, y+1, x+gw, y+gh);
                        Canvas->Pen->Color = clDkGray;
                        Canvas->MoveTo (x, y);
                        Canvas->LineTo (x, y+gh);
                        Canvas->LineTo (x+gw, y+gh);
                        Canvas->LineTo (x+gw, y);
                        Canvas->LineTo (x, y);
                    } else {
                        Canvas->Rectangle (x, y, x+gw, y+gh);
                    }
                } else {
                    Canvas->Pixels[x][y] = Canvas->Pen->Color;
                }
            }
    } else if (frm->GewebeFarbeffekt->Checked || frm->GewebeSimulation->Checked) {
        // Nach Farben getrennt malen, schneller...
        for (int i=frm->kette.a; i<frm->kette.a+mx; i++)
            for (int j=frm->schuesse.a; j<frm->schuesse.a+my; j++) {
                bool drawhebung = frm->gewebe.feld.Get(i,j)>0;
                if (frm->sinkingshed) drawhebung = !drawhebung;
                if (drawhebung) {
                    Canvas->Brush->Color = GETPALCOL(frm->kettfarben.feld.Get(i));
                    Canvas->Pen->Color = GETPALCOL(frm->kettfarben.feld.Get(i));
                } else {
                    Canvas->Brush->Color = GETPALCOL(frm->schussfarben.feld.Get(j));
                    Canvas->Pen->Color = GETPALCOL(frm->schussfarben.feld.Get(j));
                }
                int x;
                if (frm->righttoleft) x = ClientWidth - (i-frm->kette.a+1)*gw;
                else x = (i-frm->kette.a)*gw;
                int y = ClientHeight - (j-frm->schuesse.a+1)*gh;
                if (gw>1 || gh>1) {
                    Canvas->Rectangle (x, y, x+gw, y+gh);
                    if (grid) {
                        Canvas->Pen->Color = clDkGray;
                        Canvas->MoveTo (x, y);
                        Canvas->LineTo (x, y+gh);
                        Canvas->LineTo (x+gw, y+gh);
                        Canvas->LineTo (x+gw, y);
                        Canvas->LineTo (x, y);
                    }
                } else {
                    Canvas->Pixels[x][y] = Canvas->Pen->Color;
                }
            }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TOverviewForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
    if (Key==VK_ESCAPE || Key==VK_F4) {
        ModalResult = mrOk;
        return;
    }

    switch (Key) {
        case 'i':
        case 'I':
            if (Shift.Contains(ssCtrl) && !Shift.Contains(ssAlt))
                SBZoomInClick (this);
            break;

        case 'u':
        case 'U':
            if (Shift.Contains(ssCtrl) && !Shift.Contains(ssAlt))
                SBZoomOutClick (this);
            break;

        case 'p':
        case 'P':
            if (Shift.Contains(ssCtrl)) SBPrintClick(Sender);
            break;

        case 'g':
        case 'G':
            if (Shift.Contains(ssCtrl)) SBGridClick(Sender);
            break;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TOverviewForm::SBZoomOutClick(TObject *Sender)
{
    if (zoom>1) {
        zoom--;
        Invalidate();
        if (zoom==1) SBZoomOut->Enabled = false;
        if (zoom==9) SBZoomIn->Enabled = true;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TOverviewForm::SBZoomInClick(TObject *Sender)
{
    if (zoom<10) {
        zoom++;
        Invalidate();
        if (zoom==10) SBZoomIn->Enabled = false;
        if (zoom==2) SBZoomOut->Enabled = true;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TOverviewForm::SBPrintClick(TObject *Sender)
{
    // Ausdrucken der Geschichte...
    TCursor savecursor = Cursor;
    Cursor = crHourGlass;

    Printer()->Title = (String)APP_TITLE+" - "+ExtractFileName(frm->filename);
    Printer()->BeginDoc();

    CalcPrintDimensions();

    int mx = min(maxi, frm->kette.count());
    int my = min(maxj, frm->schuesse.count());

    TCanvas* canvas = Printer()->Canvas;

    if (frm->GewebeNormal->Checked) {
        for (int i=frm->kette.a; i<frm->kette.a+mx; i++)
            for (int j=frm->schuesse.a; j<frm->schuesse.a+my; j++) {
                int x;
                if (frm->righttoleft) x = (mx+1)*gw - (i-frm->kette.a+1)*gw;
                else x = (i-frm->kette.a)*gw;
                int y = (my+1)*gh - (j-frm->schuesse.a+1)*gh;
                if (frm->gewebe.feld.Get(i, j)>0) {
                    canvas->Brush->Style = bsSolid;
                    canvas->Pen->Color = GetRangeColor(frm->gewebe.feld.Get(i, j));
                    canvas->Brush->Color = GetRangeColor(frm->gewebe.feld.Get(i, j));
                    canvas->Rectangle (mleft+x, mtop+y, mleft+x+gw, mtop+y+gh);
                }
                if (grid) {
                    canvas->Pen->Color = clBlack;
                    canvas->MoveTo (mleft+x, mtop+y);
                    canvas->LineTo (mleft+x, mtop+y+gh);
                    canvas->LineTo (mleft+x+gw, mtop+y+gh);
                    canvas->LineTo (mleft+x+gw, mtop+y);
                    canvas->LineTo (mleft+x, mtop+y);
                }
            }
    } else if (frm->GewebeFarbeffekt->Checked || frm->GewebeSimulation->Checked) {
        // Nach Farben getrennt malen, schneller...
        for (int i=frm->kette.a; i<frm->kette.a+mx; i++)
            for (int j=frm->schuesse.a; j<frm->schuesse.a+my; j++) {
                bool drawhebung = frm->gewebe.feld.Get(i,j)>0;
                if (frm->sinkingshed) drawhebung = !drawhebung;
                canvas->Brush->Style = bsSolid;
                if (drawhebung) {
                    canvas->Brush->Color = GETPALCOL(frm->kettfarben.feld.Get(i));
                    canvas->Pen->Color = GETPALCOL(frm->kettfarben.feld.Get(i));
                } else {
                    canvas->Brush->Color = GETPALCOL(frm->schussfarben.feld.Get(j));
                    canvas->Pen->Color = GETPALCOL(frm->schussfarben.feld.Get(j));
                }
                int x;
                if (frm->righttoleft) x = pwidth - (i-frm->kette.a+1)*gw;
                else x = (i-frm->kette.a)*gw;
                int y = (my+1)*gh - (j-frm->schuesse.a+1)*gh;
                canvas->Rectangle (mleft+x, mtop+y, mleft+x+gw, mtop+y+gh);
                if (grid) {
                    canvas->Pen->Color = clBlack;
                    canvas->MoveTo (mleft+x, mtop+y);
                    canvas->LineTo (mleft+x, mtop+y+gh);
                    canvas->LineTo (mleft+x+gw, mtop+y+gh);
                    canvas->LineTo (mleft+x+gw, mtop+y);
                    canvas->LineTo (mleft+x, mtop+y);
                }
            }
    }

    Printer()->EndDoc();
    Cursor = savecursor;
    ::MessageBeep (-1);
}
/*-----------------------------------------------------------------*/
void __fastcall TOverviewForm::CalcPrintDimensions()
{
    int sx = GetDeviceCaps (Printer()->Handle, LOGPIXELSX);
    int sy = GetDeviceCaps (Printer()->Handle, LOGPIXELSY);

	// Ränder in (Drucker-)Pixel berechnen
	mleft   = frm->borders.range.left*10*sx/254;
	mtop    = (frm->borders.range.top+frm->header.height)*10*sy/254;
    mright  = frm->borders.range.right*10*sx/254;
    mbottom = (frm->borders.range.bottom+frm->footer.height)*10*sy/254;

	// Effektive Seitengrösse und -position
	pwidth  = Printer()->PageWidth - mleft - mright;
	pheight = Printer()->PageHeight - mtop  - mbottom;

	// Karogrösse
    gw = 2*zoom*sx/254;
    gh = 2*zoom*sy/254;

    if (frm->faktor_kette>frm->faktor_schuss) gw = int(double(gw)*frm->faktor_kette/frm->faktor_schuss);
    else gh = int(double(gh)*frm->faktor_schuss/frm->faktor_kette);

    // Logische Seitengrösse in Karos
    maxi = pwidth / gw;
    maxj = pheight / gh;
}
/*-----------------------------------------------------------------*/
void __fastcall TOverviewForm::SBCloseClick(TObject *Sender)
{
    ModalResult = mrOk;
}
/*-----------------------------------------------------------------*/
void __fastcall TOverviewForm::SBGridClick(TObject *Sender)
{
    grid = !grid;
    SBGrid->Down = grid;
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TOverviewForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Pattern overview - DB-WEAVE", "")
    LANG_C_H(this, GE, "Bindungsübersicht - DB-WEAVE", "")
    LANG_C_H(SBClose, EN, "&Close", "")
    LANG_C_H(SBClose, GE, "&Schliessen", "")
}
/*-----------------------------------------------------------------*/

