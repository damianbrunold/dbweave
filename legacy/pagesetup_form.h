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
#ifndef pagesetup_formH
#define pagesetup_formH
/*-----------------------------------------------------------------*/
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
/*-----------------------------------------------------------------*/
class TPageSetupForm : public TForm
{
__published:	// IDE-managed Components
	TGroupBox *gb_borders;
	TLabel *l_left;
	TEdit *left;
	TLabel *l_rechts;
	TEdit *right;
	TLabel *l_top;
	TEdit *top;
	TLabel *l_bottom;
	TEdit *bottom;
	TButton *b_ok;
	TButton *b_cancel;
	TLabel *l_header;
	TEdit *headertext;
	TLabel *l_footertext;
	TEdit *footertext;
private:	// User declarations
public:		// User declarations
	__fastcall TPageSetupForm(TComponent* Owner);
};
/*-----------------------------------------------------------------*/
extern PACKAGE TPageSetupForm *PageSetupForm;
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
