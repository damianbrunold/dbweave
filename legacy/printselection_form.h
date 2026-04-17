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

#ifndef printselection_formH
#define printselection_formH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "dbw3_base.h"

class TPrintSelectionForm : public TForm
{
__published:	// IDE-managed Components
    TGroupBox *gbKette;
    TLabel *lKetteVon;
    TEdit *KetteVon;
    TLabel *lKetteBis;
    TEdit *KetteBis;
    TGroupBox *gbSchuesse;
    TLabel *lSchussVon;
    TLabel *lSchussBis;
    TEdit *SchussVon;
    TEdit *SchussBis;
    TGroupBox *gbSchaefte;
    TLabel *lSchaftVon;
    TLabel *lSchaftBis;
    TEdit *SchaftVon;
    TEdit *SchaftBis;
    TGroupBox *gbTritte;
    TLabel *lTrittVon;
    TLabel *lTrittBis;
    TEdit *TrittVon;
    TEdit *TrittBis;
    TButton *bOk;
    TButton *bCancel;
    void __fastcall bOkClick(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
public:
    SZ kette;
    SZ schuesse;
    SZ schaefte;
    SZ tritte;

    int maxkette;
    int maxschuesse;
    int maxschaefte;
    int maxtritte;

private:
    SZ __fastcall GetRange (TEdit* _von, TEdit* _bis);
    bool __fastcall CheckRange(const SZ& _range, int _max);
    void __fastcall LoadLanguage();
public:		// User declarations
    __fastcall TPrintSelectionForm(TComponent* Owner);
};

extern PACKAGE TPrintSelectionForm *PrintSelectionForm;

#endif

