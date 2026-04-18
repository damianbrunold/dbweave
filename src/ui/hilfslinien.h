/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Hilfslinien (guide lines) data container. 1:1 port of the legacy
    Hilfslinien class; storage is a growable raw Hilfslinie[] so the
    file-format code that streams `(void*)Data(), DataSize()` can be
    reused unchanged once hilfslinien fileload/save is wired up.    */

#ifndef DBWEAVE_UI_HILFSLINIEN_H
#define DBWEAVE_UI_HILFSLINIEN_H

#include "vcl_compat.h"

enum HLTYP  { HL_NONE = -1, HL_HORZ = 0, HL_VERT = 1 };
enum HLFELD { HL_LEFT = 0, HL_TOP = 0, HL_RIGHT = 1, HL_BOTTOM = 1 };

struct Hilfslinie
{
	HLTYP  typ;
	HLFELD feld;
	int    pos;
	__fastcall Hilfslinie() { typ = HL_NONE; feld = HL_LEFT; pos = 0; }
};

class Hilfslinien
{
private:
	Hilfslinie* list;
	int max;
	int last;
protected:
	void __fastcall Reallocate (int _newmax);
public:
	__fastcall Hilfslinien();
	virtual __fastcall ~Hilfslinien();
	bool __fastcall Add (HLTYP _typ, HLFELD _feld, int _pos);
	void __fastcall Update (int _index, HLFELD _feld, int _pos);
	void __fastcall Delete (int _index);
	void __fastcall Delete (Hilfslinie* _hline);
	void __fastcall DeleteAll();
	int  __fastcall GetCount() const;
	Hilfslinie* __fastcall GetLine (int _index);
	Hilfslinie* __fastcall GetLine (HLTYP _typ, HLFELD _feld, int _pos);
	void* __fastcall Data() { return list; }
	int   __fastcall DataSize() { return (last + 1) * (int)sizeof(Hilfslinie); }
	void  __fastcall SetData (Hilfslinie* _list, int _count);
};

#endif
