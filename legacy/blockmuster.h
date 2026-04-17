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
#ifndef blockmusterH
#define blockmusterH
/*-----------------------------------------------------------------*/
class Muster
{
private:
    enum { maxx = 12 };
    enum { maxy = 12 };
    char feld[maxx*maxy];
public:
    __fastcall Muster();
    void __fastcall Set (int _i, int _j, char _value);
    char __fastcall Get (int _i, int _j);
    void __fastcall Clear();
    bool __fastcall IsEmpty();
    int __fastcall SizeX();
    int __fastcall SizeY();
    int __fastcall FirstX();
    int __fastcall FirstY();

    const char* __fastcall Data() const { return feld; }
    int __fastcall DataSize() const { return maxx*maxy; }
    void __fastcall SetData (const char* _data);

    Muster& operator= (const Muster& _m);
};
/*-----------------------------------------------------------------*/
typedef Muster MUSTERARRAY[10];
typedef MUSTERARRAY* PMUSTERARRAY;
/*-----------------------------------------------------------------*/
class BlockUndoItem
{
public:
    bool isempty;
    Muster bindungen[10];
    int active;
    BlockUndoItem* prev;
    BlockUndoItem* next;
    __fastcall BlockUndoItem() { isempty = true; prev = next = NULL; active = 0; }
};
/*-----------------------------------------------------------------*/
class BlockUndo
{
public:
    PMUSTERARRAY bindungen;
    int& active;
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
/*-----------------------------------------------------------------*/
#endif

