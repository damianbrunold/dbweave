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
#ifndef printpreviewfrmH
#define printpreviewfrmH
/*-----------------------------------------------------------------*/
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ExtCtrls.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ToolWin.hpp>
/*-----------------------------------------------------------------*/
// Forward-Deklarationen
class PrPrinterPreview;
/*-----------------------------------------------------------------*/
class TPrintPreviewForm : public TForm
{
__published:	// IDE-managed Components
    TToolBar *Buttons;
    TSpeedButton *PreviousPage;
    TSpeedButton *NextPage;
    TToolButton *ToolButton2;
    TSpeedButton *Print;
    TToolButton *ToolButton3;
    TSpeedButton *Close;
    TPaintBox *PrintRange;
	void __fastcall FormPaint(TObject *Sender);
	void __fastcall CloseClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
    void __fastcall PreviousPageClick(TObject *Sender);
    void __fastcall NextPageClick(TObject *Sender);
    void __fastcall PrintClick(TObject *Sender);    
    void __fastcall PrintRangePaint(TObject *Sender);
    void __fastcall PrintRangeMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall PrintRangeMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
    void __fastcall PrintRangeMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
private:	// User declarations
    int currentpage;
	TRect printrect;
    PrPrinterPreview* printer;
    bool dragging;
protected:
    void __fastcall OnShowPrintDialog (TObject* Sender);
    void __fastcall DrawDragbar();
    void __fastcall UpdateBorders (int _x, int _y);
    void __fastcall LoadLanguage();
public:		// User declarations
	__fastcall TPrintPreviewForm(TComponent* Owner);
    virtual __fastcall ~TPrintPreviewForm();
    void __fastcall CalcPrintrect();
    void __fastcall PrintPageFrame();
    void __fastcall PrintPage();
};
/*-----------------------------------------------------------------*/
extern PACKAGE TPrintPreviewForm *PrintPreviewForm;
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/

