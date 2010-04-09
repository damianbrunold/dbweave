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
#include "felddef.h"
#include "fileformat.h"
#include "loadmap.h"

// Aus fileload.cpp
void __fastcall FieldHexToBinary (void* _dest, const void* _source, int _length);

FeldVectorChar::FeldVectorChar (int _size, unsigned char _default)
{
    feld = new unsigned char[_size];
    size = _size;
    Init (_default);
}

FeldVectorChar::~FeldVectorChar()
{
    delete[] feld;
}

void FeldVectorChar::Init (unsigned char _default)
{
    memset (feld, _default, size);
}

void FeldVectorChar::Resize (int _newsize, unsigned char _default)
{
    if (_newsize==size) return;

    unsigned char* newfeld = new unsigned char[_newsize];
    for (int i=0; i<min(size, _newsize); i++) {
        newfeld[i] = feld[i];
    }
    if (size<_newsize) {
        for (int i=size; i<_newsize; i++)
            newfeld[i] = _default;
    }
    delete[] feld;
    feld = newfeld;
    size = _newsize;
}

void FeldVectorChar::Write (const char* _section, FfWriter* _writer)
{
    dbw3_assert (_section!=0);
    dbw3_assert (_writer!=0);
    dbw3_assert (_writer->IsOpen());
    _writer->BeginSection (_section);
        _writer->WriteFieldInt ("size", size);
        _writer->WriteFieldBinary ("data", feld, sizeof(unsigned char)*size);
    _writer->EndSection();
}

void FeldVectorChar::Read (FfReader* _reader, unsigned char _default)
{
	int newsize = size;
	char* newdata = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("size", newsize, int)
			FIELD_MAP_BINARY ("data", newdata)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

	// Feld redimensionieren
    if (newsize!=size) {
        Resize (newsize, _default);
	}

	// Daten setzen
	memcpy (feld, newdata, sizeof(unsigned char)*size);
	delete[] newdata;
}

unsigned char FeldVectorChar::Get (int _i)
{
    dbw3_assert (_i<size);
    return feld[_i];
}

void FeldVectorChar::Set (int _i, unsigned char _value)
{
    dbw3_assert (_i<size);
    if (_i<size) feld[_i] = _value;
}

FeldVectorShort::FeldVectorShort (int _size, short _default)
{
    feld = new short[_size];
    size = _size;
    Init (_default);
}

FeldVectorShort::~FeldVectorShort()
{
    delete[] feld;
}

void FeldVectorShort::Init (short _default)
{
    if (_default==0) {
        memset (feld, 0, size*sizeof(short));
    } else {
        short* p = feld;
        for (int i=0; i<size; i++)
            *p++ = _default;
    }
}

void FeldVectorShort::Resize (int _newsize, short _default)
{
    if (_newsize==size) return;

    short* newfeld = new short[_newsize];
    for (int i=0; i<min(size, _newsize); i++) {
        newfeld[i] = feld[i];
    }
    if (size<_newsize) {
        for (int i=size; i<_newsize; i++)
            newfeld[i] = _default;
    }
    delete[] feld;
    feld = newfeld;
    size = _newsize;
}

void FeldVectorShort::Write (const char* _section, FfWriter* _writer)
{
    dbw3_assert (_section!=0);
    dbw3_assert (_writer!=0);
    dbw3_assert (_writer->IsOpen());
    _writer->BeginSection (_section);
        _writer->WriteFieldInt ("size", size);
        _writer->WriteFieldBinary ("data", feld, sizeof(short)*size);
        _writer->WriteFieldInt ("dummy", 0);
    _writer->EndSection();
}

void FeldVectorShort::Read (FfReader* _reader, short _default)
{
	int newsize = size;
	char* newdata = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("size", newsize, int)
			FIELD_MAP_BINARY ("data", newdata)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

	// Feld redimensionieren
    if (newsize!=size) {
        Resize (newsize, _default);
	}

	// Daten setzen
	memcpy (feld, newdata, sizeof(short)*size);
	delete[] newdata;
}

short FeldVectorShort::Get (int _i)
{
    dbw3_assert (_i<size);
    return feld[_i];
}

void FeldVectorShort::Set (int _i, short _value)
{
    dbw3_assert (_i<size);
    if (_i<size) feld[_i] = _value;
}

FeldVectorBool::FeldVectorBool (int _size, bool _default)
{
    feld = new bool[_size];
    size = _size;
    Init (_default);
}

FeldVectorBool::~FeldVectorBool()
{
    delete[] feld;
}

void FeldVectorBool::Init (bool _default)
{
    if (_default==0) {
        memset (feld, 0, size*sizeof(bool));
    } else {
        bool* p = feld;
        for (int i=0; i<size; i++)
            *p++ = _default;
    }
}

void FeldVectorBool::Resize (int _newsize, bool _default)
{
    if (_newsize==size) return;

    bool* newfeld = new bool[_newsize];
    for (int i=0; i<min(size, _newsize); i++) {
        newfeld[i] = feld[i];
    }
    if (size<_newsize) {
        for (int i=size; i<_newsize; i++)
            newfeld[i] = _default;
    }
    delete feld;
    feld = newfeld;
    size = _newsize;
}

void FeldVectorBool::Write (const char* _section, FfWriter* _writer)
{
    dbw3_assert (_section!=0);
    dbw3_assert (_writer!=0);
    dbw3_assert (_writer->IsOpen());
    _writer->BeginSection (_section);
        _writer->WriteFieldInt ("size", size);
        _writer->WriteFieldBinary ("data", feld, sizeof(bool)*size);
    _writer->EndSection();
}

void FeldVectorBool::Read (FfReader* _reader, bool _default)
{
	int newsize;
	char* newdata = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("size", newsize, int)
			FIELD_MAP_BINARY ("data", newdata)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

	// Feld redimensionieren
    if (newsize!=size) {
        Resize (newsize, _default);
	}

	// Daten setzen
	memcpy (feld, newdata, sizeof(bool)*size);
	delete[] newdata;
}

bool FeldVectorBool::Get (int _i)
{
    dbw3_assert (_i<size);
    return feld[_i];
}

void FeldVectorBool::Set (int _i, bool _value)
{
    dbw3_assert (_i<size);
    if (_i<size) feld[_i] = _value;
}

FeldGridChar::FeldGridChar (int _sizex, int _sizey, char _default)
{
    feld = new char[_sizex*_sizey];
    sizex = _sizex;
    sizey = _sizey;
    Init (_default);
}

FeldGridChar::~FeldGridChar()
{
    delete[] feld;
}

void FeldGridChar::Init (char _default)
{
    memset (feld, _default, sizex*sizey);
}

void FeldGridChar::Resize (int _newsizex, int _newsizey, char _default)
{
    if (_newsizex==sizex && _newsizey==sizey) return;

    char* newfeld = new char[_newsizex*_newsizey];
    for (int i=0; i<min(sizex, _newsizex); i++) {
        for (int j=0; j<min (sizey, _newsizey); j++) {
            newfeld[i+_newsizex*j] = feld[i+sizex*j];
        }
    }

    // Falls Feld vergrössert wurde, muss es initialisiert werden
    if (_newsizex>sizex) {
        for (int i=sizex; i<_newsizex; i++)
            for (int j=0; j<_newsizey; j++)
                newfeld[i+_newsizex*j] = _default;
    }
    if (_newsizey>sizey) {
        for (int j=sizey; j<_newsizey; j++)
            for (int i=0; i<sizex; i++)
                newfeld[i+_newsizex*j] = _default;
    }

    delete feld;
    feld = newfeld;
    sizex = _newsizex;
    sizey = _newsizey;
}

void FeldGridChar::Write (const char* _section, FfWriter* _writer)
{
    dbw3_assert (_section!=0);
    dbw3_assert (_writer!=0);
    dbw3_assert (_writer->IsOpen());
    _writer->BeginSection (_section);
        _writer->WriteFieldInt ("sizex", sizex);
        _writer->WriteFieldInt ("sizey", sizey);
        _writer->WriteFieldBinary ("data", feld, sizeof(char)*sizex*sizey);
    _writer->EndSection();
}

void FeldGridChar::Read (FfReader* _reader, char _default)
{
	int newsizex = sizex;
	int newsizey = sizey;
	char* newdata = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("sizex", newsizex, int)
			FIELD_MAP_INT ("sizey", newsizey, int)
			FIELD_MAP_BINARY ("data", newdata)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

	// Feld redimensionieren
    if (newsizex!=sizex || newsizey!=sizey) {
        Resize (newsizex, newsizey, _default);
	}

	// Daten setzen
	memcpy (feld, newdata, sizeof(char)*sizex*sizey);
	delete[] newdata;
}

char FeldGridChar::Get (int _i, int _j)
{
    dbw3_assert (_i<sizex);
    dbw3_assert (_j<sizey);
    return feld[_i+sizex*_j];
}

void FeldGridChar::Set (int _i, int _j, char _value)
{
    dbw3_assert (_i<sizex);
    dbw3_assert (_j<sizey);
    if (_i<sizex && _j<sizey) feld[_i + sizex*_j] = _value;
}

FeldVectorChar& FeldVectorChar::operator=(const FeldVectorChar& _source)
{
    if (this==&_source) return *this;

    if (size!=_source.size) {
        Resize (_source.size, 0);
        size = _source.size;
    }
    memcpy (feld, _source.feld, size);

    return *this;
}

FeldVectorShort& FeldVectorShort::operator=(const FeldVectorShort& _source)
{
    if (this==&_source) return *this;

    if (size!=_source.size) {
        Resize (_source.size, 0);
        size = _source.size;
    }
    memcpy (feld, _source.feld, size*sizeof(short));

    return *this;
}

FeldVectorBool& FeldVectorBool::operator=(const FeldVectorBool& _source)
{
    if (this==&_source) return *this;

    if (size!=_source.size) {
        Resize (_source.size, 0);
        size = _source.size;
    }
    memcpy (feld, _source.feld, size*sizeof(bool));

    return *this;
}

FeldGridChar& FeldGridChar::operator=(const FeldGridChar& _source)
{
    if (this==&_source) return *this;

    if (sizex!=_source.sizex || sizey!=_source.sizey) {
        Resize (_source.sizex, _source.sizey, 0);
        sizex = _source.sizex;
        sizey = _source.sizey;
    }
    memcpy (feld, _source.feld, sizex*sizey);

    return *this;
}

