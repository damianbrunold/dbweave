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
#include "dbw3_form.h"
/*-----------------------------------------------------------------*/
FeldBlatteinzug::FeldBlatteinzug()
: feld (DEFAULT_MAXX1, 0)
{
}
/*-----------------------------------------------------------------*/
int __fastcall FeldBlatteinzug::ScrollX()
{
    return DBWFRM->scroll_x1;
}
/*-----------------------------------------------------------------*/
int __fastcall FeldBlatteinzug::ScrollY()
{
    return 0;
}
/*-----------------------------------------------------------------*/
FeldKettfarben::FeldKettfarben()
: feld (DEFAULT_MAXX1, DEFAULT_COLORH)
{
}
/*-----------------------------------------------------------------*/
int __fastcall FeldKettfarben::ScrollX()
{
    return DBWFRM->scroll_x1;
}
/*-----------------------------------------------------------------*/
int __fastcall FeldKettfarben::ScrollY()
{
    return 0;
}
/*-----------------------------------------------------------------*/
FeldSchussfarben::FeldSchussfarben()
: feld (DEFAULT_MAXY2, DEFAULT_COLORV)
{
}
/*-----------------------------------------------------------------*/
int __fastcall FeldSchussfarben::ScrollX()
{
    return 0;
}
/*-----------------------------------------------------------------*/
int __fastcall FeldSchussfarben::ScrollY()
{
    return DBWFRM->scroll_y2;
}
/*-----------------------------------------------------------------*/
FeldEinzug::FeldEinzug()
: feld (DEFAULT_MAXX1, 0)
{
    maxy = DEFAULT_MAXY1;
	darstellung = STRICH;
}
/*-----------------------------------------------------------------*/
int __fastcall FeldEinzug::ScrollX()
{
    return DBWFRM->scroll_x1;
}
/*-----------------------------------------------------------------*/
int __fastcall FeldEinzug::ScrollY()
{
    return DBWFRM->scroll_y1;
}
/*-----------------------------------------------------------------*/
FeldAufknuepfung::FeldAufknuepfung()
: feld (DEFAULT_MAXX2, DEFAULT_MAXY1, 0)
{
	darstellung = KREUZ;
    pegplanstyle = false;
}
/*-----------------------------------------------------------------*/
int __fastcall FeldAufknuepfung::ScrollX()
{
    return DBWFRM->scroll_x2;
}
/*-----------------------------------------------------------------*/
int __fastcall FeldAufknuepfung::ScrollY()
{
    return DBWFRM->scroll_y1;
}
/*-----------------------------------------------------------------*/
FeldTrittfolge::FeldTrittfolge()
: feld (DEFAULT_MAXX2, DEFAULT_MAXY2, 0),
  isempty (DEFAULT_MAXY2, true)
{
	darstellung = PUNKT;
    einzeltritt = true;
}
/*-----------------------------------------------------------------*/
int __fastcall FeldTrittfolge::ScrollX()
{
    return DBWFRM->scroll_x2;
}
/*-----------------------------------------------------------------*/
int __fastcall FeldTrittfolge::ScrollY()
{
    return DBWFRM->scroll_y2;
}
/*-----------------------------------------------------------------*/
FeldGewebe::FeldGewebe()
: feld (DEFAULT_MAXX1, DEFAULT_MAXY2, 0)
{
}
/*-----------------------------------------------------------------*/
int __fastcall FeldGewebe::ScrollX()
{
    return DBWFRM->scroll_x1;
}
/*-----------------------------------------------------------------*/
int __fastcall FeldGewebe::ScrollY()
{
    return DBWFRM->scroll_y2;
}
/*-----------------------------------------------------------------*/
