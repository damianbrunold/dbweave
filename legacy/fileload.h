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

#ifndef fileloadH
#define fileloadH

// Forward-Deklarationen
class TDBWFRM;

#include "loadoptions.h"

class FhLoader
{
private:
	TDBWFRM* mainfrm;
    LOADPARTS loadparts;

public:
	__fastcall FhLoader (TDBWFRM* _mainfrm);
	bool __fastcall Load (LOADSTAT& _stat, LOADPARTS _loadparts = LOADALL);

private:
    // Utilities
    bool __fastcall Need (LOADOPTION _part);

	// Diverses
	bool __fastcall LoadSignatur (FfReader* _reader);
    void __fastcall LoadVersion (FfReader* _reader);
    void __fastcall LoadFileinfo (FfReader* _reader);

	// Daten
    void __fastcall LoadData (FfReader* _reader);
    void __fastcall LoadDataSize (FfReader* _reader);
    void __fastcall LoadDataFields (FfReader* _reader);
    void __fastcall LoadDataEinzug (FfReader* _reader);
    void __fastcall LoadDataAufknuepfung (FfReader* _reader);
    void __fastcall LoadDataTrittfolge (FfReader* _reader);
    void __fastcall LoadDataTrittfolgeTrittfolge (FfReader* _reader);
    void __fastcall LoadDataTrittfolgeIsEmpty (FfReader* _reader);
    void __fastcall LoadDataSchussfarben (FfReader* _reader);
    void __fastcall LoadDataKettfarben (FfReader* _reader);
    void __fastcall LoadDataBlatteinzug (FfReader* _reader);
    void __fastcall LoadDataWebstuhl (FfReader* _reader);
    void __fastcall LoadDataKlammer0 (FfReader* _reader);
    void __fastcall LoadDataKlammer1 (FfReader* _reader);
    void __fastcall LoadDataKlammer2 (FfReader* _reader);
    void __fastcall LoadDataKlammer3 (FfReader* _reader);
    void __fastcall LoadDataKlammer4 (FfReader* _reader);
    void __fastcall LoadDataKlammer5 (FfReader* _reader);
    void __fastcall LoadDataKlammer6 (FfReader* _reader);
    void __fastcall LoadDataKlammer7 (FfReader* _reader);
    void __fastcall LoadDataKlammer8 (FfReader* _reader);
    void __fastcall LoadDataCurrent (FfReader* _reader);
    void __fastcall LoadDataLast (FfReader* _reader);
    void __fastcall LoadDataDivers (FfReader* _reader);
    void __fastcall LoadDataBlockmuster (FfReader* _reader);
    void __fastcall LoadDataBereichmuster (FfReader* _reader);
    void __fastcall LoadDataHilfslinien (FfReader* _reader);
    void __fastcall LoadDataFixeinzug (FfReader* _reader);
	void __fastcall LoadDataPalette (FfReader* _reader);

	// Ansicht
    void __fastcall LoadView (FfReader* _reader);
    void __fastcall LoadViewGeneral (FfReader* _reader);
    void __fastcall LoadViewEinzug (FfReader* _reader);
    void __fastcall LoadViewAufknuepfung (FfReader* _reader);
    void __fastcall LoadViewTrittfolge (FfReader* _reader);
    void __fastcall LoadViewSchlagpatrone (FfReader* _reader);
    void __fastcall LoadViewGewebe (FfReader* _reader);
    void __fastcall LoadViewBlatteinzug (FfReader* _reader);
    void __fastcall LoadViewKettfarben (FfReader* _reader);
    void __fastcall LoadViewSchussfarben (FfReader* _reader);
    void __fastcall LoadViewPagesetup (FfReader* _reader);

    // Druckeinstellungen
    void __fastcall LoadPrint (FfReader* _reader);
    void __fastcall LoadPrintRange (FfReader* _reader);
};

#endif

