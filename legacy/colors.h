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
#ifndef colorsH
#define colorsH
/*----------------------------------------------------------------*/
#define UNDEFINED -1.0
/*----------------------------------------------------------------*/
void RGB2HSV (int r, int g, int b, float& h, float& s, float& v);
void HSV2RGB (float h, float s, float v, int& r, int& g, int& b);
/*----------------------------------------------------------------*/
#endif 
/*----------------------------------------------------------------*/
