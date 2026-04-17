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
#ifndef fixeinzug_formH
#define fixeinzug_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
/*-----------------------------------------------------------------*/
#include "cursor.h"
/*-----------------------------------------------------------------*/
class TDBWFRM;
/*-----------------------------------------------------------------*/
class TFixeinzugForm : public TForm
{
__published:	// IDE-managed Components
    TPaintBox *pbFixeinzug;
    TMainMenu *FixMenu;
    TMenuItem *MenuEinzug;
    TMenuItem *Revert;
    TMenuItem *Close;
    TScrollBar *sbHorz;
    TScrollBar *sbVert;
    TMenuItem *Grab;
    TMenuItem *N1;
    TMenuItem *Delete;
    void __fastcall RevertClick(TObject *Sender);
    void __fastcall CloseClick(TObject *Sender);
    void __fastcall pbFixeinzugMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall pbFixeinzugPaint(TObject *Sender);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall FormResize(TObject *Sender);
    void __fastcall GrabClick(TObject *Sender);
    void __fastcall DeleteClick(TObject *Sender);
private:	// User declarations
    TDBWFRM* frm;
    short* einzug;
    short firstfree;
    int   size;
    int dx, dy;
    int cx, cy;
    int mx, my;
    int maxx, maxy;
    int scrollx, scrolly;
    CURSORDIRECTION cursordirection;
    CrCursorHandler* cursorhandler;
protected:
    void __fastcall RecalcDimensions();
    void __fastcall DrawGrid (TPaintBox* _pb);
    void __fastcall DrawData (TPaintBox* _pb);
    void __fastcall DrawData (TPaintBox* _pb, int _i);
    void __fastcall _DrawCursor (TColor _col);
    void __fastcall DrawCursor();
    void __fastcall DeleteCursor();
    void __fastcall CalcRange();
    short __fastcall CalcFirstFree();
    void __fastcall DrawRange();
    void __fastcall DeleteRange();
    void __fastcall _DrawRange (bool _paint, TPaintBox* _pb);
    void __fastcall CurDirLeft();
    void __fastcall CurDirRight();
    void __fastcall CurDirUp();
    void __fastcall CurDirDown();
    void __fastcall CurLeft (bool _step);
    void __fastcall CurRight (bool _step);
    void __fastcall CurUp (bool _step);
    void __fastcall CurDown (bool _step);
    void __fastcall CurMove();
    void __fastcall LoadLanguage();
public:		// User declarations
    bool changed;
    __fastcall TFixeinzugForm(TComponent* Owner, TDBWFRM* _frm, CrCursorHandler* _ch);
};
/*-----------------------------------------------------------------*/
#endif
