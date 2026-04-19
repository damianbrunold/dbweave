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
#ifndef DBWEAVE_UI_LOADOPTIONS_H
#define DBWEAVE_UI_LOADOPTIONS_H
/*-----------------------------------------------------------------*/
enum LOADOPTION {
    LOADALL = (int)0xFFFFFFFF,
    LOADEINZUG = 1,
    LOADTRITTFOLGE = 2,
    LOADAUFKNUEPFUNG = 4,
    LOADKETTFARBEN = 8,
    LOADSCHUSSFARBEN = 16,
    LOADBLATTEINZUG = 32,
    LOADHILFSLINIEN = 64,
    LOADPALETTE = 128,
    LOADFIXEINZUG = 256,
    LOADBLOCKMUSTER = 512,
    LOADBEREICHMUSTER = 1024
};
/*-----------------------------------------------------------------*/
typedef int LOADPARTS;
/*-----------------------------------------------------------------*/
enum LOADSTAT {
    FILE_LOADED = 0,
    UNKNOWN_FAILURE,
    FILE_DOES_NOT_EXIST,
    FILE_ALREADY_OPEN,
    FILE_WITHOUT_SIGNATURE,
    FILE_CORRUPT
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
