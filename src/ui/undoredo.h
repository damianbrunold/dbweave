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
#ifndef DBWEAVE_UI_UNDOREDO_H
#define DBWEAVE_UI_UNDOREDO_H
/*-----------------------------------------------------------------*/
#include "dbw3_base.h"
/*-----------------------------------------------------------------*/
class TDBWFRM;
class QAction;
/*-----------------------------------------------------------------*/
class UrUndoItem
{
protected:
	// Daten
	bool active;
	FeldVectorShort* einzug;
	FeldGridChar*    aufknuepfung;
	FeldGridChar*    trittfolge;
	FeldVectorBool*  isempty;
	FeldVectorChar*  schussfarben;
	FeldVectorChar*  kettfarben;
	FeldVectorChar*  blatteinzug;
	FELD     kbd_feld;
	INPUTPOS kbd_pos;
	bool     pegplan;
	QAction* einzugstil;      /* was TMenuItem* in VCL */
	QAction* trittfolgenstil;

	// Verkettung
	UrUndoItem* next;
	UrUndoItem* prev;

	void Allocate (TDBWFRM* _mainfrm);

public:
 UrUndoItem (TDBWFRM* _mainfrm, UrUndoItem* _prev=0, UrUndoItem* _next=0);
 ~UrUndoItem();

	void UpdateSize();
	void SetData (TDBWFRM* _mainfrm);

	bool IsEmpty() const { return !active; }
	void Clean();

	void Undo (TDBWFRM* _mainfrm);
	void Redo (TDBWFRM* _mainfrm);

	UrUndoItem* Next() const { return next; }
	UrUndoItem* Prev() const { return prev; }

	void SetNext (UrUndoItem* _next) { next = _next; }
	void SetPrev (UrUndoItem* _prev) { prev = _prev; }
};
/*-----------------------------------------------------------------*/
class UrUndo
{
protected:
	TDBWFRM*    mainfrm;
	int         maxundo;
	UrUndoItem* first;
	UrUndoItem* current;
	bool        locked;

public:
 UrUndo (TDBWFRM* _mainfrm, int _maxundo=100);
 ~UrUndo();

	void Clear();
	void UpdateSize();

	bool Undo();
	bool Redo();

	bool CanUndo();
	bool CanRedo();

	void Snapshot();

protected:
	void Init (int _maxundo);
	void Cleanup();
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
