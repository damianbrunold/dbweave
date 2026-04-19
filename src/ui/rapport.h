/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*------------------------------------------------------------------*/
#ifndef DBWEAVE_UI_RAPPORT_H
#define DBWEAVE_UI_RAPPORT_H
/*------------------------------------------------------------------*/
#include "vcl_compat.h"
#include "dbw3_base.h"
/*------------------------------------------------------------------*/
class TDBWFRM;
class TData;
/*------------------------------------------------------------------*/
// Interface
class RpRapport
{
public:
    virtual ~RpRapport() { }

    virtual bool IsInRapport(int _i, int _j) = 0;
    virtual void UpdateRapport() = 0;
    virtual void CalcRapport() = 0;
    virtual void DrawRapport() = 0;
    virtual void ClearRapport() = 0;
    virtual void DrawDifferences(const RAPPORT& _old, const RAPPORT& _new) = 0;

public:
    static RpRapport* CreateInstance(TDBWFRM* _frm, TData* _data);
    static void ReleaseInstance(RpRapport* _rapport);
};
/*------------------------------------------------------------------*/
#endif
/*------------------------------------------------------------------*/
