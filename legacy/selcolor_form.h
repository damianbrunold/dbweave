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
#ifndef selcolor_formH
#define selcolor_formH
/*-----------------------------------------------------------------*/
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ExtCtrls.hpp>
#include <Menus.hpp>
/*-----------------------------------------------------------------*/
class TSelColorForm : public TForm
{
__published:	// IDE-managed Components
	TPaintBox *pb_palette;
    TGroupBox *gbHSV;
    TLabel *labHue;
    TLabel *Hue;
    TLabel *labSat;
    TLabel *Sat;
    TLabel *labVal;
    TLabel *Val;
    TGroupBox *gbRGB;
    TLabel *labRed;
    TLabel *Red;
    TLabel *labGreen;
    TLabel *Green;
    TLabel *labBlue;
    TLabel *Blue;
    TMainMenu *FarbMenu;
    TMenuItem *Farben1;
    TMenuItem *EditRGB;
    TMenuItem *EditHSV;
    TMenuItem *N1;
    TMenuItem *RevertChanges;
    TMenuItem *CloseIt;
    TGroupBox *GroupBox1;
    TLabel *labIndex;
    TLabel *Idx;
	void __fastcall pb_palettePaint(TObject *Sender);
	void __fastcall pb_paletteMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall EditRGBClick(TObject *Sender);
    void __fastcall EditHSVClick(TObject *Sender);
    void __fastcall RevertChangesClick(TObject *Sender);
    void __fastcall CloseItClick(TObject *Sender);
    void __fastcall FormKeyPress(TObject *Sender, char &Key);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall pb_paletteDblClick(TObject *Sender);
private:	// User declarations
    COLORREF* oldpal;
	int selcolor;
    bool mousefarbton;
    bool mousesaettigung;
    bool doedit;
	void __fastcall UpdateValues();
    void __fastcall MoveLeft (bool _bigskip=false);
    void __fastcall MoveRight (bool _bigskip=false);
    void __fastcall MoveUp (bool _bigskip=false);
    void __fastcall MoveDown (bool _bigskip=false);
    int __fastcall GetX();
    int __fastcall GetY();
    void __fastcall SetXY (int _x, int _y);
    void __fastcall RawDrawCursor (bool _draw=true);
    void __fastcall DrawCursor();
    void __fastcall EraseCursor();
    void __fastcall LoadLanguage();
public:		// User declarations
	__fastcall TSelColorForm(int _selcolor, TComponent* Owner);
	virtual __fastcall ~TSelColorForm();
};
/*-----------------------------------------------------------------*/
extern TSelColorForm *SelColorForm;
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
