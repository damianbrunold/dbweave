/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "datamodule.h"
#include "dbw3_base.h"
#include "palette.h"
#include "properties.h"

/*  Global pointer is defined here (one translation unit only). */
TData* Data = nullptr;

TData::TData()
	: MAXX1(DEFAULT_MAXX1)
	, MAXY1(DEFAULT_MAXY1)
	, MAXX2(DEFAULT_MAXX2)
	, MAXY2(DEFAULT_MAXY2)
	, properties(new FileProperties)
	, palette(new Palette)
	, color(DEFAULT_COLOR)
	, defcolorh(DEFAULT_COLORH)
	, defcolorv(DEFAULT_COLORV)
{
}

TData::~TData()
{
	delete palette;
	delete properties;
}

void __fastcall TData::ReloadLanguage()
{
	/*  Placeholder -- the legacy implementation reloads dialog
	    captions from the language.cpp string table. Ported later
	    once language.cpp lands. */
}
