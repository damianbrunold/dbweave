/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  First rendering slice. Ports just TDBWFRM::DrawGewebe with its
    GewebeNormal branch -- the view that paints each cell in its
    range colour on a clBtnFace background. The more specialised
    branches (DrawGewebeFarbeffekt, DrawGewebeSimulation,
    DrawGewebeRapport, Inverserepeat, rapport highlight) and the
    DrawEinzug / DrawAufknuepfung / DrawTrittfolge / colour-strip
    peers land in subsequent slices.

    DrawGewebe takes local (i, j) relative to the visible grid and
    looks up the absolute gewebe.feld cell via
    (scroll_x1 + _i, scroll_y2 + _j). The active QPainter comes via
    TDBWFRM::currentPainter, set by PatternCanvas::paintEvent. If the
    pointer is null (call outside paintEvent) the routine is a
    no-op -- matching what the legacy code would have done if you
    called DrawGewebe with no active form.
*/

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "draw_cell.h"
#include "rangecolors.h"
#include "colors_compat.h"
#include "legacy_colors.h"
#include "palette.h"

#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QRect>
/*-----------------------------------------------------------------*/
/*  Helper: look up a palette-indexed thread colour. Extracted so the
    Farbeffekt / Simulation paths share the same lookup as the legacy
    GETPALCOL macro (((TColor)Data->palette->GetColor(idx))).         */
static QColor palCol(int _idx)
{
    return qColorFromTColor((TColor)Data->palette->GetColor(_idx));
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DrawGewebe(int _i, int _j)
{
    QPainter* p = currentPainter;
    if (!p)
        return;

    int i = _i;
    if (righttoleft && gewebe.gw > 0) {
        i = gewebe.pos.width / gewebe.gw - i - 1;
    }

    const int x = gewebe.pos.x0 + i * gewebe.gw;
    const int y = gewebe.pos.y0 + gewebe.pos.height - (_j + 1) * gewebe.gh;
    const int xx = x + gewebe.gw;
    const int yy = y + gewebe.gh;

    const QColor bkground = legacyBtnFace();

    /*  Empty einzug OR empty trittfolge -> blank cell. */
    if (IsEmptyEinzug(scroll_x1 + _i) || IsEmptyTrittfolge(scroll_y2 + _j)) {
        p->fillRect(QRect(x + 1, y + 1, xx - x - 1, yy - y - 1), bkground);
        return;
    }

    /*  Rapport highlight takes precedence over view mode: inside
        the rapport rectangle, positive-range cells paint red on a
        btnFace background so the repeat boundary is visible.
        Matches the legacy ordering: IsInRapport + RappViewRapport
        + not GewebeNone -> DrawGewebeRapport.                    */
    const bool none_checked = GewebeNone && GewebeNone->isChecked();
    const bool show_rapport = RappViewRapport && RappViewRapport->isChecked();
    if (!none_checked && show_rapport && IsInRapport(scroll_x1 + _i, scroll_y2 + _j)) {
        DrawGewebeRapport(_i, _j, x, y, xx, yy);
        return;
    }

    /*  Dispatch on the three Gewebe* view-mode QActions. Legacy
        exposed these as a radio group; the Qt port leaves mutually-
        exclusive management to the (pending) menu port. If none is
        checked, fall through to the Normal path. */
    if (GewebeFarbeffekt && GewebeFarbeffekt->isChecked()) {
        DrawGewebeFarbeffekt(_i, _j, x, y, xx, yy);
        return;
    }
    if (GewebeSimulation && GewebeSimulation->isChecked()) {
        DrawGewebeSimulation(_i, _j, x, y, xx, yy);
        return;
    }

    /*  Default (GewebeNormal): paint the range colour. */
    const int range = gewebe.feld.Get(scroll_x1 + _i, scroll_y2 + _j);
    if (range <= 0) {
        p->fillRect(QRect(x + 1, y + 1, xx - x - 1, yy - y - 1), bkground);
        return;
    }
    p->fillRect(QRect(x + 1, y + 1, xx - x - 1, yy - y - 1),
                qColorFromTColor(GetRangeColor(range)));
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DrawGewebeRapport(int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    QPainter* p = currentPainter;
    if (!p)
        return;

    const char s = gewebe.feld.Get(scroll_x1 + _i, scroll_y2 + _j);
    const QColor fg = (s != ABBINDUNG && s > 0) ? QColor(Qt::red) : legacyBtnFace();
    p->fillRect(QRect(_x + 1, _y + 1, _xx - _x - 1, _yy - _y - 1), fg);
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DrawGewebeFarbeffekt(int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    QPainter* p = currentPainter;
    if (!p)
        return;

    const char s = gewebe.feld.Get(scroll_x1 + _i, scroll_y2 + _j);
    bool drawhebung = s > 0 && s != ABBINDUNG;
    if (sinkingshed)
        drawhebung = !drawhebung;

    const QColor col = drawhebung ? palCol(kettfarben.feld.Get(scroll_x1 + _i))
                                  : palCol(schussfarben.feld.Get(scroll_y2 + _j));

    p->fillRect(QRect(_x, _y, _xx - _x, _yy - _y), col);

    if (fewithraster)
        DrawGewebeRahmen(_i, _j);
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DrawGewebeSimulation(int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    QPainter* p = currentPainter;
    if (!p)
        return;

    /*  Thread-width: scale with cell size (gw/5) but give small
        cells minimum widths per legacy zoom table.                */
    int dw = gewebe.gw / 5;
    int dh = gewebe.gh / 5;
    if (faktor_kette == 1.0f && faktor_schuss == 1.0f) {
        switch (currentzoom) {
        case 0:
            dw = dh = 1;
            break;
        case 1:
        case 2:
        case 3:
            dw = dh = 2;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            dw = dh = 3;
            break;
        case 8:
        case 9:
            dw = dh = 4;
            break;
        }
    }
    if (dw == 0 && gewebe.gw > 2)
        dw = 1;
    if (dh == 0 && gewebe.gh > 2)
        dh = 1;

    const QColor bkg = legacyBtnFace();
    p->fillRect(QRect(_x, _y, _xx - _x, _yy - _y), bkg);

    const char s = gewebe.feld.Get(scroll_x1 + _i, scroll_y2 + _j);
    bool drawhebung = s > 0 && s != ABBINDUNG;
    if (sinkingshed)
        drawhebung = !drawhebung;

    const QColor kc = palCol(kettfarben.feld.Get(scroll_x1 + _i));
    const QColor sc = palCol(schussfarben.feld.Get(scroll_y2 + _j));

    if (drawhebung) {
        /*  Warp on top: vertical warp strip in kettfarben, with
            small weft stubs peeking out above and below.       */
        p->fillRect(QRect(_x + dw, _y, (_xx - dw) - (_x + dw), (_yy) - (_y)), kc);
        p->fillRect(QRect(_x, _y + dh, dw, (_yy - dh) - (_y + dh)), sc);
        p->fillRect(QRect(_xx - dw, _y + dh, dw, (_yy - dh) - (_y + dh)), sc);
        p->setPen(QPen(QColor(Qt::black)));
        p->drawLine(_xx - dw, _y + dh, _xx - dw, _yy - dh);
    } else {
        /*  Weft on top: horizontal weft strip in schussfarben, with
            warp stubs above and below.                            */
        p->fillRect(QRect(_x, _y + dh, (_xx) - (_x), (_yy - dh) - (_y + dh)), sc);
        p->fillRect(QRect(_x + dw, _y, (_xx - dw) - (_x + dw), dh), kc);
        p->fillRect(QRect(_x + dw, _yy - dh, (_xx - dw) - (_x + dw), dh), kc);
        p->setPen(QPen(QColor(Qt::black)));
        p->drawLine(_x + dw, _yy - dh, _xx - dw, _yy - dh);
    }
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DrawGewebeSchuss(int _j)
{
    if (gewebe.gw <= 0)
        return;
    for (int i = 0; i < gewebe.pos.width / gewebe.gw; i++)
        DrawGewebe(i, _j);
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DrawGewebeKette(int _i)
{
    if (gewebe.gh <= 0)
        return;
    for (int j = 0; j < gewebe.pos.height / gewebe.gh; j++)
        DrawGewebe(_i, j);
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DeleteGewebeSchuss(int _j)
{
    if (gewebe.gw <= 0)
        return;
    for (int i = 0; i < gewebe.pos.width / gewebe.gw; i++) {
        DrawGewebeRahmen(i, _j);
        DrawGewebe(i, _j);
    }
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DeleteGewebeKette(int _i)
{
    if (gewebe.gh <= 0)
        return;
    for (int j = 0; j < gewebe.pos.height / gewebe.gh; j++) {
        DrawGewebeRahmen(_i, j);
        DrawGewebe(_i, j);
    }
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DrawEinzug(int _i, int _j)
{
    QPainter* p = currentPainter;
    if (!p)
        return;

    int i = _i;
    if (righttoleft && einzug.gw > 0) {
        i = einzug.pos.width / einzug.gw - i - 1;
    }
    int j = _j;
    if (toptobottom && einzug.gh > 0) {
        j = einzug.pos.height / einzug.gh - j - 1;
    }

    const int x = einzug.pos.x0 + i * einzug.gw;
    const int y = einzug.pos.y0 + einzug.pos.height - (j + 1) * einzug.gh;
    const int xx = x + einzug.gw;
    const int yy = y + einzug.gh;

    const QColor bkground = legacyBtnFace();

    /*  einzug.feld holds 1-based shaft indices; cell (_i, _j) is set
        when einzug[scroll_x1+_i] - 1 == scroll_y1+_j. */
    if (einzug.feld.Get(scroll_x1 + _i) - 1 == (scroll_y1 + _j)) {
        PaintCell(*p, einzug.darstellung, x, y, xx, yy, QColor(Qt::black), /*dontclear=*/false,
                  scroll_y1 + _j, bkground);
    } else {
        ClearCell(*p, x, y, xx, yy, bkground);
    }
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DrawAufknuepfung(int _i, int _j)
{
    QPainter* p = currentPainter;
    if (!p)
        return;

    /*  Bei Schlagpatronenansicht mit pegplanstyle wird nichts
        gezeichnet -- aufknuepfung is rendered via trittfolge. */
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked() && aufknuepfung.pegplanstyle
        && !toptobottom)
        return;

    const int x = aufknuepfung.pos.x0 + _i * aufknuepfung.gw;
    int y;
    if (toptobottom)
        y = aufknuepfung.pos.y0 + _j * aufknuepfung.gh;
    else
        y = aufknuepfung.pos.y0 + aufknuepfung.pos.height - (_j + 1) * aufknuepfung.gh;
    const int xx = x + aufknuepfung.gw;
    const int yy = y + aufknuepfung.gh;

    const QColor bkground = legacyBtnFace();

    if (!ViewSchlagpatrone || !ViewSchlagpatrone->isChecked()) {
        const char s = aufknuepfung.feld.Get(scroll_x2 + _i, scroll_y1 + _j);
        if (s == AUSHEBUNG) {
            PaintCell(*p, darst_aushebung, x, y, xx, yy,
                      (darst_aushebung == AUSGEFUELLT) ? qColorFromTColor(GetRangeColor(s))
                                                       : QColor(Qt::black),
                      false, -1, bkground);
        } else if (s == ANBINDUNG) {
            PaintCell(*p, darst_anbindung, x, y, xx, yy,
                      (darst_aushebung == AUSGEFUELLT) ? qColorFromTColor(GetRangeColor(s))
                                                       : QColor(Qt::black),
                      false, -1, bkground);
        } else if (s == ABBINDUNG) {
            PaintCell(*p, darst_abbindung, x, y, xx, yy,
                      (darst_aushebung == AUSGEFUELLT) ? qColorFromTColor(GetRangeColor(s))
                                                       : QColor(Qt::black),
                      false, -1, bkground);
        } else if (s > 0) {
            PaintCell(*p, aufknuepfung.darstellung, x, y, xx, yy,
                      qColorFromTColor(GetRangeColor(s)), false, scroll_y1 + _j, bkground);
        } else {
            ClearCell(*p, x, y, xx, yy, bkground);
        }
    } else {
        /*  Schlagpatronen view: any non-zero fills as dark-grey filled. */
        if (aufknuepfung.feld.Get(scroll_x2 + _i, scroll_y1 + _j) > 0) {
            PaintCell(*p, AUSGEFUELLT, x, y, xx, yy, QColor(128, 128, 128), false, -1, bkground);
        } else {
            ClearCell(*p, x, y, xx, yy, bkground);
        }
    }
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DrawTrittfolge(int _i, int _j)
{
    QPainter* p = currentPainter;
    if (!p)
        return;

    const int x = trittfolge.pos.x0 + _i * trittfolge.gw;
    const int y = trittfolge.pos.y0 + trittfolge.pos.height - (_j + 1) * trittfolge.gh;
    const int xx = x + trittfolge.gw;
    const int yy = y + trittfolge.gh;

    const QColor bkground = legacyBtnFace();
    const bool pegplan = ViewSchlagpatrone && ViewSchlagpatrone->isChecked();

    const char s = trittfolge.feld.Get(scroll_x2 + _i, scroll_y2 + _j);
    if (s == AUSHEBUNG) {
        if (pegplan) {
            PaintCell(*p, darst_aushebung, x, y, xx, yy,
                      (darst_aushebung == AUSGEFUELLT) ? qColorFromTColor(GetRangeColor(s))
                                                       : QColor(Qt::black),
                      false, -1, bkground);
        } else {
            PaintCell(*p, trittfolge.darstellung, x, y, xx, yy, QColor(Qt::black), false, -1,
                      bkground);
        }
    } else if (s == ANBINDUNG) {
        if (pegplan) {
            PaintCell(*p, darst_anbindung, x, y, xx, yy,
                      (darst_aushebung == AUSGEFUELLT) ? qColorFromTColor(GetRangeColor(s))
                                                       : QColor(Qt::black),
                      false, -1, bkground);
        } else {
            PaintCell(*p, trittfolge.darstellung, x, y, xx, yy, QColor(Qt::black), false, -1,
                      bkground);
        }
    } else if (s == ABBINDUNG) {
        if (pegplan) {
            PaintCell(*p, darst_abbindung, x, y, xx, yy,
                      (darst_aushebung == AUSGEFUELLT) ? qColorFromTColor(GetRangeColor(s))
                                                       : QColor(Qt::black),
                      false, -1, bkground);
        } else {
            PaintCell(*p, trittfolge.darstellung, x, y, xx, yy, QColor(Qt::black), false, -1,
                      bkground);
        }
    } else if (s > 0) {
        if (pegplan) {
            PaintCell(*p, schlagpatronendarstellung, x, y, xx, yy,
                      qColorFromTColor(GetRangeColor(s)), false, scroll_x2 + _i, bkground);
        } else {
            PaintCell(*p, trittfolge.darstellung, x, y, xx, yy, QColor(Qt::black), false, -1,
                      bkground);
        }
    } else {
        ClearCell(*p, x, y, xx, yy, bkground);
    }
}
/*-----------------------------------------------------------------*/
/*  --- Rahmen (frame + strongline) painters ----------------------
    Each field's Rahmen draws a one-pixel top+left cell border in
    QPalette::Dark plus optional heavier strongline_x/y dividers in
    TDBWFRM::strongclr. These paint one cell at a time, same as the
    Draw* primitives; PatternCanvas drives the iteration. */
/*-----------------------------------------------------------------*/
void TDBWFRM::DrawEinzugRahmen(int _i, int _j)
{
    QPainter* p = currentPainter;
    if (!p)
        return;

    int i = _i;
    if (righttoleft && einzug.gw > 0) {
        i = einzug.pos.width / einzug.gw - i - 1;
    }
    int j = _j;
    if (toptobottom && einzug.gh > 0) {
        j = einzug.pos.height / einzug.gh - j - 1;
    }

    const int x = einzug.pos.x0 + i * einzug.gw;
    const int y = einzug.pos.y0 + einzug.pos.height - (j + 1) * einzug.gh;
    const int xx = x + einzug.gw;
    const int yy = y + einzug.gh;

    p->setPen(QPen(legacyBtnShadow()));
    p->drawLine(x, y, xx, y);
    p->drawLine(x, y, x, yy);

    p->setPen(QPen(strongclr));
    const int stx = einzug.pos.strongline_x;
    const int sty = einzug.pos.strongline_y;

    if (stx != 0 && _i > 0) {
        if (righttoleft) {
            if (((_i + scroll_x1) % stx) == 0 && _i != 0)
                p->drawLine(xx, y, xx, yy);
            if (((_i + 1 + scroll_x1) % stx) == 0 && einzug.gw > 0
                && _i + 1 < einzug.pos.width / einzug.gw)
                p->drawLine(x, y, x, yy);
        } else {
            if (((_i + scroll_x1) % stx) == 0 && _i != 0)
                p->drawLine(x, y, x, yy);
            if (((_i + 1 + scroll_x1) % stx) == 0 && einzug.gw > 0
                && _i + 1 < einzug.pos.width / einzug.gw)
                p->drawLine(xx, y, xx, yy);
        }
    }
    if (sty != 0 && _j > 0) {
        if (toptobottom) {
            if (((_j + scroll_y1 + 1) % sty) == 0 && einzug.gh > 0
                && _j + 1 < einzug.pos.height / einzug.gh)
                p->drawLine(x, yy, xx, yy);
            if (((_j + scroll_y1) % sty) == 0 && _j != 0)
                p->drawLine(x, y, xx, y);
        } else {
            if (((_j + scroll_y1 + 1) % sty) == 0 && einzug.gh > 0
                && _j + 1 < einzug.pos.height / einzug.gh)
                p->drawLine(x, y, xx, y);
            if (((_j + scroll_y1) % sty) == 0 && _j != 0)
                p->drawLine(x, yy, xx, yy);
        }
    }
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DrawAufknuepfungRahmen(int _i, int _j)
{
    QPainter* p = currentPainter;
    if (!p)
        return;

    int j = _j;
    if (toptobottom && einzug.gh > 0) {
        j = einzug.pos.height / einzug.gh - j - 1;
    }

    const int x = aufknuepfung.pos.x0 + _i * aufknuepfung.gw;
    const int y = aufknuepfung.pos.y0 + aufknuepfung.pos.height - (j + 1) * aufknuepfung.gh;
    const int xx = x + aufknuepfung.gw;
    const int yy = y + aufknuepfung.gh;

    p->setPen(QPen(legacyBtnShadow()));
    p->drawLine(x, y, xx, y);
    p->drawLine(x, y, x, yy);

    p->setPen(QPen(strongclr));
    const int stx = aufknuepfung.pos.strongline_x;
    const int sty = aufknuepfung.pos.strongline_y;

    if (stx != 0 && _i > 0) {
        if (((_i + scroll_x2) % stx) == 0 && _i != 0)
            p->drawLine(x, y, x, yy);
        if (((_i + 1 + scroll_x2) % stx) == 0 && aufknuepfung.gw > 0
            && _i + 1 < aufknuepfung.pos.width / aufknuepfung.gw)
            p->drawLine(xx, y, xx, yy);
    }
    if (sty != 0 && _j > 0) {
        if (toptobottom) {
            if (((_j + scroll_y1 + 1) % sty) == 0 && aufknuepfung.gh > 0
                && _j + 1 < aufknuepfung.pos.height / aufknuepfung.gh)
                p->drawLine(x, yy, xx, yy);
            if (((_j + scroll_y1) % sty) == 0 && _j != 0)
                p->drawLine(x, y, xx, y);
        } else {
            if (((_j + scroll_y1 + 1) % sty) == 0 && aufknuepfung.gh > 0
                && _j + 1 < aufknuepfung.pos.height / aufknuepfung.gh)
                p->drawLine(x, y, xx, y);
            if (((_j + scroll_y1) % sty) == 0 && _j != 0)
                p->drawLine(x, yy, xx, yy);
        }
    }
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DrawTrittfolgeRahmen(int _i, int _j)
{
    QPainter* p = currentPainter;
    if (!p)
        return;

    const int x = trittfolge.pos.x0 + _i * trittfolge.gw;
    const int y = trittfolge.pos.y0 + trittfolge.pos.height - (_j + 1) * trittfolge.gh;
    const int xx = x + trittfolge.gw;
    const int yy = y + trittfolge.gh;

    p->setPen(QPen(legacyBtnShadow()));
    p->drawLine(x, y, xx, y);
    p->drawLine(x, y, x, yy);

    p->setPen(QPen(strongclr));
    const int stx = trittfolge.pos.strongline_x;
    const int sty = trittfolge.pos.strongline_y;

    if (stx != 0 && _i > 0) {
        if (((_i + scroll_x2) % stx) == 0 && _i != 0)
            p->drawLine(x, y, x, yy);
        if (((_i + 1 + scroll_x2) % stx) == 0 && trittfolge.gw > 0
            && _i + 1 < trittfolge.pos.width / trittfolge.gw)
            p->drawLine(xx, y, xx, yy);
    }
    if (sty != 0 && _j > 0) {
        if (((_j + scroll_y2 + 1) % sty) == 0 && trittfolge.gh > 0
            && _j + 1 < trittfolge.pos.height / trittfolge.gh)
            p->drawLine(x, y, xx, y);
        if (((_j + scroll_y2) % sty) == 0 && _j != 0)
            p->drawLine(x, yy, xx, yy);
    }
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DrawGewebeRahmen(int _i, int _j)
{
    QPainter* p = currentPainter;
    if (!p)
        return;

    int i = _i;
    if (righttoleft && gewebe.gw > 0) {
        i = gewebe.pos.width / gewebe.gw - i - 1;
    }

    const int x = gewebe.pos.x0 + i * gewebe.gw;
    const int y = gewebe.pos.y0 + gewebe.pos.height - (_j + 1) * gewebe.gh;
    const int xx = x + gewebe.gw;
    const int yy = y + gewebe.gh;

    p->setPen(QPen(legacyBtnShadow()));
    p->drawLine(x, y, xx, y);
    p->drawLine(x, y, x, yy);

    p->setPen(QPen(strongclr));
    const int stx = gewebe.pos.strongline_x;
    const int sty = gewebe.pos.strongline_y;

    if (stx != 0) {
        if (righttoleft) {
            if (((_i + scroll_x1) % stx) == 0 && _i != 0)
                p->drawLine(xx, y, xx, yy);
            if (((_i + 1 + scroll_x1) % stx) == 0 && gewebe.gw > 0
                && _i + 1 < gewebe.pos.width / gewebe.gw)
                p->drawLine(x, y, x, yy);
        } else {
            if (((_i + scroll_x1) % stx) == 0 && _i != 0)
                p->drawLine(x, y, x, yy);
            if (((_i + 1 + scroll_x1) % stx) == 0 && gewebe.gw > 0
                && _i + 1 < gewebe.pos.width / gewebe.gw)
                p->drawLine(xx, y, xx, yy);
        }
    }
    if (sty != 0) {
        if (((_j + scroll_y2 + 1) % sty) == 0 && gewebe.gh > 0
            && _j + 1 < gewebe.pos.height / gewebe.gh)
            p->drawLine(x, y, xx, y);
        if (((_j + scroll_y2) % sty) == 0 && _j != 0)
            p->drawLine(x, yy, xx, yy);
    }
}
/*-----------------------------------------------------------------*/
