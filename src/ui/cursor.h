/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*-----------------------------------------------------------------*/
#ifndef DBWEAVE_UI_CURSOR_H
#define DBWEAVE_UI_CURSOR_H
/*-----------------------------------------------------------------*/
#include "vcl_compat.h"      /* */
#include "enums.h"
#include "shift_compat.h"
/*-----------------------------------------------------------------*/
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
	virtual ~CrCursorHandler() {}

	virtual void Init() = 0;

	virtual void ToggleField (TShiftState _shift) = 0;
	virtual void SetField (bool _set, TShiftState _shift) = 0;
	virtual void SetCursor (FELD _feld, int _i, int _j, bool _clearselection=true) = 0;
	virtual void CheckCursorPos() = 0;

	virtual void MoveCursorLeft (int _steps, bool _select) = 0;
	virtual void MoveCursorRight (int _steps, bool _select) = 0;
	virtual void MoveCursorUp (int _steps, bool _select) = 0;
	virtual void MoveCursorDown (int _steps, bool _select) = 0;

	virtual void GotoNextField() = 0;
	virtual void GotoPrevField() = 0;
	virtual void GotoField (FELD _feld) = 0;
	virtual void SetInvisible (FELD _feld) = 0;

	virtual void DrawCursor() = 0;
	virtual void DeleteCursor() = 0;
	virtual void ToggleCursor() = 0;

	virtual void SetCursorDirection (CURSORDIRECTION _cd) = 0;
	virtual CURSORDIRECTION GetCursorDirection() = 0;

	virtual void SetCursorLocked (bool _locked=true) = 0;

	virtual void DisableCursor() = 0;
	virtual void EnableCursor() = 0;

	virtual void CheckLocked() = 0;

	/*  Accessor exposing the current focused feld. Not in the legacy
	    public interface but useful for tests without reaching into the
	    implementation. */
	virtual FELD CurrentFeld() const = 0;

public:
	static CrCursorHandler* CreateInstance (TDBWFRM* _frm, TData* _data);
	static void Release (CrCursorHandler* _cursorhandler);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
