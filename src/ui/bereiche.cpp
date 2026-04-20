/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Phase 5 gap-fill: handler ports of legacy
      - schlagpatrone.cpp (3 clickhandlers)
      - einzugtrittfolge.cpp (2 clickhandlers)
      - trittfolge.cpp (ClearTrittfolgeClick, TfSpiegelnClick)
      - bereiche.cpp (RangePatternsClick, BereicheFillPattern)
    Plus the bereichmuster / bereichundo state the range-substitution
    dialog needs.                                                 */

#include "mainwindow.h"
#include "datamodule.h"
#include "undoredo.h"
#include "cursor.h"
#include "blockmusterdialog.h"
#include "language.h"
#include "assert_compat.h"

#include <algorithm>

/*-----------------------------------------------------------------*/
/*  schlagpatrone.cpp ports. */
void TDBWFRM::ClearTrittfolgeClick()
{
    /*  Not gated by GewebeLocked: this clears trittfolge (and the
        corresponding gewebe rows in lockstep) without triggering a
        recalc-from-gewebe, so it doesn't regenerate einzug /
        trittfolge / aufknuepfung from scratch. Matches legacy. */
    for (int j = 0; j < Data->MAXY2; j++) {
        for (int i = 0; i < Data->MAXX2; i++)
            trittfolge.feld.Set(i, j, 0);
        for (int i = 0; i < Data->MAXX1; i++)
            gewebe.feld.Set(i, j, 0);
    }
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

void TDBWFRM::EzSpiegelnClick()
{
    /*  Mirror einzug (and the gewebe warp columns) horizontally across
        [kette.a, kette.b]. Legacy additionally flipped EzBelassen to
        keep the manually mirrored shaft numbering across the next
        recalc, but EzBelassen is now folded into EzMinimalZ so that
        guard is a no-op. Not gated by GewebeLocked: legacy reserved
        the pattern-lock check for interactive cell edits (cursor /
        mouse), not menu-driven mirror ops. */
    for (int i = kette.a; i <= (kette.b - kette.a) / 2; i++) {
        const int mirror = kette.b - (i - kette.a);
        short s = einzug.feld.Get(i);
        einzug.feld.Set(i, einzug.feld.Get(mirror));
        einzug.feld.Set(mirror, s);
        for (int j = schuesse.a; j <= schuesse.b; j++) {
            char g = gewebe.feld.Get(i, j);
            gewebe.feld.Set(i, j, gewebe.feld.Get(mirror, j));
            gewebe.feld.Set(mirror, j, g);
        }
    }
    UpdateRapport();
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

void TDBWFRM::TfSpiegelnClick()
{
    /*  TfBelassen flip from the legacy implementation dropped for the
        same reason as EzSpiegelnClick: TfBelassen now routes through
        TfMinimalZ, so forcing it would not preserve the mirrored
        treadle numbering anyway. Not gated by GewebeLocked: legacy
        reserved the pattern-lock check for interactive cell edits,
        not menu-driven mirror ops. */
    for (int j = schuesse.a; j <= (schuesse.b - schuesse.a) / 2; j++) {
        for (int i = 0; i < Data->MAXX2; i++) {
            char s = trittfolge.feld.Get(i, j);
            trittfolge.feld.Set(i, j, trittfolge.feld.Get(i, schuesse.b - (j - schuesse.a)));
            trittfolge.feld.Set(i, schuesse.b - (j - schuesse.a), s);
        }
        for (int i = kette.a; i <= kette.b; i++) {
            char s = gewebe.feld.Get(i, j);
            gewebe.feld.Set(i, j, gewebe.feld.Get(i, schuesse.b - (j - schuesse.a)));
            gewebe.feld.Set(i, schuesse.b - (j - schuesse.a), s);
        }
    }
    UpdateRapport();
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

void TDBWFRM::ClearSchlagpatroneClick()
{
    ClearTrittfolgeClick();
}
void TDBWFRM::SpSpiegelnClick()
{
    TfSpiegelnClick();
}

void TDBWFRM::SpInvertClick()
{
    if (!ViewSchlagpatrone || !ViewSchlagpatrone->isChecked())
        return;
    /*  Not gated by GewebeLocked: inverts only the trittfolge, never
        touches gewebe, and never triggers RecalcAll.              */

    const int t1 = GetFirstTritt();
    const int t2 = GetLastTritt();
    if (t2 < t1)
        return;

    for (int j = schuesse.a; j <= schuesse.b; j++) {
        for (int i = t1; i <= t2; i++) {
            char s = trittfolge.feld.Get(i, j);
            trittfolge.feld.Set(i, j, char(s == 0 ? currentrange : -s));
        }
        for (int i = kette.a; i <= kette.b; i++) {
            char s = gewebe.feld.Get(i, j);
            gewebe.feld.Set(i, j, char(s == 0 ? currentrange : -s));
        }
    }
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
/*  einzugtrittfolge.cpp — "Copy threading ↔ treadling" helpers. */
void TDBWFRM::CopyEinzugTrittfolgeClick()
{
    /*  Clear einzug. */
    for (int i = 0; i < Data->MAXX1; i++)
        einzug.feld.Set(i, 0);

    /*  Copy first-set treadle column of each weft row into einzug. */
    const int maxi = std::min(int(Data->MAXY2), int(Data->MAXX1));
    for (int i = 0; i < maxi; i++) {
        int schaft = 0;
        for (int ii = 0; ii < Data->MAXX2; ii++)
            if (trittfolge.feld.Get(ii, i) > 0) {
                schaft = ii + 1;
                break;
            }
        einzug.feld.Set(i, short(schaft));
    }

    CalcRange();
    CalcRapport();
    RecalcGewebe();
    refresh();
    if (undo)
        undo->Snapshot();
    SetModified();
}

void TDBWFRM::CopyTrittfolgeEinzugClick()
{
    /*  Clear trittfolge. */
    for (int j = 0; j < Data->MAXY2; j++) {
        for (int i = 0; i < Data->MAXX2; i++)
            trittfolge.feld.Set(i, j, 0);
    }

    /*  Copy einzug shaft number into the matching treadle column. */
    const int maxj = std::min(int(Data->MAXY2), int(Data->MAXX1));
    for (int j = 0; j < maxj; j++) {
        const int schaft = einzug.feld.Get(j);
        if (schaft != 0)
            trittfolge.feld.Set(schaft - 1, j, 1);
    }

    CalcRange();
    CalcRapport();
    RecalcGewebe();
    refresh();
    if (undo)
        undo->Snapshot();
    SetModified();
}

/*-----------------------------------------------------------------*/
/*  bereiche.cpp — range-substitution: open the Blockmuster dialog
    in withrange=false mode on the bereichmuster slots, then replay
    them over the selection on Apply.                               */
void TDBWFRM::RangePatternsClick()
{
    if (GewebeLocked())
        return;
    if (cursorhandler)
        cursorhandler->DisableCursor();
    if (!bereichundo) {
        if (cursorhandler)
            cursorhandler->EnableCursor();
        return;
    }

    BlockmusterDialog dlg(this, *bereichundo, &bereichmuster, currentbm,
                          LANG_STR("Substitute ranges with patterns", "Bereichsmusterung"),
                          /*withRange=*/false);
    if (dlg.exec() == QDialog::Accepted) {
        const int x = dlg.mx + 1;
        const int y = dlg.my + 1;

        if (ViewSchlagpatrone)
            ViewSchlagpatrone->setChecked(true);
        trittfolge.einzeltritt = false;

        BereicheFillPattern(x, y);

        CalcRange();
        CalcRapport();
        RecalcAll();

        refresh();
        if (undo)
            undo->Snapshot();
    }
    SetModified();
    if (cursorhandler)
        cursorhandler->EnableCursor();
}

void TDBWFRM::BereicheFillPattern(int _x, int _y)
{
    (void)_x;
    (void)_y;
    int i0, i1, j0, j1;

    RANGE savesel = selection;
    selection.Normalize();
    if (selection.Valid()) {
        i0 = selection.begin.i;
        i1 = selection.end.i;
        j0 = selection.begin.j;
        j1 = selection.end.j;
    } else {
        i0 = kette.a;
        i1 = kette.b;
        j0 = schuesse.a;
        j1 = schuesse.b;
    }
    selection = savesel;

    for (int i = i0; i <= i1; i++) {
        for (int j = j0; j <= j1; j++) {
            const char bindung = gewebe.feld.Get(i, j);
            if (bindung < 0 || bindung >= 10)
                continue;
            const int sx = bereichmuster[int(bindung)].SizeX();
            const int sy = bereichmuster[int(bindung)].SizeY();
            if (sx == 0 || sy == 0)
                continue;
            gewebe.feld.Set(i, j, bereichmuster[int(bindung)].Get(i % sx, j % sy));
        }
    }
}
