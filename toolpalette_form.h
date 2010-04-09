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
#ifndef toolpalette_formH
#define toolpalette_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ToolWin.hpp>
/*-----------------------------------------------------------------*/
class TToolpaletteForm : public TForm
{
__published:	// IDE-managed Components
    TToolBar *ToolsToolbar;
    TSpeedButton *toolNormal;
    TSpeedButton *toolLine;
    TSpeedButton *toolRectangle;
    TSpeedButton *toolFilledRectangle;
    TSpeedButton *toolEllipse;
    TSpeedButton *toolFilledEllipse;
    TToolButton *ToolButton1;
    TSpeedButton *toolLinesize1;
    TSpeedButton *toolLinesize2;
    TSpeedButton *toolLinesize3;
    TSpeedButton *toolLinesize4;
    TToolButton *ToolButton2;
    TSpeedButton *toolConstrained;
    void __fastcall toolNormalClick(TObject *Sender);
    void __fastcall toolLineClick(TObject *Sender);
    void __fastcall toolRectangleClick(TObject *Sender);
    void __fastcall toolFilledRectangleClick(TObject *Sender);
    void __fastcall toolEllipseClick(TObject *Sender);
    void __fastcall toolFilledEllipseClick(TObject *Sender);
    void __fastcall toolLinesize1Click(TObject *Sender);
    void __fastcall toolLinesize2Click(TObject *Sender);
    void __fastcall toolLinesize3Click(TObject *Sender);
    void __fastcall toolLinesize4Click(TObject *Sender);
    void __fastcall FormHide(TObject *Sender);
    void __fastcall FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
public:		// User declarations
    __fastcall TToolpaletteForm(TComponent* Owner);
    void __fastcall ReloadLanguage();
};
/*-----------------------------------------------------------------*/
extern PACKAGE TToolpaletteForm *ToolpaletteForm;
/*-----------------------------------------------------------------*/
#endif
