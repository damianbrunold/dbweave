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
#ifndef DBWEAVE_UI_EINZUG_H
#define DBWEAVE_UI_EINZUG_H
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
class TDBWFRM;
class TData;
/*-----------------------------------------------------------------*/
// Interface
class EinzugRearrange
{
public:
    virtual ~EinzugRearrange() { }

    virtual void Rearrange() = 0;
    virtual void NormalZ() = 0;
    virtual void NormalS() = 0;
    virtual void GeradeZ() = 0;
    virtual void GeradeS() = 0;
    virtual void Chorig2() = 0;
    virtual void Chorig3() = 0;

    virtual bool IsEmptySchaft(int _j) = 0;
    virtual void RedrawSchaft(int _j) = 0;
    virtual void RedrawAufknuepfungSchaft(int _j) = 0;
    virtual void EliminateEmptySchaft() = 0;
    virtual void SwitchSchaefte(int _a, int _b) = 0;

public:
    static EinzugRearrange* CreateInstance(TDBWFRM* _frm, TData* _data);
    static void ReleaseInstance(EinzugRearrange* _einzug);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
