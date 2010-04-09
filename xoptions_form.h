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

#ifndef xoptions_formH
#define xoptions_formH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>

class TXOptionsForm : public TForm
{
__published:	// IDE-managed Components
    TPageControl *OptOptions;
    TButton *bOk;
    TButton *bCancel;
    TTabSheet *OptAusmasse;
    TTabSheet *OptSchenien;
    TTabSheet *OptSettings;
    TTabSheet *OptSymbols;
    TTabSheet *OptDivers;
    TCheckBox *FEWithRaster;
    TCheckBox *AltFarbpalette;
    TCheckBox *AltLiftplanstyle;
    TComboBox *cbEinzug;
    TLabel *lEinzug;
    TLabel *lTrittfolge;
    TComboBox *cbTrittfolge;
    TLabel *lAufknuepfung;
    TComboBox *cbAufknuepfung;
    TLabel *lSchlagpatrone;
    TComboBox *cbSchlagpatrone;
    TCheckBox *EinzugUnten;
    TCheckBox *RightToLeft;
    TGroupBox *gbTrittvergabe;
    TRadioButton *Einzeltritt;
    TRadioButton *Multitritt;
    TGroupBox *gbRisingSinking;
    TRadioButton *RisingShed;
    TRadioButton *SinkingShed;
    TGroupBox *gbSchenien;
    TLabel *lHorizontal;
    TEdit *SchenienHorz;
    TUpDown *upSchenienHorz;
    TLabel *lVertikal;
    TEdit *SchenienVert;
    TUpDown *upSchenienVert;
    TGroupBox *gbFeldgroessen;
    TGroupBox *gbVisibility;
    TLabel *lSchaefte;
    TEdit *Schaefte;
    TUpDown *upSchaefte;
    TLabel *lTritte;
    TEdit *Tritte;
    TUpDown *upTritte;
    TLabel *lKettfaeden;
    TEdit *Kette;
    TUpDown *upKette;
    TLabel *lSchuesse;
    TEdit *Schuesse;
    TUpDown *upSchuesse;
    TLabel *lSchaefteVis;
    TEdit *SchaefteVis;
    TUpDown *upSchaefteVis;
    TLabel *lTritteVis;
    TEdit *TritteVis;
    TUpDown *upTritteVis;
    TLabel *lAushebung;
    TLabel *lAnbindung;
    TLabel *lAbbindung;
    TComboBox *cbAushebung;
    TComboBox *cbAnbindung;
    TComboBox *cbAbbindung;
    TCheckBox *TopToBottom;
private:	// User declarations
    void __fastcall LoadLanguage();
    void __fastcall LoadCombo(TComboBox* _cb, bool _withnumber=false);
public:		// User declarations
    __fastcall TXOptionsForm(TComponent* Owner);
};

#endif

