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
#include <vcl\printers.hpp>
#include <mem.h>
#include <math.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "felddef.h"
#include "dbw3_strings.h"
#include "datamodule.h"
#include "properties.h"
#include "pagesetup_form.h"
#include "printpreview_form.h"
#include "printcancel_form.h"
#include "print.h"
/*-----------------------------------------------------------------*/
#define CHECKCANCEL \
    Application->ProcessMessages(); \
    if (canceldlg && !cancelfrm->Visible) goto g_exit;
/*-----------------------------------------------------------------*/
#define XSCALE(x) \
    ((int)(x_scale*(x)))
/*-----------------------------------------------------------------*/
#define YSCALE(y) \
    ((int)(y_scale*(y)))
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::InitBorders()
{
	// Felder initialisieren
	borders.range.left   = 20;
	borders.range.right  = 20;
	borders.range.top    = 25;
	borders.range.bottom = 25;

	header.height = 6;
	header.text = String(APP_TITLE)+PAGE_HEADER_DEFAULT;
	footer.height = 6;
	footer.text = ""; // Default: Keine Seitennummerierung da eh 99% einseitig!
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FileSetPageClick(TObject *Sender)
{
	// Optionsdialog für Ränder, Kopf- und Fusszeile
	TPageSetupForm* frm = new TPageSetupForm(this);
	frm->left->Text = FloatToStr (((double)borders.range.left)/10) + " cm";
	frm->right->Text = FloatToStr (((double)borders.range.right)/10) + " cm";
	frm->top->Text = FloatToStr (((double)borders.range.top)/10) + " cm";
	frm->bottom->Text = FloatToStr (((double)borders.range.bottom)/10) + " cm";
	frm->headertext->Text = header.text;
	frm->footertext->Text = footer.text;
	if (frm->ShowModal()==mrOk) {
		borders.range.left = (int)(atof(frm->left->Text.c_str())*10);
		borders.range.right = (int)(atof(frm->right->Text.c_str())*10);
		borders.range.top = (int)(atof(frm->top->Text.c_str())*10);
		borders.range.bottom = (int)(atof(frm->bottom->Text.c_str())*10);
		header.text = frm->headertext->Text;
		footer.text = frm->footertext->Text;
        SetModified();
	}
	delete frm;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::OnShowPrintDialog (TObject* Sender)
{
    ::SetFocus (Data->PrintDialog->Handle);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::OnShowPrinterSetupDialog (TObject* Sender)
{
    ::SetFocus (Data->PrinterSetupDialog->Handle);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FilePrintClick(TObject *Sender)
{
    PrPrinterPrint printer(this, Data);
    TNotifyEvent old = Data->PrintDialog->OnShow;
    Data->PrintDialog->OnShow = OnShowPrintDialog;
    int pages = printer.GetPages();
    if (pages>1) {
        Data->PrintDialog->MaxPage = pages;
        Data->PrintDialog->MinPage = 1;
        TPrintDialogOptions opt = Data->PrintDialog->Options;
        opt = opt << poPageNums;
        Data->PrintDialog->Options = opt;
    } else {
        Data->PrintDialog->MaxPage = 1;
        Data->PrintDialog->MinPage = 1;
        TPrintDialogOptions opt = Data->PrintDialog->Options;
        opt = opt >> poPageNums;
        Data->PrintDialog->Options = opt;
    }
    if (Data->PrintDialog->Execute()) {
        TCursor oldcursor = Cursor;
        Cursor = crHourGlass;
        if (Data->PrintDialog->PrintRange==prAllPages)
            printer.Print();
        else
    		printer.Print (Data->PrintDialog->FromPage,
                           Data->PrintDialog->ToPage);
        Cursor = oldcursor;
    }
    Data->PrintDialog->OnShow = old;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FilePrintpreviewClick(TObject *Sender)
{
    try {
        TPrintPreviewForm* frm = new TPrintPreviewForm(this);
        Hide();
        frm->ShowModal();
        Show();
        delete frm;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBPreviewClick(TObject *Sender)
{
    FilePrintpreviewClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBFilePrintClick(TObject *Sender)
{
    // Ohne Druckdialog!
    TCursor oldcursor = Cursor;
    Cursor = crHourGlass;
    PrPrinterPrint printer(this, Data);
    printer.Print();
    Cursor = oldcursor;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FilePrintSetupClick(TObject *Sender)
{
    TNotifyEvent old = Data->PrintDialog->OnShow;
    Data->PrinterSetupDialog->OnShow = OnShowPrinterSetupDialog;
    Data->PrinterSetupDialog->Execute();
    Data->PrinterSetupDialog->OnShow = old;
}
/*-----------------------------------------------------------------*/
__fastcall PrPrinter::PrPrinter (TDBWFRM* _mainfrm, TData* _data)
{
    failed = false;
    dontcalc = false;
    preview = false;
	mainfrm = _mainfrm;
	data = _data;
    canceldlg = true;
    blackandwhite = false;
    try {
        cancelfrm = new TPrintCancelForm(mainfrm);
        cancelfrm->Hide();
    } catch(...) {
        cancelfrm = NULL;
    }
    x_scale = 1.0;
    y_scale = 1.0;
    pages = 0;
}
/*-----------------------------------------------------------------*/
__fastcall PrPrinter::~PrPrinter()
{
    delete cancelfrm;
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::SetCanvas (TCanvas* _canvas)
{
    canvas = _canvas;
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::SetExtent (int _width, int _height)
{
    cwidth  = _width;
    cheight = _height;
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::SetScale (double _xscale, double _yscale)
{
    x_scale = _xscale;
    y_scale = _yscale;
}
/*-----------------------------------------------------------------*/
int __fastcall PrPrinter::GetPages()
{
    SetExtent (Printer()->PageWidth, Printer()->PageHeight);
    CalcBaseDimensions();
    if (!dontcalc) {
    	CalcSchaefte();
	    CalcTritte();
    	CalcKette();
	    CalcSchuesse();
    }
    if (ClipToPage(1)) {
        return pages;
    }
    return 12;
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::Print (int _von/*=1*/, int _bis/*=-1*/)
{
    failed = false;
    BeforePrint();
    CalcBaseDimensions();
    pagenr = _von;

    // Feststellen, ob Farbdrucker verwendet...
    blackandwhite = GetDeviceCaps(Printer()->Handle, BITSPIXEL)*GetDeviceCaps(Printer()->Handle, PLANES)==1;

    try {
        if (canceldlg) {
            cancelfrm->currentpage->Caption = (String)PR_PRINTING1+IntToStr(pagenr)+PR_PRINTING2;
            cancelfrm->Show();
            if (!preview) mainfrm->Enabled = false;
            Application->ProcessMessages();
        }
        while ((_bis==-1 || pagenr<=_bis) && (failed=!PrintPage (pagenr, pagenr>_von))==false) {
            ++pagenr;
            if (_bis!=-1 && pagenr>_bis) break;
            if ((pagenr-_von) >= pages) break;
            if (canceldlg) {
                if (!cancelfrm->Visible) break;
                cancelfrm->currentpage->Caption = (String)PR_PRINTING1+IntToStr(pagenr)+PR_PRINTING2;
                Application->ProcessMessages();
            }
        }
        if (canceldlg) {
            if (!preview) mainfrm->Enabled = true;
            cancelfrm->Hide();
        }
    } catch (...) {
        if (!preview) mainfrm->Enabled = true;
    }

    AfterPrint();
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrintRange (const SZ& _kette,
                                       const SZ& _schuesse,
                                       const SZ& _schaefte,
                                       const SZ& _tritte)
{
    kette = _kette;
    schuesse = _schuesse;
    schaefte = _schaefte;
    tritte = _tritte;
    dontcalc = true;
    Print();
    dontcalc = false;
}
/*-----------------------------------------------------------------*/
void __fastcall SelectTextFont (TCanvas* _canvas, double x_scale)
{
	_canvas->Font->Name = "Arial";
    int fntheight = XSCALE(10);
    if (fntheight==0) fntheight = 1;
	_canvas->Font->Size = fntheight;
	_canvas->Font->Pitch = fpVariable;
	_canvas->Font->Color = clBlack;
	_canvas->Brush->Color = clWhite;
}
/*-----------------------------------------------------------------*/
bool __fastcall PrPrinter::PrintPage (int _page, bool _newpage)
{
    bool success = false;

	// Ausgabebereich festlegen
    if (!dontcalc) {
    	CalcSchaefte();
	    CalcTritte();
    	CalcKette();
	    CalcSchuesse();
    }

    // Seite berechnen
    if (!ClipToPage (_page)) goto g_exit;
    CalcDimensions();

    // Seitenumbruch!
    if (_newpage) NewPage();

	// Kopfzeile
	PrepareHeader();
	SelectTextFont (canvas, x_scale);
    canvas->TextOut (XSCALE(mleft), YSCALE(y_header), header);
    CHECKCANCEL

	// Fusszeile
	PrepareFooter();
	SelectTextFont (canvas, x_scale);
    canvas->TextOut (XSCALE(mleft), YSCALE(y_footer), footer);
    CHECKCANCEL

    // Daten
    PrintKettfarben();
    PrintEinzug();
    PrintBlatteinzug();
    PrintGewebe();
    PrintAufknuepfung();
    PrintTrittfolge();
    PrintSchussfarben();
    PrintHilfslinien();

    success = true;

g_exit:
    return success;
}
/*-----------------------------------------------------------------*/
void __fastcall ReplacePattern (String& _string, const char* _pattern, const String& _data)
{
	int pos = _string.AnsiPos (_pattern);
	if (pos) {
    	String data = _data;
		if (_pattern[0]=='(' && _pattern[strlen(_pattern)-1]==')') {
			if (data!="") data = (String)"(" + data + ")";
		}

		_string = _string.SubString (1, pos-1) + data +
 			      _string.SubString (pos+strlen(_pattern), _string.Length());
	}
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrepareHeader()
{
	String hdr = mainfrm->header.text;

	// Mustername ist Dateiname ohne Pfad und ohne Extension
    String file = ExtractFileName(mainfrm->filename);
    int ext = file.AnsiPos (".dbw");
    if (ext) file = file.SubString (1, ext-1);

	ReplacePattern (hdr, PLACEHOLDER_MUSTER1, file);
	ReplacePattern (hdr, PLACEHOLDER_MUSTER, file);
	ReplacePattern (hdr, PLACEHOLDER_DATEI1, mainfrm->filename);
	ReplacePattern (hdr, PLACEHOLDER_DATEI, mainfrm->filename);
	ReplacePattern (hdr, PLACEHOLDER_AUTOR1, Data->properties->Author());
	ReplacePattern (hdr, PLACEHOLDER_AUTOR, Data->properties->Author());
	ReplacePattern (hdr, PLACEHOLDER_AUTOR3, Data->properties->Author());
	ReplacePattern (hdr, PLACEHOLDER_AUTOR2, Data->properties->Author());
	ReplacePattern (hdr, PLACEHOLDER_ORG1, Data->properties->Organization());
	ReplacePattern (hdr, PLACEHOLDER_ORG, Data->properties->Organization());
    ReplacePattern (hdr, PLACEHOLDER_SEITE, IntToStr (pagenr));

	ReplacePattern (hdr, PLACEHOLDER_E_MUSTER1, file);
	ReplacePattern (hdr, PLACEHOLDER_E_MUSTER, file);
	ReplacePattern (hdr, PLACEHOLDER_E_DATEI1, mainfrm->filename);
	ReplacePattern (hdr, PLACEHOLDER_E_DATEI, mainfrm->filename);
	ReplacePattern (hdr, PLACEHOLDER_E_AUTOR1, Data->properties->Author());
	ReplacePattern (hdr, PLACEHOLDER_E_AUTOR, Data->properties->Author());
	ReplacePattern (hdr, PLACEHOLDER_E_ORG1, Data->properties->Organization());
	ReplacePattern (hdr, PLACEHOLDER_E_ORG, Data->properties->Organization());
    ReplacePattern (hdr, PLACEHOLDER_E_SEITE, IntToStr (pagenr));

	header = hdr;
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::PrepareFooter()
{
	String ftr = mainfrm->footer.text;

	// Mustername ist Dateiname ohne Pfad und ohne Extension
    String file = ExtractFileName(mainfrm->filename);
    int ext = file.AnsiPos (".dbw");
    if (ext) file = file.SubString (0, ext-1);

	ReplacePattern (ftr, PLACEHOLDER_MUSTER1, file);
	ReplacePattern (ftr, PLACEHOLDER_MUSTER, file);
	ReplacePattern (ftr, PLACEHOLDER_DATEI1, mainfrm->filename);
	ReplacePattern (ftr, PLACEHOLDER_DATEI, mainfrm->filename);
	ReplacePattern (ftr, PLACEHOLDER_AUTOR1, Data->properties->Author());
	ReplacePattern (ftr, PLACEHOLDER_AUTOR, Data->properties->Author());
	ReplacePattern (ftr, PLACEHOLDER_AUTOR3, Data->properties->Author());
	ReplacePattern (ftr, PLACEHOLDER_AUTOR2, Data->properties->Author());
	ReplacePattern (ftr, PLACEHOLDER_ORG1, Data->properties->Organization());
	ReplacePattern (ftr, PLACEHOLDER_ORG, Data->properties->Organization());
    ReplacePattern (ftr, PLACEHOLDER_SEITE, IntToStr (pagenr));

	ReplacePattern (ftr, PLACEHOLDER_E_MUSTER1, file);
	ReplacePattern (ftr, PLACEHOLDER_E_MUSTER, file);
	ReplacePattern (ftr, PLACEHOLDER_E_DATEI1, mainfrm->filename);
	ReplacePattern (ftr, PLACEHOLDER_E_DATEI, mainfrm->filename);
	ReplacePattern (ftr, PLACEHOLDER_E_AUTOR1, Data->properties->Author());
	ReplacePattern (ftr, PLACEHOLDER_E_AUTOR, Data->properties->Author());
	ReplacePattern (ftr, PLACEHOLDER_E_ORG1, Data->properties->Organization());
	ReplacePattern (ftr, PLACEHOLDER_E_ORG, Data->properties->Organization());
    ReplacePattern (ftr, PLACEHOLDER_E_SEITE, IntToStr (pagenr));

	footer = ftr;
}
/*-----------------------------------------------------------------*/
__fastcall PrPrinterPrint::PrPrinterPrint (TDBWFRM* _mainfrm, TData* _data)
: PrPrinter (_mainfrm, _data)
{
    preview = false;
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinterPrint::BeforePrint()
{
    Printer()->Title = (String)APP_TITLE+" - "+ExtractFileName(mainfrm->filename);
    Printer()->BeginDoc();
    SetCanvas (Printer()->Canvas);
    SetExtent (Printer()->PageWidth, Printer()->PageHeight);
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinterPrint::AfterPrint()
{
    Printer()->EndDoc();
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinterPrint::NewPage()
{
    Printer()->NewPage();
}
/*-----------------------------------------------------------------*/
__fastcall PrPrinterPreview::PrPrinterPreview (TDBWFRM* _mainfrm, TData* _data, TPaintBox* _preview)
: PrPrinter (_mainfrm, _data)
{
    preview = true;
    pb = _preview;
    SetCanvas (_preview->Canvas);
    canceldlg = false; // Im Preview braucht es keinen Abbrechendialog!
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinterPreview::BeforePrint()
{
    // SetCanvas wird bereits im Constructor aufgerufen!
    SetExtent (pb->ClientWidth, pb->ClientHeight);
    SetScale ((double)cwidth/Printer()->PageWidth, (double)cheight/Printer()->PageHeight);
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinterPreview::AfterPrint()
{
    // Nichts zu tun
}
/*-----------------------------------------------------------------*/

