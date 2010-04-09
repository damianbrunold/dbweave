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
#ifndef farbauswahl1_formH
#define farbauswahl1_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
/*-----------------------------------------------------------------*/
class TChooseRGBForm : public TForm
{
__published:	// IDE-managed Components
    TPaintBox *pbRed;
    TPaintBox *pbGreen;
    TPaintBox *pbBlue;
    TLabel *lRot;
    TLabel *lGruen;
    TLabel *lBlau;
    TTrackBar *tbRed;
    TTrackBar *tbGreen;
    TTrackBar *tbBlue;
    TLabel *lMuster;
    TPaintBox *pbFarbmuster;
    TButton *bOk;
    TButton *bCancel;
    TLabel *RedVal;
    TLabel *GreenVal;
    TLabel *BlueVal;
    void __fastcall pbRedPaint(TObject *Sender);
    void __fastcall pbGreenPaint(TObject *Sender);
    void __fastcall pbBluePaint(TObject *Sender);
    void __fastcall pbFarbmusterPaint(TObject *Sender);
    void __fastcall tbRedChange(TObject *Sender);
    void __fastcall tbGreenChange(TObject *Sender);
    void __fastcall tbBlueChange(TObject *Sender);
private:	// User declarations
    int red;
    int green;
    int blue;
    void __fastcall UpdateValues();
    void __fastcall LoadLanguage();
public:		// User declarations
    __fastcall TChooseRGBForm(TComponent* Owner);
    COLORREF __fastcall GetSelectedColor();
    void __fastcall SelectColor (COLORREF _color);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
