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

#ifndef felddefH
#define felddefH

#include "enums.h"

// Forward-Deklarationen
class FfReader;
class FfWriter;

class FeldVectorChar
{
private:
    int size;
    unsigned char* feld;

public:
    FeldVectorChar (int _size, unsigned char _default);
    virtual ~FeldVectorChar();
    void Init (unsigned char _default);
    void Resize (int _newsize, unsigned char _default);
    void Write (const char* _section, FfWriter* _writer);
    void Read (FfReader* _reader, unsigned char _default);
    unsigned char Get (int _i);
    void Set (int _i, unsigned char _value);
    int Size() const { return size; }
    FeldVectorChar& operator=(const FeldVectorChar& _source);
};

class FeldVectorShort
{
private:
    int size;
    short* feld;

public:
    FeldVectorShort (int _size, short _default);
    virtual ~FeldVectorShort();
    void Init (short _default);
    void Resize (int _newsize, short _default);
    void Write (const char* _section, FfWriter* _writer);
    void Read (FfReader* _reader, short _default);
    short Get (int _i);
    void Set (int _i, short _value);
    int Size() const { return size; }
    FeldVectorShort& operator=(const FeldVectorShort& _source);
};

class FeldVectorBool
{
private:
    int size;
    bool* feld;

public:
    FeldVectorBool (int _size, bool _default);
    virtual ~FeldVectorBool();
    void Init (bool _default);
    void Resize (int _newsize, bool _default);
    void Write (const char* _section, FfWriter* _writer);
    void Read (FfReader* _reader, bool _default);
    bool Get (int _i);
    void Set (int _i, bool value);
    int Size() const { return size; }
    FeldVectorBool& operator=(const FeldVectorBool& _source);
};

class FeldGridChar
{
private:
    int sizex, sizey;
    char* feld;

public:
    FeldGridChar (int _sizex, int _sizey, char _default);
    virtual ~FeldGridChar();
    void Init (char _default);
    void Resize (int _newsizex, int _newsizey, char _default);
    void Write (const char* _section, FfWriter* _writer);
    void Read (FfReader* _reader, char _default);
    char Get (int _i, int _j);
    void Set (int _i, int _j, char _value);
    int SizeX() const { return sizex; }
    int SizeY() const { return sizey; }
    FeldGridChar& operator=(const FeldGridChar& _source);
};

#endif


