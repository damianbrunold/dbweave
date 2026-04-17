/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope: the ten non-click helper methods from legacy/
    trittfolge.cpp (IsEmptyTritt, GetFirstNonemptyTritt, RedrawTritt,
    RedrawAufknuepfungTritt, MoveTritt, AufknuepfungsspalteEqual,
    MergeTritte, EliminateEmptyTritt, SwitchTritte, RearrangeTritte).
    The seven Tf*Click / ClearTrittfolgeClick menu handlers are
    deferred -- they call undo->Snapshot + SetModified + re-dispatch
    to RearrangeTritte, which the menu-wiring slice will hook up.
*/

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
#include <QAction>
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::IsEmptyTritt (int _i)
{
	return freietritte[_i];
}
/*-----------------------------------------------------------------*/
int __fastcall TDBWFRM::GetFirstNonemptyTritt (int _i)
{
	for (int i=_i; i<Data->MAXX2; i++)
		if (!IsEmptyTritt (i)) return i;
	return -1;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RedrawTritt (int _i)
{
	if (!ViewTrittfolge || !ViewTrittfolge->isChecked()) return;
	if (trittfolge.gw<=0 || trittfolge.gh<=0) return;
	if (_i<scroll_x2 || _i>=scroll_x2+trittfolge.pos.width/trittfolge.gw) return; // nicht sichtbar
	for (int j=0; j<trittfolge.pos.height/trittfolge.gh; j++)
		DrawTrittfolge (_i-scroll_x2, j);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RedrawAufknuepfungTritt (int _i)
{
	if (!ViewEinzug || !ViewEinzug->isChecked()) return;
	if (!ViewTrittfolge || !ViewTrittfolge->isChecked()) return;
	if (aufknuepfung.gw<=0 || aufknuepfung.gh<=0) return;
	if (_i<scroll_x2 || _i>=scroll_x2+aufknuepfung.pos.width/aufknuepfung.gw) return; // nicht sichtbar
	for (int j=scroll_y1; j<scroll_y1+aufknuepfung.pos.height/aufknuepfung.gh; j++)
		DrawAufknuepfung (_i-scroll_x2, j-scroll_y1);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::MoveTritt (int _von, int _nach)
{
	int j;
	for (j=schuesse.a; j<=schuesse.b; j++) {
		trittfolge.feld.Set (_nach, j, trittfolge.feld.Get (_von, j));
		trittfolge.feld.Set (_von, j, 0);
	}
	for (j=0; j<Data->MAXY1; j++) {
		aufknuepfung.feld.Set (_nach, j, aufknuepfung.feld.Get (_von, j));
		aufknuepfung.feld.Set (_von, j, 0);
	}
	RedrawTritt (_von);
	RedrawTritt (_nach);
	RedrawAufknuepfungTritt (_von);
	RedrawAufknuepfungTritt (_nach);
	freietritte[_nach] = freietritte[_von];
	freietritte[_von] = true;
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::AufknuepfungsspalteEqual (int _i1, int _i2)
{
	bool nonempty = false;
	for (int j=0; j<Data->MAXY1; j++) {
		char s1 = aufknuepfung.feld.Get(_i1, j);
		char s2 = aufknuepfung.feld.Get(_i2, j);
		if (s1>0 || s2>0) nonempty = true;
		if (s1<=0 && s2>0) return false;
		if (s1>0 && s2<=0) return false;
		if (s1>0 && s2>0 && s1!=s2) return false;
	}
	return nonempty ? true : false;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::MergeTritte()
{
	bool found;

	if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked()) return;

	do {
		found = false;
		// Identische Aufknuepfungsspalten suchen
		for (int i=0; i<Data->MAXX2; i++)
			for (int ii=0; ii<i; ii++)
				if (AufknuepfungsspalteEqual(i, ii)) {
					// Tritte zusammenlegen
					for (int j=schuesse.a; j<=schuesse.b; j++)
						if (trittfolge.feld.Get(i, j)>0) {
							trittfolge.feld.Set(ii, j, trittfolge.feld.Get(i, j));
							trittfolge.feld.Set (i, j, 0);
						}
					RedrawTritt (i);
					RedrawTritt (ii);
					freietritte[i] = true;
					// Aufknuepfung bereinigen
					for (int j=0; j<Data->MAXY1; j++)
						aufknuepfung.feld.Set (i, j, 0);
					RedrawAufknuepfungTritt (i);

					found = true;
					continue; // Suche restarten
				}
	} while (found);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EliminateEmptyTritt()
{
	for (int i=0; i<Data->MAXX2; i++)
		if (IsEmptyTritt (i)) {
			int firstnonempty = GetFirstNonemptyTritt(i);
			if (firstnonempty==-1 || firstnonempty<i)
				return;
			MoveTritt (firstnonempty, i);
		}
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SwitchTritte (int _a, int _b)
{
	int j;
	for (j=schuesse.a; j<=schuesse.b; j++) {
		char temp = trittfolge.feld.Get (_a, j);
		trittfolge.feld.Set (_a, j, trittfolge.feld.Get (_b, j));
		trittfolge.feld.Set (_b, j, temp);
	}
	for (j=0; j<Data->MAXY1; j++) {
		char temp = aufknuepfung.feld.Get (_a, j);
		aufknuepfung.feld.Set (_a, j, aufknuepfung.feld.Get (_b, j));
		aufknuepfung.feld.Set (_b, j, temp);
	}
	RedrawTritt (_a);
	RedrawTritt (_b);
	RedrawAufknuepfungTritt (_a);
	RedrawAufknuepfungTritt (_b);
	dbw3_assert (_a<Data->MAXX2);
	dbw3_assert (_b<Data->MAXX2);
	bool IsEmpty = freietritte[_a];
	freietritte[_a] = freietritte[_b];
	freietritte[_b] = IsEmpty;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RearrangeTritte()
{
	if (TfBelassen && TfBelassen->isChecked()) return;
	if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked()) return;

	if (schuesse.a==-1) return;

	int i1 = 0, i2 = 0, j, ii;

	// Ersten Tritt suchen
	for (ii=0; ii<Data->MAXX2; ii++)
		for (j=schuesse.a; j<=schuesse.b; j++)
			if (trittfolge.feld.Get (ii, j)>0) {
				i1 = ii;
				goto g_weiter;
			}

	// Nichts tun
	return;

g_weiter:
	// Letzten Tritt suchen
	for (ii=Data->MAXX2-1; ii>0; ii--)
		for (j=schuesse.a; j<=schuesse.b; j++)
			if (trittfolge.feld.Get (ii, j)>0) {
				i2 = ii;
				goto g_weiter2;
			}

	// Nichts tun
	return;

g_weiter2:

	MergeTritte();

	// Alle Tritte gemaess aktueller Einstellung arrangieren
	if (TfMinimalZ && TfMinimalZ->isChecked()) {
		for (j=schuesse.a; j<=schuesse.b; j++)
			for (ii=i1; ii<=i2; ii++)
				if (trittfolge.feld.Get (ii, j)>0) {
					if (ii>i1) SwitchTritte (i1, ii);
					i1++;
					if (i1>=i2) goto g_end;
				}
	} else if (TfMinimalS && TfMinimalS->isChecked()) {
		for (j=schuesse.a; j<=schuesse.b; j++)
			for (ii=i2; ii>=i1; ii--)
				if (trittfolge.feld.Get (ii, j)>0) {
					if (ii<i2) SwitchTritte (i2, ii);
					i2--;
					if (i2<=i1) goto g_end;
				}
	} else if (TfGeradeZ && TfGeradeZ->isChecked()) {
		//xxxx RTD (legacy tag -- "to do")
	} else if (TfGeradeS && TfGeradeS->isChecked()) {
		//xxxx RTD (legacy tag -- "to do")
	} else if (TfGesprungen && TfGesprungen->isChecked()) {
		int b = 0;
		for (j=schuesse.a; j<=schuesse.b; j++)
			for (ii=i1; ii<=i2; ii++)
				if (trittfolge.feld.Get (ii, j)>0) {
					if ((b%2)==0) {
						if (ii>i1 && ii<=i2) SwitchTritte (i1, ii);
						i1++;
						if (i1>=i2) goto g_end;
					} else if ((b%2)==1) {
						if (ii<i2 && ii>=i1) SwitchTritte (i2, ii);
						i2--;
						if (i2<=i1) goto g_end;
					}
					b = (b+1)%2;
				}
	}

g_end:
	// Schlussarbeiten...
	EliminateEmptyTritt();
	return;
}
/*-----------------------------------------------------------------*/
