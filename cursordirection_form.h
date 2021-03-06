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
#ifndef cursordirection_formH
#define cursordirection_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include "cursor.h"
/*-----------------------------------------------------------------*/
class TCursordirectionForm : public TForm
{
__published:	// IDE-managed Components
    TLabel *Label1;
    TSpeedButton *cdUp;
    TSpeedButton *cdDown;
    TSpeedButton *cdLeft;
    TSpeedButton *cdRight;
    TButton *bOk;
    TButton *bCancel;
    void __fastcall bOkClick(TObject *Sender);
    void __fastcall bCancelClick(TObject *Sender);
    void __fastcall cdUpClick(TObject *Sender);
    void __fastcall cdLeftClick(TObject *Sender);
    void __fastcall cdRightClick(TObject *Sender);
    void __fastcall cdDownClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
    CURSORDIRECTION cursordirection;
    __fastcall TCursordirectionForm(TComponent* Owner, CURSORDIRECTION _cd);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
