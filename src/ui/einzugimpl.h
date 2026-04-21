/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*-----------------------------------------------------------------*/
#ifndef DBWEAVE_UI_EINZUGIMPL_H
#define DBWEAVE_UI_EINZUGIMPL_H
/*-----------------------------------------------------------------*/
#include "dbw3_base.h"
#include "einzug.h"
/*-----------------------------------------------------------------*/
class EinzugRearrangeImpl : public EinzugRearrange
{
protected:
    TDBWFRM* frm;
    TData* data;

    int j1, j2;
    SZ r;

public:
    EinzugRearrangeImpl(TDBWFRM* _frm, TData* _data);
    virtual ~EinzugRearrangeImpl();

    virtual void Rearrange();
    virtual void NormalZ();
    virtual void NormalS();
    virtual void GeradeZ();
    virtual void GeradeS();
    virtual void Chorig2();
    virtual void Chorig3();

    virtual bool IsEmptySchaft(int _j);
    virtual void RedrawSchaft(int _j);
    virtual void RedrawAufknuepfungSchaft(int _j);
    virtual void EliminateEmptySchaft();
    virtual void SwitchSchaefte(int _a, int _b);

protected:
    bool IsTotalEmptySchaft(int _j);
    void CalcRange();
    SZ CalcRapportRange();
    bool EinzugEqual(int _i1, int _i2);
    int GetFirstNonemptySchaft(int _j);
    void MoveSchaft(int _von, int _nach);
    int SplitSchaft(int _searchj, int _sourcej);
    void MergeSchaefte();
    bool SchaefteEqual(int _j1, int _j2);
    void RearrangeSchaefte();
    void RedrawSchlagpatrone(int _i);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
