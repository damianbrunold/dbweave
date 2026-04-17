/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*----------------------------------------------------------------*/
#ifndef DBWEAVE_DOMAIN_COLORS_H
#define DBWEAVE_DOMAIN_COLORS_H
/*----------------------------------------------------------------*/
#define UNDEFINED -1.0
/*----------------------------------------------------------------*/
void RGB2HSV (int r, int g, int b, float& h, float& s, float& v);
void HSV2RGB (float h, float s, float v, int& r, int& g, int& b);
/*----------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------*/
