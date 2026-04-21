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
#include <cstdio>
#include <string>
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
    FfBuffer(int _defaultlen, int _increment = 255);
    virtual ~FfBuffer();
    void Add(int _ch);

private:
    void Reallocate();
};
/*-----------------------------------------------------------------*/
enum FfTokentype {
    FfInvalid = -1,
    FfSignature = 0,
    FfSection,
    FfEndSection,
    FfField,
    FfValue,
};
/*-----------------------------------------------------------------*/
struct FfToken {
    virtual ~FfToken() = default;
    virtual FfTokentype GetType()
    {
        return FfInvalid;
    }
};
/*-----------------------------------------------------------------*/
struct FfTokenSignature : public FfToken {
    virtual FfTokentype GetType()
    {
        return FfSignature;
    }
};
/*-----------------------------------------------------------------*/
struct FfTokenEndSection : public FfToken {
    virtual FfTokentype GetType()
    {
        return FfEndSection;
    }
};
/*-----------------------------------------------------------------*/
struct FfTokenBase : public FfToken {
    int length;
    void* data;
    FfTokenBase();
    virtual ~FfTokenBase();
    void SetData(FfBuffer& buffer);
};
/*-----------------------------------------------------------------*/
struct FfTokenSection : public FfTokenBase {
    virtual FfTokentype GetType()
    {
        return FfSection;
    }
};
/*-----------------------------------------------------------------*/
struct FfTokenField : public FfTokenBase {
    virtual FfTokentype GetType()
    {
        return FfField;
    }
};
/*-----------------------------------------------------------------*/
struct FfTokenValue : public FfTokenBase {
    virtual FfTokentype GetType()
    {
        return FfValue;
    }
};
/*-----------------------------------------------------------------*/
bool IsTokenEqual(FfToken* _token, const char* _id);
/*-----------------------------------------------------------------*/
/*  Decodes a 2*N hex-digit ASCII string into an N-byte buffer. The
    destination must be pre-allocated with at least N+1 bytes (the
    helper writes a trailing NUL). Inverse of FfWriter::WriteFieldBinary.
    Extracted out of the legacy fileload.cpp so domain code can hex-
    decode without dragging in the main-window dependency chain. */
void FieldHexToBinary(void* _dest, const void* _source, int _length);
/*-----------------------------------------------------------------*/
enum FfOpenFlag { FfOpenRead = 1, FfOpenWrite = 2, FfOpenOverwrite = 4 };
/*-----------------------------------------------------------------*/
class FfFile
{
private:
    std::string filename;
    std::FILE* hfile;
    int openflags;

public:
    FfFile();
    virtual ~FfFile();

    bool Open(const char* _filename, int _of);
    void Close();
    void SetInvalid();
    bool IsOpen() const;
    bool IsWriteable() const;
    bool IsReadable() const;
    FfFile& operator=(const FfFile& _file);

    void SeekBegin();
    void SetEndOfFile();
    int Read();
    int Read(void* _buffer, int _length);
    int Write(const void* _buffer, int _length);
};
/*-----------------------------------------------------------------*/
class FfBase
{
protected:
    bool assigned;
    FfFile file;

public:
    FfBase();
    virtual ~FfBase();
    bool Assign(FfFile* _file);
    virtual bool Open(const char* _filename) = 0;
    void Close();
    bool IsOpen() const;
};
/*-----------------------------------------------------------------*/
class FfReader : public FfBase
{
public:
    virtual bool Open(const char* _filename);

    void SkipSection();
    void SkipField();
    FfToken* GetToken();
};
/*-----------------------------------------------------------------*/
class FfWriter : public FfBase
{
private:
    int indent;
    bool breakfields;

public:
    FfWriter();
    virtual bool Open(const char* _filename);

    void BreakFields(bool _bf = true)
    {
        breakfields = _bf;
    }

    void WriteSignature();
    void BeginSection(const char* _section, const char* _description = 0);
    void EndSection();
    void WriteField(const char* _field, const char* _data);
    void WriteFieldBinary(const char* _field, void* _data, int _length);
    void WriteFieldInt(const char* _field, int _data);
    void WriteFieldDouble(const char* _field, double _data);

protected:
    void WriteFieldChunk(const char* _data, int _chunk, int _length, bool _indent = false);
    void Indentation();
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
