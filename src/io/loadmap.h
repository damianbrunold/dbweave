/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  The BEGIN_LOAD_MAP / FIELD_MAP_* / SECTION_MAP / END_LOAD_MAP macros
    compose a small DSL that every Read() method in the legacy codebase
    uses to drive a state machine over FfReader tokens. The macros
    reference a local variable named `_reader` (convention: every Read
    method takes `FfReader* _reader`) and throw on malformed input.

    This header must come AFTER <cstring> / <cstdlib> (for strcpy /
    atoi / atof) in the translation unit that expands it.
*/

/*-----------------------------------------------------------------*/
#ifndef DBWEAVE_IO_LOADMAP_H
#define DBWEAVE_IO_LOADMAP_H
/*-----------------------------------------------------------------*/
#include "vcl_compat.h"
#include "assert_compat.h"
#include "fileformat.h"
#include <cstdlib>
#include <cstring>
/*-----------------------------------------------------------------*/
#define	BEGIN_LOAD_MAP \
	dbw3_assert (_reader); \
	FfToken* token = _reader->GetToken(); \
	while (token) {
/*-----------------------------------------------------------------*/
#define	BEGIN_FIELD_MAP \
		if (token->GetType()==FfField) {
/*-----------------------------------------------------------------*/
#define NO_FIELDS \
			_reader->SkipField();
/*-----------------------------------------------------------------*/
#define _FIELD_MAP(fieldname, function) \
			if (IsTokenEqual (token, fieldname)) function (_reader);
/*-----------------------------------------------------------------*/
#define FIELD_MAP(fieldname, function) \
			else _FIELD_MAP(fieldname, function)
/*-----------------------------------------------------------------*/
#define _FIELD_MAP_STRING(fieldname, variable) \
			if (IsTokenEqual (token, fieldname)) { \
				delete[] variable; \
				FfToken* t = _reader->GetToken(); \
				if (!t || t->GetType()!=FfValue) throw int(0); \
				FfTokenValue* val = (FfTokenValue*)t; \
				variable = new char[val->length+1]; \
				std::strcpy (variable, (char*)val->data); \
				delete t; \
			}
/*-----------------------------------------------------------------*/
#define FIELD_MAP_STRING(fieldname, variable) \
			else _FIELD_MAP_STRING(fieldname, variable)
/*-----------------------------------------------------------------*/
#define _FIELD_MAP_INT(fieldname, variable, cast) \
			if (IsTokenEqual (token, fieldname)) { \
				FfToken* t = _reader->GetToken(); \
				if (!t || t->GetType()!=FfValue) throw int(0); \
				FfTokenValue* val = (FfTokenValue*)t; \
				variable = (cast)std::atoi((char*)val->data); \
				delete t; \
			}
/*-----------------------------------------------------------------*/
#define FIELD_MAP_INT(fieldname, variable, cast) \
			else _FIELD_MAP_INT(fieldname, variable, cast)
/*-----------------------------------------------------------------*/
#define _FIELD_MAP_DOUBLE(fieldname, variable) \
			if (IsTokenEqual (token, fieldname)) { \
				FfToken* t = _reader->GetToken(); \
				if (!t || t->GetType()!=FfValue) throw int(0); \
				FfTokenValue* val = (FfTokenValue*)t; \
				variable = std::atof((char*)val->data); \
				delete t; \
			}
/*-----------------------------------------------------------------*/
#define FIELD_MAP_DOUBLE(fieldname, variable) \
			else _FIELD_MAP_DOUBLE(fieldname, variable)
/*-----------------------------------------------------------------*/
#define _FIELD_MAP_BINARY(fieldname, variable) \
			if (IsTokenEqual (token, fieldname)) { \
				FfToken* t = _reader->GetToken(); \
				if (!t || t->GetType()!=FfValue) throw int(0); \
				FfTokenValue* val = (FfTokenValue*)t; \
				delete[] variable; \
				variable = new char[val->length/2+1]; \
				FieldHexToBinary (variable, val->data, val->length); \
				delete t; \
			}
/*-----------------------------------------------------------------*/
#define FIELD_MAP_BINARY(fieldname, variable) \
			else _FIELD_MAP_BINARY(fieldname, variable)
/*-----------------------------------------------------------------*/
#define DEFAULT_FIELD \
			else \
				_reader->SkipField();
/*-----------------------------------------------------------------*/
#define	BEGIN_SECTION_MAP \
		} else if (token->GetType()==FfSection) {
/*-----------------------------------------------------------------*/
#define NO_SECTIONS \
			_reader->SkipSection();
/*-----------------------------------------------------------------*/
#define	_SECTION_MAP(section, function) \
			if (IsTokenEqual (token, section)) function (_reader);
/*-----------------------------------------------------------------*/
#define	SECTION_MAP(section, function) \
			else if (IsTokenEqual (token, section)) function (_reader);
/*-----------------------------------------------------------------*/
#define	DEFAULT_SECTION \
			else \
				_reader->SkipSection();
/*-----------------------------------------------------------------*/
#define BEGIN_DEFAULT_MAP \
		} else if (token->GetType()==FfEndSection) { \
			delete token; \
			break; \
		} else { \
			delete token; \
			throw int(0); \
		}
/*-----------------------------------------------------------------*/
#define	END_LOAD_MAP \
		delete token; \
		token = _reader->GetToken(); \
	}
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
