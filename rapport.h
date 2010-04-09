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
#ifndef DBW3_rapportH
#define DBW3_rapportH
/*------------------------------------------------------------------*/
// Forward-Deklarationen
class TDBWFRM;
class TData;
/*------------------------------------------------------------------*/
// Interface
class RpRapport
{
public:
    virtual __fastcall ~RpRapport() {}

    virtual bool __fastcall IsInRapport (int _i, int _j) = 0;
    virtual void __fastcall UpdateRapport() = 0;
    virtual void __fastcall CalcRapport() = 0;
    virtual void __fastcall DrawRapport() = 0;
    virtual void __fastcall ClearRapport() = 0;
    virtual void __fastcall DrawDifferences (const RAPPORT& _old, const RAPPORT& _new) = 0;

public:
    static RpRapport* __fastcall CreateInstance (TDBWFRM* _frm, TData* _data);
    static void __fastcall ReleaseInstance (RpRapport* _rapport);
};
/*------------------------------------------------------------------*/
#endif
/*------------------------------------------------------------------*/
