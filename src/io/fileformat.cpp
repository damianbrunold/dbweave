/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*-----------------------------------------------------------------*/
#include "vcl_compat.h"
#include "assert_compat.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#if defined(_WIN32)
#   include <io.h>
#else
#   include <unistd.h>
#endif
/*-----------------------------------------------------------------*/
#include "fileformat.h"
/*-----------------------------------------------------------------*/
#define FF_SIGNATURE "@dbw3:file\r\n"
/*-----------------------------------------------------------------*/
const int INDENT        = 2;
const int MAXLEVELS     = 30;
/*-----------------------------------------------------------------*/
 FfBuffer::FfBuffer (int _defaultlen, int _increment/*=255*/)
{
	defaultlen = _defaultlen;
	increment  = _increment;

	buffer = new char[defaultlen+1];
	maxlength = defaultlen;
	length = 0;
	buffer[0] = 0;
}
/*-----------------------------------------------------------------*/
 FfBuffer::~FfBuffer()
{
	delete[] buffer;
}
/*-----------------------------------------------------------------*/
void FfBuffer::Add (int _ch)
{
	if (length>=maxlength)
		Reallocate();

	if (length<maxlength) {
		buffer[length++] = (char)_ch;
		buffer[length] = 0;
	}
}
/*-----------------------------------------------------------------*/
void FfBuffer::Reallocate()
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
	delete[] static_cast<char*>(data);
}
/*-----------------------------------------------------------------*/
void FfTokenBase::SetData (FfBuffer& buffer)
{
	try {
		delete[] static_cast<char*>(data);
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
 FfFile::FfFile()
{
	openflags = 0;
	filename = 0;
	hfile = nullptr;
}
/*-----------------------------------------------------------------*/
 FfFile::~FfFile()
{
	Close();
	delete[] filename;
}
/*-----------------------------------------------------------------*/
bool FfFile::Open (const char* _filename, int _of)
{
	openflags = _of;

	/*  Map FfOpen flags onto a stdio mode string.
	      Overwrite   : "wb+"  (create/truncate, read+write)
	      Write only  : "wb"   (create/truncate)    when !FfOpenRead
	      Read+Write  : "rb+"  (must exist)
	      Read only   : "rb"   (must exist)
	    The "b" suffix matters on Windows so "\r\n" is emitted verbatim
	    rather than being re-translated by the runtime. */
	const char* mode = "rb";
	if (openflags & FfOpenOverwrite) {
		mode = "wb+";
	} else if (openflags & FfOpenWrite) {
		if (openflags & FfOpenRead) mode = "rb+";
		else                         mode = "wb";
	}

	try {
		char* temp = new char[strlen(_filename)+1];
		strcpy (temp, _filename);
		Close();
		delete[] filename;
		filename = temp;
		hfile = std::fopen (filename, mode);
	} catch (...) {
		return false;
	}
	return IsOpen();
}
/*-----------------------------------------------------------------*/
void FfFile::Close()
{
	if (hfile!=nullptr) {
		std::fclose (hfile);
		hfile = nullptr;
	}
}
/*-----------------------------------------------------------------*/
bool FfFile::IsOpen() const
{
	return hfile!=nullptr;
}
/*-----------------------------------------------------------------*/
bool FfFile::IsWriteable() const
{
	return IsOpen() && (openflags&FfOpenWrite)!=0;
}
/*-----------------------------------------------------------------*/
bool FfFile::IsReadable() const
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
int FfFile::Read (void* _buffer, int _length)
{
	return (int)std::fread (_buffer, 1, (size_t)_length, hfile);
}
/*-----------------------------------------------------------------*/
int FfFile::Read()
{
	unsigned char b;
	if (std::fread (&b, 1, 1, hfile)==1) return (int)b;
	return 0;
}
/*-----------------------------------------------------------------*/
int FfFile::Write (const void* _buffer, int _length)
{
	return (int)std::fwrite (_buffer, 1, (size_t)_length, hfile);
}
/*-----------------------------------------------------------------*/
void FfFile::SeekBegin()
{
	std::fseek (hfile, 0, SEEK_SET);
}
/*-----------------------------------------------------------------*/
void FfFile::SetEndOfFile()
{
	/*  stdio has no portable truncate-to-current-position operation.
	    Flush to make sure the tell position is valid, then rely on the
	    platform: on POSIX use ftruncate(fileno(hfile), ftell(hfile));
	    on Windows use _chsize. Guarded so this compiles everywhere. */
	std::fflush (hfile);
#if defined(_WIN32)
	{
		long pos = std::ftell (hfile);
		::_chsize (::_fileno (hfile), pos);
	}
#else
	{
		long pos = std::ftell (hfile);
		::ftruncate (::fileno (hfile), pos);
	}
#endif
}
/*-----------------------------------------------------------------*/
void FfFile::SetInvalid()
{
	hfile = nullptr;
}
/*-----------------------------------------------------------------*/
 FfBase::FfBase()
{
	assigned = false;
}
/*-----------------------------------------------------------------*/
 FfBase::~FfBase()
{
	// Der FfFile-Destructor darf nicht
	// die Datei schliessen!
	if (assigned) file.SetInvalid();
}
/*-----------------------------------------------------------------*/
bool FfBase::Assign (FfFile* _file)
{
	file = *_file;
	assigned = true;
	return file.IsOpen()==_file->IsOpen();
}
/*-----------------------------------------------------------------*/
void FfBase::Close()
{
	if (!assigned) file.Close();
}
/*-----------------------------------------------------------------*/
bool FfBase::IsOpen() const
{
	return file.IsOpen();
}
/*-----------------------------------------------------------------*/
bool FfReader::Open (const char* _filename)
{
	return file.Open (_filename, FfOpenRead);
}
/*-----------------------------------------------------------------*/
void FfReader::SkipSection()
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
void FfReader::SkipField()
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
FfToken* FfReader::GetToken()
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
				if (ch==' ' || ch=='\t' || ch=='\n' || ch=='\r') ;/*ueberlesen*/
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
				// Alles ueberlesen, bis End of Line kommt!
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
				file.Read (buff, (int)strlen(FF_SIGNATURE+2));
				if (ch==FF_SIGNATURE[1] &&
					memcmp(buff, FF_SIGNATURE+2, strlen(FF_SIGNATURE+2))==0)
					token = new FfTokenSignature;
				state=EXIT;
				break;
			}

			case EXIT:
				break;
		}

		if (state!=EXIT) ch = file.Read();
	}

	return token;
}
/*-----------------------------------------------------------------*/
 FfWriter::FfWriter()
{
	indent = 0;
	breakfields = true;
}
/*-----------------------------------------------------------------*/
bool FfWriter::Open (const char* _filename)
{
	return file.Open (_filename, FfOpenWrite|FfOpenOverwrite);
}
/*-----------------------------------------------------------------*/
void FfWriter::WriteSignature()
{
	dbw3_assert (file.IsWriteable());
	file.SeekBegin(); // Die Signatur kommt immer zu Beginn des Files!
	unsigned int written = file.Write (FF_SIGNATURE, (int)strlen(FF_SIGNATURE));
	dbw3_assert (written==strlen(FF_SIGNATURE));
	(void)written;
}
/*-----------------------------------------------------------------*/
void FfWriter::Indentation()
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
void FfWriter::BeginSection (const char* _section, const char* _description/*=0*/)
{
	dbw3_assert (file.IsWriteable());
	dbw3_assert (_section);
	Indentation();
	if (_description!=0) {
		file.Write (";\r\n;\r\n; ", 8);
		file.Write (_description, (int)strlen(_description));
		file.Write ("\r\n;\r\n", 5);
	}
	file.Write ("\\", 1);
	file.Write (_section, (int)strlen(_section));
	file.Write ("{\r\n", 3);
	indent += INDENT;
}
/*-----------------------------------------------------------------*/
void FfWriter::EndSection()
{
	dbw3_assert (file.IsWriteable());
	indent -= INDENT;
	Indentation();
	file.Write ("}\r\n", 3);
}
/*-----------------------------------------------------------------*/
void FfWriter::WriteField (const char* _field, const char* _data)
{
	dbw3_assert (file.IsWriteable());
	dbw3_assert (_field);
	dbw3_assert (_data);
	Indentation();
	file.Write (_field, (int)strlen(_field));
	file.Write ("==", 2);
	if (breakfields) {
		const int chunklength = 70;
		int length = (int)strlen(_data);
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
		file.Write (_data, (int)strlen(_data));
		file.Write ("\r\n", 2);
	}
}
/*-----------------------------------------------------------------*/
void FfWriter::WriteFieldChunk (const char* _data, int _chunk, int _length, bool /*_indent*/)
{
	dbw3_assert (file.IsWriteable());
	dbw3_assert (_data);
	dbw3_assert (_chunk>0);
	file.Write (_data, _chunk);
	if (_length-_chunk > 0) file.Write ("\\", 1);
	file.Write ("\r\n", 2);
}
/*-----------------------------------------------------------------*/
static void ByteToHex (char* _buff, unsigned int _byte)
{
	char hex[] = { '0', '1', '2', '3', '4', '5', '6', '7',
	               '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	_buff[0] = hex[_byte/16];
	_buff[1] = hex[_byte%16];
}
/*-----------------------------------------------------------------*/
void FfWriter::WriteFieldBinary (const char* _field, void* _data, int _length)
{
	dbw3_assert (_data);
	dbw3_assert (_length>=0);
	try {
		// Daten in Hexdump verwandeln!
		char* hexdump = new char[2*_length+2];
#ifndef NDEBUG
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
		delete[] hexdump;
	} catch (...) {
	}
}
/*-----------------------------------------------------------------*/
void FfWriter::WriteFieldInt (const char* _field, int _data)
{
	char buff[16];
	std::snprintf (buff, sizeof(buff), "%d", _data);
	WriteField (_field, buff);
}
/*-----------------------------------------------------------------*/
void FfWriter::WriteFieldDouble (const char* _field, double _data)
{
	char buff[32];
	std::snprintf (buff, sizeof(buff), "%0.8f", _data);
	WriteField (_field, buff);
}
/*-----------------------------------------------------------------*/
static void HexToByte (char* _byte, const char* _hex)
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
/*-----------------------------------------------------------------*/
void FieldHexToBinary (void* _dest, const void* _source, int _length)
{
	dbw3_assert (_dest);
	dbw3_assert (_source);
	dbw3_assert (_length>=0);

	try {
		// Daten von Hexdump zurueckverwandeln!
		// Buffer sind schon richtig dimensioniert uebergeben worden...
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
/*-----------------------------------------------------------------*/
