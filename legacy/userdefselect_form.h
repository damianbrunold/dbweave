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
#ifndef userdefselect_formH
#define userdefselect_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
/*-----------------------------------------------------------------*/
class TUserdefSelectForm : public TForm
{
__published:	// IDE-managed Components
    TRadioButton *Userdef1;
    TRadioButton *Userdef2;
    TRadioButton *Userdef3;
    TRadioButton *Userdef4;
    TRadioButton *Userdef5;
    TRadioButton *Userdef6;
    TRadioButton *Userdef7;
    TRadioButton *Userdef8;
    TRadioButton *Userdef9;
    TRadioButton *Userdef10;
    TButton *bOk;
    TButton *bCancel;
private:	// User declarations
    void __fastcall LoadLanguage();
public:		// User declarations
    __fastcall TUserdefSelectForm(TComponent* Owner);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
