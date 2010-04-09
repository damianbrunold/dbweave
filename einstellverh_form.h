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
#ifndef einstellverh_formH
#define einstellverh_formH
/*-----------------------------------------------------------------*/
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
/*-----------------------------------------------------------------*/
class TEinstellVerhaeltnisForm : public TForm
{
__published:    // IDE-managed Components
    TLabel *lSchuss;
    TEdit *FaktorSchuss;
    TLabel *lKette;
    TEdit *FaktorKette;
    TButton *bOk;
    TButton *bCancel;
private:    // User declarations
    void __fastcall LoadLanguage();
public:        // User declarations
    __fastcall TEinstellVerhaeltnisForm(TComponent* Owner);
};
/*-----------------------------------------------------------------*/
extern PACKAGE TEinstellVerhaeltnisForm *EinstellVerhaeltnisForm;
/*-----------------------------------------------------------------*/
#endif
