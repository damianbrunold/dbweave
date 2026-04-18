/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope:
      * Arrow-key cursor motion (one cell per press, Shift extends
        selection).
      * Space toggles the current cell via cursorhandler.
      * Digits 1-9 set currentrange; 0 unset/paints background.
      * Tab / Shift+Tab (and Enter) switch focused field.

    Deferred:
      * Ctrl/Alt-modified large-skip steps (needs CURSOR_LARGE_SKIP_X
        calibration from the viewport size).
      * Home/End/PageUp/PageDown (HandleHomeKey etc.).
      * Copy/paste shortcuts (K, E, T) -- need clipboard ops.
      * F12 highlight.
      * Userdef shortcuts (Shift+Ctrl+C/V/B).
      * Ctrl+Alt cursor-direction toggle.
*/

#include "mainwindow.h"
#include "cursor.h"
#include "enums.h"

#include <Qt>

/*  Apply a state-apply op at the current cursor position of the
    active field. Used by Space and digit-0 to paint / clear; the
    legacy cursor->ToggleField and SetField dispatch here in the
    CrFeld subclasses -- the port routes through TDBWFRM::Set*
    directly because the cursor's Set/Toggle are still stubs.    */
static void applyAtCursor (TDBWFRM* frm, bool _set, int _range)
{
	/*  fb.kbd.{i,j} are viewport-local; Set* editor ops expect
	    viewport-local indices (they add scroll internally), so no
	    scroll addition is needed here.                          */
	switch (frm->kbd_field) {
	case GEWEBE:
		frm->SetGewebe(frm->gewebe.kbd.i, frm->gewebe.kbd.j, _set, _range);
		break;
	case EINZUG:
		if (_set) frm->SetEinzug(frm->einzug.kbd.i, frm->einzug.kbd.j);
		break;
	case AUFKNUEPFUNG:
		frm->SetAufknuepfung(frm->aufknuepfung.kbd.i, frm->aufknuepfung.kbd.j, _set, _range);
		break;
	case TRITTFOLGE:
		frm->SetTrittfolge(frm->trittfolge.kbd.i, frm->trittfolge.kbd.j, _set, _range);
		break;
	case BLATTEINZUG:
		if (_set) frm->SetBlatteinzug(frm->blatteinzug.kbd.i);
		break;
	case KETTFARBEN:
		if (_set) frm->SetKettfarben(frm->kettfarben.kbd.i);
		break;
	case SCHUSSFARBEN:
		if (_set) frm->SetSchussfarben(frm->schussfarben.kbd.j);
		break;
	default:
		break;
	}
}

void __fastcall TDBWFRM::handleCanvasKeyPress (int _key, int _modifiers)
{
	if (!cursorhandler) return;

	const bool shift = (_modifiers & Qt::ShiftModifier) != 0;
	/*  The cursor API takes a TShiftState compat object; use the
	    select-only convention from the keyboard layer. The Move*
	    methods' _select flag comes through as shift only.          */

	switch (_key) {
	case Qt::Key_Left:
		cursorhandler->MoveCursorLeft(1, shift);
		refresh();
		return;
	case Qt::Key_Right:
		cursorhandler->MoveCursorRight(1, shift);
		refresh();
		return;
	case Qt::Key_Up:
		cursorhandler->MoveCursorUp(1, shift);
		refresh();
		return;
	case Qt::Key_Down:
		cursorhandler->MoveCursorDown(1, shift);
		refresh();
		return;

	case Qt::Key_Space:
		applyAtCursor(this, /*_set=*/!shift, currentrange);
		refresh();
		return;

	case Qt::Key_Return:
	case Qt::Key_Enter:
	case Qt::Key_Tab:
		cursorhandler->GotoNextField();
		refresh();
		return;

	case Qt::Key_Backtab:
		cursorhandler->GotoPrevField();
		refresh();
		return;
	}

	if (_key >= Qt::Key_0 && _key <= Qt::Key_9) {
		const int digit = _key - Qt::Key_0;
		if (digit == 0) {
			applyAtCursor(this, /*_set=*/false, 0);
		} else {
			currentrange = digit;
		}
		UpdateStatusBar();
		refresh();
	}
}
