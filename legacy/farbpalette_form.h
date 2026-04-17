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
#ifndef farbpalette_formH
#define farbpalette_formH
/*-----------------------------------------------------------------*/
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <ExtCtrls.hpp>
/*-----------------------------------------------------------------*/
class TFarbPalette : public TForm
{
__published:    // IDE-managed Components
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall FormKeyPress(TObject *Sender, char &Key);
private:    // User declarations
    void __fastcall _RawDrawCursor (bool _erase=false);
    void __fastcall DrawCursor();
    void __fastcall EraseCursor();
    void __fastcall SetCursor (int _scrx, int _scry);
    int DRAWSIZE;
public:        // User declarations
    __fastcall TFarbPalette(TComponent* Owner);
    void __fastcall UpdatePalette();
    void __fastcall ReloadLanguage();
};
/*-----------------------------------------------------------------*/
extern PACKAGE TFarbPalette *FarbPalette;
/*-----------------------------------------------------------------*/
#endif
