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
#ifndef userdef_entername_formH
#define userdef_entername_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
/*-----------------------------------------------------------------*/
class TUserdefEnternameForm : public TForm
{
__published:	// IDE-managed Components
    TLabel *labName;
    TEdit *Name;
    TButton *bOK;
    TButton *bCancel;
private:	// User declarations
    void __fastcall LoadLanguage();
public:		// User declarations
    __fastcall TUserdefEnternameForm(TComponent* Owner);
};
/*-----------------------------------------------------------------*/
String __fastcall GetUserdefName (String _default);
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
