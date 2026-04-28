/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "draw_cell.h"

#include <QBrush>
#include <QFont>
#include <QPen>
#include <QRect>
#include <QString>

/*  Coordinate conventions
    ------------------------
    A cell occupies pixels [_x, _xx] x [_y, _yy]; the grid-frame lines
    at _x, _xx, _y, _yy are shared with the neighbouring cells and are
    painted elsewhere. The cell interior is [_x+1, _xx-1] x [_y+1, _yy-1].
    Symbols leave an additional 1-pixel margin inside the interior,
    i.e. they live in [_x+2, _xx-2] x [_y+2, _yy-2].

    Endpoints are inclusive throughout, matching QPainter::drawLine,
    QPainter::fillRect(QRect, ...) and QPainter::drawEllipse(QRect).
    That means, for pixel X spanning cols A..B inclusive, the call is
    fillRect(QRect(A, Y, B - A + 1, ...)) and drawLine(A, Y, B, Y).

    All cell rendering intentionally disables antialiasing so output
    is deterministic across Qt minor versions and CPU architectures,
    which is a hard requirement for the golden-pixel tests in
    tests/test_draw_cell.cpp. */

namespace
{

void line(QPainter& p, int x1, int y1, int x2, int y2)
{
    p.drawLine(x1, y1, x2, y2);
}

/*  Inclusive-inclusive pixel rect: fills pixels [x1, x2] x [y1, y2]. */
void fillBox(QPainter& p, int x1, int y1, int x2, int y2, const QColor& c)
{
    p.fillRect(QRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1), c);
}

/*  Stroke thickness for line-based symbols (KREUZ, KREIS,
    STEIGEND, FALLEND, SMALL*). Only odd values -- Qt's integer
    pens with anti-aliasing disabled rasterise crisply only at
    odd widths, where the stroke is pixel-centred on the geometric
    line. Thresholds chosen so the golden-pixel tests at SZ=20 /
    OSZ=21 still land in the 1-pixel regime.                   */
int strokeFor(int _size)
{
    if (_size < 22)
        return 1;
    if (_size < 36)
        return 3;
    return 5;
}

/*  Width for fillRect-based features (PUNKT dot, STRICH bar,
    SMALLKREIS small-cell fallback). EVEN widths -- odd widths
    straddling an integer axis can never be pixel-symmetric,
    whereas even widths placed as [cx-w/2+1, cx+w/2] pair up
    under the {_x+k, _xx-k} mirror at any odd cell size.      */
int dotFor(int _size)
{
    if (_size < 22)
        return 2;
    if (_size < 36)
        return 4;
    return 6;
}

/*  Centre a bar of width `_w` around cell-midline column `_cx`.
    For even widths the bar straddles _cx with a 1-pixel bias to
    the right (matches the legacy 2-pixel STRICH at cx, cx+1). */
int barLeft(int _cx, int _w)
{
    return _cx - _w / 2 + (_w % 2 == 0 ? 1 : 0);
}

} // namespace

void ClearCell(QPainter& _p, int _x, int _y, int _xx, int _yy, const QColor& _bkground)
{
    /*  Cell interior: pixels [_x+1, _xx-1] x [_y+1, _yy-1] inclusive. */
    fillBox(_p, _x + 1, _y + 1, _xx - 1, _yy - 1, _bkground);
}

void PaintCell(QPainter& _p, DARSTELLUNG _darstellung, int _x, int _y, int _xx, int _yy,
               const QColor& _color, bool _dontclear, int _number, const QColor& _bkground,
               int _fontHeight)
{
    _p.setRenderHint(QPainter::Antialiasing, false);

    if (!_dontclear)
        ClearCell(_p, _x, _y, _xx, _yy, _bkground);

    /*  Scale the symbol weight with the cell size so strokes that
        were 1-pixel at small cells don't look hair-thin at large
        zoom. The line-centre inset d = 2 + sw/2 keeps the stroke's
        outer edge at pixel _x+2 (and _xx-2, etc.) regardless of
        stroke width, preserving the 1-pixel margin between the
        cell border and the symbol.                              */
    const int size = _xx - _x;
    const int sw = strokeFor(size);
    const int d = 2 + sw / 2;

    /*  Use Qt's default cap style (SquareCap). FlatCap is tempting
        ("don't extend past the geometric endpoint") but at pen
        width 1 the raster engine then excludes the endpoint pixel
        entirely, so drawLine(2, 18, 18, 2) stops at (17, 3) -- the
        exact GDI-vs-Qt endpoint trap we already rewrote the coords
        to avoid. With SquareCap, width 1 includes both endpoints;
        wider pens do overshoot by sw/2 along the line, but our d
        inset keeps the overshoot inside the 1-pixel cell margin. */
    _p.setPen(QPen(_color, sw));
    _p.setBrush(QBrush(_color));

    switch (_darstellung) {
    case AUSGEFUELLT:
        fillBox(_p, _x + 2, _y + 2, _xx - 2, _yy - 2, _color);
        break;

    case STRICH: {
        /*  Centred vertical bar, always even width. At small cells
            this is the legacy 2-pixel bar; at larger cells it grows
            together with the dot size so the whole symbol family
            scales in step.                                         */
        const int cx = (_x + _xx) / 2;
        const int bar = dotFor(size);
        const int left = barLeft(cx, bar);
        fillBox(_p, left, _y + 2, left + bar - 1, _yy - 2, _color);
        break;
    }

    case KREUZ:
        line(_p, _x + d, _yy - d, _xx - d, _y + d);
        line(_p, _x + d, _y + d, _xx - d, _yy - d);
        break;

    case PUNKT: {
        /*  Centred square dot. dotFor scales with cell size; at
            small cells this is 2x2, matching the legacy feel. */
        const int dot = dotFor(size);
        const int cx = (_x + _xx) / 2;
        const int cy = (_y + _yy) / 2;
        const int left = barLeft(cx, dot);
        const int top = barLeft(cy, dot);
        fillBox(_p, left, top, left + dot - 1, top + dot - 1, _color);
        break;
    }

    case KREIS:
        /*  Note: QPainter::drawEllipse(QRect(x,y,w,h)) renders into
            pixel bounding box [x, x+w] x [y, y+h] -- inclusive on
            BOTH ends, unlike fillRect. With line-centre inset d,
            the ellipse bounding rect is [_x+d, _xx-d] x ... so
            width is (_xx-d) - (_x+d) = _xx - _x - 2d.             */
        _p.setBrush(Qt::NoBrush);
        _p.drawEllipse(QRect(_x + d, _y + d, _xx - _x - 2 * d, _yy - _y - 2 * d));
        _p.setBrush(QBrush(_color));
        break;

    case STEIGEND:
        line(_p, _x + d, _yy - d, _xx - d, _y + d);
        break;

    case FALLEND:
        line(_p, _x + d, _y + d, _xx - d, _yy - d);
        break;

    case SMALLKREIS:
        if (size >= 9) {
            const int sd = d + 2; /* SMALL* sits 2 extra pixels in */
            _p.setBrush(Qt::NoBrush);
            _p.drawEllipse(QRect(_x + sd, _y + sd, _xx - _x - 2 * sd, _yy - _y - 2 * sd));
            _p.setBrush(QBrush(_color));
        } else {
            const int dot = dotFor(size);
            const int cx = (_x + _xx) / 2;
            const int cy = (_y + _yy) / 2;
            const int left = barLeft(cx, dot);
            const int top = barLeft(cy, dot);
            fillBox(_p, left, top, left + dot - 1, top + dot - 1, _color);
        }
        break;

    case SMALLKREUZ:
        if (size >= 9) {
            const int sd = d + 2;
            line(_p, _x + sd, _yy - sd, _xx - sd, _y + sd);
            line(_p, _x + sd, _y + sd, _xx - sd, _yy - sd);
        } else {
            line(_p, _x + d, _yy - d, _xx - d, _y + d);
            line(_p, _x + d, _y + d, _xx - d, _yy - d);
        }
        break;

    case NUMBER:
        if (_number >= 0) {
            /*  Default font height scales with the cell so 1-2 digit
                shaft numbers remain legible across zoom levels. The
                old fixed 6-px default was unreadable on a 20+ px
                cell. Caller can still override via _fontHeight. */
            int h = _fontHeight;
            if (h <= 0) {
                h = (size * 2) / 3;
                if (h < 7)
                    h = 7;
                if (h > size - 4)
                    h = size - 4;
            }
            QFont f = _p.font();
            f.setPixelSize(h);
            _p.setFont(f);
            _p.setPen(QPen(_color));
            const QString nr = QString::number(_number + 1);
            _p.drawText(QRect(_x + 1, _y + 1, _xx - _x - 2, _yy - _y - 2), Qt::AlignCenter, nr);
        } else {
            fillBox(_p, _x + 2, _y + 2, _xx - 2, _yy - 2, _color);
        }
        break;

    case HDASH: {
        /*  Horizontal mirror of STRICH: a centred horizontal bar
            spanning the cell width, even thickness scaled like
            STRICH's vertical bar.                                  */
        const int cy = (_y + _yy) / 2;
        const int bar = dotFor(size);
        const int top = barLeft(cy, bar);
        fillBox(_p, _x + 2, top, _xx - 2, top + bar - 1, _color);
        break;
    }

    case PLUS: {
        /*  '+' = centred vertical bar (STRICH) + centred horizontal
            bar (HDASH) sharing the cell midline.                   */
        const int cx = (_x + _xx) / 2;
        const int cy = (_y + _yy) / 2;
        const int bar = dotFor(size);
        const int left = barLeft(cx, bar);
        const int top = barLeft(cy, bar);
        fillBox(_p, left, _y + 2, left + bar - 1, _yy - 2, _color);
        fillBox(_p, _x + 2, top, _xx - 2, top + bar - 1, _color);
        break;
    }
    }
}
