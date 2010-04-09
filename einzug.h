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
#ifndef DBW3_einzugH
#define DBW3_einzugH
/*-----------------------------------------------------------------*/
// Forward-Deklarationen
class TDBWFRM;
class TData;
/*-----------------------------------------------------------------*/
// Interface
class EinzugRearrange
{
public:
    virtual __fastcall ~EinzugRearrange() {}

    virtual void __fastcall Rearrange() = 0;
    virtual void __fastcall NormalZ() = 0;
    virtual void __fastcall NormalS() = 0;
    virtual void __fastcall GeradeZ() = 0;
    virtual void __fastcall GeradeS() = 0;
    virtual void __fastcall Chorig2() = 0;
    virtual void __fastcall Chorig3() = 0;
    virtual void __fastcall Fixiert() = 0;
    virtual void __fastcall Belassen() = 0;

    virtual bool __fastcall IsEmptySchaft (int _j) = 0;
    virtual void __fastcall RedrawSchaft (int _j) = 0;
    virtual void __fastcall RedrawAufknuepfungSchaft (int _j) = 0;
    virtual void __fastcall EliminateEmptySchaft() = 0;
    virtual void __fastcall SwitchSchaefte (int _a, int _b) = 0;

public:
    static EinzugRearrange* __fastcall CreateInstance (TDBWFRM* _frm, TData* _data);
    static void __fastcall ReleaseInstance (EinzugRearrange* _einzug);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
