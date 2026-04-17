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
#ifndef techinfo_formH
#define techinfo_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
/*-----------------------------------------------------------------*/
class TTechinfoForm : public TForm
{
__published:	// IDE-managed Components
    TButton *bOk;
    TRichEdit *infos;
private:	// User declarations
    void __fastcall LoadLanguage();
    void __fastcall GatherData();
    void __fastcall Add (const String& _data, bool _bold=false);
public:		// User declarations
    __fastcall TTechinfoForm(TComponent* Owner);
    __fastcall ~TTechinfoForm();
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
