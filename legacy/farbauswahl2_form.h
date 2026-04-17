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
#ifndef farbauswahl2_formH
#define farbauswahl2_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
/*-----------------------------------------------------------------*/
class TChoosePaletteForm : public TForm
{
__published:	// IDE-managed Components
    TButton *bOk;
    TButton *bCancel;
    TPaintBox *pbColors;
    TEdit *DummyEdit;
    void __fastcall DummyEditKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall pbColorsPaint(TObject *Sender);
    void __fastcall bOkClick(TObject *Sender);
    void __fastcall DummyEditExit(TObject *Sender);
    void __fastcall DummyEditEnter(TObject *Sender);
    void __fastcall pbColorsMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
private:	// User declarations
    int DRAWSIZE;
    enum { GRIDSIZE=16 };
    int index;
    void __fastcall _RawDrawCursor (bool _erase=false);
    void __fastcall DrawCursor();
    void __fastcall EraseCursor();
    void __fastcall SetCursor (int _scrx, int _scry);
    void __fastcall LoadLanguage();
public:		// User declarations
    __fastcall TChoosePaletteForm(TComponent* Owner);
    COLORREF __fastcall GetSelectedColor();
    int __fastcall GetSelectedIndex();
    void __fastcall SelectColor (COLORREF _color);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
