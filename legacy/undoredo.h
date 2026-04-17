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
#ifndef undoredoH
#define undoredoH
/*-----------------------------------------------------------------*/
#include "dbw3_base.h"
/*-----------------------------------------------------------------*/
// Forward-Deklarationen
class TDBWFRM;
/*-----------------------------------------------------------------*/
class UrUndoItem
{
protected:
    // Daten
    bool active;
    FeldVectorShort* einzug;
    FeldGridChar* aufknuepfung;
    FeldGridChar* trittfolge;
    FeldVectorBool* isempty;
    FeldVectorChar* schussfarben;
    FeldVectorChar* kettfarben;
    FeldVectorChar* blatteinzug;
    FELD kbd_feld;
    INPUTPOS kbd_pos;
    bool pegplan;
    TMenuItem* einzugstil;
    TMenuItem* trittfolgenstil;

    // Verkettung
	UrUndoItem* next;
	UrUndoItem* prev;

    void __fastcall Allocate (TDBWFRM* _mainfrm);

public:
	__fastcall UrUndoItem (TDBWFRM* _mainfrm, UrUndoItem* _prev=0, UrUndoItem* _next=0);
	__fastcall ~UrUndoItem();

    void __fastcall UpdateSize();
    void __fastcall SetData (TDBWFRM* _mainfrm);

	bool __fastcall IsEmpty() const { return !active; }
	void __fastcall Clean();

	void __fastcall Undo (TDBWFRM* _mainfrm);
	void __fastcall Redo (TDBWFRM* _mainfrm);

	UrUndoItem* __fastcall Next() const { return next; }
	UrUndoItem* __fastcall Prev() const { return prev; }

	void __fastcall SetNext (UrUndoItem* _next) { next = _next; }
	void __fastcall SetPrev (UrUndoItem* _prev) { prev = _prev; }
};
/*-----------------------------------------------------------------*/
class UrUndo
{
protected:
	TDBWFRM* mainfrm;
    int maxundo;
	UrUndoItem* first;
	UrUndoItem* current;
    bool locked;

public:
	__fastcall UrUndo (TDBWFRM* _mainfrm, int _maxundo=100);
	__fastcall ~UrUndo();

    void __fastcall Clear();
    void __fastcall UpdateSize();

	bool __fastcall Undo();
	bool __fastcall Redo();

    bool __fastcall CanUndo();
    bool __fastcall CanRedo();

	void __fastcall Snapshot();

protected:
	void __fastcall Init (int _maxundo);
	void __fastcall Cleanup();
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/

