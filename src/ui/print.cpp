/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Core of the PrPrinter port — ctor/dtor, Print loop, header/
    footer substitution, and the per-subclass Before/After/NewPage
    overrides. The TCanvas*/ /*->canvas field becomes a QPainter*
    targeting either a QPrinter (live job) or the preview's paint
    device.                                                       */

#include "print.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "properties.h"
#include "dbw3_strings.h"

#include <QApplication>
#include <QFileInfo>
#include <QFont>
#include <QPageLayout>
#include <QPainter>
#include <QPrinter>

/*-----------------------------------------------------------------*/
#define XSCALE(x) (int(x_scale * (x)))
#define YSCALE(y) (int(y_scale * (y)))

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::InitBorders ()
{
	borders.range.left   = 20;
	borders.range.right  = 20;
	borders.range.top    = 25;
	borders.range.bottom = 25;
	header.height = 6;
	header.text   = QStringLiteral("DB-WEAVE") + QString(PAGE_HEADER_DEFAULT);
	footer.height = 6;
	footer.text.clear();
}

/*-----------------------------------------------------------------*/
PrPrinter::PrPrinter (TDBWFRM* _mainfrm, TData* _data)
	: mainfrm(_mainfrm), data(_data)
{
}

PrPrinter::~PrPrinter () = default;

void PrPrinter::SetCanvas (QPainter* _c, QPaintDevice* _dev, QPrinter* _pr)
{
	canvas  = _c;
	device  = _dev;
	printer = _pr;
}

void PrPrinter::SetExtent (int _w, int _h) { cwidth = _w; cheight = _h; }
void PrPrinter::SetScale  (double _xs, double _ys) { x_scale = _xs; y_scale = _ys; }

int PrPrinter::dpiX () const { return device ? device->logicalDpiX() : 96; }
int PrPrinter::dpiY () const { return device ? device->logicalDpiY() : 96; }

int PrPrinter::pageWidth () const
{
	if (!printer) return cwidth;
	return printer->pageLayout().paintRectPixels(printer->resolution()).width();
}

int PrPrinter::pageHeight () const
{
	if (!printer) return cheight;
	return printer->pageLayout().paintRectPixels(printer->resolution()).height();
}

/*-----------------------------------------------------------------*/
int PrPrinter::GetPages ()
{
	SetExtent(pageWidth(), pageHeight());
	CalcBaseDimensions();
	if (!dontcalc) {
		CalcSchaefte();
		CalcTritte();
		CalcKette();
		CalcSchuesse();
	}
	if (ClipToPage(1)) return pages;
	return 12;
}

/*-----------------------------------------------------------------*/
void PrPrinter::Print (int _von, int _bis)
{
	failed = false;
	BeforePrint();
	CalcBaseDimensions();
	pagenr = _von;

	/*  BITSPIXEL*PLANES==1 was the legacy b/w check. For Qt just
	    ask the QPrinter directly. */
	blackandwhite = printer && printer->colorMode() == QPrinter::GrayScale;

	while ((_bis == -1 || pagenr <= _bis) && (failed = !PrintPage(pagenr, pagenr > _von)) == false) {
		++pagenr;
		if (_bis != -1 && pagenr > _bis) break;
		if ((pagenr - _von) >= pages) break;
	}

	AfterPrint();
}

/*-----------------------------------------------------------------*/
void PrPrinter::PrintRange (const SZ& _kette, const SZ& _schuesse,
                            const SZ& _schaefte, const SZ& _tritte)
{
	kette    = _kette;
	schuesse = _schuesse;
	schaefte = _schaefte;
	tritte   = _tritte;
	dontcalc = true;
	Print();
	dontcalc = false;
}

/*-----------------------------------------------------------------*/
static void selectTextFont (QPainter& _p, double _xscale)
{
	int fntHeight = int(_xscale * 10);
	if (fntHeight == 0) fntHeight = 1;
	QFont f(QStringLiteral("Arial"));
	f.setPointSize(fntHeight);
	_p.setFont(f);
	_p.setPen(Qt::black);
}

/*-----------------------------------------------------------------*/
bool PrPrinter::PrintPage (int _page, bool _newpage)
{
	/*  Ausgabebereich festlegen. */
	if (!dontcalc) {
		CalcSchaefte();
		CalcTritte();
		CalcKette();
		CalcSchuesse();
	}

	/*  Seite berechnen. */
	if (!ClipToPage(_page)) return false;
	CalcDimensions();

	/*  Seitenumbruch. */
	if (_newpage) NewPage();

	/*  Kopfzeile. */
	PrepareHeader();
	selectTextFont(*canvas, x_scale);
	canvas->drawText(XSCALE(mleft), YSCALE(y_header), header);

	/*  Fusszeile. */
	PrepareFooter();
	selectTextFont(*canvas, x_scale);
	canvas->drawText(XSCALE(mleft), YSCALE(y_footer), footer);

	/*  Daten. */
	PrintKettfarben();
	PrintEinzug();
	PrintBlatteinzug();
	PrintGewebe();
	PrintAufknuepfung();
	PrintTrittfolge();
	PrintSchussfarben();
	PrintHilfslinien();

	return true;
}

/*-----------------------------------------------------------------*/
static QString replacePattern (QString _s, const char* _pattern, const QString& _data)
{
	const QString p = QString::fromLatin1(_pattern);
	int pos = _s.indexOf(p);
	if (pos < 0) return _s;
	QString data = _data;
	const int plen = p.length();
	/*  (Pattern) form: only wrap in parens if data non-empty. */
	if (p.startsWith('(') && p.endsWith(')')) {
		if (!data.isEmpty()) data = QStringLiteral("(") + data + QStringLiteral(")");
	}
	return _s.left(pos) + data + _s.mid(pos + plen);
}

static QString expandTokens (QString _s, TDBWFRM* _frm, int _pageNr)
{
	const QString fn = QFileInfo(_frm->filename).fileName();
	QString muster = fn;
	const int ext = muster.indexOf(QStringLiteral(".dbw"));
	if (ext > 0) muster = muster.left(ext);

	const QString author = QString::fromUtf8(Data->properties ? Data->properties->Author() : "");
	const QString org    = QString::fromUtf8(Data->properties ? Data->properties->Organization() : "");
	const QString pgs    = QString::number(_pageNr);

	_s = replacePattern(_s, PLACEHOLDER_MUSTER1, muster);
	_s = replacePattern(_s, PLACEHOLDER_MUSTER,  muster);
	_s = replacePattern(_s, PLACEHOLDER_DATEI1,  _frm->filename);
	_s = replacePattern(_s, PLACEHOLDER_DATEI,   _frm->filename);
	_s = replacePattern(_s, PLACEHOLDER_AUTOR1,  author);
	_s = replacePattern(_s, PLACEHOLDER_AUTOR,   author);
	_s = replacePattern(_s, PLACEHOLDER_AUTOR3,  author);
	_s = replacePattern(_s, PLACEHOLDER_AUTOR2,  author);
	_s = replacePattern(_s, PLACEHOLDER_ORG1,    org);
	_s = replacePattern(_s, PLACEHOLDER_ORG,     org);
	_s = replacePattern(_s, PLACEHOLDER_SEITE,   pgs);

	_s = replacePattern(_s, PLACEHOLDER_E_MUSTER1, muster);
	_s = replacePattern(_s, PLACEHOLDER_E_MUSTER,  muster);
	_s = replacePattern(_s, PLACEHOLDER_E_DATEI1,  _frm->filename);
	_s = replacePattern(_s, PLACEHOLDER_E_DATEI,   _frm->filename);
	_s = replacePattern(_s, PLACEHOLDER_E_AUTOR1,  author);
	_s = replacePattern(_s, PLACEHOLDER_E_AUTOR,   author);
	_s = replacePattern(_s, PLACEHOLDER_E_ORG1,    org);
	_s = replacePattern(_s, PLACEHOLDER_E_ORG,     org);
	_s = replacePattern(_s, PLACEHOLDER_E_SEITE,   pgs);
	return _s;
}

void PrPrinter::PrepareHeader () { header = expandTokens(mainfrm->header.text, mainfrm, pagenr); }
void PrPrinter::PrepareFooter () { footer = expandTokens(mainfrm->footer.text, mainfrm, pagenr); }

/*-----------------------------------------------------------------*/
PrPrinterPrint::PrPrinterPrint (TDBWFRM* _m, TData* _d, QPrinter* _pr)
	: PrPrinter(_m, _d)
{
	preview = false;
	printer = _pr;
}

void PrPrinterPrint::BeforePrint ()
{
	/*  The caller hands us a ready-to-paint QPrinter. Create a
	    QPainter here; it stays alive for the whole Print() loop. */
	canvas = new QPainter(printer);
	device = printer;
	SetExtent(pageWidth(), pageHeight());
}

void PrPrinterPrint::AfterPrint ()
{
	if (canvas) { canvas->end(); delete canvas; canvas = nullptr; }
}

void PrPrinterPrint::NewPage ()
{
	if (printer) printer->newPage();
}

/*-----------------------------------------------------------------*/
PrPrinterPreview::PrPrinterPreview (TDBWFRM* _m, TData* _d, QPrinter* _pr)
	: PrPrinter(_m, _d)
{
	preview = true;
	printer = _pr;
}

void PrPrinterPreview::BeforePrint ()
{
	/*  QPrintPreviewDialog passes us a QPrinter already configured
	    for the preview device; create a QPainter on it. */
	canvas = new QPainter(printer);
	device = printer;
	SetExtent(pageWidth(), pageHeight());
}

void PrPrinterPreview::AfterPrint ()
{
	if (canvas) { canvas->end(); delete canvas; canvas = nullptr; }
}

void PrPrinterPreview::NewPage ()
{
	if (printer) printer->newPage();
}
