/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#ifndef DBWEAVE_UI_RECALC_H
#define DBWEAVE_UI_RECALC_H

#include "vcl_compat.h"

class TDBWFRM;
class TData;

/*  Full einzug / trittfolge / aufknuepfung rebuilder. Ported
    verbatim from legacy/recalc.cpp. Consumed by TDBWFRM wrappers
    (RecalcAll / RecalcSchlagpatrone / RecalcTrittfolgeAufknuepfung
    / RecalcFixEinzug) and by the bereiche / importbmp /
    insertbindung / rapportieren paths that mutate gewebe
    wholesale. */
class RcRecalcAll
{
protected:
	TData*   data;
	TDBWFRM* frm;
	bool     schlagpatrone;
	int      k1, k2;
	int      s1, s2;

public:
	__fastcall RcRecalcAll (TDBWFRM* _frm, TData* _data, bool _schlagpatrone = false);

	void __fastcall Recalc();
	void __fastcall RecalcEinzugFixiert();
	void __fastcall RecalcSchlagpatrone();
	void __fastcall RecalcTrittfolgeAufknuepfung();
	void __fastcall RecalcAufknuepfung();
	void __fastcall CalcK();
	void __fastcall CalcS();

protected:
	void __fastcall RecalcEinzug();
	void __fastcall RecalcTrittfolge();

	bool __fastcall KettfadenEqual   (int _a, int _b);
	bool __fastcall SchussfadenEqual (int _a, int _b);

	bool __fastcall KettfadenEmpty   (int _a);
	bool __fastcall SchussfadenEmpty (int _a);

	short __fastcall GetSchaft (int _a);
};

#endif
