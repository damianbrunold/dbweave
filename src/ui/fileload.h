/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  "@dbw3:file" loader. */

#ifndef DBWEAVE_UI_FILELOAD_H
#define DBWEAVE_UI_FILELOAD_H

#include "loadoptions.h"

class TDBWFRM;
class FfReader;

class FhLoader
{
private:
    TDBWFRM* mainfrm;
    LOADPARTS loadparts;

public:
    explicit FhLoader(TDBWFRM* _mainfrm);
    bool Load(LOADSTAT& _stat, LOADPARTS _loadparts = LOADALL);

private:
    bool Need(LOADOPTION _part) const;

    bool LoadSignatur(FfReader* _reader);
    void LoadVersion(FfReader* _reader);
    void LoadProperties(FfReader* _reader);
    void LoadData(FfReader* _reader);
    void LoadDataSize(FfReader* _reader);
    void LoadDataFields(FfReader* _reader);
    void LoadDataEinzug(FfReader* _reader);
    void LoadDataAufknuepfung(FfReader* _reader);
    void LoadDataTrittfolge(FfReader* _reader);
    void LoadDataTrittfolgeTrittfolge(FfReader* _reader);
    void LoadDataTrittfolgeIsEmpty(FfReader* _reader);
    void LoadDataSchussfarben(FfReader* _reader);
    void LoadDataKettfarben(FfReader* _reader);
    void LoadDataBlatteinzug(FfReader* _reader);
    void LoadDataPalette(FfReader* _reader);
    void LoadDataHilfslinien(FfReader* _reader);
    void LoadDataWebstuhl(FfReader* _reader);
    void LoadDataWebstuhlKlammer(FfReader* _reader, int _index);
    void LoadDataWebstuhlCurrent(FfReader* _reader);
    void LoadDataWebstuhlLast(FfReader* _reader);
    void LoadDataWebstuhlDivers(FfReader* _reader);
    void LoadDataFixeinzug(FfReader* _reader);
    void LoadDataBlockmuster(FfReader* _reader);
    void LoadDataBereichmuster(FfReader* _reader);
    void LoadPrint(FfReader* _reader);
    void LoadPrintRange(FfReader* _reader);
    void LoadView(FfReader* _reader);
    void LoadViewGeneral(FfReader* _reader);
    void LoadViewGewebe(FfReader* _reader);
    void LoadViewEinzug(FfReader* _reader);
    void LoadViewAufknuepfung(FfReader* _reader);
    void LoadViewTrittfolge(FfReader* _reader);
    void LoadViewSchlagpatrone(FfReader* _reader);
    void LoadViewBlatteinzug(FfReader* _reader);
    void LoadViewKettfarben(FfReader* _reader);
    void LoadViewSchussfarben(FfReader* _reader);
    void LoadViewPagesetup(FfReader* _reader);
};

#endif
