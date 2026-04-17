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
void __fastcall TDBWFRM::InvalidateFeld (GRIDPOS _gridpos)
{
    RECT rc;
    rc.left   = _gridpos.x0;
    rc.right  = _gridpos.x0 + _gridpos.width;
    rc.top    = _gridpos.y0;
    rc.bottom = _gridpos.y0 + _gridpos.height;

    InvalidateRect (Handle, &rc, true);
}
/*-----------------------------------------------------------------*/
