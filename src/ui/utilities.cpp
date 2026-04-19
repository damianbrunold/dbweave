/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Pattern utility ops ported from legacy/utilities.cpp:
      FillKoeper -- twill fill from a single seed column / row.
      SwapSide   -- flip the loom from rising-shed to sinking-shed.
      KettLancierung / SchussLancierung -- thread-ratio spreader
        driven by the TEnterVV dialog (EnterVVDialog in the port).
*/

#include "mainwindow.h"
#include "datamodule.h"
#include "undoredo.h"
#include "entervvdialog.h"

#include <algorithm>

void __fastcall TDBWFRM::FillKoeper()
{
	RANGE savesel = selection;
	selection.Normalize();
	if (!selection.Valid() || selection.feld != GEWEBE) {
		selection = savesel;
		return;
	}

	if (selection.begin.i == selection.end.i) {
		/*  Vertical seed: extend diagonally to the right. */
		const int dy     = selection.end.j - selection.begin.j;
		const int iStart = selection.begin.i;
		int       iEnd   = selection.begin.i + dy;

		int i = iStart + 1;
		for (/**/; i <= iEnd; i++) {
			bool ok = true;
			for (int j = selection.begin.j; j <= selection.end.j; j++) {
				if (gewebe.feld.Get(i, j) > 0) { ok = false; break; }
			}
			if (!ok) break;
		}
		iEnd = i - 1;

		if (iEnd >= iStart) {
			for (int ii = iStart + 1; ii <= iEnd; ii++) {
				for (int j = selection.begin.j; j <= selection.begin.j + dy; j++) {
					int jj = j - selection.begin.j - (ii - iStart);
					while (jj < 0) jj += (dy + 1);
					while (jj > dy) jj -= (dy + 1);
					if (gewebe.feld.Get(iStart, selection.begin.j + jj) > 0)
						DoSetGewebe(ii - scroll_x1, j - scroll_y2, false, currentrange);
				}
			}
			if (!(ViewSchlagpatrone && ViewSchlagpatrone->isChecked()))
				RearrangeTritte();
			CalcRangeKette();
			CalcRangeSchuesse();
			CalcRapport();
			selection.end.i = std::min(iEnd, scroll_x1 + gewebe.pos.width / (gewebe.gw > 0 ? gewebe.gw : 1) - 1);
			SetModified();
			refresh();
			if (undo) undo->Snapshot();
		}
	} else if (selection.begin.j == selection.end.j) {
		/*  Horizontal seed: extend diagonally upward. */
		const int dx     = selection.end.i - selection.begin.i;
		const int jStart = selection.begin.j;
		int       jEnd   = selection.begin.j + dx;

		int j = jStart + 1;
		for (/**/; j <= jEnd; j++) {
			bool ok = true;
			for (int i = selection.begin.i; i <= selection.end.i; i++) {
				if (gewebe.feld.Get(i, j) > 0) { ok = false; break; }
			}
			if (!ok) break;
		}
		jEnd = j - 1;

		if (jEnd >= jStart) {
			for (int jj = jStart + 1; jj <= jEnd; jj++) {
				for (int i = selection.begin.i; i <= selection.begin.i + dx; i++) {
					int ii = i - selection.begin.i - (jj - jStart);
					while (ii < 0) ii += (dx + 1);
					while (ii > dx) ii -= (dx + 1);
					if (gewebe.feld.Get(selection.begin.i + ii, jStart) > 0)
						DoSetGewebe(i - scroll_x1, jj - scroll_y2, false, currentrange);
				}
			}
			if (!(ViewSchlagpatrone && ViewSchlagpatrone->isChecked()))
				RearrangeTritte();
			CalcRangeKette();
			CalcRangeSchuesse();
			CalcRapport();
			selection.end.j = std::min(jEnd, scroll_y2 + gewebe.pos.height / (gewebe.gh > 0 ? gewebe.gh : 1) - 1);
			SetModified();
			refresh();
			if (undo) undo->Snapshot();
		}
	}
	/*  selection is intentionally preserved so the resized rectangle
	    stays visible to the user.                                */
	(void)savesel;
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SwapSide()
{
	const bool pegplan = ViewSchlagpatrone && ViewSchlagpatrone->isChecked();
	if (!pegplan) {
		/*  Invert the aufknuepfung across every in-use (shaft,
		    treadle) cell. */
		for (int i = 0; i < Data->MAXX2; i++) {
			if (!freietritte[i]) {
				for (int j = 0; j < Data->MAXY1; j++) {
					if (!freieschaefte[j]) {
						const char s = aufknuepfung.feld.Get(i, j);
						aufknuepfung.feld.Set(i, j, char(s == 0 ? currentrange : -s));
					}
				}
			}
		}
	} else {
		/*  Invert the schlagpatrone in the active weft range. */
		const int maxShaft = std::min(Data->MAXY1, Data->MAXX2);
		for (int i = 0; i < maxShaft; i++) {
			if (!freieschaefte[i]) {
				for (int j = schuesse.a; j <= schuesse.b; j++) {
					if (!trittfolge.isempty.Get(j)) {
						const char s = trittfolge.feld.Get(i, j);
						trittfolge.feld.Set(i, j, char(s == 0 ? currentrange : -s));
					}
				}
			}
		}
	}

	/*  Mirror the warp side: einzug, kettfarben, blatteinzug swap
	    around the kette centre. */
	if (kette.a != -1 && kette.b != -1) {
		const int mid = (kette.b - kette.a + 1) / 2;
		for (int i = kette.a; i < kette.a + mid; i++) {
			const int mirror = kette.b - (i - kette.a);
			{
				const short s = einzug.feld.Get(i);
				einzug.feld.Set(i,      einzug.feld.Get(mirror));
				einzug.feld.Set(mirror, s);
			}
			{
				const char c = kettfarben.feld.Get(i);
				kettfarben.feld.Set(i,      kettfarben.feld.Get(mirror));
				kettfarben.feld.Set(mirror, c);
			}
			{
				const bool b = blatteinzug.feld.Get(i);
				blatteinzug.feld.Set(i,      blatteinzug.feld.Get(mirror));
				blatteinzug.feld.Set(mirror, b);
			}
		}
	}

	RecalcGewebe();
	CalcRangeSchuesse();
	CalcRangeKette();
	UpdateRapport();
	SetModified();
	refresh();
	if (undo) undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::KettLancierungClick()
{
	EnterVVDialog dlg(this);
	if (dlg.exec() != QDialog::Accepted) return;

	int vv[6], maxi = 0;
	for (int k = 0; k < 6; k++) {
		vv[k] = dlg.value(k);
		if (vv[k] != 0) maxi = k + 1;
	}

	if (maxi > 1 && (maxi % 2) != 0) {
		vv[0] += vv[maxi - 1];
		vv[maxi - 1] = 0;
		maxi--;
	}
	if (maxi <= 1) return;

	int a, b;
	RANGE savesel = selection;
	selection.Normalize();
	if (selection.Valid()) {
		a = selection.begin.i;
		b = selection.end.i;
	} else {
		a = kette.a;
		b = kette.b;
	}
	selection = savesel;

	int needed = b - a + 1;
	int i = a;
	int idx = 0;
	do {
		int v = vv[idx];
		while (i <= b && v-- > 0) i++;
		idx = (idx + 1) % maxi;
		if (i <= b + 1) {
			needed += vv[idx];
			idx = (idx + 1) % maxi;
		}
	} while (i <= b);
	if (a + needed >= Data->MAXX1) needed = Data->MAXX1 - 1 - a;

	int* buff = new int[needed];
	idx = 0;
	int ii = 0;
	for (int i = a; i <= b; i++) {
		for (int j = 0; j < vv[idx]; j++) {
			buff[ii++] = i++;
			if (ii >= needed || i - 1 > b) break;
		}
		i--;
		if (ii >= needed) break;
		idx = (idx + 1) % maxi;
		for (int j = 0; j < vv[idx]; j++) {
			buff[ii++] = -1;
			if (ii >= needed) break;
		}
		idx = (idx + 1) % maxi;
		if (ii >= needed) break;
	}

	/*  Restdaten auf Seite kopieren. */
	const int maxii = std::min((int)Data->MAXX1, kette.b - b + 1);
	for (int i = b + maxii - 1; i > b; i--) {
		einzug.feld.Set(a + needed + i - b - 1, einzug.feld.Get(i));
		for (int j = schuesse.a; j <= schuesse.b; j++)
			gewebe.feld.Set(a + needed + i - b - 1, j, gewebe.feld.Get(i, j));
	}
	/*  Auseinanderziehen. */
	for (int i = a + needed - 1; i >= a; i--) {
		if (buff[i - a] == -1) {
			einzug.feld.Set(i, 0);
			for (int j = schuesse.a; j <= schuesse.b; j++)
				gewebe.feld.Set(i, j, 0);
		} else {
			einzug.feld.Set(i, einzug.feld.Get(buff[i - a]));
			for (int j = schuesse.a; j <= schuesse.b; j++)
				gewebe.feld.Set(i, j, gewebe.feld.Get(buff[i - a], j));
		}
	}
	delete[] buff;

	CalcRangeKette();
	RecalcFreieSchaefte();
	UpdateRapport();
	SetModified();
	refresh();
	if (undo) undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SchussLancierungClick()
{
	EnterVVDialog dlg(this);
	if (dlg.exec() != QDialog::Accepted) return;

	int vv[6], maxi = 0;
	for (int k = 0; k < 6; k++) {
		vv[k] = dlg.value(k);
		if (vv[k] != 0) maxi = k + 1;
	}

	if (maxi > 1 && (maxi % 2) != 0) {
		vv[0] += vv[maxi - 1];
		vv[maxi - 1] = 0;
		maxi--;
	}
	if (maxi <= 1) return;

	int a, b;
	RANGE savesel = selection;
	selection.Normalize();
	if (selection.Valid()) {
		a = selection.begin.j;
		b = selection.end.j;
	} else {
		a = schuesse.a;
		b = schuesse.b;
	}
	selection = savesel;

	int needed = b - a + 1;
	int i = a;
	int idx = 0;
	do {
		int v = vv[idx];
		while (i <= b && v-- > 0) i++;
		idx = (idx + 1) % maxi;
		if (i <= b + 1) {
			needed += vv[idx];
			idx = (idx + 1) % maxi;
		}
	} while (i <= b);
	if (a + needed >= Data->MAXY2) needed = Data->MAXY2 - 1 - a;

	int* buff = new int[needed];
	idx = 0;
	int ii = 0;
	for (int i = a; i <= b; i++) {
		for (int j = 0; j < vv[idx]; j++) {
			buff[ii++] = i++;
			if (ii >= needed || i - 1 > b) break;
		}
		i--;
		if (ii >= needed) break;
		idx = (idx + 1) % maxi;
		for (int j = 0; j < vv[idx]; j++) {
			buff[ii++] = -1;
			if (ii >= needed) break;
		}
		idx = (idx + 1) % maxi;
		if (ii >= needed) break;
	}

	/*  Restdaten auf Seite kopieren. */
	const int maxjj = std::min((int)Data->MAXY2, schuesse.b - b + 1);
	for (int j = b + maxjj - 1; j > b; j--) {
		trittfolge.isempty.Set(a + needed + j - b - 1, trittfolge.isempty.Get(j));
		for (int i = 0; i < Data->MAXX2; i++)
			trittfolge.feld.Set(i, a + needed + j - b - 1, trittfolge.feld.Get(i, j));
		for (int i = kette.a; i <= kette.b; i++)
			gewebe.feld.Set(i, a + needed + j - b - 1, gewebe.feld.Get(i, j));
	}
	/*  Auseinanderziehen. */
	for (int j = a + needed - 1; j >= a; j--) {
		if (buff[j - a] == -1) {
			trittfolge.isempty.Set(j, true);
			for (int i = 0; i < Data->MAXX2; i++)
				trittfolge.feld.Set(i, j, 0);
			for (int i = kette.a; i <= kette.b; i++)
				gewebe.feld.Set(i, j, 0);
		} else {
			trittfolge.isempty.Set(j, trittfolge.isempty.Get(buff[j - a]));
			for (int i = 0; i < Data->MAXX2; i++)
				trittfolge.feld.Set(i, j, trittfolge.feld.Get(i, buff[j - a]));
			for (int i = kette.a; i <= kette.b; i++)
				gewebe.feld.Set(i, j, gewebe.feld.Get(i, buff[j - a]));
		}
	}
	delete[] buff;

	CalcRangeSchuesse();
	RecalcFreieTritte();
	UpdateRapport();
	SetModified();
	refresh();
	if (undo) undo->Snapshot();
}
