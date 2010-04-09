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
#ifndef paletteH
#define paletteH
/*-----------------------------------------------------------------*/
// Forward-Deklarationen
class FfWriter;
class FfReader;
/*-----------------------------------------------------------------*/
#define MAX_PAL_ENTRY 236
/*-----------------------------------------------------------------*/
class Palette
{
public:
    COLORREF data[MAX_PAL_ENTRY];
    bool palette2;

public:
	__fastcall Palette();
	virtual __fastcall ~Palette();
    void __fastcall InitPalette();
    void __fastcall Save (FfWriter* _writer, bool _format37);
    void __fastcall Load (FfReader* _reader);
	COLORREF __fastcall GetColor (int _index);
	void __fastcall SetColor (int _index, COLORREF _color);
    void __fastcall SetPaletteType (bool _palette2);

private:
    void __fastcall SetPaletteEntry (int _i, BYTE _red, BYTE _green, BYTE _blue);
    void __fastcall FillDefaultPalette();
    void __fastcall FillDefaultPalette2();
    void __fastcall FillDefaultPalette3();
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/

