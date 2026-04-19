/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Occupied-range bookkeeping for the kette (warp) and schuesse (weft)
    axes. Ported verbatim from legacy/range.cpp. The TDBWFRM stub
    bodies for CalcRangeKette / CalcRangeSchuesse / CalcRange /
    UpdateRange that lived in mainwindow.cpp have been removed; the
    real implementations here supersede them.
*/

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
/*-----------------------------------------------------------------*/
// CalcRangeSchuesse: Berechnet den belegten Schussbereich
void TDBWFRM::CalcRangeSchuesse()
{
    schuesse.a = schuesse.b = -1;
    bool first = true;
    for (int j = 0; j < Data->MAXY2; j++) {
        if (!IsEmptySchuss(j)) {
            if (first) {
                schuesse.a = j;
                first = false;
            } else
                schuesse.b = j;
        }
    }
    if (schuesse.a != -1 && schuesse.b == -1)
        schuesse.b = schuesse.a;
}
/*-----------------------------------------------------------------*/
// CalcRangeKette: Berechnet den belegten Kettbereich
void TDBWFRM::CalcRangeKette()
{
    kette.a = kette.b = -1;
    bool first = true;
    for (int i = 0; i < Data->MAXX1; i++) {
        if (!IsEmptyKette(i)) {
            if (first) {
                kette.a = i;
                first = false;
            } else
                kette.b = i;
        }
    }
    if (kette.a != -1 && kette.b == -1)
        kette.b = kette.a;
}
/*-----------------------------------------------------------------*/
void TDBWFRM::CalcRange()
{
    CalcRangeKette();
    CalcRangeSchuesse();
}
/*-----------------------------------------------------------------*/
void TDBWFRM::UpdateRange(int _i, int _j, bool _set)
{
    // _i==-1 nur Schuesse updaten
    // _j==-1 nur Kette updaten

    SZ oldkette = kette;
    SZ oldschuesse = schuesse;

    // Kette nachfuehren
    if (_i != -1) {
        if (_set) {
            if (_i < kette.a)
                kette.a = _i;
            else if (_i > kette.b)
                kette.b = _i;
            if (kette.a == -1)
                kette.a = kette.b;
            if (kette.b == -1)
                kette.b = kette.a;
        } else {
            if (_i == kette.a) {
                for (int i = kette.a; i <= kette.b; i++)
                    if (!IsEmptyKetteNurGewebe(i)) {
                        kette.a = i;
                        goto g_k_weiter;
                    }
                kette.a = kette.b = -1;
            } else if (_i == kette.b) {
                for (int i = kette.b; i >= kette.a; i--)
                    if (!IsEmptyKetteNurGewebe(i)) {
                        kette.b = i;
                        goto g_k_weiter;
                    }
                kette.a = kette.b = -1;
            }
        g_k_weiter:;
        }
    }

    // Schuesse nachfuehren
    if (_j != -1) {
        if (_set) {
            if (_j < schuesse.a)
                schuesse.a = _j;
            else if (_j > schuesse.b)
                schuesse.b = _j;
            if (schuesse.a == -1)
                schuesse.a = schuesse.b;
            if (schuesse.b == -1)
                schuesse.b = schuesse.a;
        } else {
            if (_j == schuesse.a) {
                for (int j = schuesse.a; j <= schuesse.b; j++)
                    if (!IsEmptySchussNurGewebe(j)) {
                        schuesse.a = j;
                        goto g_s_weiter;
                    }
                schuesse.a = schuesse.b = -1;
            } else if (_j == schuesse.b) {
                for (int j = schuesse.b; j >= schuesse.a; j--)
                    if (!IsEmptySchussNurGewebe(j)) {
                        schuesse.b = j;
                        goto g_s_weiter;
                    }
                schuesse.a = schuesse.b = -1;
            }
        g_s_weiter:;
        }
    }

    if (kette.a != oldkette.a || kette.b != oldkette.b || schuesse.a != oldschuesse.a
        || schuesse.b != oldschuesse.b) {
        UpdateStatusBar();
    }
}
/*-----------------------------------------------------------------*/
bool TDBWFRM::IsEmptySchuss(int _j)
{
    if (!trittfolge.isempty.Get(_j))
        return false;
    for (int i = 0; i < Data->MAXX1; i++)
        if (gewebe.feld.Get(i, _j) > 0)
            return false;
    return true;
}
/*-----------------------------------------------------------------*/
bool TDBWFRM::IsEmptyKette(int _i)
{
    if (einzug.feld.Get(_i) != 0)
        return false;
    for (int j = 0; j < Data->MAXY2; j++)
        if (gewebe.feld.Get(_i, j) > 0)
            return false;
    return true;
}
/*-----------------------------------------------------------------*/
bool TDBWFRM::IsEmptySchussNurGewebe(int _j)
{
    for (int i = 0; i < Data->MAXX1; i++)
        if (gewebe.feld.Get(i, _j) > 0)
            return false;
    return true;
}
/*-----------------------------------------------------------------*/
bool TDBWFRM::IsEmptyKetteNurGewebe(int _i)
{
    for (int j = 0; j < Data->MAXY2; j++)
        if (gewebe.feld.Get(_i, j) > 0)
            return false;
    return true;
}
/*-----------------------------------------------------------------*/
