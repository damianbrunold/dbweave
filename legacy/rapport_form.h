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
#ifndef rapport_formH
#define rapport_formH
/*-----------------------------------------------------------------*/
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
/*-----------------------------------------------------------------*/
class TRapportForm : public TForm
{
__published:	// IDE-managed Components
    TLabel *labelHorz;
    TEdit *rapp_horz;
    TLabel *labelVert;
    TEdit *rapp_vert;
    TButton *bOk;
    TButton *bCancel;
    TCheckBox *RappAll;
    TCheckBox *RappColors;
    void __fastcall RappAllClick(TObject *Sender);
private:	// User declarations
    void __fastcall LoadLanguage();
public:		// User declarations
    __fastcall TRapportForm(TComponent* Owner);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
