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
    if (GewebeLocked())
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

        /*  Replicate the gewebe cell to every other rapport position.
            einzug / trittfolge are rebuilt from the resulting gewebe
            by the single RecalcAll() below, so no manual einzug /
            trittfolge fixup is needed here.                        */
        int i = i0;
        while (i <= kette.b) {
            int j = j0;
            while (j <= schuesse.b) {
                if (i != _i + scroll_x1 || j != _j + scroll_y2) {
                    if (!_set)
                        ToggleGewebe(i, j);
                    else
                        gewebe.feld.Set(i, j, char(_range));
                }
                j += rapport.sr.count();
            }
            i += rapport.kr.count();
        }
    }

    /*  Single recalc over the final gewebe -- running it inside
        DoSetGewebe would see only the first cell and allocate stray
        shafts / treadles for the unreplicated rapport copies.      */
    RecalcAll();

    UpdateRapport();

    setCursor(oldcursor);
    SetModified();
    refresh();

    dbw3_assert(undo != 0);
    if (undo)
        undo->Snapshot();
}
/*-----------------------------------------------------------------*/
/*  DoSetGewebe only mutates the single gewebe cell + updates the
    range. The caller is responsible for running RecalcAll() once
    after all related cell writes (rapport replication, FillKoeper
    diagonal replication, ...) have completed.                     */
void TDBWFRM::DoSetGewebe(int _i, int _j, bool _set, int _range)
{
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
}
/*-----------------------------------------------------------------*/
