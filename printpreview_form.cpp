/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File printpreview_form.cpp
 * Copyright (C) 1998-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#include <vcl\printers.hpp>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "printpreview_form.h"
#include "datamodule.h"
#include "print.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TPrintPreviewForm::TPrintPreviewForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
    
    try {
        printer = new PrPrinterPreview (DBWFRM, Data, PrintRange);
    } catch (...) {
        printer = 0;
    }
    currentpage = 1;
    dragging = false;
	CalcPrintrect();
}
/*-----------------------------------------------------------------*/
__fastcall TPrintPreviewForm::~TPrintPreviewForm()
{
    delete printer;
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::FormResize(TObject *Sender)
{
	CalcPrintrect();
	Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::CalcPrintrect()
{
    // Hier sollte je nach Druckmodus (Portrait od. Landscape)
    // das Druckrechteck auf dem Vorschau-Canvas bestimmt werden.
    // Achtung: Ein kleiner Rahmen sollte übrig bleiben, sodass
    // die weisse Seite auf dem Bildschirm schön (mit Schatten!)
    // sichtbar ist.

	int height, width, heightrest, widthrest;
    double xfaktor;
    double yfaktor;

    if (Printer()->Orientation == poPortrait) {
        xfaktor = (double)Printer()->PageWidth / Printer()->PageHeight;
        yfaktor = (double)Printer()->PageHeight / Printer()->PageWidth;
		height = ClientRect.Bottom - 10 - Buttons->Height;
    	width = (int)((double)height*xfaktor);
        heightrest = 0;
    	widthrest = ClientRect.Right - 10 - width;
        if (widthrest<0) { // Tja, andersrum berechnen
            width = ClientRect.Right - 10;
            height = (int)((double)width*yfaktor);
            heightrest = ClientRect.Bottom - 10 - Buttons->Height - height;
            widthrest = 0;
        }
    } else {
        yfaktor = (double)Printer()->PageWidth / Printer()->PageHeight;
        xfaktor = (double)Printer()->PageHeight / Printer()->PageWidth;
    	width = ClientRect.Right - 10;
        height = (int)((double)width*xfaktor);
        heightrest = ClientRect.Bottom - 10 - Buttons->Height - height;
        widthrest = 0;
        if (heightrest<0) { // Tja, andersrum berechnen
        	height = ClientRect.Bottom - 10 - Buttons->Height;
            width = (int)((double)height*yfaktor);
            widthrest = ClientRect.Right - 10 - width;
            heightrest = 0;
        }
    }

	printrect.Left   = ClientRect.Left + 5 + widthrest/2;
    printrect.Top    = ClientRect.Top + Buttons->Height + 5 + heightrest/2;
    printrect.Bottom = ClientRect.Bottom - 5 - heightrest/2;
    printrect.Right  = ClientRect.Right - 5 - widthrest/2;

    PrintRange->Left   = printrect.Left;
    PrintRange->Top    = printrect.Top;
    PrintRange->Height = printrect.Bottom - printrect.Top;
    PrintRange->Width  = printrect.Right - printrect.Left;
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::PrintPageFrame()
{
	// Druck den Schatten der Vorschauseite...
    Canvas->Pen->Color = clGray;
    Canvas->Brush->Color = clGray;
    Canvas->Rectangle (printrect.Right+1, printrect.Top+4, printrect.Right+4, printrect.Bottom+4);
    Canvas->Rectangle (printrect.Left+4, printrect.Bottom+1, printrect.Right+1, printrect.Bottom+4);
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::FormPaint(TObject *Sender)
{
    PrintPageFrame();
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::PrintRangePaint(TObject *Sender)
{
    TCursor oldcursor = Cursor;
    Cursor = crHourGlass;
    PrintRange->Canvas->Pen->Color = clBlack;
    PrintRange->Canvas->Brush->Color = clWhite;
    PrintRange->Canvas->Rectangle (0, 0, PrintRange->Width, PrintRange->Height);
    PrintPage();
    Cursor = oldcursor;
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::PrintPage()
{
	printer->Print (currentpage, currentpage);
    if (printer->Failed() && currentpage>0) {
        --currentpage;
        ::MessageBeep(MB_OK);
        Invalidate();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::CloseClick(TObject *Sender)
{
	ModalResult = mrOk;
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::PreviousPageClick(TObject *Sender)
{
    if (currentpage>1) {
        --currentpage;
        Invalidate();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::NextPageClick(TObject *Sender)
{
    //xxx Wie erkenne ich, dass keine Seiten mehr vorhanden sind?
    //xxx Vielleicht sollte PrPrinter einen PageCount bieten...
    ++currentpage;
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::OnShowPrintDialog (TObject* Sender)
{
    ::SetFocus (Data->PrintDialog->Handle);
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::PrintClick(TObject *Sender)
{
    TNotifyEvent old = Data->PrintDialog->OnShow;
    Data->PrintDialog->OnShow = OnShowPrintDialog;
    if (Data->PrintDialog->Execute()) {
        TCursor oldcursor = Cursor;
        Cursor = crHourGlass;
		PrPrinterPrint printer(DBWFRM, Data);
		printer.Print();
        Cursor = oldcursor;

        ModalResult = mrOk;
    }
    Data->PrintDialog->OnShow = old;
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::UpdateBorders (int _x, int _y)
{
    if (_x<0 || _x>=PrintRange->ClientWidth) return;
    if (_y<0 || _y>=PrintRange->ClientHeight) return;

    // Skalierung bestimmen
    double x_scale = (double)Printer()->PageWidth/PrintRange->ClientWidth;
    double y_scale = (double)Printer()->PageHeight/PrintRange->ClientHeight;

    // umrechnen
    int sx = GetDeviceCaps (Printer()->Handle, LOGPIXELSX);
    int sy = GetDeviceCaps (Printer()->Handle, LOGPIXELSY);
	int x0 = (int)(x_scale*254*_x/10/sx);
	int y0 = (int)(y_scale*254*_y/10/sy);

    // Seitenrand setzen
    if (x0>=0) DBWFRM->borders.range.left = x0;
    if (y0>=0) DBWFRM->borders.range.top = y0;
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::DrawDragbar()
{
    // Seitenrand aus DBWFRM holen
    int x0 = DBWFRM->borders.range.left;
    int y0 = DBWFRM->borders.range.top;

    // Skalierung bestimmen
    double x_scale = (double)PrintRange->ClientWidth/Printer()->PageWidth;
    double y_scale = (double)PrintRange->ClientHeight/Printer()->PageHeight;

    // umrechnen
    int sx = GetDeviceCaps (Printer()->Handle, LOGPIXELSX);
    int sy = GetDeviceCaps (Printer()->Handle, LOGPIXELSY);
	x0 = (int)(x_scale*x0*10*sx/254);
	y0 = (int)(y_scale*y0*10*sy/254);

    // Linien zeichnen
    TPenMode savedmode = Canvas->Pen->Mode;
    PrintRange->Canvas->Pen->Mode = pmNot;
    PrintRange->Canvas->Pen->Color = clBlack;
    PrintRange->Canvas->MoveTo (x0, y0);
    PrintRange->Canvas->LineTo (x0, PrintRange->ClientHeight);
    PrintRange->Canvas->MoveTo (x0+1, y0);
    PrintRange->Canvas->LineTo (PrintRange->ClientWidth, y0);
    PrintRange->Canvas->Pen->Mode = savedmode;
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::PrintRangeMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (!dragging) {
//xxx ist noch unfertig, darum auskommentiert
//        dragging = true;
//        UpdateBorders (X, Y);
//        DrawDragbar();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::PrintRangeMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
    if (dragging) {
        DrawDragbar();
        // Position ändern
        UpdateBorders (X, Y);
        DrawDragbar();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::PrintRangeMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (dragging) {
        dragging = false;
        Invalidate();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintPreviewForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Print preview - DB-WEAVE", "")
    LANG_C_H(this, GE, "Druckvorschau - DB-WEAVE", "")
    LANG_C_H(Close, EN, "&Close", "")
    LANG_C_H(Close, GE, "&Schliessen", "")
}
/*-----------------------------------------------------------------*/


