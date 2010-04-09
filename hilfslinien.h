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
#ifndef hilfslinienH
#define hilfslinienH
/*-----------------------------------------------------------------*/
#define HLINECOLOR clBlue
/*-----------------------------------------------------------------*/
enum HLTYP { HL_NONE=-1, HL_HORZ=0, HL_VERT=1 };
enum HLFELD { HL_LEFT=0, HL_TOP=0, HL_RIGHT=1, HL_BOTTOM=1 };
/*-----------------------------------------------------------------*/
struct Hilfslinie
{
    HLTYP  typ;
    HLFELD feld;
    int    pos;
    // Zum Feld feld: Eine Hilfslinie geht immer über die
    // ganze Breite resp. die ganze Höhe des Fensters.
    // Nun kann aber zum Beispiel eine waagrechte Hilfslinie
    // entweder in Einzug/Aufknüpfung oder in Gewebe/Trittfolge
    // liegen. Deshalb muss man ein "feld"-Feld führen. Dieses
    // merkt aber nur an, ob oben oder unten resp. links
    // oder rechts...
    // Die pos bezieht sich auf globale Koordination, d.h.
    // für eine waagrechte untere Hilfslinie liegen die
    // Werte zwischen 0 und MAXY2
    __fastcall Hilfslinie() { typ=HL_NONE; }
};
/*-----------------------------------------------------------------*/
class Hilfslinien
{
private:
    Hilfslinie* list;
    int max;   // Soviele Einträge kann Liste aufnehmen
    int last;  // Soviele sind effektiv drin
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
    int __fastcall GetCount() const;
    Hilfslinie* __fastcall GetLine (int _index);
    Hilfslinie* __fastcall GetLine (HLTYP _typ, HLFELD _feld, int _pos);
    void* __fastcall Data() { return list; }
    int __fastcall DataSize() { return (last+1)*sizeof(Hilfslinie); }
    void __fastcall SetData (Hilfslinie* _list, int _count);
};
/*-----------------------------------------------------------------*/
#endif
