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
#ifndef loadoptionsH
#define loadoptionsH
/*-----------------------------------------------------------------*/
enum LOADOPTION {
	LOADALL = 0xFFFFFFFF,
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
	FILE_LOADED=0,
	UNKNOWN_FAILURE,
	FILE_DOES_NOT_EXIST,
	FILE_ALREADY_OPEN,
	FILE_WITHOUT_SIGNATURE,
	FILE_CORRUPT
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
