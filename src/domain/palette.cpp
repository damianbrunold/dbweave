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
#include <cstdint>
#include <cstdlib>  /* rand */
#include <cstring>  /* memcpy */
/*-----------------------------------------------------------------*/
#include "colors.h"
#include "palette.h"
#include "fileformat.h"
#include "loadmap.h"
/*-----------------------------------------------------------------*/
/*  Win32 palette structs used by the pre-format-3.7 on-disk layout.
    Declared here so backwards-compatible loading of old .dbw files
    continues to work without dragging in <windows.h>. The exact byte
    layout matters: palVersion/palNumEntries are little-endian uint16,
    followed by packed PALETTEENTRY records (4 bytes each, no padding).
    The legacy code on Win32 produced these via LOGPALETTE from the
    GDI headers; the layout is identical.                            */
#pragma pack(push, 1)
struct PALETTEENTRY_legacy {
	BYTE peRed;
	BYTE peGreen;
	BYTE peBlue;
	BYTE peFlags;
};
struct LOGPALETTE_legacy {
	std::uint16_t       palVersion;
	std::uint16_t       palNumEntries;
	PALETTEENTRY_legacy palPalEntry[1];
};
#pragma pack(pop)
/*-----------------------------------------------------------------*/
 Palette::Palette()
{
	palette2 = false;
	InitPalette();
}
/*-----------------------------------------------------------------*/
 Palette::~Palette()
{
	//
}
/*-----------------------------------------------------------------*/
void Palette::SetPaletteType (bool _palette2)
{
	palette2 = _palette2;
	InitPalette();
}
/*-----------------------------------------------------------------*/
void Palette::InitPalette()
{
	if (!palette2) FillDefaultPalette ();
	else FillDefaultPalette2 ();
}
/*-----------------------------------------------------------------*/
void Palette::SetPaletteEntry (int _i, BYTE _red, BYTE _green, BYTE _blue)
{
	data[_i] = RGB(_red, _green, _blue);
}
/*-----------------------------------------------------------------*/
void Palette::FillDefaultPalette()
{
	int i = 0;
	SetPaletteEntry (i++, 0xff, 0x00, 0x00);
	SetPaletteEntry (i++, 0xea, 0x00, 0x00);
	SetPaletteEntry (i++, 0xd5, 0x00, 0x00);
	SetPaletteEntry (i++, 0xc0, 0x00, 0x00);
	SetPaletteEntry (i++, 0xab, 0x00, 0x00);
	SetPaletteEntry (i++, 0x96, 0x00, 0x00);
	SetPaletteEntry (i++, 0x80, 0x00, 0x00);
	SetPaletteEntry (i++, 0xf6, 0xff, 0x00);
	SetPaletteEntry (i++, 0xe3, 0xea, 0x00);
	SetPaletteEntry (i++, 0xcf, 0xd5, 0x00);
	SetPaletteEntry (i++, 0xbb, 0xc0, 0x00);
	SetPaletteEntry (i++, 0xa8, 0xab, 0x00);
	SetPaletteEntry (i++, 0x94, 0x96, 0x00);
	SetPaletteEntry (i++, 0x80, 0x80, 0x00);
	SetPaletteEntry (i++, 0x00, 0xff, 0x00);
	SetPaletteEntry (i++, 0x00, 0xde, 0x00);
	SetPaletteEntry (i++, 0x00, 0xbc, 0x00);
	SetPaletteEntry (i++, 0x00, 0x9a, 0x00);
	SetPaletteEntry (i++, 0x00, 0x79, 0x00);
	SetPaletteEntry (i++, 0x00, 0x57, 0x00);
	SetPaletteEntry (i++, 0x00, 0x35, 0x00);
	SetPaletteEntry (i++, 0x00, 0x00, 0xff);
	SetPaletteEntry (i++, 0x00, 0x00, 0xde);
	SetPaletteEntry (i++, 0x00, 0x00, 0xbc);
	SetPaletteEntry (i++, 0x00, 0x00, 0x99);
	SetPaletteEntry (i++, 0x00, 0x00, 0x78);
	SetPaletteEntry (i++, 0x00, 0x00, 0x55);
	SetPaletteEntry (i++, 0x00, 0x00, 0x33);
	SetPaletteEntry (i++, 0xff, 0x00, 0xff);
	SetPaletteEntry (i++, 0xe0, 0x00, 0xe0);
	SetPaletteEntry (i++, 0xc0, 0x00, 0xc0);
	SetPaletteEntry (i++, 0xa0, 0x00, 0xa0);
	SetPaletteEntry (i++, 0x80, 0x00, 0x80);
	SetPaletteEntry (i++, 0x60, 0x00, 0x60);
	SetPaletteEntry (i++, 0x40, 0x00, 0x40);
	SetPaletteEntry (i++, 0x00, 0xff, 0xf6);
	SetPaletteEntry (i++, 0x00, 0xde, 0xd6);
	SetPaletteEntry (i++, 0x00, 0xbc, 0xb5);
	SetPaletteEntry (i++, 0x00, 0x99, 0x93);
	SetPaletteEntry (i++, 0x00, 0x78, 0x73);
	SetPaletteEntry (i++, 0x00, 0x55, 0x51);
	SetPaletteEntry (i++, 0x00, 0x33, 0x30);
	SetPaletteEntry (i++, 0xff, 0x7b, 0x00);
	SetPaletteEntry (i++, 0xdf, 0x6c, 0x00);
	SetPaletteEntry (i++, 0xbf, 0x5c, 0x00);
	SetPaletteEntry (i++, 0x9e, 0x4c, 0x00);
	SetPaletteEntry (i++, 0x7e, 0x3c, 0x00);
	SetPaletteEntry (i++, 0x5d, 0x2c, 0x00);
	SetPaletteEntry (i++, 0x3c, 0x1c, 0x00);
	SetPaletteEntry (i++, 0x00, 0x8c, 0xff);
	SetPaletteEntry (i++, 0x00, 0x7b, 0xe0);
	SetPaletteEntry (i++, 0x00, 0x6a, 0xc0);
	SetPaletteEntry (i++, 0x00, 0x58, 0xa0);
	SetPaletteEntry (i++, 0x00, 0x47, 0x81);
	SetPaletteEntry (i++, 0x00, 0x35, 0x61);
	SetPaletteEntry (i++, 0x00, 0x23, 0x41);
	SetPaletteEntry (i++, 0x83, 0xff, 0x00);
	SetPaletteEntry (i++, 0x73, 0xe0, 0x00);
	SetPaletteEntry (i++, 0x63, 0xc0, 0x00);
	SetPaletteEntry (i++, 0x52, 0xa0, 0x00);
	SetPaletteEntry (i++, 0x42, 0x81, 0x00);
	SetPaletteEntry (i++, 0x32, 0x61, 0x00);
	SetPaletteEntry (i++, 0x21, 0x41, 0x00);
	SetPaletteEntry (i++, 0x00, 0xff, 0x66);
	SetPaletteEntry (i++, 0x00, 0xe1, 0x5a);
	SetPaletteEntry (i++, 0x00, 0xc2, 0x4e);
	SetPaletteEntry (i++, 0x00, 0xa3, 0x41);
	SetPaletteEntry (i++, 0x00, 0x84, 0x35);
	SetPaletteEntry (i++, 0x00, 0x65, 0x29);
	SetPaletteEntry (i++, 0x00, 0x46, 0x1c);
	SetPaletteEntry (i++, 0xff, 0xff, 0xff);
	SetPaletteEntry (i++, 0xf6, 0xf6, 0xf6);
	SetPaletteEntry (i++, 0xed, 0xed, 0xed);
	SetPaletteEntry (i++, 0xe3, 0xe3, 0xe3);
	SetPaletteEntry (i++, 0xda, 0xda, 0xda);
	SetPaletteEntry (i++, 0xd0, 0xd0, 0xd0);
	SetPaletteEntry (i++, 0xc7, 0xc7, 0xc7);
	SetPaletteEntry (i++, 0xbd, 0xbd, 0xbd);
	SetPaletteEntry (i++, 0xb4, 0xb4, 0xb4);
	SetPaletteEntry (i++, 0xab, 0xab, 0xab);
	SetPaletteEntry (i++, 0xa1, 0xa1, 0xa1);
	SetPaletteEntry (i++, 0x98, 0x98, 0x98);
	SetPaletteEntry (i++, 0x8e, 0x8e, 0x8e);
	SetPaletteEntry (i++, 0x85, 0x85, 0x85);
	SetPaletteEntry (i++, 0x7b, 0x7b, 0x7b);
	SetPaletteEntry (i++, 0x72, 0x72, 0x72);
	SetPaletteEntry (i++, 0x68, 0x68, 0x68);
	SetPaletteEntry (i++, 0x5f, 0x5f, 0x5f);
	SetPaletteEntry (i++, 0x56, 0x56, 0x56);
	SetPaletteEntry (i++, 0x4c, 0x4c, 0x4c);
	SetPaletteEntry (i++, 0x43, 0x43, 0x43);
	SetPaletteEntry (i++, 0x39, 0x39, 0x39);
	SetPaletteEntry (i++, 0x30, 0x30, 0x30);
	SetPaletteEntry (i++, 0x26, 0x26, 0x26);
	SetPaletteEntry (i++, 0x1d, 0x1d, 0x1d);
	SetPaletteEntry (i++, 0x13, 0x13, 0x13);
	SetPaletteEntry (i++, 0x0a, 0x0a, 0x0a);
	SetPaletteEntry (i++, 0x00, 0x00, 0x00);
	SetPaletteEntry (i++, 0x00, 0x00, 0x00);
	SetPaletteEntry (i++, 0xff, 0x7c, 0x7c);
	SetPaletteEntry (i++, 0xea, 0x72, 0x72);
	SetPaletteEntry (i++, 0xd5, 0x68, 0x68);
	SetPaletteEntry (i++, 0xc0, 0x5e, 0x5e);
	SetPaletteEntry (i++, 0xab, 0x54, 0x54);
	SetPaletteEntry (i++, 0x96, 0x4a, 0x4a);
	SetPaletteEntry (i++, 0x80, 0x3f, 0x3f);
	SetPaletteEntry (i++, 0xfa, 0xff, 0x7e);
	SetPaletteEntry (i++, 0xe6, 0xea, 0x74);
	SetPaletteEntry (i++, 0xd2, 0xd5, 0x6a);
	SetPaletteEntry (i++, 0xbd, 0xc0, 0x5f);
	SetPaletteEntry (i++, 0xa9, 0xab, 0x55);
	SetPaletteEntry (i++, 0x95, 0x96, 0x4a);
	SetPaletteEntry (i++, 0x80, 0x80, 0x3f);
	SetPaletteEntry (i++, 0x7e, 0xff, 0x7e);
	SetPaletteEntry (i++, 0x6e, 0xde, 0x6e);
	SetPaletteEntry (i++, 0x5d, 0xbc, 0x5d);
	SetPaletteEntry (i++, 0x4c, 0x9a, 0x4c);
	SetPaletteEntry (i++, 0x3c, 0x79, 0x3c);
	SetPaletteEntry (i++, 0x2b, 0x57, 0x2b);
	SetPaletteEntry (i++, 0x1a, 0x35, 0x1a);
	SetPaletteEntry (i++, 0x7e, 0x7e, 0xff);
	SetPaletteEntry (i++, 0x6e, 0x6e, 0xde);
	SetPaletteEntry (i++, 0x5d, 0x5d, 0xbc);
	SetPaletteEntry (i++, 0x4c, 0x4c, 0x99);
	SetPaletteEntry (i++, 0x3b, 0x3b, 0x78);
	SetPaletteEntry (i++, 0x2a, 0x2a, 0x55);
	SetPaletteEntry (i++, 0x19, 0x19, 0x33);
	SetPaletteEntry (i++, 0xff, 0x7e, 0xff);
	SetPaletteEntry (i++, 0xe0, 0x6f, 0xe0);
	SetPaletteEntry (i++, 0xc0, 0x5f, 0xc0);
	SetPaletteEntry (i++, 0xa0, 0x4f, 0xa0);
	SetPaletteEntry (i++, 0x80, 0x3f, 0x80);
	SetPaletteEntry (i++, 0x60, 0x2f, 0x60);
	SetPaletteEntry (i++, 0x40, 0x1f, 0x40);
	SetPaletteEntry (i++, 0x7e, 0xff, 0xf8);
	SetPaletteEntry (i++, 0x6e, 0xde, 0xd7);
	SetPaletteEntry (i++, 0x5d, 0xbc, 0xb6);
	SetPaletteEntry (i++, 0x4c, 0x99, 0x95);
	SetPaletteEntry (i++, 0x3b, 0x78, 0x74);
	SetPaletteEntry (i++, 0x2a, 0x55, 0x53);
	SetPaletteEntry (i++, 0x19, 0x33, 0x31);
	SetPaletteEntry (i++, 0xff, 0xba, 0x7e);
	SetPaletteEntry (i++, 0xdf, 0xa3, 0x6e);
	SetPaletteEntry (i++, 0xbf, 0x8b, 0x5e);
	SetPaletteEntry (i++, 0x9e, 0x73, 0x4e);
	SetPaletteEntry (i++, 0x7e, 0x5b, 0x3e);
	SetPaletteEntry (i++, 0x5d, 0x43, 0x2e);
	SetPaletteEntry (i++, 0x3c, 0x2b, 0x1d);
	SetPaletteEntry (i++, 0x7e, 0xc5, 0xff);
	SetPaletteEntry (i++, 0x6f, 0xad, 0xe0);
	SetPaletteEntry (i++, 0x5f, 0x95, 0xc0);
	SetPaletteEntry (i++, 0x4f, 0x7c, 0xa0);
	SetPaletteEntry (i++, 0x40, 0x64, 0x81);
	SetPaletteEntry (i++, 0x30, 0x4b, 0x61);
	SetPaletteEntry (i++, 0x20, 0x32, 0x41);
	SetPaletteEntry (i++, 0xc1, 0xff, 0x7e);
	SetPaletteEntry (i++, 0xaa, 0xe0, 0x6f);
	SetPaletteEntry (i++, 0x92, 0xc0, 0x5f);
	SetPaletteEntry (i++, 0x79, 0xa0, 0x4f);
	SetPaletteEntry (i++, 0x62, 0x81, 0x40);
	SetPaletteEntry (i++, 0x49, 0x61, 0x30);
	SetPaletteEntry (i++, 0x31, 0x41, 0x20);
	SetPaletteEntry (i++, 0x7e, 0xff, 0xb2);
	SetPaletteEntry (i++, 0x6f, 0xe1, 0x9d);
	SetPaletteEntry (i++, 0x60, 0xc2, 0x87);
	SetPaletteEntry (i++, 0x50, 0xa3, 0x71);
	SetPaletteEntry (i++, 0x41, 0x84, 0x5c);
	SetPaletteEntry (i++, 0x32, 0x65, 0x46);
	SetPaletteEntry (i++, 0x22, 0x46, 0x30);
	SetPaletteEntry (i++, 0xff, 0x00, 0x04);
	SetPaletteEntry (i++, 0xff, 0x00, 0x38);
	SetPaletteEntry (i++, 0xff, 0x00, 0x6d);
	SetPaletteEntry (i++, 0xff, 0x00, 0xa1);
	SetPaletteEntry (i++, 0xff, 0x00, 0xd6);
	SetPaletteEntry (i++, 0xf2, 0x00, 0xff);
	SetPaletteEntry (i++, 0xbe, 0x00, 0xff);
	SetPaletteEntry (i++, 0x89, 0x00, 0xff);
	SetPaletteEntry (i++, 0x55, 0x00, 0xff);
	SetPaletteEntry (i++, 0x20, 0x00, 0xff);
	SetPaletteEntry (i++, 0x00, 0x14, 0xff);
	SetPaletteEntry (i++, 0x00, 0x48, 0xff);
	SetPaletteEntry (i++, 0x00, 0x7d, 0xff);
	SetPaletteEntry (i++, 0x00, 0xb2, 0xff);
	SetPaletteEntry (i++, 0x00, 0xe6, 0xff);
	SetPaletteEntry (i++, 0x00, 0xff, 0xe2);
	SetPaletteEntry (i++, 0x00, 0xff, 0xae);
	SetPaletteEntry (i++, 0x00, 0xff, 0x79);
	SetPaletteEntry (i++, 0x00, 0xff, 0x44);
	SetPaletteEntry (i++, 0x00, 0xff, 0x10);
	SetPaletteEntry (i++, 0x24, 0xff, 0x00);
	SetPaletteEntry (i++, 0x59, 0xff, 0x00);
	SetPaletteEntry (i++, 0x8d, 0xff, 0x00);
	SetPaletteEntry (i++, 0xc2, 0xff, 0x00);
	SetPaletteEntry (i++, 0xf6, 0xff, 0x00);
	SetPaletteEntry (i++, 0xff, 0xd2, 0x00);
	SetPaletteEntry (i++, 0xff, 0x9d, 0x00);
	SetPaletteEntry (i++, 0xff, 0x69, 0x00);
	SetPaletteEntry (i++, 0xff, 0x34, 0x00);
	SetPaletteEntry (i++, 0xff, 0x00, 0x00);
	SetPaletteEntry (i++, 0x80, 0x00, 0x04);
	SetPaletteEntry (i++, 0x80, 0x00, 0x1e);
	SetPaletteEntry (i++, 0x80, 0x00, 0x38);
	SetPaletteEntry (i++, 0x80, 0x00, 0x53);
	SetPaletteEntry (i++, 0x80, 0x00, 0x6d);
	SetPaletteEntry (i++, 0x78, 0x00, 0x80);
	SetPaletteEntry (i++, 0x5d, 0x00, 0x80);
	SetPaletteEntry (i++, 0x43, 0x00, 0x80);
	SetPaletteEntry (i++, 0x29, 0x00, 0x80);
	SetPaletteEntry (i++, 0x0e, 0x00, 0x80);
	SetPaletteEntry (i++, 0x00, 0x0b, 0x80);
	SetPaletteEntry (i++, 0x00, 0x25, 0x80);
	SetPaletteEntry (i++, 0x00, 0x40, 0x80);
	SetPaletteEntry (i++, 0x00, 0x5a, 0x80);
	SetPaletteEntry (i++, 0x00, 0x74, 0x80);
	SetPaletteEntry (i++, 0x00, 0x80, 0x70);
	SetPaletteEntry (i++, 0x00, 0x80, 0x56);
	SetPaletteEntry (i++, 0x00, 0x80, 0x3c);
	SetPaletteEntry (i++, 0x00, 0x80, 0x21);
	SetPaletteEntry (i++, 0x00, 0x80, 0x07);
	SetPaletteEntry (i++, 0x12, 0x80, 0x00);
	SetPaletteEntry (i++, 0x2d, 0x80, 0x00);
	SetPaletteEntry (i++, 0x47, 0x80, 0x00);
	SetPaletteEntry (i++, 0x61, 0x80, 0x00);
	SetPaletteEntry (i++, 0x7c, 0x80, 0x00);
	SetPaletteEntry (i++, 0x80, 0x69, 0x00);
	SetPaletteEntry (i++, 0x80, 0x4f, 0x00);
	SetPaletteEntry (i++, 0x80, 0x34, 0x00);
	SetPaletteEntry (i++, 0x80, 0x1a, 0x00);
	SetPaletteEntry (i++, 0x80, 0x00, 0x00);
	SetPaletteEntry (i++, 0x80, 0x3f, 0x3f);
	SetPaletteEntry (i++, 0x80, 0x80, 0x3f);
	SetPaletteEntry (i++, 0x3f, 0x80, 0x3f);
	SetPaletteEntry (i++, 0x3f, 0x42, 0x80);
	SetPaletteEntry (i++, 0xff, 0xff, 0xff);
	SetPaletteEntry (i++, 0x05, 0x4b, 0x69);
	SetPaletteEntry (i++, 0xdd, 0xdc, 0xdc);
	SetPaletteEntry (i++, 0x00, 0x00, 0x00);
}
/*-----------------------------------------------------------------*/
void Palette::FillDefaultPalette2()
{
	int i, red, green, blue;

	int div = 9;
	float hstep = 360.0/(236/div);

	int idx = 0;
	// 1
	float h = hstep;
	for (i=0; i<236/div; i++) {
		HSV2RGB (h, 1.0, 1.0, red, green, blue);
		SetPaletteEntry (idx, (BYTE)red, (BYTE)green, (BYTE)blue);
		++idx;
		h += hstep;
	}

	// 2
	h = hstep;
	for (i=0; i<236/div; i++) {
		HSV2RGB (h, 0.8, 1.0, red, green, blue);
		SetPaletteEntry (idx, (BYTE)red, (BYTE)green, (BYTE)blue);
		++idx;
		h += hstep;
	}

	// 3
	h = hstep;
	for (i=0; i<236/div; i++) {
		HSV2RGB (h, 0.5, 1.0, red, green, blue);
		SetPaletteEntry (idx, (BYTE)red, (BYTE)green, (BYTE)blue);
		++idx;
		h += hstep;
	}

	// 4
	h = hstep;
	for (i=0; i<236/div; i++) {
		HSV2RGB (h, 0.3, 1.0, red, green, blue);
		SetPaletteEntry (idx, (BYTE)red, (BYTE)green, (BYTE)blue);
		++idx;
		h += hstep;
	}

	// 5
	h = hstep;
	for (i=0; i<236/div; i++) {
		HSV2RGB (h, 1.0, 0.8, red, green, blue);
		SetPaletteEntry (idx, (BYTE)red, (BYTE)green, (BYTE)blue);
		++idx;
		h += hstep;
	}

	// 6
	h = hstep;
	for (i=0; i<236/div; i++) {
		HSV2RGB (h, 1.0, 0.5, red, green, blue);
		SetPaletteEntry (idx, (BYTE)red, (BYTE)green, (BYTE)blue);
		++idx;
		h += hstep;
	}

	// 7
	h = hstep;
	for (i=0; i<236/div; i++) {
		HSV2RGB (h, 1.0, 0.3, red, green, blue);
		SetPaletteEntry (idx, (BYTE)red, (BYTE)green, (BYTE)blue);
		++idx;
		h += hstep;
	}

	// 8
	h = hstep;
	for (i=0; i<236/div; i++) {
		HSV2RGB (h, 0.5, 0.5, red, green, blue);
		SetPaletteEntry (idx, (BYTE)red, (BYTE)green, (BYTE)blue);
		++idx;
		h += hstep;
	}

	// 9
	for (i=0; i<236/div; i++) {
		HSV2RGB (0.0, 0.01, 1.0/(236/div)*i, red, green, blue);
		SetPaletteEntry (idx, (BYTE)red, (BYTE)green, (BYTE)blue);
		++idx;
	}

	while (idx<MAX_PAL_ENTRY) {
		SetPaletteEntry (idx, (BYTE)(std::rand()%256), (BYTE)(std::rand()%256), (BYTE)(std::rand()%256));
		++ idx;
	}

	SetPaletteEntry (MAX_PAL_ENTRY-3, 255, 255, 255);
	SetPaletteEntry (MAX_PAL_ENTRY-2, 5, 75, 105);
	SetPaletteEntry (MAX_PAL_ENTRY-1, 221, 220, 220);
}
/*-----------------------------------------------------------------*/
void Palette::FillDefaultPalette3()
{
	int i, j, k, red, green, blue;

	float hstep = 360.0/(14+2);
	float sstep = 1.0/(4+2);
	float vstep = 1.0/(4+2);

	float h = hstep;
	float s = sstep;
	float v = vstep;

	int idx = 0;
	for (i=0; i<14; i++) {
		for (j=0; j<4; j++) {
			for (k=0; k<4; k++) {
				HSV2RGB (h, s, v, red, green, blue);
				SetPaletteEntry (idx, (BYTE)red, (BYTE)green, (BYTE)blue);
				++idx;
				v += vstep;
			}
			s += sstep;
		}
		h += hstep;
	}
}
/*-----------------------------------------------------------------*/
void Palette::Save (FfWriter* _writer, bool _format37)
{
	if (!_writer->IsOpen()) return;
	if (_format37) {
		_writer->BeginSection ("palette");
			_writer->WriteFieldInt ("size", MAX_PAL_ENTRY);
			_writer->WriteFieldBinary ("data2", data, sizeof(COLORREF)*MAX_PAL_ENTRY);
		_writer->EndSection();
	} else {
		// Altes Format zuerst aufbauen
		try {
			/*  Allocate and write exactly as many bytes as the Win32
			    original: sizeof(LOGPALETTE) counts one entry, then one
			    more entry per palette slot. This wastes 4 bytes but
			    preserves the byte-identical on-disk layout. */
			const std::size_t bytes = sizeof(LOGPALETTE_legacy) + sizeof(PALETTEENTRY_legacy) * MAX_PAL_ENTRY;
			LOGPALETTE_legacy* pal = (LOGPALETTE_legacy*) new char[bytes];
			pal->palVersion = 0x300;
			pal->palNumEntries = MAX_PAL_ENTRY;
			for (int i=0; i<MAX_PAL_ENTRY; i++) {
				pal->palPalEntry[i].peRed   = GetRValue(data[i]);
				pal->palPalEntry[i].peGreen = GetGValue(data[i]);
				pal->palPalEntry[i].peBlue  = GetBValue(data[i]);
				pal->palPalEntry[i].peFlags = 0;
			}
			_writer->BeginSection ("palette");
				_writer->WriteFieldInt ("size", MAX_PAL_ENTRY);
				_writer->WriteFieldBinary ("data", pal, (int)bytes);
			_writer->EndSection();
			delete[] (char*)pal;
		} catch(...) {
		}
	}
}
/*-----------------------------------------------------------------*/
void Palette::Load (FfReader* _reader)
{
	int newsize = MAX_PAL_ENTRY;
	(void)newsize;
	char* newdata = 0;
	char* olddata = 0;

	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("size", newsize, int)
			FIELD_MAP_BINARY ("data2", newdata)
			FIELD_MAP_BINARY ("data", olddata)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP

	(void)newsize;

	if (newdata==0 && olddata!=0) {
		// Altes Palettenformat konvertieren
		for (int i=0; i<MAX_PAL_ENTRY; i++) {
			LOGPALETTE_legacy* pLogPalette = (LOGPALETTE_legacy*)olddata;
			int red = pLogPalette->palPalEntry[i].peRed;
			int green = pLogPalette->palPalEntry[i].peGreen;
			int blue = pLogPalette->palPalEntry[i].peBlue;
			data[i] = RGB(red,green,blue);
		}
	} else if (newdata!=0) {
		memcpy (data, newdata, sizeof(COLORREF)*MAX_PAL_ENTRY);
	}

	delete[] newdata;
	delete[] olddata;
}
/*-----------------------------------------------------------------*/
COLORREF Palette::GetColor (int _index)
{
	return data[_index];
}
/*-----------------------------------------------------------------*/
void Palette::SetColor (int _index, COLORREF _color)
{
	data[_index] = _color;
}
/*-----------------------------------------------------------------*/
