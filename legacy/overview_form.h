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
#ifndef overview_formH
#define overview_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ToolWin.hpp>
/*-----------------------------------------------------------------*/
class TDBWFRM;
/*-----------------------------------------------------------------*/
class TOverviewForm : public TForm
{
__published:	// IDE-managed Components
    TToolBar *Toolbar;
    TSpeedButton *SBZoomOut;
    TSpeedButton *SBZoomIn;
    TToolButton *ToolButton1;
    TToolButton *ToolButton2;
    TSpeedButton *SBPrint;
    TSpeedButton *SBGrid;
    TSpeedButton *SBClose;
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall SBZoomOutClick(TObject *Sender);
    void __fastcall SBZoomInClick(TObject *Sender);
    void __fastcall SBPrintClick(TObject *Sender);
    void __fastcall SBCloseClick(TObject *Sender);
    void __fastcall SBGridClick(TObject *Sender);
private:	// User declarations
    TDBWFRM* frm;
    int zoom;
    bool grid;
    // Druckdaten
    int gw;
    int gh;
    int mleft;
    int mright;
    int mtop;
    int mbottom;
    int pwidth;
    int pheight;
    int maxi;
    int maxj;
    void __fastcall CalcPrintDimensions();
    void __fastcall LoadLanguage();
public:		// User declarations
    __fastcall TOverviewForm(TComponent* Owner, TDBWFRM* _frm);
};
/*-----------------------------------------------------------------*/
#endif
