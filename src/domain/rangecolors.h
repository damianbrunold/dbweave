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
#ifndef DBWEAVE_DOMAIN_RANGECOLORS_H
#define DBWEAVE_DOMAIN_RANGECOLORS_H
/*-----------------------------------------------------------------*/
#include "colors_compat.h"
/*-----------------------------------------------------------------*/
extern TColor rangecolortable[13];
extern TColor col_anbindung;
extern TColor col_abbindung;
/*-----------------------------------------------------------------*/
TColor GetRangeColor (int _s);
/*-----------------------------------------------------------------*/
/*  Legacy InitRangeColors(TCanvas*) probed BITSPIXEL to pick between
    the palette-safe and nice colour sets. Modern systems are always
    >= 16 bpp, so the port picks the nice set unconditionally and the
    init function is dropped.                                        */
#endif
/*-----------------------------------------------------------------*/
