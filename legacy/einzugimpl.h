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
#ifndef DBW3_einzugimplH
#define DBW3_einzugimplH
/*-----------------------------------------------------------------*/
#include "einzug.h"
/*-----------------------------------------------------------------*/
class EinzugRearrangeImpl : public EinzugRearrange
{
protected:
    TDBWFRM* frm;
    TData* data;

    int j1, j2;
    SZ  r;

public:
    __fastcall EinzugRearrangeImpl (TDBWFRM* _frm, TData* _data);
    virtual __fastcall ~EinzugRearrangeImpl();

    virtual void __fastcall Rearrange();
    virtual void __fastcall NormalZ();
    virtual void __fastcall NormalS();
    virtual void __fastcall GeradeZ();
    virtual void __fastcall GeradeS();
    virtual void __fastcall Chorig2();
    virtual void __fastcall Chorig3();
    virtual void __fastcall Fixiert();
    virtual void __fastcall Belassen();

    virtual bool __fastcall IsEmptySchaft (int _j);
    virtual void __fastcall RedrawSchaft (int _j);
    virtual void __fastcall RedrawAufknuepfungSchaft (int _j);
    virtual void __fastcall EliminateEmptySchaft();
    virtual void __fastcall SwitchSchaefte (int _a, int _b);

protected:
    bool __fastcall IsTotalEmptySchaft (int _j);
    void __fastcall CalcRange();
    SZ   __fastcall CalcRapportRange();
    bool __fastcall EinzugEqual (int _i1, int _i2);
    int  __fastcall GetFirstNonemptySchaft (int _j);
    void __fastcall MoveSchaft (int _von, int _nach);
    int  __fastcall SplitSchaft (int _searchj, int _sourcej);
    void __fastcall MergeSchaefte();
    bool __fastcall SchaefteEqual (int _j1, int _j2);
    void __fastcall RearrangeSchaefte();
    void __fastcall RedrawSchlagpatrone (int _i);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
