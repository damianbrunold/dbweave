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
#ifndef farbverlauf_formH
#define farbverlauf_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
/*-----------------------------------------------------------------*/
class TDBWFRM;
/*-----------------------------------------------------------------*/
class TFarbverlaufForm : public TForm
{
__published:	// IDE-managed Components
    TPaintBox *pbStartfarbe;
    TPaintBox *pbEndfarbe;
    TLabel *lStufen;
    TEdit *Abstufungen;
    TPaintBox *pbFarbverlauf;
    TLabel *lPosition;
    TEdit *Position;
    TButton *bOk;
    TButton *bCancel;
    TCheckBox *nodividers;
    TTrackBar *Gewichtung;
    TLabel *labGewichtung;
    TGroupBox *gbColormodel;
    TRadioButton *ModelRGB;
    TRadioButton *ModelHSV;
    TButton *bStartcolor;
    TButton *bEndcolor;
    TPopupMenu *PopupMenuFarbwahl;
    TMenuItem *ChooseColorHSV;
    TMenuItem *ChooseColorRGB;
    TMenuItem *ChooseColorPalette;
    TMenuItem *ChooseColorOther;
    void __fastcall pbStartfarbeClick(TObject *Sender);
    void __fastcall pbEndfarbeClick(TObject *Sender);
    void __fastcall pbFarbverlaufClick(TObject *Sender);
    void __fastcall bOkClick(TObject *Sender);
    void __fastcall pbStartfarbePaint(TObject *Sender);
    void __fastcall pbEndfarbePaint(TObject *Sender);
    void __fastcall pbFarbverlaufPaint(TObject *Sender);
    void __fastcall AbstufungenChange(TObject *Sender);
    void __fastcall nodividersClick(TObject *Sender);
    void __fastcall GewichtungChange(TObject *Sender);
    void __fastcall bStartcolorClick(TObject *Sender);
    void __fastcall bEndcolorClick(TObject *Sender);
    void __fastcall ModelRGBClick(TObject *Sender);
    void __fastcall ModelHSVClick(TObject *Sender);
    void __fastcall ChooseColorHSVClick(TObject *Sender);
    void __fastcall ChooseColorRGBClick(TObject *Sender);
    void __fastcall ChooseColorPaletteClick(TObject *Sender);
    void __fastcall ChooseColorOtherClick(TObject *Sender);
private:	// User declarations
    TDBWFRM* frm;
    COLORREF startcolor;
    COLORREF endcolor;
    COLORREF table[MAX_PAL_ENTRY];
    int count;
    bool start;
    void __fastcall CreateFarbverlauf();
    void __fastcall FarbverlaufRGB (int _abstufungen);
    void __fastcall FarbverlaufHSV (int _abstufungen);
    COLORREF __fastcall SelectColorRGB (COLORREF _col);
    COLORREF __fastcall SelectColorHSV (COLORREF _col);
    COLORREF __fastcall SelectColorPalette (COLORREF _col);
    void __fastcall LoadLanguage();
public:		// User declarations
    __fastcall TFarbverlaufForm(TDBWFRM* _frm, TComponent* Owner);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
