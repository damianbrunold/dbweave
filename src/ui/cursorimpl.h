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
#ifndef DBWEAVE_UI_CURSORIMPL_H
#define DBWEAVE_UI_CURSORIMPL_H
/*-----------------------------------------------------------------*/
#include "dbw3_base.h"
#include "cursor.h"
/*-----------------------------------------------------------------*/
class TDBWFRM;
class TData;
/*-----------------------------------------------------------------*/
struct CrSharedCoord
{
	int x1;
	int x2;
	int y1;
	int y2;
 CrSharedCoord() { Init(); }
	void Init() { x1 = x2 = y1 = y2 = 0; }
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
 CrFeld (FELD _feld, TDBWFRM* _frm, TData* _data, FeldBase& _fb, CrSharedCoord& _sharedcoord, CrShareFlags _shareflags, CrFeld* _prev);
	virtual ~CrFeld();

	bool IsVisible() const;

	void Toggle (TShiftState _shift);
	void Set (bool _set, TShiftState _shift);
	void SetCursor (int _i, int _j);
	void CheckCursorPos();

	void MoveCursorLeft (int _step, bool _select);
	void MoveCursorRight (int _step, bool _select);
	void MoveCursorUp (int _step, bool _select);
	void MoveCursorDown (int _step, bool _select);

	void DrawCursor();
	void DeleteCursor();
	void ToggleCursor();

	void SyncSharedCoord();

protected:
	void DisableCursor();
	void EnableCursor();
	void UpdateSharedCoord (int _i, int _j);
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
 CrCursorHandlerImpl (TDBWFRM* _frm, TData* _data);
	virtual ~CrCursorHandlerImpl();

	virtual void Init();

	virtual void ToggleField (TShiftState _shift);
	virtual void SetField (bool _set, TShiftState _shift);
	virtual void SetCursor (FELD _feld, int _i, int _j, bool _clearselection=true);
	virtual void CheckCursorPos();

	virtual void MoveCursorLeft (int _step, bool _select);
	virtual void MoveCursorRight (int _step, bool _select);
	virtual void MoveCursorUp (int _step, bool _select);
	virtual void MoveCursorDown (int _step, bool _select);

	virtual void GotoNextField();
	virtual void GotoPrevField();
	virtual void GotoField (FELD _feld);
	virtual void SetInvisible (FELD _feld);

	virtual void DrawCursor();
	virtual void DeleteCursor();
	virtual void ToggleCursor();

	virtual void SetCursorDirection (CURSORDIRECTION _cd);
	virtual CURSORDIRECTION GetCursorDirection();

	virtual void SetCursorLocked (bool _locked=true);

	virtual void DisableCursor();
	virtual void EnableCursor();

	virtual void CheckLocked();

	virtual FELD CurrentFeld() const { return feld ? feld->feld : INVALID; }

protected:
	CrFeld* GetFeld (FELD _feld);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
