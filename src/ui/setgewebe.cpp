/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  The big one. Toggling a gewebe (fabric) cell cascades through:
      * einzug reassignment / equality search / free-shaft allocation
      * trittfolge reassignment / equality search / free-treadle allocation
      * aufknuepfung patch-up (when not in pegplan view)
      * range update
      * optional rapport replication across kette.a..b x schuesse.a..b
      * rearrange / eliminate-empty / minimize passes.
    Ported data-faithfully; all Canvas paint calls are dropped in favour
    of a single update() at the top-level SetGewebe tail.
*/

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "undoredo.h"
#include "einzug.h"
#include <QCursor>
/*-----------------------------------------------------------------*/
void TDBWFRM::SetGewebe(int _i, int _j, bool _set, int _range)
{
    if (GewebeNone && GewebeNone->isChecked())
        return;

    QCursor oldcursor = cursor();
    setCursor(Qt::WaitCursor);

    // Punkt setzen
    DoSetGewebe(_i, _j, _set, _range);

    // ggf. Punkt replizieren!
    if (RappViewRapport && RappViewRapport->isChecked()
        && IsInRapport(_i + scroll_x1, _j + scroll_y2)) {
        int i0 = _i + scroll_x1;
        int j0 = _j + scroll_y2;

        if (i0 > kette.a) {
            while (i0 > kette.a)
                i0 -= rapport.kr.count();
            if (i0 < kette.a)
                i0 += rapport.kr.count();
        }

        if (j0 > schuesse.a) {
            while (j0 > schuesse.a)
                j0 -= rapport.sr.count();
            if (j0 < schuesse.a)
                j0 += rapport.sr.count();
        }

        int i = i0;
        while (i <= kette.b) {
            int j = j0;
            while (j <= schuesse.b) {
                if (i != _i + scroll_x1 || j != _j + scroll_y2) {
                    // Gewebepunkt toggeln
                    if (!_set)
                        ToggleGewebe(i, j);
                    else
                        gewebe.feld.Set(i, j, char(_range));

                    // Einzug kopieren
                    einzug.feld.Set(i, einzug.feld.Get(_i + scroll_x1));

                    // Tritt kopieren
                    for (int ii = 0; ii < Data->MAXX2; ii++)
                        trittfolge.feld.Set(ii, j, trittfolge.feld.Get(ii, _j + scroll_y2));
                    trittfolge.isempty.Set(j, trittfolge.isempty.Get(_j + scroll_y2));
                }
                j += rapport.sr.count();
            }
            i += rapport.kr.count();
        }
        RecalcFreieSchaefte();
        RecalcFreieTritte();
    }

    RearrangeSchaefte();

    if (!ViewSchlagpatrone || !ViewSchlagpatrone->isChecked()) {
        EliminateEmptyTritt();
        RearrangeTritte();
    }

    UpdateRapport();

    setCursor(oldcursor);
    SetModified();
    refresh();

    dbw3_assert(undo != 0);
    if (undo)
        undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DoSetGewebe(int _i, int _j, bool _set, int _range)
{
    // Feld toggeln
    char oldstate = gewebe.feld.Get(_i + scroll_x1, _j + scroll_y2);
    if (!_set)
        gewebe.feld.Set(_i + scroll_x1, _j + scroll_y2,
                        char(oldstate <= 0 ? currentrange : -oldstate));
    else
        gewebe.feld.Set(_i + scroll_x1, _j + scroll_y2, char(_range));
    if (!_set)
        UpdateRange(_i + scroll_x1, _j + scroll_y2, oldstate <= 0);
    else
        UpdateRange(_i + scroll_x1, _j + scroll_y2, _range > 0);

    /*  The legacy hand-rolled rebuild (find-identical-column /
        reuse-shaft / per-column aufknuepfung writes / pegplan-branch
        trittfolge writes / freieschaefte+freietritte bookkeeping)
        is superseded by a full RcRecalcAll pass. RecalcEinzug /
        RecalcTrittfolge already reuse matching shafts / treadles via
        KettfadenEqual / SchussfadenEqual and rebuild the aufknuepfung
        from scratch. Identity assignment in the einzug / trittfolge
        strips may renumber compared to legacy's "closest match" rule;
        the underlying weave is unchanged.                           */
    RecalcAll();
}
/*-----------------------------------------------------------------*/
