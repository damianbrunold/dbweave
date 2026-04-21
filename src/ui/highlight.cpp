/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port of legacy/highlight.cpp. Legacy repaints dependent cells
    with a red cell-color (via DrawEinzug/DrawTrittfolge/... which
    carry a TColor arg in the C++Builder version) while the user
    presses & holds the highlight toolbar button, and repaints them
    in black on release.

    The port models highlight as a modeless checkable toggle (F12
    from the main toolbar). When on, this routine -- called from
    PatternCanvas::paintEvent after the main render -- lays a
    translucent red rectangle on top of every dependent cell. Moving
    the keyboard cursor updates the overlay via the standard
    refresh() path. This keeps the per-cell Draw* routines
    colourless (they always paint in their natural colour).        */

#include "mainwindow.h"
#include "datamodule.h"

#include <QPainter>

/*-----------------------------------------------------------------*/
void TDBWFRM::DrawHighlight()
{
    if (!highlight || !currentPainter)
        return;

    const int gew_cols = gewebe.gw > 0 ? gewebe.pos.width / gewebe.gw : 0;
    const int gew_rows = gewebe.gh > 0 ? gewebe.pos.height / gewebe.gh : 0;
    const int ez_cols = einzug.gw > 0 ? einzug.pos.width / einzug.gw : 0;
    const int ez_rows = einzug.gh > 0 ? einzug.pos.height / einzug.gh : 0;
    const int au_cols = aufknuepfung.gw > 0 ? aufknuepfung.pos.width / aufknuepfung.gw : 0;
    const int au_rows = aufknuepfung.gh > 0 ? aufknuepfung.pos.height / aufknuepfung.gh : 0;
    const int tf_cols = trittfolge.gw > 0 ? trittfolge.pos.width / trittfolge.gw : 0;
    const int tf_rows = trittfolge.gh > 0 ? trittfolge.pos.height / trittfolge.gh : 0;

    /*  Local (0-based, visible-grid) → viewport rect. Mirrors the
        flip math in draw.cpp exactly:
          - DrawGewebe        righttoleft flips i; y always bottom-origin.
          - DrawEinzug        righttoleft flips i; toptobottom flips j.
          - DrawAufknuepfung  no i-flip; toptobottom switches y-origin.
          - DrawTrittfolge    no flips; y always bottom-origin.
        Getting this wrong produces a visibly mirrored highlight: the
        trittfolge → gewebe case below feeds trittfolge.kbd.j straight
        through as gewebe's local j, and gewebe doesn't honour
        toptobottom even when the flag is on.                        */
    auto rectGewebe = [&](int _i, int _j) -> QRect {
        if (_i < 0 || _j < 0 || _i >= gew_cols || _j >= gew_rows)
            return QRect();
        int vi = _i;
        if (righttoleft)
            vi = gew_cols - 1 - vi;
        const int x = gewebe.pos.x0 + vi * gewebe.gw;
        const int y = gewebe.pos.y0 + gewebe.pos.height - (_j + 1) * gewebe.gh;
        return QRect(x, y, gewebe.gw, gewebe.gh);
    };
    auto rectEinzug = [&](int _i, int _j) -> QRect {
        if (_i < 0 || _j < 0 || _i >= ez_cols || _j >= ez_rows)
            return QRect();
        int vi = _i;
        if (righttoleft)
            vi = ez_cols - 1 - vi;
        int vj = _j;
        if (toptobottom)
            vj = ez_rows - 1 - vj;
        const int x = einzug.pos.x0 + vi * einzug.gw;
        const int y = einzug.pos.y0 + einzug.pos.height - (vj + 1) * einzug.gh;
        return QRect(x, y, einzug.gw, einzug.gh);
    };
    auto rectAufknuepfung = [&](int _i, int _j) -> QRect {
        if (_i < 0 || _j < 0 || _i >= au_cols || _j >= au_rows)
            return QRect();
        const int x = aufknuepfung.pos.x0 + _i * aufknuepfung.gw;
        int y;
        if (toptobottom)
            y = aufknuepfung.pos.y0 + _j * aufknuepfung.gh;
        else
            y = aufknuepfung.pos.y0 + aufknuepfung.pos.height - (_j + 1) * aufknuepfung.gh;
        return QRect(x, y, aufknuepfung.gw, aufknuepfung.gh);
    };
    auto rectTrittfolge = [&](int _i, int _j) -> QRect {
        if (_i < 0 || _j < 0 || _i >= tf_cols || _j >= tf_rows)
            return QRect();
        const int x = trittfolge.pos.x0 + _i * trittfolge.gw;
        const int y = trittfolge.pos.y0 + trittfolge.pos.height - (_j + 1) * trittfolge.gh;
        return QRect(x, y, trittfolge.gw, trittfolge.gh);
    };

    QPainter& p = *currentPainter;
    p.save();
    const QColor overlay(255, 0, 0, 110);
    auto paint = [&](const QRect& _r) {
        if (!_r.isEmpty())
            p.fillRect(_r, overlay);
    };

    const bool pegplan = ViewSchlagpatrone && ViewSchlagpatrone->isChecked();
    const bool normalMode = GewebeNormal && GewebeNormal->isChecked();

    /*  Direct structural port of legacy _DrawHighlight.            */
    if (kbd_field == GEWEBE) {
        const int i = gewebe.kbd.i + scroll_x1;
        const int j = gewebe.kbd.j + scroll_y2;
        const short ez = einzug.feld.Get(i);
        if (ez > scroll_y1 && ez <= ez_rows + scroll_y1) {
            paint(rectEinzug(i - scroll_x1, ez - 1 - scroll_y1));
            if (!pegplan) {
                for (int ii = scroll_x2; ii < scroll_x2 + tf_cols; ii++) {
                    const char s = trittfolge.feld.Get(ii, j);
                    if (s > 0) {
                        const char s1 = gewebe.feld.Get(i, j);
                        if (s1 == 0 || aufknuepfung.feld.Get(ii, ez - 1) > 0) {
                            paint(rectTrittfolge(ii - scroll_x2, j - scroll_y2));
                            paint(rectAufknuepfung(ii - scroll_x2, ez - 1 - scroll_y1));
                        }
                    }
                }
            } else if (ez - 1 >= scroll_x2 && ez - 1 < scroll_x2 + tf_cols) {
                paint(rectTrittfolge(ez - 1 - scroll_x2, j - scroll_y2));
            }
        }
    } else if (kbd_field == AUFKNUEPFUNG) {
        const int i = aufknuepfung.kbd.i + scroll_x2;
        const int j = aufknuepfung.kbd.j + scroll_y1;
        for (int ii = scroll_x1; ii < scroll_x1 + ez_cols; ii++) {
            if (einzug.feld.Get(ii) == j + 1) {
                paint(rectEinzug(ii - scroll_x1, j - scroll_y1));
                if (normalMode) {
                    for (int jj = scroll_y2; jj < scroll_y2 + tf_rows; jj++) {
                        if (trittfolge.feld.Get(i, jj) > 0) {
                            paint(rectTrittfolge(i - scroll_x2, jj - scroll_y2));
                            paint(rectGewebe(ii - scroll_x1, jj - scroll_y2));
                        }
                    }
                }
            }
        }
    } else if (kbd_field == TRITTFOLGE) {
        if (pegplan) {
            for (int ii = scroll_x1; ii < scroll_x1 + gew_cols; ii++) {
                if (einzug.feld.Get(ii) == trittfolge.kbd.i + scroll_x2 + 1)
                    paint(rectGewebe(ii - scroll_x1, trittfolge.kbd.j));
            }
            if (ViewEinzug && ViewEinzug->isChecked()) {
                for (int ii = scroll_x1; ii < scroll_x1 + ez_cols; ii++) {
                    if (einzug.feld.Get(ii) == trittfolge.kbd.i + scroll_x2 + 1)
                        paint(rectEinzug(ii - scroll_x1, trittfolge.kbd.i));
                }
            }
        } else {
            for (int ii = scroll_x1; ii < scroll_x1 + gew_cols; ii++) {
                if (gewebe.feld.Get(ii, trittfolge.kbd.j + scroll_y2) > 0)
                    paint(rectGewebe(ii - scroll_x1, trittfolge.kbd.j));
            }
            for (int jj = scroll_y1; jj < scroll_y1 + au_rows; jj++) {
                if (aufknuepfung.feld.Get(trittfolge.kbd.i + scroll_x2, jj) > 0)
                    paint(rectAufknuepfung(trittfolge.kbd.i, jj - scroll_y1));
            }
        }
    } else if (kbd_field == EINZUG) {
        if (normalMode) {
            for (int jj = scroll_y2; jj < scroll_y2 + gew_rows; jj++) {
                if (gewebe.feld.Get(einzug.kbd.i + scroll_x1, jj) > 0)
                    paint(rectGewebe(einzug.kbd.i, jj));
            }
        }
        if (!pegplan) {
            for (int ii = scroll_x2; ii < scroll_x2 + au_cols; ii++) {
                if (aufknuepfung.feld.Get(ii, einzug.kbd.j + scroll_y1) > 0)
                    paint(rectAufknuepfung(ii - scroll_x2, einzug.kbd.j));
            }
        }
    }

    p.restore();
}
