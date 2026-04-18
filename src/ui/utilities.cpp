/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Pattern utility ops. Two are ported here: FillKoeper (twill
    fill from a single-column/row seed) and SwapSide (flip the
    loom from rising-shed to sinking-shed by inverting the tie-up
    + mirroring the warp side). The KettLancierung and
    SchussLancierung ops are deferred -- they require the TEnterVV
    dialog, which is Phase 7.                                    */

#include "mainwindow.h"
#include "datamodule.h"
#include "undoredo.h"

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
