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
#include "datamodule.h"

#include <Qt>
#include <algorithm>

/*  CrFeld::Set equivalent -- unconditional write at the current
    cursor position. Used by digit 1..9 (write currentrange) and
    digit 0 (write 0 -- hard clear). Self-toggle fields (EINZUG /
    BLATTEINZUG) and colour strips only react when _set is true,
    matching the legacy CrFeld::Set (which only has cases for
    GEWEBE / AUFKNUEPFUNG / TRITTFOLGE).                         */
static void applyAtCursor(TDBWFRM* frm, bool _set, int _range)
{
    switch (frm->kbd_field) {
    case GEWEBE:
        frm->SetGewebe(frm->gewebe.kbd.i, frm->gewebe.kbd.j, _set, _range);
        break;
    case EINZUG:
        if (_set)
            frm->SetEinzug(frm->einzug.kbd.i, frm->einzug.kbd.j);
        break;
    case AUFKNUEPFUNG:
        frm->SetAufknuepfung(frm->aufknuepfung.kbd.i, frm->aufknuepfung.kbd.j, _set, _range);
        break;
    case TRITTFOLGE:
        frm->SetTrittfolge(frm->trittfolge.kbd.i, frm->trittfolge.kbd.j, _set, _range);
        break;
    case BLATTEINZUG:
        if (_set)
            frm->SetBlatteinzug(frm->blatteinzug.kbd.i);
        break;
    case KETTFARBEN:
        if (_set)
            frm->SetKettfarben(frm->kettfarben.kbd.i);
        break;
    case SCHUSSFARBEN:
        if (_set)
            frm->SetSchussfarben(frm->schussfarben.kbd.j);
        break;
    default:
        break;
    }
}


void TDBWFRM::handleCanvasKeyPress(int _key, int _modifiers)
{
    if (!cursorhandler)
        return;

    const bool shift = (_modifiers & Qt::ShiftModifier) != 0;
    /*  The cursor API takes a TShiftState compat object; use the
        select-only convention from the keyboard layer. The Move*
        methods' _select flag comes through as shift only.          */

    /*  Ctrl / Alt widens the arrow step to speed navigation.
        Legacy used the field's strongline distance; the port fixes
        it at 4 / 8 cells which matches the default strongline=4. */
    const bool alt = (_modifiers & Qt::AltModifier) != 0;
    const bool ctrl = (_modifiers & Qt::ControlModifier) != 0;
    const int step = alt ? 8 : (ctrl ? 4 : 1);

    switch (_key) {
    case Qt::Key_Left:
        cursorhandler->MoveCursorLeft(step, shift);
        refresh();
        return;
    case Qt::Key_Right:
        cursorhandler->MoveCursorRight(step, shift);
        refresh();
        return;
    case Qt::Key_Up:
        cursorhandler->MoveCursorUp(step, shift);
        refresh();
        return;
    case Qt::Key_Down:
        cursorhandler->MoveCursorDown(step, shift);
        refresh();
        return;

    case Qt::Key_Space: {
        /*  Verbatim legacy dispatch: Space -> ToggleField(shift)
            -> CrFeld::Toggle (see src/ui/cursor.cpp). Shift+Space
            on KETTFARBEN / SCHUSSFARBEN picks the colour under the
            cursor into Data->color.                             */
        int bits = 0;
        if (shift)
            bits |= ssShift;
        if (_modifiers & Qt::ControlModifier)
            bits |= ssCtrl;
        if (_modifiers & Qt::AltModifier)
            bits |= ssAlt;
        cursorhandler->ToggleField(TShiftState(bits));
        refresh();
        return;
    }

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

    case Qt::Key_Home: {
        /*  Jump cursor to the first visible column of the active
            field; scroll to expose column 0 when already there.   */
        FeldBase* fb = nullptr;
        int* sx = nullptr;
        int sy = 0;
        FELD f = kbd_field;
        switch (kbd_field) {
        case GEWEBE:
            fb = &gewebe;
            sx = &scroll_x1;
            sy = scroll_y2;
            break;
        case EINZUG:
            fb = &einzug;
            sx = &scroll_x1;
            sy = scroll_y1;
            break;
        case BLATTEINZUG:
            fb = &blatteinzug;
            sx = &scroll_x1;
            sy = 0;
            break;
        case KETTFARBEN:
            fb = &kettfarben;
            sx = &scroll_x1;
            sy = 0;
            break;
        case AUFKNUEPFUNG:
            fb = &aufknuepfung;
            sx = &scroll_x2;
            sy = scroll_y1;
            break;
        case TRITTFOLGE:
            fb = &trittfolge;
            sx = &scroll_x2;
            sy = scroll_y2;
            break;
        default:
            return;
        }
        if (fb && sx) {
            if (fb->kbd.i > 0) {
                cursorhandler->SetCursor(f, *sx, sy + fb->kbd.j, true);
            } else {
                *sx = 0;
                cursorhandler->SetCursor(f, 0, sy + fb->kbd.j, true);
            }
            refresh();
        }
        return;
    }

    case Qt::Key_End: {
        FeldBase* fb = nullptr;
        int* sx = nullptr;
        int sy = 0;
        int maxx = 0;
        FELD f = kbd_field;
        switch (kbd_field) {
        case GEWEBE:
            fb = &gewebe;
            sx = &scroll_x1;
            sy = scroll_y2;
            maxx = Data->MAXX1;
            break;
        case EINZUG:
            fb = &einzug;
            sx = &scroll_x1;
            sy = scroll_y1;
            maxx = Data->MAXX1;
            break;
        case BLATTEINZUG:
            fb = &blatteinzug;
            sx = &scroll_x1;
            sy = 0;
            maxx = Data->MAXX1;
            break;
        case KETTFARBEN:
            fb = &kettfarben;
            sx = &scroll_x1;
            sy = 0;
            maxx = Data->MAXX1;
            break;
        case AUFKNUEPFUNG:
            fb = &aufknuepfung;
            sx = &scroll_x2;
            sy = scroll_y1;
            maxx = Data->MAXX2;
            break;
        case TRITTFOLGE:
            fb = &trittfolge;
            sx = &scroll_x2;
            sy = scroll_y2;
            maxx = Data->MAXX2;
            break;
        default:
            return;
        }
        if (fb && sx) {
            const int cols = fb->gw > 0 ? fb->pos.width / fb->gw : 0;
            if (fb->kbd.i < cols - 1) {
                cursorhandler->SetCursor(f, *sx + cols - 1, sy + fb->kbd.j, true);
            } else {
                *sx = std::max(0, maxx - cols);
                cursorhandler->SetCursor(f, maxx - 1, sy + fb->kbd.j, true);
            }
            refresh();
        }
        return;
    }

    case Qt::Key_PageUp: {
        /*  The y axis grows up in dbweave; PageUp moves the cursor
            toward higher j. */
        FeldBase* fb = nullptr;
        int* sy = nullptr;
        int sx = 0;
        int maxy = 0;
        FELD f = kbd_field;
        switch (kbd_field) {
        case GEWEBE:
            fb = &gewebe;
            sx = scroll_x1;
            sy = &scroll_y2;
            maxy = Data->MAXY2;
            break;
        case EINZUG:
            fb = &einzug;
            sx = scroll_x1;
            sy = &scroll_y1;
            maxy = Data->MAXY1;
            break;
        case AUFKNUEPFUNG:
            fb = &aufknuepfung;
            sx = scroll_x2;
            sy = &scroll_y1;
            maxy = Data->MAXY1;
            break;
        case TRITTFOLGE:
            fb = &trittfolge;
            sx = scroll_x2;
            sy = &scroll_y2;
            maxy = Data->MAXY2;
            break;
        case SCHUSSFARBEN:
            fb = &schussfarben;
            sx = 0;
            sy = &scroll_y2;
            maxy = Data->MAXY2;
            break;
        default:
            return;
        }
        if (fb && sy) {
            const int rows = fb->gh > 0 ? fb->pos.height / fb->gh : 0;
            if (fb->kbd.j < rows - 1) {
                cursorhandler->SetCursor(f, sx + fb->kbd.i, *sy + rows - 1, true);
            } else {
                *sy = std::min(*sy + rows, std::max(0, maxy - rows));
                cursorhandler->SetCursor(f, sx + fb->kbd.i, *sy + rows - 1, true);
            }
            refresh();
        }
        return;
    }

    case Qt::Key_PageDown: {
        FeldBase* fb = nullptr;
        int* sy = nullptr;
        int sx = 0;
        FELD f = kbd_field;
        switch (kbd_field) {
        case GEWEBE:
            fb = &gewebe;
            sx = scroll_x1;
            sy = &scroll_y2;
            break;
        case EINZUG:
            fb = &einzug;
            sx = scroll_x1;
            sy = &scroll_y1;
            break;
        case AUFKNUEPFUNG:
            fb = &aufknuepfung;
            sx = scroll_x2;
            sy = &scroll_y1;
            break;
        case TRITTFOLGE:
            fb = &trittfolge;
            sx = scroll_x2;
            sy = &scroll_y2;
            break;
        case SCHUSSFARBEN:
            fb = &schussfarben;
            sx = 0;
            sy = &scroll_y2;
            break;
        default:
            return;
        }
        if (fb && sy) {
            const int rows = fb->gh > 0 ? fb->pos.height / fb->gh : 0;
            if (fb->kbd.j > 0) {
                cursorhandler->SetCursor(f, sx + fb->kbd.i, *sy, true);
            } else {
                *sy = std::max(0, *sy - rows);
                cursorhandler->SetCursor(f, sx + fb->kbd.i, *sy, true);
            }
            refresh();
        }
        return;
    }
    }

    if (_key >= Qt::Key_0 && _key <= Qt::Key_9) {
        const int digit = _key - Qt::Key_0;
        const bool ctrlShift
            = (_modifiers & Qt::ControlModifier) && (_modifiers & Qt::ShiftModifier);
        if (digit == 0) {
            /*  '0' always clears the cell under the cursor, regardless
                of any active selection. */
            applyAtCursor(this, /*_set=*/false, 0);
        } else if (ctrlShift && selection.Valid()) {
            /*  Ctrl+Shift+digit: re-colour non-zero cells in the
                selection (legacy SwitchRange). */
            SwitchRange(digit);
        } else if (selection.Valid()) {
            /*  Fill the current selection with the picked range. */
            currentrange = digit;
            ApplyRangeToSelection(digit);
        } else {
            currentrange = digit;
        }
        UpdateStatusBar();
        refresh();
    }
}
