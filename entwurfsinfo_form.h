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
#ifndef entwurfsinfo_formH
#define entwurfsinfo_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
/*-----------------------------------------------------------------*/
class TDBWFRM;
/*-----------------------------------------------------------------*/
class TEntwurfsinfoForm : public TForm
{
__published:	// IDE-managed Components
    TButton *bOk;
    TButton *bPrint;
    TTreeView *categories;
    TRichEdit *text;
    TButton *bExport;
    TRichEdit *rAusmasse;
    TRichEdit *rFarben;
    TRichEdit *rLitzen;
    TRichEdit *rFlottierungen;
    TRichEdit *dummy;
    TSaveDialog *SaveDialog;
    void __fastcall bPrintClick(TObject *Sender);
    void __fastcall categoriesChange(TObject *Sender, TTreeNode *Node);
    void __fastcall bExportClick(TObject *Sender);
private:
    TDBWFRM* frm;

    TStrings* ausmasse;
    TStrings* farben;
    TStrings* litzen;
    TStrings* flottierungen;

    bool read_ausmasse;
    bool read_farben;
    bool read_litzen;
    bool read_flottierungen;

    void __fastcall CopyStrings (TStrings* _dest, TStrings* _src, bool _append=false);
    void __fastcall InsertHeader();
    void __fastcall InsertHeading (const String& _heading);

    void __fastcall LoadLanguage();

public:		// User declarations
    __fastcall TEntwurfsinfoForm(TDBWFRM* _frm, TComponent* Owner);
    void __fastcall InitInfos();
    void __fastcall OnAusmasse (bool _clear=true);
    void __fastcall OnFarben (bool _clear=true);
    void __fastcall OnLitzen (bool _clear=true);
    void __fastcall OnFlottierungen (bool _clear=true);
};
/*-----------------------------------------------------------------*/
#endif
