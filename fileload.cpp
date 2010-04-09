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

#include <vcl\vcl.h>
#include <mem.h>
#pragma hdrstop

#include "assert.h"
#include "dbw3_form.h"
#include "felddef.h"
#include "dbw3_strings.h"
#include "farbpalette_form.h"
#include "datamodule.h"
#include "properties.h"
#include "palette.h"
#include "fileformat.h"
#include "loadmap.h"
#include "fileload.h"
#include "blockmuster.h"
#include "hilfslinien.h"
#include "steuerung_form.h"

static void __fastcall HexToByte (char* _byte, char* _hex)
{
	dbw3_assert (_byte);
	dbw3_assert (_hex);

	int byte;

	switch (_hex[0]) {
		case '0': byte = 0; break;
		case '1': byte = 1; break;
		case '2': byte = 2; break;
		case '3': byte = 3; break;
		case '4': byte = 4; break;
		case '5': byte = 5; break;
		case '6': byte = 6; break;
		case '7': byte = 7; break;
		case '8': byte = 8; break;
		case '9': byte = 9; break;
		case 'a': byte = 10; break;
		case 'b': byte = 11; break;
		case 'c': byte = 12; break;
		case 'd': byte = 13; break;
		case 'e': byte = 14; break;
		case 'f': byte = 15; break;
        default:  byte = 0; break;
	}

	switch (_hex[1]) {
		case '0': byte = byte*16 + 0; break;
		case '1': byte = byte*16 + 1; break;
		case '2': byte = byte*16 + 2; break;
		case '3': byte = byte*16 + 3; break;
		case '4': byte = byte*16 + 4; break;
		case '5': byte = byte*16 + 5; break;
		case '6': byte = byte*16 + 6; break;
		case '7': byte = byte*16 + 7; break;
		case '8': byte = byte*16 + 8; break;
		case '9': byte = byte*16 + 9; break;
		case 'a': byte = byte*16 + 10; break;
		case 'b': byte = byte*16 + 11; break;
		case 'c': byte = byte*16 + 12; break;
		case 'd': byte = byte*16 + 13; break;
		case 'e': byte = byte*16 + 14; break;
		case 'f': byte = byte*16 + 15; break;
        default:  byte = 0; break;
	}

	*_byte = (char)byte;
}

void __fastcall FieldHexToBinary (void* _dest, const void* _source, int _length)
{
	dbw3_assert (_dest);
	dbw3_assert (_source);
	dbw3_assert (_length>=0);

	try {
		// Daten von Hexdump zurückverwandeln!
		// Buffer sind schon richtig dimensioniert übergeben worden...
		char* d = (char*)_dest;
		char* s = (char*)_source;
		for (int i=0; i<_length/2; i++) {
			HexToByte (d, s);
            s += 2;
            d++;
		}
		*d = 0;
	} catch (...) {
	}
}

bool __fastcall TDBWFRM::Load(LOADSTAT& _stat, LOADPARTS _loadparts/*=LOADALL*/)
{
    try {
    	FhLoader loader(this);
	    return loader.Load (_stat, _loadparts);
    } catch (...) {
        return false;
    }
}

__fastcall FhLoader::FhLoader (TDBWFRM* _mainfrm)
{
	dbw3_assert (_mainfrm);
	mainfrm = _mainfrm;
    loadparts = LOADALL;
}

bool __fastcall FhLoader::Need (LOADOPTION _part)
{
    return ((unsigned int)(loadparts&_part))==_part ;
}

bool __fastcall FhLoader::Load(LOADSTAT& _stat, LOADPARTS _loadparts/*=LOADALL*/)
{
    loadparts = _loadparts;
    if (mainfrm->filename.IsEmpty()) return false;
    if (!FileExists(mainfrm->filename.c_str())) {
		_stat = FILE_DOES_NOT_EXIST;
		return false;
	}

	try {

        // Datei ggf. öffnen
        if (!mainfrm->file->IsOpen()) {
            mainfrm->file->Open (mainfrm->filename.c_str(), FfOpenRead|FfOpenWrite);
        } else {
            mainfrm->file->SeekBegin();
        }

        // Datei muss offen sein!
        if (!mainfrm->file->IsOpen()) {
			_stat = FILE_ALREADY_OPEN;
			return false;
		}

        // FileReader erstellen
        FfReader reader;
        reader.Assign (mainfrm->file);

        // Daten lesen (Signatur muss zu Beginn der Datei sein!)
		if (!LoadSignatur (&reader)) {
			_stat = FILE_WITHOUT_SIGNATURE;
            mainfrm->file->Close();
			return false;
		}
		FfReader* _reader = &reader; // Damit die Load-Makros funktionieren!
        BEGIN_LOAD_MAP
            BEGIN_FIELD_MAP
                NO_FIELDS
            BEGIN_SECTION_MAP
                _SECTION_MAP("version", LoadVersion)
                SECTION_MAP ("properties", LoadFileinfo)
                SECTION_MAP ("data", LoadData)
                SECTION_MAP ("view", LoadView)
                SECTION_MAP ("printsettings", LoadPrint)
                DEFAULT_SECTION
            BEGIN_DEFAULT_MAP
        END_LOAD_MAP
	} catch (...) {
		_stat = FILE_CORRUPT;
		mainfrm->file->Close();
        return false;
	}

	return true;
}

bool __fastcall FhLoader::LoadSignatur (FfReader* _reader)
{
	dbw3_assert (_reader);
	FfToken* token = _reader->GetToken();
	bool success = token && token->GetType()==FfSignature;
	delete token;
	return success;
}

void __fastcall FhLoader::LoadVersion (FfReader* _reader)
{
	char* fmt = 0;
	char* ver = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_STRING("fmt", fmt)
			FIELD_MAP_STRING ("ver", ver)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

	delete[] fmt;
	delete[] ver;
}

void __fastcall FhLoader::LoadFileinfo (FfReader* _reader)
{
	dbw3_assert (_reader);
    if (Need(LOADALL)) Data->properties->Load (_reader);
    else _reader->SkipSection();
}

void __fastcall FhLoader::LoadData (FfReader* _reader)
{
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			NO_FIELDS
		BEGIN_SECTION_MAP
			_SECTION_MAP("size", LoadDataSize)
			SECTION_MAP ("fields", LoadDataFields)
            SECTION_MAP ("webstuhl", LoadDataWebstuhl)
			SECTION_MAP ("palette", LoadDataPalette)
            SECTION_MAP ("blockmuster", LoadDataBlockmuster)
            SECTION_MAP ("bereichmuster", LoadDataBereichmuster)
            SECTION_MAP ("hilfslinien", LoadDataHilfslinien)
			DEFAULT_SECTION
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
}

void __fastcall FhLoader::LoadDataSize (FfReader* _reader)
{
	dbw3_assert (_reader);
    int maxx1 = 300;
    int maxy1 = 30;
    int maxx2 = 30;
    int maxy2 = 300;
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("maxx1", maxx1, int)
			FIELD_MAP_INT ("maxy1", maxy1, int)
			FIELD_MAP_INT ("maxx2", maxx2, int)
			FIELD_MAP_INT ("maxy2", maxy2, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    // Damit die Feldgrössen stimmen!
    if (Need(LOADALL)) {
        Data->MAXX1 = maxx1;
        Data->MAXY1 = maxy1; if (mainfrm->hvisible > Data->MAXY1) mainfrm->hvisible = Data->MAXY1;
        Data->MAXX2 = maxx2; if (mainfrm->wvisible > Data->MAXX2) mainfrm->wvisible = Data->MAXX2;
        Data->MAXY2 = maxy2;
        mainfrm->AllocBuffers(true);
    } else {
        if (Need(LOADEINZUG)) {
            Data->MAXX1 = maxx1;
            mainfrm->AllocBuffersX1();
            if (!Need(LOADKETTFARBEN)) mainfrm->kettfarben.feld.Resize (Data->MAXX1, Data->defcolorh);
            if (!Need(LOADBLATTEINZUG)) mainfrm->blatteinzug.feld.Resize (Data->MAXX1, 0);
        }
        if (Need(LOADKETTFARBEN)) {
            Data->MAXX1 = maxx1;
            mainfrm->AllocBuffersX1();
            if (!Need(LOADEINZUG)) mainfrm->einzug.feld.Resize (Data->MAXX1, 0);
            if (!Need(LOADEINZUG) && !Need(LOADBLATTEINZUG)) mainfrm->blatteinzug.feld.Resize (Data->MAXX1, 0);
        }
        if (Need(LOADBLATTEINZUG)) {
            Data->MAXX1 = maxx1;
            mainfrm->AllocBuffersX1();
            if (!Need(LOADKETTFARBEN) && !Need(LOADEINZUG)) mainfrm->einzug.feld.Resize (Data->MAXX1, 0);
            if (!Need(LOADEINZUG) && !Need(LOADKETTFARBEN)) mainfrm->kettfarben.feld.Resize (Data->MAXX1, Data->defcolorh);
        }

        if (Need(LOADAUFKNUEPFUNG)) {
            Data->MAXX2 = maxx2; if (mainfrm->wvisible > Data->MAXX2) mainfrm->wvisible = Data->MAXX2;
            Data->MAXY1 = maxy1; if (mainfrm->hvisible > Data->MAXY1) mainfrm->hvisible = Data->MAXY1;
            mainfrm->AllocBuffersX2();
            mainfrm->AllocBuffersY1();
            if (!Need(LOADTRITTFOLGE)) mainfrm->trittfolge.feld.Resize (Data->MAXX2, Data->MAXY2, 0);
        }

        if (Need(LOADTRITTFOLGE)) {
            Data->MAXX2 = maxx2; if (mainfrm->wvisible > Data->MAXX2) mainfrm->wvisible = Data->MAXX2;
            Data->MAXY2 = maxy2;
            mainfrm->AllocBuffersX2();
            mainfrm->AllocBuffersY2();
            if (!Need(LOADAUFKNUEPFUNG)) mainfrm->aufknuepfung.feld.Resize(Data->MAXX2, Data->MAXY1, 0);
            if (!Need(LOADSCHUSSFARBEN)) mainfrm->schussfarben.feld.Resize(Data->MAXY2, Data->defcolorv);
        }

        if (Need(LOADSCHUSSFARBEN)) {
            Data->MAXY2 = maxy2;
            mainfrm->AllocBuffersY2();
            if (!Need(LOADTRITTFOLGE)) mainfrm->trittfolge.feld.Resize(Data->MAXX2, Data->MAXY2, 0);
        }
    }
}

void __fastcall FhLoader::LoadDataFields (FfReader* _reader)
{
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			NO_FIELDS
		BEGIN_SECTION_MAP
			_SECTION_MAP("einzug", LoadDataEinzug)
			SECTION_MAP ("aufknuepfung", LoadDataAufknuepfung)
			SECTION_MAP ("trittfolge", LoadDataTrittfolge)
			SECTION_MAP ("schussfarben", LoadDataSchussfarben)
			SECTION_MAP ("kettfarben", LoadDataKettfarben)
			SECTION_MAP ("blatteinzug", LoadDataBlatteinzug)
            SECTION_MAP ("fixeinzug", LoadDataFixeinzug)
			DEFAULT_SECTION
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
}

void __fastcall FhLoader::LoadDataEinzug (FfReader* _reader)
{
    if (Need(LOADEINZUG)) mainfrm->einzug.feld.Read (_reader, 0);
    else _reader->SkipSection();
}

void __fastcall FhLoader::LoadDataAufknuepfung (FfReader* _reader)
{
    if (Need(LOADAUFKNUEPFUNG)) mainfrm->aufknuepfung.feld.Read (_reader, 0);
    else _reader->SkipSection();
}

void __fastcall FhLoader::LoadDataTrittfolge (FfReader* _reader)
{
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			NO_FIELDS
		BEGIN_SECTION_MAP
			_SECTION_MAP("trittfolge", LoadDataTrittfolgeTrittfolge)
			SECTION_MAP ("isempty", LoadDataTrittfolgeIsEmpty)
			DEFAULT_SECTION
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
}

void __fastcall FhLoader::LoadDataTrittfolgeTrittfolge (FfReader* _reader)
{
    if (Need(LOADTRITTFOLGE)) mainfrm->trittfolge.feld.Read (_reader, 0);
    else _reader->SkipSection();
}

void __fastcall FhLoader::LoadDataTrittfolgeIsEmpty (FfReader* _reader)
{
    if (Need(LOADTRITTFOLGE)) mainfrm->trittfolge.isempty.Read (_reader, 1);
    else _reader->SkipSection();
}

void __fastcall FhLoader::LoadDataSchussfarben (FfReader* _reader)
{
    if (Need(LOADSCHUSSFARBEN)) mainfrm->schussfarben.feld.Read (_reader, Data->defcolorv);
    else _reader->SkipSection();
}

void __fastcall FhLoader::LoadDataKettfarben (FfReader* _reader)
{
    if (Need(LOADKETTFARBEN)) mainfrm->kettfarben.feld.Read (_reader, Data->defcolorh);
    else _reader->SkipSection();
}

void __fastcall FhLoader::LoadDataBlatteinzug (FfReader* _reader)
{
    if (Need(LOADBLATTEINZUG)) mainfrm->blatteinzug.feld.Read (_reader, 0);
    else _reader->SkipSection();
}

void __fastcall FhLoader::LoadDataFixeinzug (FfReader* _reader)
{
    char* data = NULL;
    short firstfree = 1;
    int fixsize = Data->MAXX1;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_BINARY("fixeinzug", data)
			FIELD_MAP_INT ("firstfree", firstfree, short)
			FIELD_MAP_INT ("fixsize", fixsize, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADFIXEINZUG)) {
        mainfrm->firstfree = firstfree;
        if (data) {
            delete[] mainfrm->fixeinzug;
            mainfrm->fixeinzug = (short*)data;
            mainfrm->fixsize = fixsize;
        }
    }
}

void __fastcall FhLoader::LoadDataWebstuhl (FfReader* _reader)
{
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			NO_FIELDS
		BEGIN_SECTION_MAP
			_SECTION_MAP("klammer0", LoadDataKlammer0)
			SECTION_MAP ("klammer1", LoadDataKlammer1)
			SECTION_MAP ("klammer2", LoadDataKlammer2)
			SECTION_MAP ("klammer3", LoadDataKlammer3)
			SECTION_MAP ("klammer4", LoadDataKlammer4)
			SECTION_MAP ("klammer5", LoadDataKlammer5)
			SECTION_MAP ("klammer6", LoadDataKlammer6)
			SECTION_MAP ("klammer7", LoadDataKlammer7)
			SECTION_MAP ("klammer8", LoadDataKlammer8)
            SECTION_MAP ("current", LoadDataCurrent)
            SECTION_MAP ("last", LoadDataLast)
            SECTION_MAP ("divers", LoadDataDivers)
			DEFAULT_SECTION
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
}

void __fastcall FhLoader::LoadDataKlammer0 (FfReader* _reader)
{
    int first = 0;
	int last = 0;
	int repetitions = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("first", first, int)
			FIELD_MAP_INT ("last", last, int)
			FIELD_MAP_INT ("repetitions", repetitions, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        mainfrm->klammern[0].first = first;
        mainfrm->klammern[0].last = last;
        mainfrm->klammern[0].repetitions = repetitions;
    }
}

void __fastcall FhLoader::LoadDataKlammer1 (FfReader* _reader)
{
    int first = 0;
	int last = 0;
	int repetitions = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("first", first, int)
			FIELD_MAP_INT ("last", last, int)
			FIELD_MAP_INT ("repetitions", repetitions, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        mainfrm->klammern[1].first = first;
        mainfrm->klammern[1].last = last;
        mainfrm->klammern[1].repetitions = repetitions;
    }
}

void __fastcall FhLoader::LoadDataKlammer2 (FfReader* _reader)
{
    int first = 0;
	int last = 0;
	int repetitions = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("first", first, int)
			FIELD_MAP_INT ("last", last, int)
			FIELD_MAP_INT ("repetitions", repetitions, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        mainfrm->klammern[2].first = first;
        mainfrm->klammern[2].last = last;
        mainfrm->klammern[2].repetitions = repetitions;
    }
}

void __fastcall FhLoader::LoadDataKlammer3 (FfReader* _reader)
{
    int first = 0;
	int last = 0;
	int repetitions = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("first", first, int)
			FIELD_MAP_INT ("last", last, int)
			FIELD_MAP_INT ("repetitions", repetitions, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        mainfrm->klammern[3].first = first;
        mainfrm->klammern[3].last = last;
        mainfrm->klammern[3].repetitions = repetitions;
    }
}

void __fastcall FhLoader::LoadDataKlammer4 (FfReader* _reader)
{
    int first = 0;
	int last = 0;
	int repetitions = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("first", first, int)
			FIELD_MAP_INT ("last", last, int)
			FIELD_MAP_INT ("repetitions", repetitions, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        mainfrm->klammern[4].first = first;
        mainfrm->klammern[4].last = last;
        mainfrm->klammern[4].repetitions = repetitions;
    }
}

void __fastcall FhLoader::LoadDataKlammer5 (FfReader* _reader)
{
    int first = 0;
	int last = 0;
	int repetitions = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("first", first, int)
			FIELD_MAP_INT ("last", last, int)
			FIELD_MAP_INT ("repetitions", repetitions, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        mainfrm->klammern[5].first = first;
        mainfrm->klammern[5].last = last;
        mainfrm->klammern[5].repetitions = repetitions;
    }
}

void __fastcall FhLoader::LoadDataKlammer6 (FfReader* _reader)
{
    int first = 0;
	int last = 0;
	int repetitions = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("first", first, int)
			FIELD_MAP_INT ("last", last, int)
			FIELD_MAP_INT ("repetitions", repetitions, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        mainfrm->klammern[6].first = first;
        mainfrm->klammern[6].last = last;
        mainfrm->klammern[6].repetitions = repetitions;
    }
}

void __fastcall FhLoader::LoadDataKlammer7 (FfReader* _reader)
{
    int first = 0;
	int last = 0;
	int repetitions = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("first", first, int)
			FIELD_MAP_INT ("last", last, int)
			FIELD_MAP_INT ("repetitions", repetitions, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        mainfrm->klammern[7].first = first;
        mainfrm->klammern[7].last = last;
        mainfrm->klammern[7].repetitions = repetitions;
    }
}

void __fastcall FhLoader::LoadDataKlammer8 (FfReader* _reader)
{
    int first = 0;
	int last = 0;
	int repetitions = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("first", first, int)
			FIELD_MAP_INT ("last", last, int)
			FIELD_MAP_INT ("repetitions", repetitions, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        mainfrm->klammern[8].first = first;
        mainfrm->klammern[8].last = last;
        mainfrm->klammern[8].repetitions = repetitions;
    }
}

void __fastcall FhLoader::LoadDataCurrent (FfReader* _reader)
{
    int pos = 0;
	int kl = 0;
	int rep = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("position", pos, int)
			FIELD_MAP_INT ("klammer", kl, int)
			FIELD_MAP_INT ("repetition", rep, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        STRGFRM->weave_position = pos;
        STRGFRM->weave_klammer = kl;
        STRGFRM->weave_repetition = rep;
    }
}

void __fastcall FhLoader::LoadDataLast (FfReader* _reader)
{
    int pos = 0;
	int kl = 0;
	int rep = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("position", pos, int)
			FIELD_MAP_INT ("klammer", kl, int)
			FIELD_MAP_INT ("repetition", rep, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        STRGFRM->last_position = pos;
        STRGFRM->last_klammer = kl;
        STRGFRM->last_repetition = rep;
    }
}

void __fastcall FhLoader::LoadDataDivers (FfReader* _reader)
{
    bool trsel = true;
	int scrly = 0;
	bool frsch = true;
    bool weaving = false;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("schussselected", trsel, bool)
			FIELD_MAP_INT ("scrolly", scrly, int)
			FIELD_MAP_INT ("firstschuss", frsch, bool)
            FIELD_MAP_INT ("weaving", weaving, bool)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        STRGFRM->schussselected = trsel;
        STRGFRM->scrolly = scrly;
        STRGFRM->firstschuss = frsch;
        mainfrm->weaving = weaving;
    }
}

void __fastcall FhLoader::LoadDataPalette (FfReader* _reader)
{
	if (Need(LOADPALETTE)) Data->palette->Load(_reader);
	else _reader->SkipSection();
}

void __fastcall FhLoader::LoadDataBlockmuster (FfReader* _reader)
{
    char* data[10];
    for (int i=0; i<10; i++) data[i] = NULL;
    bool einzugz = true;
    bool trittfolgez = true;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_BINARY("bindung0", data[0])
			FIELD_MAP_BINARY ("bindung1", data[1])
			FIELD_MAP_BINARY ("bindung2", data[2])
			FIELD_MAP_BINARY ("bindung3", data[3])
			FIELD_MAP_BINARY ("bindung4", data[4])
			FIELD_MAP_BINARY ("bindung5", data[5])
			FIELD_MAP_BINARY ("bindung6", data[6])
			FIELD_MAP_BINARY ("bindung7", data[7])
			FIELD_MAP_BINARY ("bindung8", data[8])
			FIELD_MAP_BINARY ("bindung9", data[9])
			FIELD_MAP_INT ("einzugz", einzugz, bool)
            FIELD_MAP_INT ("trittfolgez", trittfolgez, bool)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADBLOCKMUSTER)) {
        mainfrm->einzugZ = einzugz;
        mainfrm->trittfolgeZ = trittfolgez;
        for (int i=0; i<10; i++)
            if (data[i]) {
                mainfrm->blockmuster[i].SetData (data[i]);
                delete[] data[i];
            }
    }
}

void __fastcall FhLoader::LoadDataBereichmuster (FfReader* _reader)
{
    char* data[10];
    for (int i=0; i<10; i++) data[i] = NULL;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_BINARY("bindung0", data[0])
			FIELD_MAP_BINARY ("bindung1", data[1])
			FIELD_MAP_BINARY ("bindung2", data[2])
			FIELD_MAP_BINARY ("bindung3", data[3])
			FIELD_MAP_BINARY ("bindung4", data[4])
			FIELD_MAP_BINARY ("bindung5", data[5])
			FIELD_MAP_BINARY ("bindung6", data[6])
			FIELD_MAP_BINARY ("bindung7", data[7])
			FIELD_MAP_BINARY ("bindung8", data[8])
			FIELD_MAP_BINARY ("bindung9", data[9])
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADBEREICHMUSTER)) {
        for (int i=0; i<10; i++)
            if (data[i]) {
                mainfrm->bereichmuster[i].SetData (data[i]);
                delete[] data[i];
            }
    }
}

void __fastcall FhLoader::LoadDataHilfslinien (FfReader* _reader)
{
    int count = 0;
    char* data = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("count", count, int)
            FIELD_MAP_BINARY ("list", data)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADHILFSLINIEN)) {
        mainfrm->hlines.SetData ((Hilfslinie*)data, count);
        // data NICHT löschen! Wird in Liste aufgenommen...
    }
}

void __fastcall FhLoader::LoadView (FfReader* _reader)
{
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			NO_FIELDS
		BEGIN_SECTION_MAP
			_SECTION_MAP("general", LoadViewGeneral)
			SECTION_MAP ("einzug", LoadViewEinzug)
			SECTION_MAP ("aufknuepfung", LoadViewAufknuepfung)
			SECTION_MAP ("trittfolge", LoadViewTrittfolge)
			SECTION_MAP ("schlagpatrone", LoadViewSchlagpatrone)
			SECTION_MAP ("gewebe", LoadViewGewebe)
			SECTION_MAP ("blatteinzug", LoadViewBlatteinzug)
			SECTION_MAP ("kettfarben", LoadViewKettfarben)
			SECTION_MAP ("schussfarben", LoadViewSchussfarben)
			SECTION_MAP ("pagesetup", LoadViewPagesetup)
			DEFAULT_SECTION
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
}

void __fastcall FhLoader::LoadViewGeneral (FfReader* _reader)
{
    double faktor_kette = 1.0;
    double faktor_schuss = 1.0;
    int hebung = true;
    unsigned char color = 1;
    int currentzoom = 3;
    int viewfarbpalette = 0;
    int viewschlagpatrone = 0;
    int rapviewrapport = 0;
    int viewhlines = 1;
    int viewrighttoleft = 0;
    int viewtoptobottom = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_DOUBLE("faktor_kette", faktor_kette)
			FIELD_MAP_DOUBLE ("faktor_schuss", faktor_schuss)
			FIELD_MAP_INT ("hebung", hebung, int)
			FIELD_MAP_INT ("color", color, unsigned char)
			FIELD_MAP_INT ("zoom", currentzoom, int)
			FIELD_MAP_INT ("viewpalette", viewfarbpalette, int)
			FIELD_MAP_INT ("viewpegplan", viewschlagpatrone, int)
            FIELD_MAP_INT ("viewrapport", rapviewrapport, int)
            FIELD_MAP_INT ("viewhlines", viewhlines, int)
            FIELD_MAP_INT ("righttoleft", viewrighttoleft, int)
            FIELD_MAP_INT ("toptobottom", viewtoptobottom, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        mainfrm->faktor_kette = faktor_kette;
        mainfrm->faktor_schuss = faktor_schuss;
        mainfrm->sinkingshed = !hebung;
        Data->color = color;
        mainfrm->currentzoom = currentzoom;
        mainfrm->ViewFarbpalette->Checked = viewfarbpalette;
        mainfrm->ViewSchlagpatrone->Checked = viewschlagpatrone;
        mainfrm->RappViewRapport->Checked = rapviewrapport;
        mainfrm->ViewHlines->Checked = viewhlines;
        mainfrm->righttoleft = viewrighttoleft;
        mainfrm->toptobottom = viewtoptobottom;
        mainfrm->UpdateScrollbars();
        if (!viewrighttoleft) mainfrm->sb_horz1->Position = mainfrm->scroll_x1;
        else mainfrm->sb_horz1->Position = mainfrm->sb_horz1->Max - mainfrm->scroll_x1;
        if (viewtoptobottom) mainfrm->sb_vert1->Position = mainfrm->scroll_y1;
        else mainfrm->sb_vert1->Position = mainfrm->sb_vert1->Max - mainfrm->scroll_y1;
    } else if (Need(LOADEINZUG) && !Need(LOADALL)) {
        mainfrm->righttoleft = viewrighttoleft;
        mainfrm->toptobottom = viewtoptobottom;
    } else if (Need(LOADHILFSLINIEN) && !Need(LOADALL)) {
        mainfrm->ViewHlines->Checked = true;
    } else if (Need(LOADPALETTE) && !Need(LOADALL)) {
        mainfrm->ViewFarbpalette->Checked = true;
    }
}

void __fastcall FhLoader::LoadViewEinzug (FfReader* _reader)
{
    int style = 1;
	int visible = 1;
	int down = 0;
	DARSTELLUNG darst = PUNKT;
	int stronglinex = 4;
	int strongliney = 4;
	int hvisible = 12;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("visible", visible, int)
			FIELD_MAP_INT ("down", down, int)
			FIELD_MAP_INT ("viewtype", darst, DARSTELLUNG)
			FIELD_MAP_INT ("stronglinex", stronglinex, int)
			FIELD_MAP_INT ("strongliney", strongliney, int)
			FIELD_MAP_INT ("hvisible", hvisible, int)
            FIELD_MAP_INT ("style", style, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need (LOADEINZUG)) {
        if (!Need(LOADALL)) visible = true;
        mainfrm->ViewEinzug->Checked = visible;
        mainfrm->ViewEinzugPopup->Checked = mainfrm->ViewEinzug->Checked;
        mainfrm->einzugunten = down;
        mainfrm->einzug.darstellung = darst;
        mainfrm->hvisible = hvisible;
        switch (style) {
            case 0: mainfrm->EzFixiert->Checked = true; break;
            case 1: mainfrm->EzMinimalZ->Checked = true; break;
            case 2: mainfrm->EzMinimalS->Checked = true; break;
            case 3: mainfrm->EzGeradeZ->Checked = true; break;
            case 4: mainfrm->EzGeradeS->Checked = true; break;
            case 5: mainfrm->EzBelassen->Checked = true; break;
            case 6: mainfrm->EzChorig2->Checked = true; break;
            case 7: mainfrm->EzChorig3->Checked = true; break;
            default:
                dbw3_assert(false);
                mainfrm->EzMinimalZ->Checked = true;
                break;
        }
    }
    if (Need(LOADALL)) {
        mainfrm->einzug.pos.strongline_x = stronglinex;
        mainfrm->einzug.pos.strongline_y = strongliney;
    }
}

void __fastcall FhLoader::LoadViewAufknuepfung (FfReader* _reader)
{
	DARSTELLUNG darst = KREUZ;
	int stronglinex = 4;
	int strongliney = 4;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT ("viewtype", darst, DARSTELLUNG)
			FIELD_MAP_INT ("stronglinex", stronglinex, int)
			FIELD_MAP_INT ("strongliney", strongliney, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADAUFKNUEPFUNG)) {
        if (!Need(LOADALL)) {
            mainfrm->ViewEinzug->Checked = true;
            mainfrm->ViewEinzugPopup->Checked = mainfrm->ViewEinzug->Checked;
            mainfrm->ViewTrittfolge->Checked = true;
            mainfrm->ViewTrittfolgePopup->Checked = mainfrm->ViewTrittfolge->Checked;
        }
        mainfrm->aufknuepfung.darstellung = darst;
    }
    if (Need(LOADALL)) {
        mainfrm->aufknuepfung.pos.strongline_x = stronglinex;
        mainfrm->aufknuepfung.pos.strongline_y = strongliney;
    }
}

void __fastcall FhLoader::LoadViewTrittfolge (FfReader* _reader)
{
    int style = 1;
	int visible = 1;
	int left = 0;
	DARSTELLUNG darst = PUNKT;
	int stronglinex = 4;
	int strongliney = 4;
	bool einzeltritt = true;
	int wvisible = 12;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("visible", visible, int)
			FIELD_MAP_INT ("left", left, int)
			FIELD_MAP_INT ("viewtype", darst, DARSTELLUNG)
			FIELD_MAP_INT ("stronglinex", stronglinex, int)
			FIELD_MAP_INT ("strongliney", strongliney, int)
			FIELD_MAP_INT ("single", einzeltritt, bool)
			FIELD_MAP_INT ("wvisible", wvisible, int)
            FIELD_MAP_INT ("style", style, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADTRITTFOLGE)) {
        if (!Need(LOADALL)) visible = true;
        mainfrm->ViewTrittfolge->Checked = visible;
        mainfrm->ViewTrittfolgePopup->Checked = mainfrm->ViewTrittfolge->Checked;
//        mainfrm->ViewTrittfolgeLinks->Checked = left;
        (void)left;
        mainfrm->trittfolge.darstellung = darst;
        mainfrm->trittfolge.einzeltritt = einzeltritt;
        mainfrm->wvisible = wvisible;
        switch (style) {
            case 0: mainfrm->TfBelassen->Checked = true; break;
            case 1: mainfrm->TfMinimalZ->Checked = true; break;
            case 2: mainfrm->TfMinimalS->Checked = true; break;
            case 3: mainfrm->TfGeradeZ->Checked = true; break;
            case 4: mainfrm->TfGeradeS->Checked = true; break;
            case 5: mainfrm->TfGesprungen->Checked = true; break;
            default:
                dbw3_assert(false);
                mainfrm->TfMinimalZ->Checked = true;
                break;
        }
    }
    if (Need(LOADALL)) {
        mainfrm->trittfolge.pos.strongline_x = stronglinex;
        mainfrm->trittfolge.pos.strongline_y = strongliney;
    }
}

void __fastcall FhLoader::LoadViewSchlagpatrone (FfReader* _reader)
{
	DARSTELLUNG darst = AUSGEFUELLT;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT ("viewtype", darst, DARSTELLUNG)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADTRITTFOLGE)) {
		mainfrm->schlagpatronendarstellung = darst;
    }
}

void __fastcall FhLoader::LoadViewGewebe (FfReader* _reader)
{
	int state = 0;
	int lock = 0;
	int stronglinex = 4;
	int strongliney = 4;
	int withgrid = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("state", state, int)
			FIELD_MAP_INT ("locked", lock, int)
			FIELD_MAP_INT ("stronglinex", stronglinex, int)
			FIELD_MAP_INT ("strongliney", strongliney, int)
            FIELD_MAP_INT ("withgrid", withgrid, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        mainfrm->OptionsLockGewebe->Checked = lock;
        mainfrm->gewebe.pos.strongline_x = stronglinex;
        mainfrm->gewebe.pos.strongline_y = strongliney;
        if (state==0) mainfrm->SelectGewebeNormal();
        else if (state==1) mainfrm->SelectGewebeFarbeffekt();
        else if (state==2) mainfrm->SelectGewebeSimulation();
        else if (state==3) mainfrm->SelectGewebeNone();
        else mainfrm->SelectGewebeNormal();
        mainfrm->fewithraster = withgrid;
    }
}

void __fastcall FhLoader::LoadViewBlatteinzug (FfReader* _reader)
{
	int viewblatteinzug = 1;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("visible", viewblatteinzug, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADBLATTEINZUG)) {
        if (!Need(LOADALL)) viewblatteinzug = true;
        mainfrm->ViewBlatteinzug->Checked = viewblatteinzug;
        mainfrm->ViewBlatteinzugPopup->Checked = mainfrm->ViewBlatteinzug->Checked;
    }
}

void __fastcall FhLoader::LoadViewKettfarben (FfReader* _reader)
{
	int viewfarbe = 1;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("visible", viewfarbe, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADKETTFARBEN)) {
        if (!Need(LOADALL)) viewfarbe = true;
        mainfrm->ViewFarbe->Checked = viewfarbe;
        mainfrm->ViewFarbePopup->Checked = mainfrm->ViewFarbe->Checked;
    }
}

void __fastcall FhLoader::LoadViewSchussfarben (FfReader* _reader)
{
	int viewfarbe = 1;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("visible", viewfarbe, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADSCHUSSFARBEN)) {
        if (!Need(LOADALL)) viewfarbe = true;
        mainfrm->ViewFarbe->Checked = viewfarbe;
        mainfrm->ViewFarbePopup->Checked = mainfrm->ViewFarbe->Checked;
    }
}

void __fastcall FhLoader::LoadViewPagesetup (FfReader* _reader)
{
	char* headertext = 0;
	char* footertext = 0;
	int topborder; //xxx Initialisieren
	int bottomborder;
	int rightborder;
	int leftborder;
	int headerheight;
	int footerheight;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("topmargin", topborder, int)
			FIELD_MAP_INT("bottommargin", bottomborder, int)
			FIELD_MAP_INT("leftmargin", leftborder, int)
			FIELD_MAP_INT("rightmargin", rightborder, int)
			FIELD_MAP_INT("headerheight", headerheight, int)
			FIELD_MAP_STRING("headertext", headertext)
			FIELD_MAP_INT("footerheight", footerheight, int)
			FIELD_MAP_STRING("footertext", footertext)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        mainfrm->borders.range.top = topborder;
        mainfrm->borders.range.bottom = bottomborder;
        mainfrm->borders.range.left = leftborder;
        mainfrm->borders.range.right = rightborder;
        mainfrm->header.text = headertext;
        mainfrm->footer.text = footertext;
        mainfrm->footer.height = footerheight;
        mainfrm->header.height = headerheight;
    }

	delete headertext;
	delete footertext;
}

void __fastcall FhLoader::LoadPrint (FfReader* _reader)
{
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			NO_FIELDS
		BEGIN_SECTION_MAP
			_SECTION_MAP("printrange", LoadPrintRange)
			DEFAULT_SECTION
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
}

void __fastcall FhLoader::LoadPrintRange (FfReader* _reader)
{
	int prkettea = 0;
	int prketteb = 20;
	int prschuessea = 0;
	int prschuesseb = 20;
	int prschaeftea = 0;
	int prschaefteb = 8;
	int prtrittea = 0;
	int prtritteb = 8;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("kettevon", prkettea, int)
			FIELD_MAP_INT("kettebis", prketteb, int)
			FIELD_MAP_INT("schuessevon", prschuessea, int)
			FIELD_MAP_INT("schuessebis", prschuesseb, int)
			FIELD_MAP_INT("schaeftevon", prschaeftea, int)
			FIELD_MAP_INT("schaeftebis", prschaefteb, int)
			FIELD_MAP_INT("trittevon", prtrittea, int)
			FIELD_MAP_INT("trittebis", prtritteb, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

    if (Need(LOADALL)) {
        mainfrm->printkette.a = prkettea;
        mainfrm->printkette.b = prketteb;
        mainfrm->printschuesse.a = prschuessea;
        mainfrm->printschuesse.b = prschuesseb;
        mainfrm->printschaefte.a = prschaeftea;
        mainfrm->printschaefte.b = prschaefteb;
        mainfrm->printtritte.a = prtrittea;
        mainfrm->printtritte.b = prtritteb;
    }
}


