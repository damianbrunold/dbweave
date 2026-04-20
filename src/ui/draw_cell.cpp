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

    _p.setPen(QPen(_color));
    _p.setBrush(QBrush(_color));

    switch (_darstellung) {
    case AUSGEFUELLT:
        fillBox(_p, _x + 2, _y + 2, _xx - 2, _yy - 2, _color);
        break;

    case STRICH: {
        const int cx = (_x + _xx) / 2;
        line(_p, cx, _y + 2, cx, _yy - 2);
        line(_p, cx + 1, _y + 2, cx + 1, _yy - 2);
        break;
    }

    case KREUZ:
        line(_p, _x + 2, _yy - 2, _xx - 2, _y + 2);
        line(_p, _x + 2, _y + 2, _xx - 2, _yy - 2);
        break;

    case PUNKT: {
        const int cx = (_x + _xx) / 2;
        const int cy = (_y + _yy) / 2;
        _p.fillRect(QRect(cx, cy, 2, 2), _color);
        break;
    }

    case KREIS:
        /*  Note: QPainter::drawEllipse(QRect(x,y,w,h)) renders into
            pixel bounding box [x, x+w] x [y, y+h] -- inclusive on
            BOTH ends, unlike fillRect. So for an ellipse inscribed
            in pixels [_x+2, _xx-2] x [_y+2, _yy-2] the width is
            (_xx-2) - (_x+2) = _xx - _x - 4.                         */
        _p.setBrush(Qt::NoBrush);
        _p.drawEllipse(QRect(_x + 2, _y + 2, _xx - _x - 4, _yy - _y - 4));
        _p.setBrush(QBrush(_color));
        break;

    case STEIGEND:
        line(_p, _x + 2, _yy - 2, _xx - 2, _y + 2);
        break;

    case FALLEND:
        line(_p, _x + 2, _y + 2, _xx - 2, _yy - 2);
        break;

    case SMALLKREIS:
        if (_xx - _x >= 9) {
            _p.setBrush(Qt::NoBrush);
            _p.drawEllipse(QRect(_x + 4, _y + 4, _xx - _x - 8, _yy - _y - 8));
            _p.setBrush(QBrush(_color));
        } else {
            const int cx = (_x + _xx) / 2;
            const int cy = (_y + _yy) / 2;
            _p.fillRect(QRect(cx, cy, 2, 2), _color);
        }
        break;

    case SMALLKREUZ:
        if (_xx - _x >= 9) {
            line(_p, _x + 4, _yy - 4, _xx - 4, _y + 4);
            line(_p, _x + 4, _y + 4, _xx - 4, _yy - 4);
        } else {
            line(_p, _x + 2, _yy - 2, _xx - 2, _y + 2);
            line(_p, _x + 2, _y + 2, _xx - 2, _yy - 2);
        }
        break;

    case NUMBER:
        if (_number >= 0) {
            const int h = _fontHeight > 0 ? _fontHeight : 6;
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
    }
}
