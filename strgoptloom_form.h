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

#ifndef strgoptloom_formH
#define strgoptloom_formH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "combase.h"
#include <ExtCtrls.hpp>
#include "loominterface.h"

class TStrgOptLoomForm : public TForm
{
__published:	// IDE-managed Components
    TLabel *lWebstuhl;
    TComboBox *loomlist;
    TButton *bOk;
    TButton *bCancel;
    TPanel *PanelSerial;
    TLabel *lPort;
    TComboBox *portlist;
    TPanel *PanelParallel;
    TLabel *lParallelport;
    TComboBox *lptlist;
    TButton *Info;
    TLabel *lDelay;
    TComboBox *Delay;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall bOkClick(TObject *Sender);
    void __fastcall bCancelClick(TObject *Sender);
    void __fastcall loomlistChange(TObject *Sender);
    void __fastcall InfoClick(TObject *Sender);
private:	// User declarations
    void __fastcall UpdateOptions();
    void __fastcall LoadLanguage();
public:		// User declarations
    LOOMINTERFACE intrf;
    PORT port;
    LPT  lpt;
    int  delay;
    bool IsNT;
    __fastcall TStrgOptLoomForm(TComponent* Owner);
};

#endif

