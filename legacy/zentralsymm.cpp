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
#include <mem.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "zentralsymm.h"
/*-----------------------------------------------------------------*/
__fastcall ZentralSymmChecker::ZentralSymmChecker()
{
    data = 0;
    temp = 0;
    sizex = 0;
    sizey = 0;
}
/*-----------------------------------------------------------------*/
__fastcall ZentralSymmChecker::ZentralSymmChecker (int _sizex, int _sizey)
{
    data = 0;
    temp = 0;
    sizex = 0;
    sizey = 0;

    Init (_sizex, _sizey);
}
/*-----------------------------------------------------------------*/
__fastcall ZentralSymmChecker::~ZentralSymmChecker()
{
    delete[] data;
    delete[] temp;
}
/*-----------------------------------------------------------------*/
void __fastcall ZentralSymmChecker::Init (int _sizex, int _sizey)
{
    delete[] data;
    delete[] temp;

    try {
        sizex = _sizex;
        sizey = _sizey;
        data = new char[sizex*sizey];
        temp = new char[sizex>sizey ? sizex : sizey];

        memset (data, 0, sizex*sizey*sizeof(char));
        memset (temp, 0, (sizex>sizey ? sizex : sizey)*sizeof(char));
    } catch (...) {
        data = 0;
        temp = 0;
        sizex = 0;
        sizey = 0;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall ZentralSymmChecker::SetData (int _i, int _j, char _val)
{
    dbw3_assert(_i>=0 && _i<sizex);
    dbw3_assert(_j>=0 && _j<sizey);
    data[_i*sizey+_j] = _val;
}
/*-----------------------------------------------------------------*/
char __fastcall ZentralSymmChecker::GetData (int _i, int _j)
{
    dbw3_assert(_i>=0 && _i<sizex);
    dbw3_assert(_j>=0 && _j<sizey);
    return data[_i*sizey+_j];
}
/*-----------------------------------------------------------------*/
bool __fastcall ZentralSymmChecker::IsSymmetric()
{
    for (int i=0; i<sizex/2; i++)
        for (int j=0; j<sizey; j++)
            if (GetData(i, j)!=GetData(sizex-1-i, sizey-1-j))
                return false;
    return true;
}
/*-----------------------------------------------------------------*/
void __fastcall ZentralSymmChecker::RollLeft()
{
    for (int j=0; j<sizey; j++) temp[j] = GetData (0, j);
    for (int i=1; i<sizex; i++)
        for (int j=0; j<sizey; j++)
            SetData (i-1, j, GetData(i, j));
    for (int j=0; j<sizey; j++) SetData (sizex-1, j, temp[j]);
}
/*-----------------------------------------------------------------*/
void __fastcall ZentralSymmChecker::RollUp()
{
    for (int i=0; i<sizex; i++) temp[i] = GetData(i, 0);
    for (int j=1; j<sizey; j++)
        for (int i=0; i<sizex; i++)
            SetData (i, j-1, GetData (i, j));
    for (int i=0; i<sizex; i++) SetData (i, sizey-1, temp[i]);
}
/*-----------------------------------------------------------------*/
bool ZentralSymmChecker::IsAlreadySymmetric()
{
    return IsSymmetric();
}
/*-----------------------------------------------------------------*/
bool ZentralSymmChecker::SearchSymmetry()
{
    dbw3_assert(sizex>0 && sizey>0);
    if (sizex==0 || sizey==0) return false;
    for (int i=0; i<sizex; i++) {
        RollLeft();
        if (IsSymmetric()) return true;
        for (int j=0; j<sizey; j++) {
            RollUp();
            if (IsSymmetric()) return true;
        }
    }
    return false;
}
/*-----------------------------------------------------------------*/

