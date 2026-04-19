/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Range calculations — determine which shafts/treadles/warp/weft
    threads contain data and need to be laid out on the printed
    page. Verbatim port of legacy printextent.cpp.                */

#include "print.h"
#include "mainwindow.h"
#include "datamodule.h"

/*-----------------------------------------------------------------*/
bool PrPrinter::IsEmptySchaft(int _j)
{
    for (int i = 0; i < data->MAXX1; i++)
        if (mainfrm->einzug.feld.Get(i) == _j + 1)
            return false;
    return true;
}

bool PrPrinter::IsEmptySchaftAufknuepfung(int _j)
{
    for (int i = 0; i < data->MAXX2; i++)
        if (mainfrm->aufknuepfung.feld.Get(i, _j) > 0)
            return false;
    return true;
}

bool PrPrinter::IsEmptyTritt(int _i)
{
    for (int j = 0; j < data->MAXY2; j++)
        if (mainfrm->trittfolge.feld.Get(_i, j) > 0)
            return false;
    return true;
}

bool PrPrinter::IsEmptyTrittAufknuepfung(int _i)
{
    for (int j = 0; j < data->MAXY1; j++)
        if (mainfrm->aufknuepfung.feld.Get(_i, j) > 0)
            return false;
    return true;
}

/*-----------------------------------------------------------------*/
void PrPrinter::CalcSchaefte()
{
    schaefte.a = 0;
    schaefte.b = data->MAXY1 - 1;
    const bool schlag = mainfrm->ViewSchlagpatrone && mainfrm->ViewSchlagpatrone->isChecked();
    for (int j = 0; j < data->MAXY1; j++)
        if (!IsEmptySchaft(j) || (!schlag && !IsEmptySchaftAufknuepfung(j))) {
            schaefte.a = j;
            break;
        }
    for (int j = data->MAXY1 - 1; j >= 0; j--)
        if (!IsEmptySchaft(j) || (!schlag && !IsEmptySchaftAufknuepfung(j))) {
            schaefte.b = j;
            break;
        }
}

void PrPrinter::CalcTritte()
{
    tritte.a = 0;
    tritte.b = data->MAXX2 - 1;
    const bool schlag = mainfrm->ViewSchlagpatrone && mainfrm->ViewSchlagpatrone->isChecked();
    for (int i = 0; i < data->MAXX2; i++)
        if (!IsEmptyTritt(i) || (!schlag && !IsEmptyTrittAufknuepfung(i))) {
            tritte.a = i;
            break;
        }
    for (int i = data->MAXX2 - 1; i >= 0; i--)
        if (!IsEmptyTritt(i) || (!schlag && !IsEmptyTrittAufknuepfung(i))) {
            tritte.b = i;
            break;
        }
}

void PrPrinter::CalcKette()
{
    kette.a = 0;
    kette.b = data->MAXX1 - 1;
    for (int i = 0; i < data->MAXX1; i++)
        if (!mainfrm->IsEmptyEinzug(i)) {
            kette.a = i;
            break;
        }
    for (int i = data->MAXX1 - 1; i >= 0; i--)
        if (!mainfrm->IsEmptyEinzug(i)) {
            kette.b = i;
            break;
        }
}

void PrPrinter::CalcSchuesse()
{
    schuesse.a = 0;
    schuesse.b = data->MAXY2 - 1;
    for (int j = 0; j < data->MAXY2; j++)
        if (!mainfrm->IsEmptyTrittfolge(j)) {
            schuesse.a = j;
            break;
        }
    for (int j = data->MAXY2 - 1; j >= 0; j--)
        if (!mainfrm->IsEmptyTrittfolge(j)) {
            schuesse.b = j;
            break;
        }
}
