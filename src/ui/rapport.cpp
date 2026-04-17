/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope for this slice:
      * full algorithm half -- IsInRapport, CalcRapport (CalcKetts and
        CalcSchussrapport), the two *Equal helpers;
      * STUB rendering half -- ClearRapport, DrawRapport,
        DrawDifferences. Those paint red rectangles and two-pixel
        dashed lines on the main window canvas; the code is in the
        port source with the Canvas-dependent body commented out so it
        is easy to fill in when the rendering slice lands.
*/

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "rapportimpl.h"
#include "mainwindow.h"
#include "datamodule.h"
#include <QCursor>
/*-----------------------------------------------------------------*/
__fastcall RpRapportImpl::RpRapportImpl (TDBWFRM* _frm, TData* _data)
: frm(_frm), data(_data)
{
}
/*-----------------------------------------------------------------*/
__fastcall RpRapportImpl::~RpRapportImpl()
{
}
/*-----------------------------------------------------------------*/
bool __fastcall RpRapportImpl::IsInRapport (int _i, int _j)
{
	return _i>=frm->rapport.kr.a && _i<=frm->rapport.kr.b &&
	       _j>=frm->rapport.sr.a && _j<=frm->rapport.sr.b;
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::UpdateRapport()
{
	if (frm->RappViewRapport && frm->RappViewRapport->isChecked()) ClearRapport();
	RAPPORT oldrapport = frm->rapport;
	CalcRapport();
	if (frm->RappViewRapport && frm->RappViewRapport->isChecked()) {
		DrawRapport();
		DrawDifferences (oldrapport, frm->rapport);
	}
	frm->UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::ClearRapport()
{
	/*  STUB: legacy paints clBtnShadow 2-pixel markers at the rapport
	    boundaries across einzug and trittfolge. Rendering slice. */
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::DrawRapport()
{
	/*  STUB: legacy paints clRed 2-pixel markers at the rapport
	    boundaries across einzug and trittfolge. Rendering slice. */
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::DrawDifferences (const RAPPORT& _old, const RAPPORT& _new)
{
	/*  STUB: when the rapport expands, the newly-revealed gewebe
	    cells need redrawing (DrawGewebe / DrawGewebeRahmen). Legacy
	    code iterates the expanded rectangle; body restored when
	    rendering lands.                                           */
	(void)_old;
	(void)_new;
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::CalcKettrapport()
{
	// Rapport loeschen
	frm->rapport.kr = SZ(0, -1);

	int i;
	int i1 = frm->kette.a;
	int i2 = frm->kette.b;
	if (i1==-1 || i2==-1) return;

	// Kettrapport bestimmen
	frm->rapport.kr.a = i1;
	frm->rapport.kr.b = i1;
g_again:
	// potentiellen Rapportanfang finden
	for (i=frm->rapport.kr.b+1; i<=i2; i++) {
		if (EinzugEqual (frm->rapport.kr.a, i))
			break;
	}
	frm->rapport.kr.b = i-1;
	// Pruefen ob es tatsaechlich Rapport ist
	for (i=frm->rapport.kr.b+1; i<=i2; i++) {
		if (!EinzugEqual (frm->rapport.kr.a+(i-frm->rapport.kr.b-1)%(frm->rapport.kr.b-frm->rapport.kr.a+1), i)) {
			frm->rapport.kr.b++;
			goto g_again;
		}
	}
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::CalcSchussrapport()
{
	// Rapport loeschen
	frm->rapport.sr = SZ(0, -1);

	int j;
	int j1 = frm->schuesse.a;
	int j2 = frm->schuesse.b;
	if (j1==-1 || j2==-1) return;

	// Schussrapport bestimmen
	frm->rapport.sr.a = j1;
	frm->rapport.sr.b = j1;
g_again:
	// potentiellen Rapportanfang finden
	for (j=frm->rapport.sr.b+1; j<=j2; j++) {
		if (TrittfolgeEqual (frm->rapport.sr.a, j))
			break;
	}
	frm->rapport.sr.b = j-1;
	// Pruefen ob es tatsaechlich Rapport ist
	for (j=frm->rapport.sr.b+1; j<=j2; j++) {
		if (!TrittfolgeEqual (frm->rapport.sr.a+(j-frm->rapport.sr.b-1)%(frm->rapport.sr.b-frm->rapport.sr.a+1), j)) {
			frm->rapport.sr.b++;
			goto g_again;
		}
	}
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::CalcRapport()
{
	if (frm->rapport.overridden) {
		return;
	}

	QCursor old = frm->cursor();
	frm->setCursor(Qt::WaitCursor);

	CalcKettrapport();
	CalcSchussrapport();

	frm->setCursor(old);
}
/*-----------------------------------------------------------------*/
bool __fastcall RpRapportImpl::TrittfolgeEqual (int _j1, int _j2)
{
	for (int i=0; i<data->MAXX2; i++) {
		char s1 = frm->trittfolge.feld.Get (i, _j1);
		char s2 = frm->trittfolge.feld.Get (i, _j2);
		if (s1<=0 && s2>0) return false;
		if (s1>0 && s2<=0) return false;
		if (s1>0 && s2>0 && s1!=s2) return false;
	}
	if (frm->kette.a!=-1 && frm->kette.b!=-1)
		for (int i=frm->kette.a; i<=frm->kette.b; i++) {
			char s1 = frm->gewebe.feld.Get(i, _j1);
			char s2 = frm->gewebe.feld.Get(i, _j2);
			if (s1<=0 && s2>0) return false;
			if (s1>0 && s2<=0) return false;
			if (s1>0 && s2>0 && s1!=s2) return false;
		}
	return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall RpRapportImpl::EinzugEqual (int _i1, int _i2)
{
	if (frm->einzug.feld.Get(_i1)!=frm->einzug.feld.Get(_i2))
		return false;
	if (frm->schuesse.a!=-1 && frm->schuesse.b!=-1)
		for (int j=frm->schuesse.a; j<=frm->schuesse.b; j++) {
			char s1 = frm->gewebe.feld.Get(_i1, j);
			char s2 = frm->gewebe.feld.Get(_i2, j);
			if (s1<=0 && s2>0) return false;
			if (s1>0 && s2<=0) return false;
			if (s1>0 && s2>0 && s1!=s2) return false;
		}
	return true;
}
/*-----------------------------------------------------------------*/
RpRapport* __fastcall RpRapport::CreateInstance (TDBWFRM* _frm, TData* _data)
{
	RpRapport* p = NULL;
	try {
		p = new RpRapportImpl (_frm, _data);
	} catch (...) {
	}
	return p;
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapport::ReleaseInstance (RpRapport* _rapport)
{
	delete _rapport;
}
/*-----------------------------------------------------------------*/
