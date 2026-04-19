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
#include "dbw3_base.h"
#include "mainwindow.h"
/*-----------------------------------------------------------------*/
FeldBlatteinzug::FeldBlatteinzug()
    : feld(DEFAULT_MAXX1, 0)
{
}
/*-----------------------------------------------------------------*/
int FeldBlatteinzug::ScrollX()
{
    return DBWFRM->scroll_x1;
}
/*-----------------------------------------------------------------*/
int FeldBlatteinzug::ScrollY()
{
    return 0;
}
/*-----------------------------------------------------------------*/
FeldKettfarben::FeldKettfarben()
    : feld(DEFAULT_MAXX1, DEFAULT_COLORH)
{
}
/*-----------------------------------------------------------------*/
int FeldKettfarben::ScrollX()
{
    return DBWFRM->scroll_x1;
}
/*-----------------------------------------------------------------*/
int FeldKettfarben::ScrollY()
{
    return 0;
}
/*-----------------------------------------------------------------*/
FeldSchussfarben::FeldSchussfarben()
    : feld(DEFAULT_MAXY2, DEFAULT_COLORV)
{
}
/*-----------------------------------------------------------------*/
int FeldSchussfarben::ScrollX()
{
    return 0;
}
/*-----------------------------------------------------------------*/
int FeldSchussfarben::ScrollY()
{
    return DBWFRM->scroll_y2;
}
/*-----------------------------------------------------------------*/
FeldEinzug::FeldEinzug()
    : feld(DEFAULT_MAXX1, 0)
{
    maxy = DEFAULT_MAXY1;
    darstellung = STRICH;
}
/*-----------------------------------------------------------------*/
int FeldEinzug::ScrollX()
{
    return DBWFRM->scroll_x1;
}
/*-----------------------------------------------------------------*/
int FeldEinzug::ScrollY()
{
    return DBWFRM->scroll_y1;
}
/*-----------------------------------------------------------------*/
FeldAufknuepfung::FeldAufknuepfung()
    : feld(DEFAULT_MAXX2, DEFAULT_MAXY1, 0)
{
    darstellung = KREUZ;
    pegplanstyle = false;
}
/*-----------------------------------------------------------------*/
int FeldAufknuepfung::ScrollX()
{
    return DBWFRM->scroll_x2;
}
/*-----------------------------------------------------------------*/
int FeldAufknuepfung::ScrollY()
{
    return DBWFRM->scroll_y1;
}
/*-----------------------------------------------------------------*/
FeldTrittfolge::FeldTrittfolge()
    : feld(DEFAULT_MAXX2, DEFAULT_MAXY2, 0)
    , isempty(DEFAULT_MAXY2, true)
{
    darstellung = PUNKT;
    einzeltritt = true;
}
/*-----------------------------------------------------------------*/
int FeldTrittfolge::ScrollX()
{
    return DBWFRM->scroll_x2;
}
/*-----------------------------------------------------------------*/
int FeldTrittfolge::ScrollY()
{
    return DBWFRM->scroll_y2;
}
/*-----------------------------------------------------------------*/
FeldGewebe::FeldGewebe()
    : feld(DEFAULT_MAXX1, DEFAULT_MAXY2, 0)
{
}
/*-----------------------------------------------------------------*/
int FeldGewebe::ScrollX()
{
    return DBWFRM->scroll_x1;
}
/*-----------------------------------------------------------------*/
int FeldGewebe::ScrollY()
{
    return DBWFRM->scroll_y2;
}
/*-----------------------------------------------------------------*/
