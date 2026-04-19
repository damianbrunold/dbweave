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
#ifndef DBWEAVE_DOMAIN_PALETTE_H
#define DBWEAVE_DOMAIN_PALETTE_H
/*-----------------------------------------------------------------*/
#include "vcl_compat.h"
#include "colors_compat.h" /* COLORREF */
/*-----------------------------------------------------------------*/
// Forward-Deklarationen
class FfWriter;
class FfReader;
/*-----------------------------------------------------------------*/
#define MAX_PAL_ENTRY 236
/*-----------------------------------------------------------------*/
class Palette
{
public:
    COLORREF data[MAX_PAL_ENTRY];
    bool palette2;

public:
    Palette();
    virtual ~Palette();
    void InitPalette();
    void Save(FfWriter* _writer, bool _format37);
    void Load(FfReader* _reader);
    COLORREF GetColor(int _index);
    void SetColor(int _index, COLORREF _color);
    void SetPaletteType(bool _palette2);

private:
    void SetPaletteEntry(int _i, BYTE _red, BYTE _green, BYTE _blue);
    void FillDefaultPalette();
    void FillDefaultPalette2();
    void FillDefaultPalette3();
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
