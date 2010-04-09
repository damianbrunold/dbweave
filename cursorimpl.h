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
#ifndef cursorimplH
#define cursorimplH
/*-----------------------------------------------------------------*/
#include "dbw3_base.h"
#include "cursor.h"
/*-----------------------------------------------------------------*/
// Forward-Deklarationen
class TDBWFRM;
class TData;
/*-----------------------------------------------------------------*/
struct CrSharedCoord
{
    int x1;
    int x2;
    int y1;
    int y2;
    __fastcall CrSharedCoord() { Init(); }
    void __fastcall Init() { x1 = x2 = y1 = y2 = 0; }
};
/*-----------------------------------------------------------------*/
typedef int CrShareFlags;
enum { CrShareX1=1, CrShareX2=2, CrShareY1=4, CrShareY2=8 };
/*-----------------------------------------------------------------*/
class CrFeld
{
private:
    TDBWFRM* frm;
    TData* data;

public:
    FELD feld;
    FeldBase& fb;
    CrSharedCoord& sharedcoord;
    CrShareFlags   shareflags;
    CURSORDIRECTION cursordirection;

    // Doppelte Verkettung
    CrFeld* prev;
    CrFeld* next;

public:
    __fastcall CrFeld (FELD _feld, TDBWFRM* _frm, TData* _data, FeldBase& _fb, CrSharedCoord& _sharedcoord, CrShareFlags _shareflags, CrFeld* _prev);
    virtual __fastcall ~CrFeld();

    bool __fastcall IsVisible() const;

    void __fastcall Toggle (TShiftState _shift);
    void __fastcall Set (bool _set, TShiftState _shift);
    void __fastcall SetCursor (int _i, int _j);
    void __fastcall CheckCursorPos();

    void __fastcall MoveCursorLeft (int _step, bool _select);
    void __fastcall MoveCursorRight (int _step, bool _select);
    void __fastcall MoveCursorUp (int _step, bool _select);
    void __fastcall MoveCursorDown (int _step, bool _select);

    void __fastcall DrawCursor();
    void __fastcall DeleteCursor();
    void __fastcall ToggleCursor();

    void __fastcall SyncSharedCoord();

protected:
    void __fastcall DisableCursor();
    void __fastcall EnableCursor();
    void __fastcall UpdateSharedCoord (int _i, int _j);
};
/*-----------------------------------------------------------------*/
class CrCursorHandlerImpl : public CrCursorHandler
{
private:
    TDBWFRM* frm;
    TData* data;

    CrFeld* feld;
    CrSharedCoord sharedcoord;
    bool locked;

public:
    __fastcall CrCursorHandlerImpl (TDBWFRM* _frm, TData* _data);
    virtual __fastcall ~CrCursorHandlerImpl();

    virtual void __fastcall Init();

    virtual void __fastcall ToggleField (TShiftState _shift);
    virtual void __fastcall SetField (bool _set, TShiftState _shift);
    virtual void __fastcall SetCursor (FELD _feld, int _i, int _j, bool _clearselection=true);
    virtual void __fastcall CheckCursorPos();

    virtual void __fastcall MoveCursorLeft (int _step, bool _select);
    virtual void __fastcall MoveCursorRight (int _step, bool _select);
    virtual void __fastcall MoveCursorUp (int _step, bool _select);
    virtual void __fastcall MoveCursorDown (int _step, bool _select);

    virtual void __fastcall GotoNextField();
    virtual void __fastcall GotoPrevField();
    virtual void __fastcall GotoField (FELD _feld);
    virtual void __fastcall SetInvisible (FELD _feld);

    virtual void __fastcall DrawCursor();
    virtual void __fastcall DeleteCursor();
    virtual void __fastcall ToggleCursor();

    virtual void __fastcall SetCursorDirection (CURSORDIRECTION _cd);
    virtual CURSORDIRECTION __fastcall GetCursorDirection();

    virtual void __fastcall SetCursorLocked (bool _locked=true);

    virtual void __fastcall DisableCursor();
    virtual void __fastcall EnableCursor();

    void __fastcall CheckLocked();
protected:
    CrFeld* GetFeld (FELD _feld);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
