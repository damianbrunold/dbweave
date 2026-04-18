/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Minimal "@dbw3:file" loader -- the first slice of a legacy
    FhLoader port. Handles:
        - signature
        - data/size        (MAXX1/MAXX2/MAXY1/MAXY2)
        - data/fields      (einzug, aufknuepfung, trittfolge,
                            schussfarben, kettfarben, blatteinzug,
                            trittfolge.isempty)
        - data/palette     (via Palette::Load)
        - properties       (reads Author/Organization/Remarks into a
                            FileProperties owned by TData)
    Skips:
        - version          (tolerated, not stored)
        - data/webstuhl    (klammer brace state -- Phase 11 loom)
        - data/blockmuster (not ported yet)
        - data/bereichmuster
        - data/hilfslinien
        - data/fixeinzug
        - view             (UI toggle persistence -- later)
        - printsettings    (printer layout -- Phase 8)

    Skipped sections are consumed (so the parser advances past them)
    but their contents are discarded. */

#ifndef DBWEAVE_UI_FILELOAD_H
#define DBWEAVE_UI_FILELOAD_H

#include "loadoptions.h"

class TDBWFRM;
class FfReader;

class FhLoader
{
private:
	TDBWFRM*  mainfrm;
	LOADPARTS loadparts;

public:
	explicit FhLoader (TDBWFRM* _mainfrm);
	bool Load (LOADSTAT& _stat, LOADPARTS _loadparts = LOADALL);

private:
	bool Need (LOADOPTION _part) const;

	bool LoadSignatur (FfReader* _reader);
	void LoadVersion  (FfReader* _reader);
	void LoadData     (FfReader* _reader);
	void LoadDataSize (FfReader* _reader);
	void LoadDataFields (FfReader* _reader);
	void LoadDataEinzug (FfReader* _reader);
	void LoadDataAufknuepfung (FfReader* _reader);
	void LoadDataTrittfolge (FfReader* _reader);
	void LoadDataTrittfolgeTrittfolge (FfReader* _reader);
	void LoadDataTrittfolgeIsEmpty (FfReader* _reader);
	void LoadDataSchussfarben (FfReader* _reader);
	void LoadDataKettfarben (FfReader* _reader);
	void LoadDataBlatteinzug (FfReader* _reader);
	void LoadDataPalette (FfReader* _reader);
	void LoadDataHilfslinien (FfReader* _reader);
};

#endif
