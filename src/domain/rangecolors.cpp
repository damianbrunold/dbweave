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
#include "rangecolors.h"
#include "colors_compat.h"
/*-----------------------------------------------------------------*/
/*  Default "nice" palette for ranges 1..9 plus the AUSHEBUNG /
    ANBINDUNG / ABBINDUNG special slots. Legacy init would flip these
    from palette-safe values to the same RGBs we start with here, so
    the runtime colour table is always the nice set.                */
TColor rangecolortable[13] = { (TColor)RGB(0, 0, 0),       /* range 1 */
                               (TColor)RGB(50, 50, 255),   /* range 2 */
                               (TColor)RGB(128, 0, 0),     /* range 3 */
                               (TColor)RGB(0, 140, 255),   /* range 4 */
                               (TColor)RGB(56, 56, 56),    /* range 5 */
                               (TColor)RGB(0, 194, 78),    /* range 6 */
                               (TColor)RGB(255, 123, 0),   /* range 7 */
                               (TColor)RGB(255, 210, 0),   /* range 8 */
                               (TColor)RGB(0, 87, 0),      /* range 9 */
                               /*  Legacy gave AUSHEBUNG and ANBINDUNG the same 128
                                   grey in AUSGEFUELLT mode, which left the two
                                   indistinguishable in the Patrone view. Use a
                                   darker shade for AUSHEBUNG so the three
                                   special ranges are visually distinct even
                                   when every darst_* is set to AUSGEFUELLT.   */
                               (TColor)RGB(96, 96, 96),    /* AUSHEBUNG (10) */
                               (TColor)RGB(128, 128, 128), /* ANBINDUNG (11) */
                               (TColor)RGB(255, 255, 255), /* ABBINDUNG (12) */
                               (TColor)RGB(255, 255, 255) };
/*-----------------------------------------------------------------*/
TColor col_anbindung = (TColor)RGB(72, 160, 72);
TColor col_abbindung = (TColor)RGB(255, 255, 110);
/*-----------------------------------------------------------------*/
TColor GetRangeColor(int _s)
{
    /*  AUSHEBUNG=10 ANBINDUNG=11 ABBINDUNG=12; indices 9/10/11 in
        rangecolortable. Range 1..9 maps to indices 0..8. */
    if (_s == 10)
        return rangecolortable[9];
    if (_s == 11)
        return rangecolortable[10];
    if (_s == 12)
        return rangecolortable[11];
    if (_s > 0 && _s <= 9)
        return rangecolortable[_s - 1];
    return (TColor)RGB(0, 0, 0);
}
/*-----------------------------------------------------------------*/
