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

/*  VCL's TCanvas::Rectangle(x1, y1, x2, y2) draws the outline using the
    current pen and fills the interior with the current brush, inclusive
    of (x1, y1) and EXCLUSIVE of (x2, y2). Translating to QPainter means
    drawRect with width = x2-x1-1, height = y2-y1-1 when we want the same
    coverage, because QPainter::drawRect draws the outline ON the edge
    pixels of the given rectangle (its "cosmetic" pen semantic expands
    outward). Matching legacy pixel output exactly requires compensating
    for that one-pixel difference.

    All cell rendering intentionally disables antialiasing so the output
    is deterministic across Qt minor versions and CPU architectures,
    which is a hard requirement for the golden-pixel tests in
    tests/test_draw_cell.cpp. */

namespace {

void fillRect (QPainter& p, int x1, int y1, int x2, int y2, const QColor& c)
{
	p.fillRect(QRect(x1, y1, x2 - x1, y2 - y1), c);
}

void line (QPainter& p, int x1, int y1, int x2, int y2)
{
	p.drawLine(x1, y1, x2, y2);
}

/*  The VCL Rectangle primitive outlines AND fills. We reproduce that
    by painting one solid rectangle using the current brush; the pen
    colour has already been set to match by PaintCell. */
void solidRect (QPainter& p, int x1, int y1, int x2, int y2)
{
	p.fillRect(QRect(x1, y1, x2 - x1, y2 - y1), p.brush().color());
}

} // namespace

void ClearCell (QPainter& _p, int _x, int _y, int _xx, int _yy, const QColor& _bkground)
{
	/*  Legacy: Rectangle(_x+1, _y+1, _xx, _yy) -> inclusive (_x+1, _y+1),
	    exclusive (_xx, _yy). Under fillRect this is a box of
	    (_xx - _x - 1) x (_yy - _y - 1) starting at (_x+1, _y+1).       */
	fillRect(_p, _x + 1, _y + 1, _xx, _yy, _bkground);
}

void PaintCell (QPainter& _p,
                DARSTELLUNG _darstellung,
                int _x, int _y, int _xx, int _yy,
                const QColor& _color,
                bool          _dontclear,
                int           _number,
                const QColor& _bkground,
                int           _fontHeight)
{
	_p.setRenderHint(QPainter::Antialiasing, false);

	if (!_dontclear) ClearCell(_p, _x, _y, _xx, _yy, _bkground);

	_p.setPen(QPen(_color));
	_p.setBrush(QBrush(_color));

	switch (_darstellung) {
		case AUSGEFUELLT:
			solidRect(_p, _x + 2, _y + 2, _xx - 1, _yy - 1);
			break;

		case STRICH:
			line(_p, (_x+_xx)/2,     _yy-2, (_x+_xx)/2,     _y+1);
			line(_p, (_x+_xx)/2 + 1, _yy-2, (_x+_xx)/2 + 1, _y+1);
			break;

		case KREUZ:
			line(_p, _x + 2, _yy - 2, _xx - 1, _y  + 1);
			line(_p, _x + 2, _y  + 2, _xx - 1, _yy - 1);
			break;

		case PUNKT: {
			const int x = (_x + _xx) / 2;
			const int y = (_y + _yy) / 2;
			line(_p, x, y,     x + 2, y);
			line(_p, x, y + 1, x + 2, y + 1);
			break;
		}

		case KREIS:
			/*  Legacy: Arc(x1, y1, x2, y2, sx, sy, ex, ey) with start
			    == end == (x, y) draws a full ellipse. */
			_p.setBrush(Qt::NoBrush);
			_p.drawEllipse(QRect(_x + 2, _y + 2, (_xx - 1) - (_x + 2), (_yy - 1) - (_y + 2)));
			_p.setBrush(QBrush(_color));
			break;

		case STEIGEND:
			line(_p, _x + 2, _yy - 2, _xx - 1, _y  + 1);
			break;

		case FALLEND:
			line(_p, _x + 2, _y  + 2, _xx - 1, _yy - 1);
			break;

		case SMALLKREIS:
			if (_xx - _x >= 9) {
				_p.setBrush(Qt::NoBrush);
				_p.drawEllipse(QRect(_x + 4, _y + 4, (_xx - 3) - (_x + 4), (_yy - 3) - (_y + 4)));
				_p.setBrush(QBrush(_color));
			} else {
				const int x = (_x + _xx) / 2;
				const int y = (_y + _yy) / 2;
				line(_p, x, y,     x + 2, y);
				line(_p, x, y + 1, x + 2, y + 1);
			}
			break;

		case SMALLKREUZ:
			if (_xx - _x >= 9) {
				line(_p, _x + 4, _yy - 4, _xx - 3, _y  + 3);
				line(_p, _x + 4, _y  + 4, _xx - 3, _yy - 3);
			} else {
				line(_p, _x + 2, _yy - 2, _xx - 1, _y  + 1);
				line(_p, _x + 2, _y  + 2, _xx - 1, _yy - 1);
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
				_p.drawText(QRect(_x + 1, _y + 1, (_xx - 1) - (_x + 1), (_yy - 1) - (_y + 1)),
				            Qt::AlignCenter, nr);
			} else {
				solidRect(_p, _x + 2, _y + 2, _xx - 1, _yy - 1);
			}
			break;
	}
}
