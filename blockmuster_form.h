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
#ifndef blockmuster_formH
#define blockmuster_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
/*-----------------------------------------------------------------*/
#include "blockmuster.h"
#include "cursor.h"
/*-----------------------------------------------------------------*/
enum { maxx = 12 };
enum { maxy = 12 };
/*-----------------------------------------------------------------*/
class TBlockmusterForm : public TForm
{
__published:	// IDE-managed Components
    TMainMenu *BlockmusterMenu;
    TMenuItem *BlockMuster;
    TMenuItem *BlockDoit;
    TMenuItem *BlockClose;
    TMenuItem *BlockEinzug;
    TMenuItem *EinzugZ;
    TMenuItem *EinzugS;
    TMenuItem *BlockTrittfolge;
    TMenuItem *TrittfolgeZ;
    TMenuItem *TrittfolgeS;
    TPaintBox *pbBindung;
    TMenuItem *MenuBindung1;
    TMenuItem *EditDelete;
    TMenuItem *EditSpiegelnH;
    TMenuItem *EditSpiegelnV;
    TMenuItem *EditRotieren;
    TMenuItem *EditInvertieren;
    TMenuItem *N3;
    TMenuItem *Muster1;
    TMenuItem *Leinwand;
    TMenuItem *Koeper22;
    TMenuItem *Koeper33;
    TMenuItem *Koeper44;
    TMenuItem *Kettkoeper21;
    TMenuItem *Kettkoeper31;
    TMenuItem *Kettkoeper32;
    TMenuItem *Kettkoeper41;
    TMenuItem *Kettkoeper51;
    TMenuItem *Kettkoeper52;
    TMenuItem *Atlas5;
    TMenuItem *Atlas7;
    TMenuItem *Atlas9;
    TMenuItem *Panama21;
    TMenuItem *Panama22;
    TMenuItem *LeftRoll;
    TMenuItem *EditRollUp;
    TMenuItem *EditRollDown;
    TMenuItem *EditRollLeft;
    TMenuItem *EditRollRight;
    TMenuItem *N5;
    TMenuItem *MusterUndo;
    TMenuItem *MusterRedo;
    TTimer *updatetimer;
    TMenuItem *Kettkoeper42;
    TMenuItem *Kettkoeper43;
    TMenuItem *Kettkoeper53;
    TLabel *description;
    TMenuItem *N1;
    TMenuItem *Bindung0;
    TMenuItem *Bindung1;
    TMenuItem *Bindung2;
    TMenuItem *Bindung3;
    TMenuItem *Bindung4;
    TMenuItem *Bindung5;
    TMenuItem *Bindung6;
    TMenuItem *Bindung7;
    TMenuItem *Bindung8;
    TMenuItem *Bindung9;
    TMenuItem *CopyFrom;
    TMenuItem *CopyFromB0;
    TMenuItem *CopyFromB1;
    TMenuItem *CopyFromB9;
    TMenuItem *CopyFromB8;
    TMenuItem *CopyFromB7;
    TMenuItem *CopyFromB6;
    TMenuItem *CopyFromB5;
    TMenuItem *CopyFromB4;
    TMenuItem *CopyFromB3;
    TMenuItem *CopyFromB2;
    TLabel *belegte;
    TPaintBox *pbUsed;
    TMenuItem *EditCentralsym;
    void __fastcall pbBindungPaint(TObject *Sender);
    void __fastcall pbBindungMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall BlockDoitClick(TObject *Sender);
    void __fastcall BlockCloseClick(TObject *Sender);
    void __fastcall EinzugSClick(TObject *Sender);
    void __fastcall EinzugZClick(TObject *Sender);
    void __fastcall TrittfolgeZClick(TObject *Sender);
    void __fastcall TrittfolgeSClick(TObject *Sender);
    void __fastcall EditDeleteClick(TObject *Sender);
    void __fastcall EditSpiegelnHClick(TObject *Sender);
    void __fastcall EditSpiegelnVClick(TObject *Sender);
    void __fastcall EditRotierenClick(TObject *Sender);
    void __fastcall EditInvertierenClick(TObject *Sender);
    void __fastcall LeinwandClick(TObject *Sender);
    void __fastcall Koeper22Click(TObject *Sender);
    void __fastcall Koeper33Click(TObject *Sender);
    void __fastcall Koeper44Click(TObject *Sender);
    void __fastcall Atlas5Click(TObject *Sender);
    void __fastcall Atlas7Click(TObject *Sender);
    void __fastcall Atlas9Click(TObject *Sender);
    void __fastcall Panama21Click(TObject *Sender);
    void __fastcall Panama22Click(TObject *Sender);
    void __fastcall EditRollUpClick(TObject *Sender);
    void __fastcall EditRollDownClick(TObject *Sender);
    void __fastcall EditRollLeftClick(TObject *Sender);
    void __fastcall EditRollRightClick(TObject *Sender);
    void __fastcall MusterUndoClick(TObject *Sender);
    void __fastcall MusterRedoClick(TObject *Sender);
    void __fastcall updatetimerTimer(TObject *Sender);
    void __fastcall FormActivate(TObject *Sender);
    void __fastcall FormDeactivate(TObject *Sender);
    void __fastcall Kettkoeper21Click(TObject *Sender);
    void __fastcall Kettkoeper31Click(TObject *Sender);
    void __fastcall Kettkoeper41Click(TObject *Sender);
    void __fastcall Kettkoeper51Click(TObject *Sender);
    void __fastcall Kettkoeper32Click(TObject *Sender);
    void __fastcall Kettkoeper42Click(TObject *Sender);
    void __fastcall Kettkoeper52Click(TObject *Sender);
    void __fastcall Kettkoeper43Click(TObject *Sender);
    void __fastcall Kettkoeper53Click(TObject *Sender);
    void __fastcall Bindung0Click(TObject *Sender);
    void __fastcall Bindung1Click(TObject *Sender);
    void __fastcall Bindung2Click(TObject *Sender);
    void __fastcall Bindung3Click(TObject *Sender);
    void __fastcall Bindung4Click(TObject *Sender);
    void __fastcall Bindung5Click(TObject *Sender);
    void __fastcall Bindung6Click(TObject *Sender);
    void __fastcall Bindung7Click(TObject *Sender);
    void __fastcall Bindung8Click(TObject *Sender);
    void __fastcall Bindung9Click(TObject *Sender);
    void __fastcall CopyFromB0Click(TObject *Sender);
    void __fastcall CopyFromB1Click(TObject *Sender);
    void __fastcall CopyFromB2Click(TObject *Sender);
    void __fastcall CopyFromB3Click(TObject *Sender);
    void __fastcall CopyFromB4Click(TObject *Sender);
    void __fastcall CopyFromB5Click(TObject *Sender);
    void __fastcall CopyFromB6Click(TObject *Sender);
    void __fastcall CopyFromB7Click(TObject *Sender);
    void __fastcall CopyFromB8Click(TObject *Sender);
    void __fastcall CopyFromB9Click(TObject *Sender);
    void __fastcall pbUsedPaint(TObject *Sender);
    void __fastcall EditCentralsymClick(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
private:	// User declarations
    int dx;
    int dy;
    int cx; // Cursorposition
    int cy;
    TColor __fastcall GetColor (int _s);
    void __fastcall Neuzeichnen();
    void __fastcall DrawGrid (TPaintBox* _pb);
    void __fastcall DrawCursor();
    void __fastcall DeleteCursor();
    void __fastcall CalcRange();
    void __fastcall DrawRange();
    void __fastcall DeleteRange();
    void __fastcall _DrawRange (bool _paint, TPaintBox* _pb);
    void __fastcall MusterKoeper (int _current, int _h, int _s);
    void __fastcall MusterAtlas (int _current, int _n);
    void __fastcall MusterPanama (int _current, int _h, int _s);
    void __fastcall RollUp (Muster& _b);
    void __fastcall RollDown (Muster& _b);
    void __fastcall RollLeft (Muster& _b);
    void __fastcall RollRight (Muster& _b);
    void __fastcall Grab(int _from);
    char __fastcall Toggle (char _s);
    void __fastcall LoadLanguage();
protected:
    CURSORDIRECTION cursordirection;
    CrCursorHandler* cursorhandler;
    BlockUndo& undo;
    int& current;
    PMUSTERARRAY bindungen;
    bool withrange;
public:
    int mx; // Belegter Bereich
    int my;
public:		// User declarations
    __fastcall TBlockmusterForm(TComponent* Owner, BlockUndo& _undo,
        PMUSTERARRAY _bindungen, int& _current, CrCursorHandler* _ch,
        String _caption, bool _eztr);
};
/*-----------------------------------------------------------------*/
extern PACKAGE TBlockmusterForm *BlockmusterForm;
/*-----------------------------------------------------------------*/
#endif
