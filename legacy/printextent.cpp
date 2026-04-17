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
#include <vcl\printers.hpp>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "felddef.h"
#include "datamodule.h"
#include "print.h"
/*-----------------------------------------------------------------*/
bool __fastcall PrPrinter::IsEmptySchaft (int _j)
{
    for (int i=0; i<data->MAXX1; i++)
        if (mainfrm->einzug.feld.Get(i)==_j+1) return false;
    return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall PrPrinter::IsEmptySchaftAufknuepfung (int _j)
{
    for (int i=0; i<data->MAXX2; i++)
        if (mainfrm->aufknuepfung.feld.Get(i,_j)>0) return false;
    return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall PrPrinter::IsEmptyTritt (int _i)
{
    for (int j=0; j<data->MAXY2; j++)
        if (mainfrm->trittfolge.feld.Get(_i, j)>0) return false;
    return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall PrPrinter::IsEmptyTrittAufknuepfung (int _i)
{
    for (int j=0; j<data->MAXY1; j++)
        if (mainfrm->aufknuepfung.feld.Get(_i, j)>0) return false;
    return true;
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::CalcSchaefte()
{
	schaefte.a = 0;
    schaefte.b = data->MAXY1-1;

	// ersten Schaft bestimmen
	for (int j=0; j<data->MAXY1; j++) {
		if (!IsEmptySchaft(j) || (!mainfrm->ViewSchlagpatrone->Checked && !IsEmptySchaftAufknuepfung(j))) {
			schaefte.a = j;
			break;
		}
	}

	// letzten Schaft bestimmen
	for (int j=data->MAXY1-1; j>=0; j--) {
		if (!IsEmptySchaft(j) || (!mainfrm->ViewSchlagpatrone->Checked && !IsEmptySchaftAufknuepfung(j))) {
			schaefte.b = j;
			break;
		}
	}
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::CalcTritte()
{
	tritte.a = 0;
    tritte.b = data->MAXX2-1;

	// ersten Schaft bestimmen
	for (int i=0; i<data->MAXX2; i++) {
		if (!IsEmptyTritt(i) || (!mainfrm->ViewSchlagpatrone->Checked && !IsEmptyTrittAufknuepfung(i))) {
			tritte.a = i;
			break;
		}
	}

	// letzten Schaft bestimmen
	for (int i=data->MAXX2-1; i>=0; i--) {
		if (!IsEmptyTritt(i) || (!mainfrm->ViewSchlagpatrone->Checked && !IsEmptyTrittAufknuepfung(i))) {
			tritte.b = i;
			break;
		}
	}
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::CalcKette()
{
	kette.a = 0;
    kette.b = data->MAXX1-1;

	// ersten Kettfaden bestimmen
	for (int i=0; i<data->MAXX1; i++) {
		if (!mainfrm->IsEmptyEinzug (i)) {
			kette.a = i;
			break;
		}
	}

	// letzten Kettfaden bestimmen
	for (int i=data->MAXX1-1; i>=0; i--) {
		if (!mainfrm->IsEmptyEinzug (i)) {
			kette.b = i;
			break;
		}
	}
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::CalcSchuesse()
{
	schuesse.a = 0;
    schuesse.b = data->MAXY2-1;

	// ersten Schussfaden bestimmen
	for (int j=0; j<data->MAXY2; j++) {
		if (!mainfrm->IsEmptyTrittfolge (j)) {
			schuesse.a = j;
			break;
		}
	}

	// letzten Schussfaden bestimmen
	for (int j=data->MAXY2-1; j>=0; j--) {
		if (!mainfrm->IsEmptyTrittfolge (j)) {
			schuesse.b = j;
			break;
		}
	}
}
/*-----------------------------------------------------------------*/
