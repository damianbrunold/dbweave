/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  "@dbw3:file" text file format reader/writer. Portable port of the
    original Win32 HANDLE-based implementation: FfFile now wraps a
    std::FILE* opened with the standard fopen/fread/fwrite primitives
    so the on-disk byte stream, including "\r\n" line endings, remains
    identical to files produced by the legacy dbw.exe.
*/

/*-----------------------------------------------------------------*/
#ifndef DBWEAVE_IO_FILEFORMAT_H
#define DBWEAVE_IO_FILEFORMAT_H
/*-----------------------------------------------------------------*/
#include "vcl_compat.h"    /* neutralises __fastcall etc. */
#include <cstdio>
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
	virtual ~FfToken() = default;
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
/*  Decodes a 2*N hex-digit ASCII string into an N-byte buffer. The
    destination must be pre-allocated with at least N+1 bytes (the
    helper writes a trailing NUL). Inverse of FfWriter::WriteFieldBinary.
    Extracted out of the legacy fileload.cpp so domain code can hex-
    decode without dragging in the main-window dependency chain. */
void __fastcall FieldHexToBinary (void* _dest, const void* _source, int _length);
/*-----------------------------------------------------------------*/
enum FfOpenFlag { FfOpenRead=1, FfOpenWrite=2, FfOpenOverwrite=4 };
/*-----------------------------------------------------------------*/
class FfFile
{
private:
	char*       filename;
	std::FILE*  hfile;
	int         openflags;

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
	bool   assigned;
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
