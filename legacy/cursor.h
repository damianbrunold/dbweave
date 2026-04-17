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
#ifndef cursorH
#define cursorH
/*-----------------------------------------------------------------*/
#include "enums.h"
/*-----------------------------------------------------------------*/
// Forward-Deklarationen
class TDBWFRM;
class TData;
/*-----------------------------------------------------------------*/
typedef int CURSORDIRECTION;
/*-----------------------------------------------------------------*/
enum enumCURSORDIRECTION
{
    CD_NONE = 0,
    CD_DEFAULT = 1,
    CD_UP = 1,
    CD_LEFT = 2,
    CD_DOWN = 4,
    CD_RIGHT = 8
};
/*-----------------------------------------------------------------*/
// Interface
class CrCursorHandler
{
public:
    virtual __fastcall ~CrCursorHandler() {}

    virtual void __fastcall Init() = 0;

    virtual void __fastcall ToggleField (TShiftState _shift) = 0;
    virtual void __fastcall SetField (bool _set, TShiftState _shift) = 0;
    virtual void __fastcall SetCursor (FELD _feld, int _i, int _j, bool _clearselection=true) = 0;
    virtual void __fastcall CheckCursorPos() = 0;

    virtual void __fastcall MoveCursorLeft (int _steps, bool _select) = 0;
    virtual void __fastcall MoveCursorRight (int _steps, bool _select) = 0;
    virtual void __fastcall MoveCursorUp (int _steps, bool _select) = 0;
    virtual void __fastcall MoveCursorDown (int _steps, bool _select) = 0;

    virtual void __fastcall GotoNextField() = 0;
    virtual void __fastcall GotoPrevField() = 0;
    virtual void __fastcall GotoField (FELD _feld) = 0;
    virtual void __fastcall SetInvisible (FELD _feld) = 0;

    virtual void __fastcall DrawCursor() = 0;
    virtual void __fastcall DeleteCursor() = 0;
    virtual void __fastcall ToggleCursor() = 0;

    virtual void __fastcall SetCursorDirection (CURSORDIRECTION _cd) = 0;
    virtual CURSORDIRECTION __fastcall GetCursorDirection() = 0;

    virtual void __fastcall SetCursorLocked (bool _locked=true) = 0;

    virtual void __fastcall DisableCursor() = 0;
    virtual void __fastcall EnableCursor() = 0;

    virtual void __fastcall CheckLocked() = 0;
public:
    static CrCursorHandler* __fastcall CreateInstance (TDBWFRM* _frm, TData* _data);
    static void __fastcall Release (CrCursorHandler* _cursorhandler);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
