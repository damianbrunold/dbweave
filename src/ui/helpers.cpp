/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Collection of small TDBWFRM data-level helpers that the legacy
    codebase kept scattered across dbw3_form.cpp, setgewebe.cpp, and
    redraw.cpp. Consolidated here so setops.cpp siblings don't each
    need their own stub file. */

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "undoredo.h"
#include <algorithm>
#include <cstring>
/*-----------------------------------------------------------------*/
using std::max;
/*-----------------------------------------------------------------*/
void TDBWFRM::ToggleGewebe(int _i, int _j)
{
    char s = gewebe.feld.Get(_i, _j);
    gewebe.feld.Set(_i, _j, char(s <= 0 ? currentrange : -s));
}
/*-----------------------------------------------------------------*/
void TDBWFRM::ToggleAufknuepfung(int _i, int _j)
{
    char s = aufknuepfung.feld.Get(_i, _j);
    aufknuepfung.feld.Set(_i, _j, char(s <= 0 ? currentrange : -s));
}
/*-----------------------------------------------------------------*/
bool TDBWFRM::IsEmptyEinzug(int _i)
{
    return einzug.feld.Get(_i) == 0;
}
/*-----------------------------------------------------------------*/
bool TDBWFRM::IsEmptyTrittfolge(int _j)
{
    return trittfolge.isempty.Get(_j);
}
/*-----------------------------------------------------------------*/
void TDBWFRM::RecalcTrittfolgeEmpty(int _j)
{
    for (int i = 0; i < Data->MAXX2; i++)
        if (trittfolge.feld.Get(i, _j) > 0) {
            trittfolge.isempty.Set(_j, false);
            return;
        }
    trittfolge.isempty.Set(_j, true);
}
/*-----------------------------------------------------------------*/
bool TDBWFRM::KettfadenEqual(int _a, int _b)
{
    for (int j = 0; j < Data->MAXY2; j++) {
        char s1 = gewebe.feld.Get(_a, j);
        char s2 = gewebe.feld.Get(_b, j);
        if ((s1 <= 0 && s2 > 0) || (s1 > 0 && s2 <= 0))
            return false;
        if (s1 > 0 && s2 > 0 && s1 != s2)
            return false;
    }
    return true;
}
/*-----------------------------------------------------------------*/
short TDBWFRM::GetFreeEinzug()
{
    for (int i = 0; i < Data->MAXY1; i++)
        if (freieschaefte[i] == true)
            return short(i + 1);
    short free = short(Data->MAXY1);
    ExtendSchaefte();
    dbw3_assert(freieschaefte[free] == true);
    return free;
}
/*-----------------------------------------------------------------*/
short TDBWFRM::GetFreeTritt()
{
    for (short i = 0; i < Data->MAXX2; i++)
        if (freietritte[i] == true)
            return i;
    short free = short(Data->MAXX2);
    ExtendTritte();
    dbw3_assert(freietritte[free] == true);
    return free;
}
/*-----------------------------------------------------------------*/
void TDBWFRM::CopyTritt(int _von, int _nach)
{
    for (int i = 0; i < Data->MAXX2; i++)
        trittfolge.feld.Set(i, _nach, trittfolge.feld.Get(i, _von));
}
/*-----------------------------------------------------------------*/
void TDBWFRM::_ExtendTritte(int _max)
{
    if (Data->MAXX2 < _max) {
        Data->MAXX2 = _max;
        aufknuepfung.feld.Resize(Data->MAXX2, Data->MAXY1, 0);
        trittfolge.feld.Resize(Data->MAXX2, Data->MAXY2, 0);
        bool* pNew = NULL;
        try {
            pNew = new bool[Data->MAXX2];
        } catch (...) {
            dbw3_assert(false);
        }
        for (int i = 0; i < Data->MAXX2 - 10; i++)
            pNew[i] = freietritte[i];
        for (int i = Data->MAXX2 - 10; i < Data->MAXX2; i++)
            pNew[i] = true;
        delete[] freietritte;
        freietritte = pNew;
    }
}
/*-----------------------------------------------------------------*/
void TDBWFRM::_ExtendSchaefte(int _max)
{
    if (Data->MAXY1 < _max) {
        Data->MAXY1 = _max;
        aufknuepfung.feld.Resize(Data->MAXX2, Data->MAXY1, 0);
        bool* pNew = NULL;
        try {
            pNew = new bool[Data->MAXY1];
        } catch (...) {
        }
        for (int i = 0; i < Data->MAXY1 - 10; i++)
            pNew[i] = freieschaefte[i];
        for (int i = Data->MAXY1 - 10; i < Data->MAXY1; i++)
            pNew[i] = true;
        delete[] freieschaefte;
        freieschaefte = pNew;
    }
}
/*-----------------------------------------------------------------*/
void TDBWFRM::ExtendTritte()
{
    int newmax = max(int(Data->MAXX2) + 10, int(Data->MAXY1));
    _ExtendTritte(newmax);
    _ExtendSchaefte(newmax);
    if (undo)
        undo->UpdateSize();
    UpdateScrollbars();
}
/*-----------------------------------------------------------------*/
void TDBWFRM::ExtendSchaefte()
{
    int newmax = max(int(Data->MAXX2), int(Data->MAXY1) + 10);
    _ExtendTritte(newmax);
    _ExtendSchaefte(newmax);
    if (undo)
        undo->UpdateSize();
    UpdateScrollbars();
}
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
/*  --- AllocBuffers* ---------------------------------------------
    The legacy TDBWFRM::AllocBuffers* methods reallocated the raw
    bool* freieschaefte / freietritte arrays in addition to resizing
    the Feld members. The port's Feld*.Resize already grows the
    backing storage, so AllocBuffers* here rebuilds just the
    "freie" flag arrays to match Data->MAX*. Called by the file
    loader after Data->MAX* is updated.                            */
/*-----------------------------------------------------------------*/
static void resizeFreie(bool*& _arr, int _newsize)
{
    if (!_arr) {
        _arr = new bool[_newsize];
        for (int i = 0; i < _newsize; i++)
            _arr[i] = true;
        return;
    }
    bool* p = new bool[_newsize];
    for (int i = 0; i < _newsize; i++)
        p[i] = true;
    delete[] _arr;
    _arr = p;
}
/*-----------------------------------------------------------------*/
static void resizeScratch(char*& _buf, int _newsize)
{
    delete[] _buf;
    _buf = new char[_newsize];
    std::memset(_buf, 0, _newsize);
}

void TDBWFRM::AllocBuffersX1()
{
    einzug.feld.Resize(Data->MAXX1, 0);
    kettfarben.feld.Resize(Data->MAXX1, Data->defcolorh);
    blatteinzug.feld.Resize(Data->MAXX1, 0);
    gewebe.feld.Resize(Data->MAXX1, Data->MAXY2, 0);
    resizeScratch(xbuf, Data->MAXX1);
}
/*-----------------------------------------------------------------*/
void TDBWFRM::AllocBuffersX2()
{
    aufknuepfung.feld.Resize(Data->MAXX2, Data->MAXY1, 0);
    trittfolge.feld.Resize(Data->MAXX2, Data->MAXY2, 0);
    resizeFreie(freietritte, Data->MAXX2);
}
/*-----------------------------------------------------------------*/
void TDBWFRM::AllocBuffersY1()
{
    aufknuepfung.feld.Resize(Data->MAXX2, Data->MAXY1, 0);
    einzug.maxy = Data->MAXY1;
    resizeFreie(freieschaefte, Data->MAXY1);
}
/*-----------------------------------------------------------------*/
void TDBWFRM::AllocBuffersY2()
{
    trittfolge.feld.Resize(Data->MAXX2, Data->MAXY2, 0);
    trittfolge.isempty.Resize(Data->MAXY2, true);
    schussfarben.feld.Resize(Data->MAXY2, Data->defcolorv);
    gewebe.feld.Resize(Data->MAXX1, Data->MAXY2, 0);
    resizeScratch(ybuf, Data->MAXY2);
}
/*-----------------------------------------------------------------*/
void TDBWFRM::AllocBuffers(bool _clear)
{
    (void)_clear; /* legacy cleared; we unconditionally preserve */
    AllocBuffersX1();
    AllocBuffersX2();
    AllocBuffersY1();
    AllocBuffersY2();
}
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/*  SetAusmasse: resize the document dimensions (warps, shafts,
    treadles, wefts) + visibility caps. Verbatim port of legacy/
    xoptions_form.cpp:SetAusmasse. Called from ShowOptions when
    the user accepts a resize.                                    */
void TDBWFRM::SetAusmasse(int _x1, int _y1, int _x2, int _y2, int _vx2, int _vy1)
{
    /*  #Schaefte == #Tritte */
    if (_y1 > _x2)
        _x2 = _y1;
    if (_x2 > _y1)
        _y1 = _x2;

    /*  Feld u. U. redimensionieren. */
    if (_x1 != Data->MAXX1) {
        blatteinzug.feld.Resize(_x1, 0);
        kettfarben.feld.Resize(_x1, Data->defcolorh);
        einzug.feld.Resize(_x1, 0);
        einzug.maxy = _y1;
    }
    if (_x1 != Data->MAXX1 || _y2 != Data->MAXY2)
        gewebe.feld.Resize(_x1, _y2, 0);
    if (_x2 != Data->MAXX2 || _y1 != Data->MAXY1)
        aufknuepfung.feld.Resize(_x2, _y1, 0);
    if (_x2 != Data->MAXX2 || _y2 != Data->MAXY2) {
        trittfolge.feld.Resize(_x2, _y2, 0);
        trittfolge.isempty.Resize(_y2, true);
    }
    if (_y2 != Data->MAXY2)
        schussfarben.feld.Resize(_y2, Data->defcolorv);

    const bool reallocbufs
        = _x1 != Data->MAXX1 || _y2 != Data->MAXY2 || _x2 != Data->MAXX2 || _y1 != Data->MAXY1;

    Data->MAXX1 = _x1;
    Data->MAXY1 = _y1;
    Data->MAXX2 = _x2;
    Data->MAXY2 = _y2;

    if (reallocbufs)
        AllocBuffers(false);

    if (hvisible == 0 && _vy1 != 0) {
        if (ViewEinzug)
            ViewEinzug->setChecked(true);
    }
    if (wvisible == 0 && _vx2 != 0) {
        if (ViewTrittfolge)
            ViewTrittfolge->setChecked(true);
    }

    hvisible = _vy1;
    wvisible = _vx2;

    if (Data->MAXY1 < hvisible)
        hvisible = Data->MAXY1;
    if (Data->MAXX2 < wvisible)
        wvisible = Data->MAXX2;

    if (hvisible == 0) {
        if (ViewEinzug)
            ViewEinzug->setChecked(false);
    }
    if (wvisible == 0) {
        if (ViewTrittfolge)
            ViewTrittfolge->setChecked(false);
    }

    CalcRange();
}
