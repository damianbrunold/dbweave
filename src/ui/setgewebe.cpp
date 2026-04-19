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
    int i, j, k;
    bool wasemptyeinzug = IsEmptyEinzug(_i + scroll_x1);
    bool wasemptytrittfolge = IsEmptyTrittfolge(_j + scroll_y2);
    (void)wasemptyeinzug; /* used only by the stripped Canvas-draw tail */
    (void)wasemptytrittfolge;

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

    // Einzug loeschen
    int oldeinzug = einzug.feld.Get(_i + scroll_x1);
    einzug.feld.Set(_i + scroll_x1, 0);
    if (oldeinzug != 0) {
        freieschaefte[oldeinzug - 1] = true;
        for (int k = 0; k < Data->MAXX1; k++)
            if (einzug.feld.Get(k) == (short)oldeinzug) {
                freieschaefte[oldeinzug - 1] = false;
                break;
            }
    }

    // Bestimmen, ob Einzug berechnet werden muss
    bool bNeedEinzugRecalc = false;
    for (j = 0; j < Data->MAXY2; j++) {
        char s = gewebe.feld.Get(_i + scroll_x1, j);
        if (s > 0) {
            bNeedEinzugRecalc = true;
            break;
        }
    }

    // Einzug neuberechnen
    short neweinzug = 0;
    if (bNeedEinzugRecalc) {
        bool bEqualCol = false;
        for (i = 0; i < Data->MAXX1; i++) {
            if (i == _i + scroll_x1)
                continue;
            bEqualCol = true;
            for (j = 0; j < Data->MAXY2; j++) {
                if (IsEmptyEinzug(i)
                    || (gewebe.feld.Get(i, j) != gewebe.feld.Get(_i + scroll_x1, j))) {
                    bEqualCol = false;
                    break;
                }
            }
            if (bEqualCol)
                break;
        }
        if (bEqualCol)
            neweinzug = einzug.feld.Get(i);
        else
            neweinzug = GetFreeEinzug();
        einzug.feld.Set(_i + scroll_x1, neweinzug);
        if (neweinzug > 0)
            freieschaefte[neweinzug - 1] = false;
        if (!bEqualCol && ViewSchlagpatrone && ViewSchlagpatrone->isChecked()) {
            for (int j = 0; j < Data->MAXY2; j++) {
                char s = gewebe.feld.Get(_i + scroll_x1, j);
                if (s > 0) {
                    if (neweinzug != 0) {
                        trittfolge.feld.Set(neweinzug - 1, j, s);
                        dbw3_assert(neweinzug - 1 < Data->MAXX2);
                        freietritte[neweinzug - 1] = false;
                    }
                }
            }
        }
    }

    // Falls Schaft leer: Aufknuepfung anpassen!
    if (oldeinzug != 0) {
        if (freieschaefte[oldeinzug - 1]) {
            if (!ViewSchlagpatrone || !ViewSchlagpatrone->isChecked()) {
                for (i = 0; i < Data->MAXX2; i++)
                    aufknuepfung.feld.Set(i, oldeinzug - 1, 0);
            } else {
                for (j = 0; j < Data->MAXY2; j++) {
                    trittfolge.feld.Set(oldeinzug - 1, j, 0);
                    RecalcTrittfolgeEmpty(j);
                }
                dbw3_assert(oldeinzug - 1 < Data->MAXX2);
                freietritte[oldeinzug - 1] = true;
            }
        }
    }

    // Trittfolge loeschen
    for (i = 0; i < Data->MAXX2; i++) {
        char s = trittfolge.feld.Get(i, _j + scroll_y2);
        if (s > 0) {
            trittfolge.feld.Set(i, _j + scroll_y2, 0);
            bool bEmptyTritt = true;
            for (j = 0; j < Data->MAXY2; j++)
                if (trittfolge.feld.Get(i, j) > 0) {
                    bEmptyTritt = false;
                    break;
                }
            if (bEmptyTritt && (!ViewSchlagpatrone || !ViewSchlagpatrone->isChecked()))
                for (j = 0; j < Data->MAXY1; j++)
                    aufknuepfung.feld.Set(i, j, 0);
            if (bEmptyTritt)
                freietritte[i] = true;
        }
    }

    // Bestimmen, ob Trittfolge berechnet werden muss
    bool bNeedTrittfolgeRecalc = false;
    for (i = 0; i < Data->MAXX1; i++) {
        char s = gewebe.feld.Get(i, _j + scroll_y2);
        if (s > 0) {
            bNeedTrittfolgeRecalc = true;
            break;
        }
    }

    // Trittfolge neuberechnen
    if (bNeedTrittfolgeRecalc) {
        if (!ViewSchlagpatrone || !ViewSchlagpatrone->isChecked()) {
            bool bEqualRow = false;
            for (j = 0; j < Data->MAXY2; j++) {
                if (j == _j + scroll_y2)
                    continue;
                bEqualRow = true;
                if (IsEmptyTrittfolge(j))
                    bEqualRow = false;
                else {
                    for (i = 0; i < Data->MAXX1; i++) {
                        if (gewebe.feld.Get(i, j) != gewebe.feld.Get(i, _j + scroll_y2)) {
                            bEqualRow = false;
                            break;
                        }
                    }
                }
                if (bEqualRow)
                    break;
            }
            if (bEqualRow)
                CopyTritt(j, _j + scroll_y2);
            else {
                short freetritt = GetFreeTritt();
                if (freetritt != -1) {
                    trittfolge.feld.Set(freetritt, _j + scroll_y2, 1);
                    dbw3_assert(freetritt < Data->MAXX2);
                    freietritte[freetritt] = false;
                }
            }
        } else {
            for (int i = 0; i < Data->MAXX1; i++) {
                char c = gewebe.feld.Get(i, _j + scroll_y2);
                if (c > 0) {
                    int jj = einzug.feld.Get(i);
                    if (jj != 0) {
                        trittfolge.feld.Set(jj - 1, _j + scroll_y2, c);
                        dbw3_assert(jj - 1 < Data->MAXX2);
                        freietritte[jj - 1] = false;
                        trittfolge.isempty.Set(_j + scroll_y2, false);
                    }
                }
            }
        }
    }

    RecalcTrittfolgeEmpty(_j + scroll_y2);

    // Aufknuepfung nachfuehren
    if (!ViewSchlagpatrone || !ViewSchlagpatrone->isChecked()) {
        // Schussfaden
        for (i = 0; i < Data->MAXX1; i++)
            if (einzug.feld.Get(i))
                for (k = 0; k < Data->MAXX2; k++)
                    if (trittfolge.feld.Get(k, _j + scroll_y2) > 0)
                        aufknuepfung.feld.Set(k, einzug.feld.Get(i) - 1,
                                              gewebe.feld.Get(i, _j + scroll_y2));
        // Kettfaden
        if (einzug.feld.Get(_i + scroll_x1))
            for (j = 0; j < Data->MAXY2; j++)
                for (k = 0; k < Data->MAXX2; k++)
                    if (trittfolge.feld.Get(k, j) > 0)
                        aufknuepfung.feld.Set(k, einzug.feld.Get(_i + scroll_x1) - 1,
                                              gewebe.feld.Get(_i + scroll_x1, j));
    }
}
/*-----------------------------------------------------------------*/
