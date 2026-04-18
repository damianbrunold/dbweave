/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Edit-menu operations on the selection.

    Clipboard format (matches legacy exactly):
        dbw\r\n
        WWWW HHHH\r\n
        row0-chars\r\n
        ...
        row(H-1)-chars\r\n
    Each row char encodes one cell:
        GEWEBE / AUFKNUEPFUNG / TRITTFOLGE: 'k' + value
        EINZUG: 'l' if the shaft equals this row's j, 'k' otherwise
        BLATTEINZUG: 'l' / 'k'
    The port only copies/pastes GEWEBE / EINZUG / AUFKNUEPFUNG /
    TRITTFOLGE (the paintable fields on which selection actually
    builds -- strip fields are edited by drag, not rectangle).

    Deferred: RollUp/Down/Left/Right selection shifts, Central
    symmetry (EditCentralsymClick).                              */

#include "mainwindow.h"
#include "datamodule.h"
#include "cursor.h"
#include "undoredo.h"
#include "assert_compat.h"

#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QString>

#include <algorithm>
#include <cstring>

static void recalcRangesAndRapport (TDBWFRM* frm)
{
	frm->CalcRangeKette();
	frm->CalcRangeSchuesse();
	frm->UpdateRapport();
}

/*-----------------------------------------------------------------*/
/*  Serialise one row (j) of the current selection into `_buff`
    using the legacy character encoding. Caller sizes the buffer
    to (selection.width + 1). */
static void getSelectionLine (TDBWFRM* frm, const RANGE& _sel, int _j, char* _buff)
{
	switch (_sel.feld) {
	case GEWEBE:
		for (int i = _sel.begin.i; i <= _sel.end.i; i++)
			*_buff++ = char('k' + frm->gewebe.feld.Get(i, _j));
		break;
	case TRITTFOLGE:
		for (int i = _sel.begin.i; i <= _sel.end.i; i++)
			*_buff++ = char('k' + frm->trittfolge.feld.Get(i, _j));
		break;
	case AUFKNUEPFUNG:
		for (int i = _sel.begin.i; i <= _sel.end.i; i++)
			*_buff++ = char('k' + frm->aufknuepfung.feld.Get(i, _j));
		break;
	case EINZUG:
		for (int i = _sel.begin.i; i <= _sel.end.i; i++)
			*_buff++ = (frm->einzug.feld.Get(i) == _j + 1) ? 'l' : 'k';
		break;
	case BLATTEINZUG:
		for (int i = _sel.begin.i; i <= _sel.end.i; i++)
			*_buff++ = frm->blatteinzug.feld.Get(i) ? 'l' : 'k';
		break;
	default: break;
	}
	*_buff = '\0';
}

/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::CopySelection (bool _movecursor)
{
	RANGE savesel = selection;
	selection.Normalize();
	if (!selection.Valid()) { selection = savesel; return false; }

	const int w = selection.end.i - selection.begin.i + 1;
	const int h = selection.end.j - selection.begin.j + 1;

	QByteArray out;
	out.append("dbw\r\n");
	out.append(QByteArray::number(w, 10).rightJustified(4, '0'));
	out.append(' ');
	out.append(QByteArray::number(h, 10).rightJustified(4, '0'));
	out.append("\r\n");

	QByteArray row(w + 1, Qt::Uninitialized);
	/*  Legacy writes rows top-to-bottom, i.e. from j=end.j down to
	    j=begin.j, because the visual top of the field is the
	    highest j index.                                          */
	for (int j = selection.begin.j; j <= selection.end.j; j++) {
		const int rowJ = selection.end.j - (j - selection.begin.j);
		getSelectionLine(this, selection, rowJ, row.data());
		out.append(row.constData(), w);
		out.append("\r\n");
	}

	QApplication::clipboard()->setText(QString::fromLatin1(out));

	if (_movecursor && cursorhandler) {
		if (selection.feld == TRITTFOLGE)
			cursorhandler->SetCursor(selection.feld, selection.begin.i, selection.end.j + 1, true);
		else
			cursorhandler->SetCursor(selection.feld, selection.end.i + 1, selection.begin.j, true);
	} else {
		selection = savesel;
	}
	return true;
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::CutSelection()
{
	RANGE savesel = selection;
	if (CopySelection(/*_movecursor=*/false)) {
		selection = savesel;
		DeleteSelection();
		ClearSelection();
	}
	if (undo) undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::PasteSelection (bool _transparent)
{
	const QString s = QApplication::clipboard()->text();
	if (!s.startsWith(QLatin1String("dbw"))) return;

	const QByteArray raw = s.toLatin1();
	const char* ptr = raw.constData();
	/*  Skip "dbw\r\n" (5 bytes) and parse width/height. */
	if (raw.size() < 16) return;
	const int x = std::atoi(ptr + 5);
	const int y = std::atoi(ptr + 10);
	if (x <= 0 || y <= 0) return;
	ptr += 16;   /* "dbw\r\n" + "WWWW HHHH\r\n" = 5 + 11 */

	PT start;
	switch (kbd_field) {
	case GEWEBE: {
		for (int j = y - 1; j >= 0; j--) {
			const int jj = scroll_y2 + gewebe.kbd.j + j;
			if (jj < Data->MAXY2) {
				for (int i = 0; i < x; i++) {
					const int ii = scroll_x1 + gewebe.kbd.i + i;
					if (ii >= Data->MAXX1) break;
					if (ptr[i] != 'k' || !_transparent)
						gewebe.feld.Set(ii, jj, char(ptr[i] - 'k'));
				}
			}
			ptr += x + 2;   /* row + "\r\n" */
		}
		start = PT(scroll_x1 + gewebe.kbd.i, scroll_y2 + gewebe.kbd.j);
		if (cursorhandler) cursorhandler->SetCursor(GEWEBE, start.i + x, start.j, true);
		break;
	}
	case EINZUG: {
		for (int j = y - 1; j >= 0; j--) {
			const int jj = scroll_y1 + einzug.kbd.j + j;
			if (jj < Data->MAXY2) {
				for (int i = 0; i < x; i++) {
					const int ii = scroll_x1 + einzug.kbd.i + i;
					if (ii >= Data->MAXX1) break;
					if (ptr[i] == 'l')
						einzug.feld.Set(ii, short(jj + 1));
				}
			}
			ptr += x + 2;
		}
		RecalcGewebe();
		start = PT(scroll_x1 + einzug.kbd.i, scroll_y1 + einzug.kbd.j);
		if (cursorhandler) cursorhandler->SetCursor(EINZUG, start.i + x, start.j, true);
		break;
	}
	case TRITTFOLGE: {
		for (int j = y - 1; j >= 0; j--) {
			const int jj = scroll_y2 + trittfolge.kbd.j + j;
			if (jj < Data->MAXY2) {
				for (int i = 0; i < x; i++) {
					const int ii = scroll_x2 + trittfolge.kbd.i + i;
					if (ii >= Data->MAXX2) break;
					if (ptr[i] != 'k' || !_transparent)
						trittfolge.feld.Set(ii, jj, char(ptr[i] - 'k'));
				}
				RecalcTrittfolgeEmpty(jj);
			}
			ptr += x + 2;
		}
		RecalcGewebe();
		start = PT(scroll_x2 + trittfolge.kbd.i, scroll_y2 + trittfolge.kbd.j);
		if (cursorhandler) cursorhandler->SetCursor(TRITTFOLGE, start.i, start.j + y, true);
		break;
	}
	case AUFKNUEPFUNG: {
		for (int j = y - 1; j >= 0; j--) {
			const int jj = scroll_y1 + aufknuepfung.kbd.j + j;
			if (jj < Data->MAXY1) {
				for (int i = 0; i < x; i++) {
					const int ii = scroll_x2 + aufknuepfung.kbd.i + i;
					if (ii >= Data->MAXX2) break;
					if (ptr[i] != 'k' || !_transparent)
						aufknuepfung.feld.Set(ii, jj, char(ptr[i] - 'k'));
				}
			}
			ptr += x + 2;
		}
		RecalcGewebe();
		start = PT(scroll_x2 + aufknuepfung.kbd.i, scroll_y1 + aufknuepfung.kbd.j);
		if (cursorhandler) cursorhandler->SetCursor(AUFKNUEPFUNG, start.i + x, start.j, true);
		break;
	}
	default:
		return;
	}

	/*  Build a selection covering the pasted region. */
	selection.feld = kbd_field;
	selection.begin = start;
	selection.end   = PT(start.i + x - 1, start.j + y - 1);

	recalcRangesAndRapport(this);
	SetModified();
	refresh();
	if (undo) undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DeleteSelection()
{
	RANGE savesel = selection;
	selection.Normalize();
	if (!selection.Valid()) { selection = savesel; return; }

	switch (selection.feld) {
	case GEWEBE:
		/*  Direct gewebe edits are final -- RecalcGewebe would
		    clobber them by re-deriving from einzug x aufknuepfung x
		    trittfolge. Only the ranges/rapport need refreshing. */
		for (int i = selection.begin.i; i <= selection.end.i; i++)
			for (int j = selection.begin.j; j <= selection.end.j; j++)
				gewebe.feld.Set(i, j, 0);
		break;
	case EINZUG:
		for (int i = selection.begin.i; i <= selection.end.i; i++)
			einzug.feld.Set(i, 0);
		RecalcGewebe();
		break;
	case TRITTFOLGE:
		for (int i = selection.begin.i; i <= selection.end.i; i++)
			for (int j = selection.begin.j; j <= selection.end.j; j++)
				trittfolge.feld.Set(i, j, 0);
		for (int j = selection.begin.j; j <= selection.end.j; j++)
			RecalcTrittfolgeEmpty(j);
		RecalcGewebe();
		break;
	case AUFKNUEPFUNG:
		for (int i = selection.begin.i; i <= selection.end.i; i++)
			for (int j = selection.begin.j; j <= selection.end.j; j++)
				aufknuepfung.feld.Set(i, j, 0);
		RecalcGewebe();
		break;
	default: break;
	}
	recalcRangesAndRapport(this);
	selection = savesel;
	SetModified();
	refresh();
	if (undo) undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::InvertSelection()
{
	RANGE savesel = selection;
	selection.Normalize();
	if (!selection.Valid()) { selection = savesel; return; }

	switch (selection.feld) {
	case GEWEBE:
		for (int i = selection.begin.i; i <= selection.end.i; i++)
			for (int j = selection.begin.j; j <= selection.end.j; j++) {
				const char s = gewebe.feld.Get(i, j);
				gewebe.feld.Set(i, j, char(s == 0 ? currentrange : -s));
			}
		break;
	case AUFKNUEPFUNG:
		for (int i = selection.begin.i; i <= selection.end.i; i++)
			for (int j = selection.begin.j; j <= selection.end.j; j++) {
				const char s = aufknuepfung.feld.Get(i, j);
				aufknuepfung.feld.Set(i, j, char(s == 0 ? currentrange : -s));
			}
		RecalcGewebe();
		break;
	case TRITTFOLGE: {
		const bool singleTritt = trittfolge.einzeltritt &&
		                         !(ViewSchlagpatrone && ViewSchlagpatrone->isChecked());
		if (singleTritt) break;   /* legacy rings the bell and no-ops */
		for (int j = selection.begin.j; j <= selection.end.j; j++)
			for (int i = selection.begin.i; i <= selection.end.i; i++) {
				const char s = trittfolge.feld.Get(i, j);
				trittfolge.feld.Set(i, j, char(s == 0 ? currentrange : -s));
			}
		for (int j = selection.begin.j; j <= selection.end.j; j++)
			RecalcTrittfolgeEmpty(j);
		RecalcGewebe();
		break;
	}
	default: break;
	}
	recalcRangesAndRapport(this);
	selection = savesel;
	SetModified();
	refresh();
	if (undo) undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::MirrorHorzSelection()
{
	RANGE savesel = selection;
	selection.Normalize();
	if (!selection.Valid()) { selection = savesel; return; }

	auto mirrorRow = [&](auto get, auto set) {
		for (int i = selection.begin.i; i <= (selection.begin.i + selection.end.i) / 2; i++) {
			const int mi = selection.end.i - i + selection.begin.i;
			for (int j = selection.begin.j; j <= selection.end.j; j++) {
				const auto t = get(i, j);
				set(i, j, get(mi, j));
				set(mi, j, t);
			}
		}
	};

	switch (selection.feld) {
	case GEWEBE:
		mirrorRow(
		    [this](int i, int j) { return gewebe.feld.Get(i, j); },
		    [this](int i, int j, char v) { gewebe.feld.Set(i, j, v); });
		break;
	case AUFKNUEPFUNG:
		mirrorRow(
		    [this](int i, int j) { return aufknuepfung.feld.Get(i, j); },
		    [this](int i, int j, char v) { aufknuepfung.feld.Set(i, j, v); });
		RecalcGewebe();
		break;
	case EINZUG:
		for (int i = selection.begin.i; i <= (selection.begin.i + selection.end.i) / 2; i++) {
			const int mi = selection.end.i - i + selection.begin.i;
			const short t = einzug.feld.Get(i);
			einzug.feld.Set(i,  einzug.feld.Get(mi));
			einzug.feld.Set(mi, t);
		}
		RecalcGewebe();
		break;
	case TRITTFOLGE:
		mirrorRow(
		    [this](int i, int j) { return trittfolge.feld.Get(i, j); },
		    [this](int i, int j, char v) { trittfolge.feld.Set(i, j, v); });
		for (int j = selection.begin.j; j <= selection.end.j; j++)
			RecalcTrittfolgeEmpty(j);
		RecalcGewebe();
		break;
	default: break;
	}
	recalcRangesAndRapport(this);
	selection = savesel;
	SetModified();
	refresh();
	if (undo) undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::MirrorVertSelection()
{
	RANGE savesel = selection;
	selection.Normalize();
	if (!selection.Valid()) { selection = savesel; return; }

	auto mirrorCol = [&](auto get, auto set) {
		for (int i = selection.begin.i; i <= selection.end.i; i++) {
			for (int j = selection.begin.j; j <= (selection.begin.j + selection.end.j) / 2; j++) {
				const int mj = selection.end.j - j + selection.begin.j;
				const auto t = get(i, j);
				set(i, j,  get(i, mj));
				set(i, mj, t);
			}
		}
	};

	switch (selection.feld) {
	case GEWEBE:
		mirrorCol(
		    [this](int i, int j) { return gewebe.feld.Get(i, j); },
		    [this](int i, int j, char v) { gewebe.feld.Set(i, j, v); });
		break;
	case AUFKNUEPFUNG:
		mirrorCol(
		    [this](int i, int j) { return aufknuepfung.feld.Get(i, j); },
		    [this](int i, int j, char v) { aufknuepfung.feld.Set(i, j, v); });
		RecalcGewebe();
		break;
	case EINZUG:
		/*  Vertical mirror of einzug = remap shaft index within the
		    selected band. */
		for (int i = selection.begin.i; i <= selection.end.i; i++) {
			const short s = einzug.feld.Get(i);
			einzug.feld.Set(i, short(selection.end.j - s + 1 + selection.begin.j + 1));
		}
		RecalcGewebe();
		break;
	case TRITTFOLGE:
		mirrorCol(
		    [this](int i, int j) { return trittfolge.feld.Get(i, j); },
		    [this](int i, int j, char v) { trittfolge.feld.Set(i, j, v); });
		for (int j = selection.begin.j; j <= selection.end.j; j++)
			RecalcTrittfolgeEmpty(j);
		RecalcGewebe();
		break;
	default: break;
	}
	recalcRangesAndRapport(this);
	selection = savesel;
	SetModified();
	refresh();
	if (undo) undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RotateSelection()
{
	RANGE savesel = selection;
	selection.Normalize();
	if (!selection.Valid()) { selection = savesel; return; }
	if (selection.end.i - selection.begin.i !=
	    selection.end.j - selection.begin.j) {
		/*  Legacy MessageBeeps on non-square. */
		selection = savesel;
		return;
	}

	const int dx = selection.end.i - selection.begin.i + 1;
	const int dy = selection.end.j - selection.begin.j + 1;
	QByteArray buf(dx * dy, Qt::Uninitialized);
	char* b = buf.data();

	auto rotate = [&](auto get, auto set) {
		for (int i = selection.begin.i; i <= selection.end.i; i++)
			for (int j = selection.begin.j; j <= selection.end.j; j++)
				b[(i - selection.begin.i) + (j - selection.begin.j) * dx] = get(i, j);
		for (int i = selection.begin.i; i <= selection.end.i; i++)
			for (int j = selection.begin.j; j <= selection.end.j; j++)
				set(selection.begin.i + (j - selection.begin.j),
				    selection.begin.j + (selection.end.i - i),
				    b[(i - selection.begin.i) + (j - selection.begin.j) * dx]);
	};

	switch (selection.feld) {
	case GEWEBE:
		rotate(
		    [this](int i, int j) { return gewebe.feld.Get(i, j); },
		    [this](int i, int j, char v) { gewebe.feld.Set(i, j, v); });
		break;
	case AUFKNUEPFUNG:
		rotate(
		    [this](int i, int j) { return aufknuepfung.feld.Get(i, j); },
		    [this](int i, int j, char v) { aufknuepfung.feld.Set(i, j, v); });
		RecalcGewebe();
		break;
	case TRITTFOLGE: {
		const bool singleTritt = trittfolge.einzeltritt &&
		                         !(ViewSchlagpatrone && ViewSchlagpatrone->isChecked());
		if (singleTritt) break;
		rotate(
		    [this](int i, int j) { return trittfolge.feld.Get(i, j); },
		    [this](int i, int j, char v) { trittfolge.feld.Set(i, j, v); });
		for (int j = selection.begin.j; j <= selection.end.j; j++)
			RecalcTrittfolgeEmpty(j);
		RecalcGewebe();
		break;
	}
	default: break;
	}
	recalcRangesAndRapport(this);
	selection = savesel;
	SetModified();
	refresh();
	if (undo) undo->Snapshot();
}
