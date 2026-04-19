/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "properties.h"
#include "assert_compat.h"
#include "fileformat.h"
#include "loadmap.h"
#include <cstring>

__fastcall FileProperties::FileProperties()
{
	author = organization = remarks = 0;
}

__fastcall FileProperties::~FileProperties()
{
	delete[] author;
	delete[] organization;
	delete[] remarks;
}

void __fastcall FileProperties::SetAuthor (const char* _author)
{
	delete[] author;
	author = new char [std::strlen(_author)+1];
	std::strcpy (author, _author);
}

void __fastcall FileProperties::SetOrganization (const char* _organization)
{
	delete[] organization;
	organization = new char [std::strlen(_organization)+1];
	std::strcpy (organization, _organization);
}

void __fastcall FileProperties::SetRemarks (const char* _remarks)
{
	delete[] remarks;
	remarks = new char [std::strlen(_remarks)+1];
	std::strcpy (remarks, _remarks);
}

const char* __fastcall FileProperties::Author       () const { return author       ? author       : ""; }
const char* __fastcall FileProperties::Organization () const { return organization ? organization : ""; }
const char* __fastcall FileProperties::Remarks      () const { return remarks      ? remarks      : ""; }

void __fastcall FileProperties::RemoveNL (char* _data)
{
	if (!_data) return;
	char* p1 = _data;
	char* p2 = _data;
	while (*p1!='\0') {
		if (*p1=='\n') {
			*p2++ = '\\';
		} else if (*p1=='\r') {
			/*  skip */
		} else {
			*p2++ = *p1;
		}
		p1++;
	}
	*p2 = '\0';
}

void __fastcall FileProperties::InsertNL (char* _data)
{
	if (!_data) return;
	try {
		char* dest = new char[std::strlen(_data)+255];
		char* p1 = _data;
		char* p2 = dest;
		while (*p1!='\0') {
			if (*p1=='\\') {
				*p2++ = '\r';
				*p2++ = '\n';
			} else {
				*p2++ = *p1;
			}
			p1++;
		}
		*p2 = '\0';
		std::strcpy (_data, dest);
		delete[] dest;
	} catch (...) {
	}
}

void __fastcall FileProperties::Save (FfWriter* _writer)
{
	dbw3_assert (_writer);
	try {
		char* bemerkungen = new char[std::strlen(Remarks())+255];
		std::strcpy (bemerkungen, Remarks());
		RemoveNL(bemerkungen);
		if (!_writer->IsOpen()) { delete[] bemerkungen; return; }
		_writer->BeginSection ("properties", "Dateieigenschaften");
		_writer->WriteField ("author",       Author());
		_writer->WriteField ("organization", Organization());
		_writer->WriteField ("remarks",      bemerkungen);
		_writer->EndSection();
		delete[] bemerkungen;
	} catch (...) {
	}
}

void __fastcall FileProperties::Load (FfReader* _reader)
{
	char* bemerkungen = NULL;
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_STRING("author", author)
			FIELD_MAP_STRING ("organization", organization)
			FIELD_MAP_STRING ("remarks", bemerkungen)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
	if (bemerkungen) {
		InsertNL(bemerkungen);
		remarks = bemerkungen;
	} else {
		remarks = NULL;
	}
}

void __fastcall FileProperties::Init ()
{
	SetAuthor       ("");
	SetOrganization ("");
	SetRemarks      ("");
}
