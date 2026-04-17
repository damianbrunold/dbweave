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
#ifndef farbauswahl_formH
#define farbauswahl_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
/*-----------------------------------------------------------------*/
class TChooseHSVForm : public TForm
{
__published:	// IDE-managed Components
    TPaintBox *pbFarbkreis;
    TLabel *lFarbton;
    TPaintBox *pbSatur;
    TLabel *lSaettigung;
    TLabel *lFarbmuster;
    TPaintBox *pbMuster;
    TButton *bOK;
    TButton *bCancel;
    TLabel *l2Farbton;
    TTrackBar *tbFarbton;
    TLabel *l2Saettigung;
    TTrackBar *tbSaettigung;
    TLabel *l2Helligkeit;
    TTrackBar *tbHelligkeit;
    TLabel *HueVal;
    TLabel *SatVal;
    TLabel *ValVal;
    void __fastcall pbFarbkreisMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall pbFarbkreisPaint(TObject *Sender);
    void __fastcall pbSaturMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall pbSaturPaint(TObject *Sender);
    void __fastcall pbMusterPaint(TObject *Sender);
    void __fastcall tbFarbtonChange(TObject *Sender);
    void __fastcall tbSaettigungChange(TObject *Sender);
    void __fastcall tbHelligkeitChange(TObject *Sender);
private:	// User declarations
    short hue; // Farbton 0-360
    short sat; // Sättigung 0-255
    short val; // Helligkeit 0-255
    Graphics::TBitmap* bmpHue;
    Graphics::TBitmap* bmpSatVal;
    void __fastcall InvalidateRanges();
    void __fastcall UpdateValues(bool _sat=true, bool _val=true);
    void __fastcall LoadLanguage();
public:		// User declarations
    __fastcall TChooseHSVForm(TComponent* Owner);
    virtual __fastcall ~TChooseHSVForm();
    COLORREF __fastcall GetSelectedColor();
    void __fastcall SelectColor (COLORREF _color);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
