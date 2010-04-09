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
#include <vcl\vcl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "fileformat.h"
/*-----------------------------------------------------------------*/
#define FF_SIGNATURE "@dbw3:file\r\n"
/*-----------------------------------------------------------------*/
const int INDENT        = 2;
const int MAXLEVELS     = 30;
/*-----------------------------------------------------------------*/
__fastcall FfBuffer::FfBuffer (int _defaultlen, int _increment/*=255*/)
{
	defaultlen = _defaultlen;
	increment  = _increment;

	buffer = new char[defaultlen+1];
	maxlength = defaultlen;
	length = 0;
	buffer[0] = 0;
}
/*-----------------------------------------------------------------*/
__fastcall FfBuffer::~FfBuffer()
{
	delete[] buffer;
}
/*-----------------------------------------------------------------*/
void __fastcall FfBuffer::Add (int _ch)
{
	if (length>=maxlength)
		Reallocate();

	if (length<maxlength) {
		buffer[length++] = (char)_ch;
		buffer[length] = 0;
	}
}
/*-----------------------------------------------------------------*/
void __fastcall FfBuffer::Reallocate()
{
	try {
        char* newbuff = new char[maxlength+increment+1];
        maxlength += increment;
		memcpy (newbuff, buffer, length+1);
		delete[] buffer;
		buffer = newbuff;
	} catch (...) {
	}
}
/*-----------------------------------------------------------------*/
FfTokenBase::FfTokenBase()
{
	data = 0;
	length = 0;
}
/*-----------------------------------------------------------------*/
FfTokenBase::~FfTokenBase()
{
	delete[] data;
}
/*-----------------------------------------------------------------*/
void __fastcall FfTokenBase::SetData (FfBuffer& buffer)
{
	try {
        delete[] data;
        length = buffer.length;
        data = new char[length+1];
        memcpy (data, buffer.buffer, length+1);
	} catch (...) {
		data = 0;
		length = 0;
	}
}
/*-----------------------------------------------------------------*/
bool IsTokenEqual (FfToken* _token, const char* _id)
{
	FfTokenBase* base = (FfTokenBase*)_token;
	return strcmp ((char*)base->data, _id)==0;
}
/*-----------------------------------------------------------------*/
__fastcall FfFile::FfFile()
{
	openflags = 0;
	filename = 0;
	hfile = INVALID_HANDLE_VALUE;
}
/*-----------------------------------------------------------------*/
__fastcall FfFile::~FfFile()
{
	Close();
	delete filename;
}
/*-----------------------------------------------------------------*/
bool __fastcall FfFile::Open (const char* _filename, int _of)
{
	openflags = _of;

	DWORD access = 0;
	if (openflags&FfOpenRead)
		access |= GENERIC_READ;
	if (openflags&FfOpenWrite)
		access |= GENERIC_WRITE;

	DWORD creation;
	if (openflags&FfOpenOverwrite) creation = CREATE_ALWAYS;
	else if (openflags&FfOpenWrite) {
		if (openflags&FfOpenRead) creation = OPEN_EXISTING;
		else creation = OPEN_ALWAYS;
	} else
		creation = OPEN_EXISTING;

	try {
		char* temp = new char[strlen(_filename)+1];
		filename = temp;
		strcpy (filename, _filename);
		Close();
		hfile = CreateFile (filename, access, 0, NULL, creation,
					FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS, NULL);
//		dbw3_assert (hfile!=INVALID_HANDLE_VALUE);
	} catch (...) {
		return false;
	}
	return IsOpen();
}
/*-----------------------------------------------------------------*/
void __fastcall FfFile::Close()
{
	if (hfile!=INVALID_HANDLE_VALUE) {
		CloseHandle (hfile);
		hfile = INVALID_HANDLE_VALUE;
	}
}
/*-----------------------------------------------------------------*/
bool __fastcall FfFile::IsOpen() const
{
	return hfile!=INVALID_HANDLE_VALUE;
}
/*-----------------------------------------------------------------*/
bool __fastcall FfFile::IsWriteable() const
{
	return IsOpen() && (openflags&FfOpenWrite)!=0;
}
/*-----------------------------------------------------------------*/
bool __fastcall FfFile::IsReadable() const
{
	return IsOpen() && (openflags&FfOpenRead)!=0;
}
/*-----------------------------------------------------------------*/
FfFile& FfFile::operator= (const FfFile& _file)
{
	if (this==&_file) return *this;

	try {
		char* t = new char[strlen(_file.filename)+1];
		strcpy (t, _file.filename);
        delete[] filename;
		filename = t;
        openflags = _file.openflags;
		hfile = _file.hfile;
	} catch (...) {
	}

	return *this;
}
/*-----------------------------------------------------------------*/
int __fastcall FfFile::Read (void* _buffer, int _length)
{
	DWORD read;
	return ReadFile (hfile, _buffer, _length, &read, NULL);
}
/*-----------------------------------------------------------------*/
int __fastcall FfFile::Read()
{
	DWORD read;
	char buff[1];
	ReadFile (hfile, buff, 1, &read, NULL);
	if (read==1)
		return buff[0];
	else
		return 0;
}
/*-----------------------------------------------------------------*/
int __fastcall FfFile::Write (const void* _buffer, int _length)
{
	DWORD written;
	WriteFile (hfile, _buffer, _length, &written, NULL);
	return written;
}
/*-----------------------------------------------------------------*/
void __fastcall FfFile::SeekBegin()
{
	SetFilePointer (hfile, 0, NULL, FILE_BEGIN);
}
/*-----------------------------------------------------------------*/
void __fastcall FfFile::SetEndOfFile()
{
	::SetEndOfFile (hfile);
}
/*-----------------------------------------------------------------*/
void __fastcall FfFile::SetInvalid()
{
	hfile = INVALID_HANDLE_VALUE;
}
/*-----------------------------------------------------------------*/
__fastcall FfBase::FfBase()
{
    assigned = false;
}
/*-----------------------------------------------------------------*/
__fastcall FfBase::~FfBase()
{
    // Der FfFile-Destructor darf nicht
    // die Datei schliessen!
    if (assigned) file.SetInvalid();
}
/*-----------------------------------------------------------------*/
bool __fastcall FfBase::Assign (FfFile* _file)
{
	file = *_file;
    assigned = true;
	return file.IsOpen()==_file->IsOpen();
}
/*-----------------------------------------------------------------*/
void __fastcall FfBase::Close()
{
	if (!assigned) file.Close();
}
/*-----------------------------------------------------------------*/
bool __fastcall FfBase::IsOpen() const
{
	return file.IsOpen();
}
/*-----------------------------------------------------------------*/
bool __fastcall FfReader::Open (const char* _filename)
{
	return file.Open (_filename, FfOpenRead);
}
/*-----------------------------------------------------------------*/
void __fastcall FfReader::SkipSection()
{
	int level = 1;
	do {
		FfToken* token = GetToken();
		if (!token) throw "No Token";
		if (token->GetType()==FfSection) level++;
		else if (token->GetType()==FfEndSection) level--;
		delete token;
	} while (level!=0);
}
/*-----------------------------------------------------------------*/
void __fastcall FfReader::SkipField()
{
	FfToken* token = GetToken();
	if (!token) throw "No Token";
	if (token->GetType()!=FfValue) {
		delete token;
		throw "No Value";
	}
	delete token;
}
/*-----------------------------------------------------------------*/
FfToken* __fastcall FfReader::GetToken()
{
	// Mit einer Finite State Machine werden
	// die einzelnen Tokens ermittelt
	FfBuffer buffer(20, 1024);
	FfToken* token = NULL;
	enum STATE { BEGIN, COMMENT, SECTION, FIELD, VALUE, VALUEX, VALUEX2, SIGNATURE, EXIT };
	STATE state = BEGIN;
	int ch = file.Read();

	while (state!=EXIT && ch!=0) {
		switch (state) {
			case BEGIN:
				if (ch==' ' || ch=='\t' || ch=='\n' || ch=='\r') ;/*überlesen*/
				else if (ch==';') state=COMMENT;
				else if (ch=='\\') state=SECTION;
				else if (ch=='=') state=VALUE;
				else if (ch=='@') state=SIGNATURE;
				else if (ch=='}') {
					token = new FfTokenEndSection;
					state=EXIT;
				} else { buffer.Add (ch); state=FIELD; }
				break;

            case COMMENT:
				// Alles überlesen, bis End of Line kommt!
				if (ch=='\r' || ch=='\n') state=BEGIN;
				break;

			case SECTION:
				if (ch!='{' && ch!='\t' && ch!=' ' && ch!='\n' && ch!='\r') buffer.Add (ch);
				else {
					token = new FfTokenSection;
					((FfTokenSection*)token)->SetData (buffer);
					state=EXIT;
				}
				break;

			case FIELD:
				if (ch!='=' && ch!='\t' && ch!=' ') buffer.Add (ch);
				else {
					token = new FfTokenField;
					((FfTokenField*)token)->SetData (buffer);
					state=EXIT;
				}
				break;

			case VALUE:
				if (ch!='\r' && ch!='\n' && ch!='\\') buffer.Add (ch);
                else if (ch=='\\') {
                    state = VALUEX;
				} else {
					token = new FfTokenValue;
					((FfTokenValue*)token)->SetData (buffer);
					state=EXIT;
				}
				break;

            case VALUEX:
                if (ch!='\r' && ch!='\n') {
                    buffer.Add ('\\');
                    buffer.Add (ch);
                    state = VALUE;
                } else
                    state = VALUEX2;
                break;

            case VALUEX2:
                if (ch!='\r' && ch!='\n') {
                    buffer.Add (ch);
                    state = VALUE;
                }
                break;

			case SIGNATURE: {
				char buff[15];
				file.Read (buff, strlen(FF_SIGNATURE+2));
				if (ch==FF_SIGNATURE[1] &&
					memcmp(buff, FF_SIGNATURE+2, strlen(FF_SIGNATURE+2))==0)
					token = new FfTokenSignature;
                state=EXIT;
				break;
			}
		}

		if (state!=EXIT) ch = file.Read();
	}

	return token;
}
/*-----------------------------------------------------------------*/
__fastcall FfWriter::FfWriter()
{
	indent = 0;
    breakfields = true;
}
/*-----------------------------------------------------------------*/
bool __fastcall FfWriter::Open (const char* _filename)
{
	return file.Open (_filename, FfOpenWrite|FfOpenOverwrite);
}
/*-----------------------------------------------------------------*/
void __fastcall FfWriter::WriteSignature()
{
	dbw3_assert (file.IsWriteable());
	file.SeekBegin(); // Die Signatur kommt immer zu Beginn des Files!
	unsigned int written = file.Write (FF_SIGNATURE, strlen(FF_SIGNATURE));
	dbw3_assert (written==strlen(FF_SIGNATURE));
    (void)written;
}
/*-----------------------------------------------------------------*/
void __fastcall FfWriter::Indentation()
{
	dbw3_assert (file.IsWriteable());
	dbw3_assert (indent<=MAXLEVELS);
	char buff[INDENT*MAXLEVELS+1];
	for (int i=0; i<INDENT*indent; i++)
    	buff[i] = ' ';
	buff[INDENT*indent] = 0;
	file.Write (buff, INDENT*indent);
}
/*-----------------------------------------------------------------*/
void __fastcall FfWriter::BeginSection (const char* _section, const char* _description/*=0*/)
{
	dbw3_assert (file.IsWriteable());
	dbw3_assert (_section);
	Indentation();
	if (_description!=0) {
		file.Write (";\r\n;\r\n; ", 8);
		file.Write (_description, strlen(_description));
		file.Write ("\r\n;\r\n", 5);
	}
	file.Write ("\\", 1);
	file.Write (_section, strlen(_section));
	file.Write ("{\r\n", 3);
	indent += INDENT;
}
/*-----------------------------------------------------------------*/
void __fastcall FfWriter::EndSection()
{
	dbw3_assert (file.IsWriteable());
	indent -= INDENT;
	Indentation();
	file.Write ("}\r\n", 3);
}
/*-----------------------------------------------------------------*/
void __fastcall FfWriter::WriteField (const char* _field, const char* _data)
{
	dbw3_assert (file.IsWriteable());
	dbw3_assert (_field);
	dbw3_assert (_data);
	Indentation();
	file.Write (_field, strlen(_field));
	file.Write ("==", 2);
    if (breakfields) {
        const int chunklength = 70;
        int length = strlen(_data);
        bool first = true;
        while (length>=0) {
            if (length<=chunklength) {
                file.Write (_data, length);
                file.Write ("\r\n", 2);
                break;
            } else {
                WriteFieldChunk (_data, chunklength, length, !first);
                length -= chunklength;
                _data += chunklength;
                first = false;
            }
        }
    } else {
        file.Write (_data, strlen(_data));
        file.Write ("\r\n", 2);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall FfWriter::WriteFieldChunk (const char* _data, int _chunk, int _length, bool _indent/*=false*/)
{
    dbw3_assert (file.IsWriteable());
    dbw3_assert (_data);
    dbw3_assert (_chunk>0);
    file.Write (_data, _chunk);
    if (_length-_chunk > 0) file.Write ("\\", 1);
    file.Write ("\r\n", 2);
}
/*-----------------------------------------------------------------*/
static void __fastcall ByteToHex (char* _buff, unsigned int _byte)
{
    char hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                   '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    _buff[0] = hex[_byte/16];
    _buff[1] = hex[_byte%16];
}
/*-----------------------------------------------------------------*/
void __fastcall FfWriter::WriteFieldBinary (const char* _field, void* _data, int _length)
{
	dbw3_assert (_data);
	dbw3_assert (_length>=0);
	try {
		// Daten in Hexdump verwandeln!
		char* hexdump = new char[2*_length+2];
#ifdef _DEBUG
        memset (hexdump, 0xde, 2*_length+2);
#endif
        hexdump[0] = 0;
		char* d = hexdump;
		char* s = (char*)_data;
		for (int i=0; i<_length; i++) {
			ByteToHex (d, (unsigned char)*s);
            s++;
            d += 2;
            *d = 0;
		}
		WriteField (_field, hexdump);
	} catch (...) {
	}
}
/*-----------------------------------------------------------------*/
void __fastcall FfWriter::WriteFieldInt (const char* _field, int _data)
{
    char buff[10];
    itoa (_data, buff, 10);
    WriteField (_field, buff);
}
/*-----------------------------------------------------------------*/
void __fastcall FfWriter::WriteFieldDouble (const char* _field, double _data)
{
	char buff[25];
	sprintf (buff, "%0.8f", _data);
	WriteField (_field, buff);
}
/*-----------------------------------------------------------------*/


