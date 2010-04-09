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
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "properties.h"
#include "fileformat.h"
#include "loadmap.h"
/*-----------------------------------------------------------------*/
__fastcall FileProperties::FileProperties()
{
    author = organization = remarks = 0;
}
/*-----------------------------------------------------------------*/
__fastcall FileProperties::~FileProperties()
{
    delete[] author;
    delete[] organization;
    delete[] remarks;
}
/*-----------------------------------------------------------------*/
void __fastcall FileProperties::SetAuthor (const char* _author)
{
    delete[] author;
    author = new char [strlen(_author)+1];
    strcpy (author, _author);
}
/*-----------------------------------------------------------------*/
void __fastcall FileProperties::SetOrganization (const char* _organization)
{
    delete[] organization;
    organization = new char [strlen(_organization)+1];
    strcpy (organization, _organization);
}
/*-----------------------------------------------------------------*/
void __fastcall FileProperties::SetRemarks (const char* _remarks)
{
    delete[] remarks;
    remarks = new char [strlen(_remarks)+1];
    strcpy (remarks, _remarks);
}
/*-----------------------------------------------------------------*/
const char* __fastcall FileProperties::Author() const
{
    return author ? author : "";
}
/*-----------------------------------------------------------------*/
const char* __fastcall FileProperties::Organization() const
{
    return organization ? organization : "";
}
/*-----------------------------------------------------------------*/
const char* __fastcall FileProperties::Remarks() const
{
    return remarks ? remarks : "";
}
/*-----------------------------------------------------------------*/
void __fastcall FileProperties::RemoveNL (char* _data)
{
    if (!_data) return;
    char* p1 = _data;
    char* p2 = _data;
    while (*p1!='\0') {
        if (*p1=='\n') {
            *p2++ = '\\';
        } else if (*p1=='\r') {
            // überlesen
        } else {
            *p2++ = *p1;
        }
        p1++;
    }
    *p2 = '\0';
}
/*-----------------------------------------------------------------*/
void __fastcall FileProperties::InsertNL (char* _data)
{
    if (!_data) return;
    try {
        char* dest = new char[strlen(_data)+255];
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
        lstrcpy (_data, dest);
        delete[] dest;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall FileProperties::Save (FfWriter* _writer)
{
    dbw3_assert (_writer);
    try {
        char* bemerkungen = new char[strlen(Remarks())+255];
        lstrcpy (bemerkungen, Remarks());
        RemoveNL(bemerkungen);
        if (!_writer->IsOpen()) return;
        _writer->BeginSection ("properties", "Dateieigenschaften");
        _writer->WriteField ("author", Author());
        _writer->WriteField ("organization", Organization());
        _writer->WriteField ("remarks", bemerkungen);
        _writer->EndSection();
        delete[] bemerkungen;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
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
/*-----------------------------------------------------------------*/
void __fastcall FileProperties::Init()
{
    SetAuthor ("");
    SetOrganization ("");
    SetRemarks ("");
}
/*-----------------------------------------------------------------*/

