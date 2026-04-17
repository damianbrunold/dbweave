/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  TData is the application-wide configuration / "data module" holding
    grid dimensions, the Palette, and the default colour indices. The
    legacy code auto-instantiated this as the VCL TData form and
    reached for it through the `Data` global pointer; we keep that
    pattern because every porteded .cpp file references Data-> and
    restructuring those call sites is a non-port change.

    The VCL TData also housed a grab-bag of dialog components (open,
    save, print, etc.). Those are replaced by on-demand QDialog calls
    and are not mirrored here.
*/

#ifndef DBWEAVE_UI_DATAMODULE_H
#define DBWEAVE_UI_DATAMODULE_H

#include "vcl_compat.h"

class FileProperties;  // forward decl -- ported in a later slice
class Palette;

class TData
{
public:
	// Ausmasse der Felder. Marked volatile in legacy; we keep the
	// qualifier although it has no semantic bite here.
	volatile int MAXX1, MAXY1;
	volatile int MAXX2, MAXY2;

	FileProperties* properties;
	Palette*        palette;
	unsigned char   color;
	unsigned char   defcolorh;
	unsigned char   defcolorv;

public:
	TData();
	virtual ~TData();
	void __fastcall ReloadLanguage();
};

/*  Global pointer, populated by main(). Matches the VCL `extern PACKAGE
    TData *Data;` declaration so legacy call sites porting over compile
    unchanged. */
extern TData* Data;

#endif
