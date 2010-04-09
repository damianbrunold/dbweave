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
#ifndef printH
#define printH
/*-----------------------------------------------------------------*/
// Forward-Deklarationen
class TDBWFRM;
class TData;
class TPrintCancelForm;
/*-----------------------------------------------------------------*/
struct PrFeld
{
    int x0;
    int y0;
    int width;
    int height;
    int gw;
    int gh;
    bool visible;

    PrFeld() { Init(); gw = gh = 0; }
    void Init() { visible = false; x0 = y0 = width = height = 0; }
};
/*-----------------------------------------------------------------*/
class PrPrinter
{
protected:
	TDBWFRM* mainfrm;
	TData*   data;
	TCanvas* canvas;

    bool canceldlg; // nur wenn wirklich gedruckt wird!
    TPrintCancelForm* cancelfrm;

    bool failed;
    bool preview;
    bool blackandwhite;

    // Zu druckender Bereich
    bool dontcalc;
	SZ schaefte;
	SZ tritte;
    SZ kette;
	SZ schuesse;

    // Druckposition und -parameter der Felder
    PrFeld kettfarben;
    PrFeld einzug;
    PrFeld blatteinzug;
    PrFeld gewebe;
    PrFeld aufknuepfung;
    PrFeld trittfolge;
    PrFeld schussfarben;

    // Seitenränder
    int mleft;
    int mtop;
    int mright;
    int mbottom;

    // Effektive Seitengrösse und -position
    int pwidth;
    int pheight;
    int x_page;
    int y_page;

    // Header- und Footerposition
    int y_header;
    int y_footer;

    double x_scale, y_scale;
    int cwidth, cheight; // Blattausmasse in Pixels
    int gwidth, gheight; // Ausmasse eines Karos in Pixel

    int pages;  // Anzahl zu druckende Seiten
    int pagenr; // Aktuelle Seitennummer

	String header;
	String footer;

public:
	__fastcall PrPrinter (TDBWFRM* _mainfrm, TData* _data);
    virtual __fastcall ~PrPrinter();
	virtual void __fastcall Print (int _von=1, int _bis=-1);
    virtual void __fastcall PrintRange (const SZ& _kette,
                                        const SZ& _schuesse,
                                        const SZ& _schaefte,
                                        const SZ& _tritte);
    virtual bool __fastcall Failed() const { return failed; }
    virtual int __fastcall GetPages();

protected:
    void __fastcall SetCanvas (TCanvas* _canvas);
    void __fastcall SetExtent (int _width, int _height);
    void __fastcall SetScale (double _xscale, double _yscale);
    virtual void __fastcall BeforePrint() = 0;
    virtual void __fastcall NewPage() {};
    virtual void __fastcall AfterPrint() = 0;

private:
	void __fastcall PrPaintCell (DARSTELLUNG _darstellung, int _x, int _y, int _xx, int _yy, TColor _col=clBlack, int _number=-1);

    bool __fastcall PrintPage (int _page, bool _newpage);

    void __fastcall PrintKettfarben();
    void __fastcall PrintEinzug();
    void __fastcall PrintBlatteinzug();
    void __fastcall PrintGewebe();
    void __fastcall PrintAufknuepfung();
    void __fastcall PrintTrittfolge();
    void __fastcall PrintSchussfarben();
    void __fastcall PrintHilfslinien();
    void __fastcall PrintHilfslinie (Hilfslinie* _hline);

    void __fastcall PrintGewebeNone (int _i, int _j, int _x, int _y, int _xx, int _yy);
    void __fastcall PrintGewebeNormal (int _i, int _j, int _x, int _y, int _xx, int _yy);
    void __fastcall PrintGewebeRapport (int _i, int _j, int _x, int _y, int _xx, int _yy);
    void __fastcall PrintGewebeFarbeffekt (int _i, int _j, int _x, int _y, int _xx, int _yy);
    void __fastcall PrintGewebeSimulation (int _i, int _j, int _x, int _y, int _xx, int _yy);

	void __fastcall CalcSchaefte();
	void __fastcall CalcTritte();
	void __fastcall CalcKette();
	void __fastcall CalcSchuesse();

    bool __fastcall IsEmptySchaft (int _j);
    bool __fastcall IsEmptyTritt (int _i);
    bool __fastcall IsEmptySchaftAufknuepfung (int _j);
    bool __fastcall IsEmptyTrittAufknuepfung (int _i);

    void __fastcall CalcBaseDimensions();
    void __fastcall CalcDimensions();
    void __fastcall InitVisibility();
    bool __fastcall ClipToPage (int _page);

	void __fastcall PrepareHeader();
	void __fastcall PrepareFooter();
};
/*-----------------------------------------------------------------*/
class PrPrinterPrint : public PrPrinter
{
public:
	__fastcall PrPrinterPrint (TDBWFRM* _mainfrm, TData* _data);

protected:
    virtual void __fastcall BeforePrint();
    virtual void __fastcall AfterPrint();
    virtual void __fastcall NewPage();
};
/*-----------------------------------------------------------------*/
class PrPrinterPreview : public PrPrinter
{
public:
	__fastcall PrPrinterPreview (TDBWFRM* _mainfrm, TData* _data, TPaintBox* _preview);

private:
    TPaintBox* pb;

protected:
    virtual void __fastcall BeforePrint();
    virtual void __fastcall AfterPrint();
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
