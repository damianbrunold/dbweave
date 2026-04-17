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
#ifndef DBWEAVE_UI_RAPPORTIMPL_H
#define DBWEAVE_UI_RAPPORTIMPL_H
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
