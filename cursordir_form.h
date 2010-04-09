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
#ifndef cursordir_formH
#define cursordir_formH
/*-----------------------------------------------------------------*/
#include "cursor.h"
#include <Buttons.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
/*-----------------------------------------------------------------*/
class TCursorDirForm : public TForm
{
__published:	// IDE-managed Components
    TSpeedButton *cdUp;
    TSpeedButton *cdLeft;
    TSpeedButton *cdRight;
    TSpeedButton *cdDown;
    TButton *bOk;
    TButton *bCancel;
    void __fastcall cdUpClick(TObject *Sender);
    void __fastcall cdLeftClick(TObject *Sender);
    void __fastcall cdRightClick(TObject *Sender);
    void __fastcall cdDownClick(TObject *Sender);
    void __fastcall bOkClick(TObject *Sender);
    void __fastcall bCancelClick(TObject *Sender);
    void __fastcall FormMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
private:	// User declarations
    void __fastcall LoadLanguage();
public:		// User declarations
    CURSORDIRECTION cursordirection;
    __fastcall TCursorDirForm(TComponent* Owner, CURSORDIRECTION _cd, int _dummy);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
