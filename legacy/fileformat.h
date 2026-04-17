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
#ifndef fileformatH
#define fileformatH
/*-----------------------------------------------------------------*/
#include <stdio.h>
/*-----------------------------------------------------------------*/
class FfBuffer
{
public:
	char* buffer;
	int length;

private:
    int maxlength;
	int defaultlen;
	int increment;

public:
	__fastcall FfBuffer (int _defaultlen, int _increment=255);
	virtual __fastcall ~FfBuffer();
	void __fastcall Add (int _ch);

private:
	void __fastcall Reallocate();
};
/*-----------------------------------------------------------------*/
enum FfTokentype {
	FfInvalid=-1,
	FfSignature=0,
	FfSection,
	FfEndSection,
	FfField,
	FfValue,
};
/*-----------------------------------------------------------------*/
struct FfToken
{
	virtual FfTokentype __fastcall GetType() { return FfInvalid; }
};
/*-----------------------------------------------------------------*/
struct FfTokenSignature : public FfToken
{
	virtual FfTokentype __fastcall GetType() { return FfSignature; }
};
/*-----------------------------------------------------------------*/
struct FfTokenEndSection : public FfToken
{
	virtual FfTokentype __fastcall GetType() { return FfEndSection; }
};
/*-----------------------------------------------------------------*/
struct FfTokenBase : public FfToken
{
	int   length;
	void* data;
	FfTokenBase();
	virtual ~FfTokenBase();
	void __fastcall SetData (FfBuffer& buffer);
};
/*-----------------------------------------------------------------*/
struct FfTokenSection : public FfTokenBase
{
	virtual FfTokentype __fastcall GetType() { return FfSection; }
};
/*-----------------------------------------------------------------*/
struct FfTokenField : public FfTokenBase
{
	virtual FfTokentype __fastcall GetType() { return FfField; }
};
/*-----------------------------------------------------------------*/
struct FfTokenValue : public FfTokenBase
{
	virtual FfTokentype __fastcall GetType() { return FfValue; }
};
/*-----------------------------------------------------------------*/
bool IsTokenEqual (FfToken* _token, const char* _id);
/*-----------------------------------------------------------------*/
enum FfOpenFlag { FfOpenRead=1, FfOpenWrite=2, FfOpenOverwrite=4 };
/*-----------------------------------------------------------------*/
class FfFile
{
private:
	char*  filename;
    HANDLE hfile;
	int    openflags;

public:
	__fastcall FfFile();
	virtual __fastcall ~FfFile();

	bool __fastcall Open (const char* _filename, int _of);
	void __fastcall Close();
    void __fastcall SetInvalid();
	bool __fastcall IsOpen() const;
	bool __fastcall IsWriteable() const;
	bool __fastcall IsReadable() const;
	FfFile& operator= (const FfFile& _file);

	void __fastcall SeekBegin();
	void __fastcall SetEndOfFile();
	int __fastcall Read();
	int __fastcall Read (void* _buffer, int _length);
	int __fastcall Write (const void* _buffer, int _length);
};
/*-----------------------------------------------------------------*/
class FfBase
{
protected:
    bool assigned;
	FfFile file;

public:
    __fastcall FfBase();
    virtual __fastcall ~FfBase();
	bool __fastcall Assign (FfFile* _file);
	virtual bool __fastcall Open (const char* _filename) = 0;
	void __fastcall Close();
	bool __fastcall IsOpen() const;
};
/*-----------------------------------------------------------------*/
class FfReader : public FfBase
{
public:
	virtual bool __fastcall Open (const char* _filename);

	void __fastcall SkipSection();
	void __fastcall SkipField();
	FfToken* __fastcall GetToken();
};
/*-----------------------------------------------------------------*/
class FfWriter : public FfBase
{
private:
	int   indent;
    bool  breakfields;

public:
	__fastcall FfWriter();
	virtual bool __fastcall Open (const char* _filename);

    void __fastcall BreakFields (bool _bf=true) { breakfields = _bf; }

	void __fastcall WriteSignature();
	void __fastcall BeginSection (const char* _section, const char* _description=0);
	void __fastcall EndSection();
	void __fastcall WriteField (const char* _field, const char* _data);
	void __fastcall WriteFieldBinary (const char* _field, void* _data, int _length);
    void __fastcall WriteFieldInt (const char* _field, int _data);
	void __fastcall WriteFieldDouble (const char* _field, double _data);

protected:
    void __fastcall WriteFieldChunk (const char* _data, int _chunk, int _length, bool _indent=false);
	void __fastcall Indentation();
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
