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
#ifndef assertH
#define assertH
/*-----------------------------------------------------------------*/
#ifndef _DEBUG
#define dbw3_assert(c)
#define dbw3_trace(c)
#define dbw3_trace2(i,c)
#else
#define dbw3_assert(c) _dbw3_assert(c)
#define dbw3_trace(c) _dbw3_trace(c)
#define dbw3_trace2(i,c) _dbw3_trace(i,(c))
#endif
/*-----------------------------------------------------------------*/
#ifdef _DEBUG
void _dbw3_assert (bool _cond);
void _dbw3_trace (const char* _msg);
void _dbw3_trace (int _level, const char* _msg);
#endif
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/

