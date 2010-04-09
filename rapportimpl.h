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
/*------------------------------------------------------------------*/
#ifndef DBW3_rapportimplH
#define DBW3_rapportimplH
/*------------------------------------------------------------------*/
#include "dbw3_base.h"
#include "rapport.h"
/*------------------------------------------------------------------*/
class RpRapportImpl : public RpRapport
{
private:
    TDBWFRM* frm;
    TData*   data;

public:
    __fastcall RpRapportImpl (TDBWFRM* _frm, TData* _data);
    virtual __fastcall ~RpRapportImpl();

    virtual bool __fastcall IsInRapport (int _i, int _j);
    virtual void __fastcall UpdateRapport();
    virtual void __fastcall CalcRapport();
    virtual void __fastcall DrawRapport();
    virtual void __fastcall ClearRapport();
    virtual void __fastcall DrawDifferences (const RAPPORT& _old, const RAPPORT& _new);

protected:
    void __fastcall CalcKettrapport();
    void __fastcall CalcSchussrapport();
    bool __fastcall TrittfolgeEqual (int _j1, int _j2);
    bool __fastcall EinzugEqual (int _i1, int _i2);
};
/*------------------------------------------------------------------*/
#endif
/*------------------------------------------------------------------*/
