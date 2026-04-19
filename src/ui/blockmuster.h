/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Muster: a single 12x12 small binding pattern used by the
    blockmuster (substitution) editor. Verbatim port of
    legacy/blockmuster.{h,cpp}.

    BlockUndo: a fixed 100-slot circular undo ring over an array of
    10 Muster instances plus the "active" index.                    */

#ifndef DBWEAVE_UI_BLOCKMUSTER_H
#define DBWEAVE_UI_BLOCKMUSTER_H

#include "vcl_compat.h"

class Muster
{
public:
	enum { maxx = 12 };
	enum { maxy = 12 };
private:
	char feld[maxx*maxy];
public:
	__fastcall Muster();
	void __fastcall Set (int _i, int _j, char _value);
	char __fastcall Get (int _i, int _j);
	void __fastcall Clear();
	bool __fastcall IsEmpty();
	int  __fastcall SizeX();
	int  __fastcall SizeY();
	int  __fastcall FirstX();
	int  __fastcall FirstY();

	const char* __fastcall Data() const { return feld; }
	int         __fastcall DataSize() const { return maxx*maxy; }
	void __fastcall SetData (const char* _data);

	Muster& operator= (const Muster& _m);
};

typedef Muster MUSTERARRAY[10];
typedef MUSTERARRAY* PMUSTERARRAY;

class BlockUndoItem
{
public:
	bool isempty;
	Muster bindungen[10];
	int    active;
	BlockUndoItem* prev;
	BlockUndoItem* next;
	__fastcall BlockUndoItem() { isempty = true; prev = next = nullptr; active = 0; }
};

class BlockUndo
{
public:
	PMUSTERARRAY bindungen;
	int&         active;
	BlockUndoItem* list;
	BlockUndoItem* current;
	__fastcall BlockUndo (PMUSTERARRAY _bindungen, int& _active);
	virtual __fastcall ~BlockUndo();
	void __fastcall Allocate();
	void __fastcall Snapshot();
	void __fastcall Undo();
	void __fastcall Redo();
	bool __fastcall CanUndo();
	bool __fastcall CanRedo();
	void __fastcall Clear();
};

#endif
