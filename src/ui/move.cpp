/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Verbatim port of legacy/move.cpp. Swap an adjacent shaft or
    treadle with the current one. UpdateMoveMenu + the SB*Click
    toolbar-forwarders are not ported; Qt actions handle their
    own enablement and both the toolbar and the menu bind to the
    same SchaftMove* / TrittMove* entry points. */

#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "einzug.h"
#include "cursor.h"
#include "undoredo.h"

void __fastcall TDBWFRM::SchaftMoveUpClick ()
{
	dbw3_assert(kbd_field == EINZUG || kbd_field == AUFKNUEPFUNG);
	dbw3_assert(einzughandler);
	dbw3_assert(cursorhandler);
	if (!(kbd_field == EINZUG || kbd_field == AUFKNUEPFUNG)) return;

	int currentschaft;
	if (kbd_field == EINZUG) currentschaft = einzug.kbd.j + scroll_y1;
	else                     currentschaft = aufknuepfung.kbd.j + scroll_y1;
	if (currentschaft == -1) return;

	if (currentschaft < Data->MAXY1 - 1) {
		einzughandler->SwitchSchaefte(currentschaft, currentschaft + 1);
		cursorhandler->MoveCursorUp(1, false);
		if (EzBelassen) EzBelassen->setChecked(true);

		SetModified();
		refresh();
		if (undo) undo->Snapshot();
	}
}

void __fastcall TDBWFRM::SchaftMoveDownClick ()
{
	dbw3_assert(kbd_field == EINZUG || kbd_field == AUFKNUEPFUNG);
	dbw3_assert(einzughandler);
	dbw3_assert(cursorhandler);
	if (!(kbd_field == EINZUG || kbd_field == AUFKNUEPFUNG)) return;

	int currentschaft;
	if (kbd_field == EINZUG) currentschaft = einzug.kbd.j + scroll_y1;
	else                     currentschaft = aufknuepfung.kbd.j + scroll_y1;
	if (currentschaft == -1) return;

	if (currentschaft > 0) {
		einzughandler->SwitchSchaefte(currentschaft, currentschaft - 1);
		cursorhandler->MoveCursorDown(1, false);
		if (EzBelassen) EzBelassen->setChecked(true);

		SetModified();
		refresh();
		if (undo) undo->Snapshot();
	}
}

void __fastcall TDBWFRM::TrittMoveLeftClick ()
{
	dbw3_assert(kbd_field == TRITTFOLGE || kbd_field == AUFKNUEPFUNG);
	dbw3_assert(cursorhandler);
	if (!(kbd_field == TRITTFOLGE || kbd_field == AUFKNUEPFUNG)) return;
	if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked()) return;

	int currenttritt;
	if (kbd_field == TRITTFOLGE) currenttritt = trittfolge.kbd.i + scroll_x2;
	else                         currenttritt = aufknuepfung.kbd.i + scroll_x2;
	if (currenttritt == -1) return;

	if (currenttritt > 0) {
		SwitchTritte(currenttritt, currenttritt - 1);
		cursorhandler->MoveCursorLeft(1, false);
		if (TfBelassen) TfBelassen->setChecked(true);

		SetModified();
		refresh();
		if (undo) undo->Snapshot();
	}
}

void __fastcall TDBWFRM::TrittMoveRightClick ()
{
	dbw3_assert(kbd_field == TRITTFOLGE || kbd_field == AUFKNUEPFUNG);
	dbw3_assert(cursorhandler);
	if (!(kbd_field == TRITTFOLGE || kbd_field == AUFKNUEPFUNG)) return;
	if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked()) return;

	int currenttritt;
	if (kbd_field == TRITTFOLGE) currenttritt = trittfolge.kbd.i + scroll_x2;
	else                         currenttritt = aufknuepfung.kbd.i + scroll_x2;
	if (currenttritt == -1) return;

	if (currenttritt < Data->MAXX2 - 1) {
		SwitchTritte(currenttritt, currenttritt + 1);
		cursorhandler->MoveCursorRight(1, false);
		if (TfBelassen) TfBelassen->setChecked(true);

		SetModified();
		refresh();
		if (undo) undo->Snapshot();
	}
}
