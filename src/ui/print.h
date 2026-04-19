/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Print subsystem — Qt port of legacy print.h. The PrPrinter
    abstract class drives the per-page layout and rendering over an
    arbitrary QPainter, subclassed for either a live QPrinter job or
    an off-screen preview painter. The TCanvas calls in the legacy
    code are translated one-to-one to QPainter operations, and
    legacy `Printer()->` references become instance methods on the
    bound QPrinter/QPaintDevice.                                   */

#ifndef DBWEAVE_UI_PRINT_H
#define DBWEAVE_UI_PRINT_H

#include "dbw3_base.h"    /* SZ */
#include "enums.h"        /* DARSTELLUNG */
#include "colors_compat.h"
#include "hilfslinien.h"  /* Hilfslinie */

#include <QString>

class QPainter;
class QPaintDevice;
class QPrinter;
class TDBWFRM;
class TData;

/*-----------------------------------------------------------------*/
struct PrFeld
{
	int  x0      = 0;
	int  y0      = 0;
	int  width   = 0;
	int  height  = 0;
	int  gw      = 0;
	int  gh      = 0;
	bool visible = false;

	void Init() { visible = false; x0 = y0 = width = height = 0; }
};

/*-----------------------------------------------------------------*/
class PrPrinter
{
public:
	PrPrinter (TDBWFRM* _mainfrm, TData* _data);
	virtual ~PrPrinter ();

	virtual void Print (int _von = 1, int _bis = -1);
	virtual void PrintRange (const SZ& _kette, const SZ& _schuesse,
	                         const SZ& _schaefte, const SZ& _tritte);
	virtual bool Failed () const { return failed; }
	virtual int  GetPages ();

protected:
	TDBWFRM* mainfrm = nullptr;
	TData*   data    = nullptr;

	/*  Bound painter + device; the active subclass sets these up
	    before calling into Print(). canvas is the QPainter, device
	    is the paint device it targets, printer is only non-null for
	    live-print jobs (used for DPI + NewPage()). */
	QPainter*     canvas  = nullptr;
	QPaintDevice* device  = nullptr;
	QPrinter*     printer = nullptr;

	bool failed       = false;
	bool preview      = false;
	bool blackandwhite = false;
	bool dontcalc     = false;

	/*  Ranges to print. */
	SZ schaefte, tritte, kette, schuesse;

	/*  Per-field geometry after CalcDimensions. */
	PrFeld kettfarben, einzug, blatteinzug, gewebe, aufknuepfung, trittfolge, schussfarben;

	/*  Page margins / effective page rectangle in device pixels. */
	int mleft   = 0, mtop    = 0;
	int mright  = 0, mbottom = 0;
	int pwidth  = 0, pheight = 0;
	int x_page  = 0, y_page  = 0;
	int y_header = 0, y_footer = 0;

	double x_scale = 1.0, y_scale = 1.0;
	int    cwidth  = 0, cheight = 0;
	int    gwidth  = 0, gheight = 0;

	int pages  = 0;
	int pagenr = 0;

	QString header;
	QString footer;

	/*  Painter-bound helpers so subclasses can set up resources
	    before the main pass. */
	void SetCanvas (QPainter* _c, QPaintDevice* _dev, QPrinter* _pr);
	void SetExtent (int _w, int _h);
	void SetScale  (double _xs, double _ys);

	virtual void BeforePrint () = 0;
	virtual void NewPage     () {}
	virtual void AfterPrint  () = 0;

	/*  Device DPI helpers — replace legacy GetDeviceCaps calls. */
	int dpiX () const;
	int dpiY () const;

	/*  Legacy Printer()->Page{Width,Height}. */
	int pageWidth  () const;
	int pageHeight () const;

private:
	/*  Cell rendering primitive shared by einzug/aufknuepfung/
	    trittfolge. */
	void PrPaintCell (DARSTELLUNG _d, int _x, int _y, int _xx, int _yy,
	                  TColor _col = TColor(0), int _number = -1);

	bool PrintPage (int _page, bool _newpage);

	void PrintKettfarben   ();
	void PrintEinzug       ();
	void PrintBlatteinzug  ();
	void PrintGewebe       ();
	void PrintAufknuepfung ();
	void PrintTrittfolge   ();
	void PrintSchussfarben ();
	void PrintHilfslinien  ();
	void PrintHilfslinie   (Hilfslinie* _h);

	void PrintGewebeNone       (int _i, int _j, int _x, int _y, int _xx, int _yy);
	void PrintGewebeNormal     (int _i, int _j, int _x, int _y, int _xx, int _yy);
	void PrintGewebeRapport    (int _i, int _j, int _x, int _y, int _xx, int _yy);
	void PrintGewebeFarbeffekt (int _i, int _j, int _x, int _y, int _xx, int _yy);
	void PrintGewebeSimulation (int _i, int _j, int _x, int _y, int _xx, int _yy);

	void CalcSchaefte ();
	void CalcTritte   ();
	void CalcKette    ();
	void CalcSchuesse ();

	bool IsEmptySchaft             (int _j);
	bool IsEmptyTritt              (int _i);
	bool IsEmptySchaftAufknuepfung (int _j);
	bool IsEmptyTrittAufknuepfung  (int _i);

	void CalcBaseDimensions ();
	void CalcDimensions     ();
	void InitVisibility     ();
	bool ClipToPage         (int _page);

	void PrepareHeader ();
	void PrepareFooter ();
};

/*-----------------------------------------------------------------*/
class PrPrinterPrint : public PrPrinter
{
public:
	PrPrinterPrint (TDBWFRM* _mainfrm, TData* _data, QPrinter* _printer);

protected:
	void BeforePrint () override;
	void AfterPrint  () override;
	void NewPage     () override;
};

/*-----------------------------------------------------------------*/
/*  Preview variant: driven by QPrintPreviewDialog which owns the
    QPrinter; we just wrap the PrPrinter cycle in a paintRequested
    callback. */
class PrPrinterPreview : public PrPrinter
{
public:
	PrPrinterPreview (TDBWFRM* _mainfrm, TData* _data, QPrinter* _printer);

protected:
	void BeforePrint () override;
	void AfterPrint  () override;
	void NewPage     () override;
};

#endif
