/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Drawing tools — port of legacy tools.cpp. Each DrawTool* method
    rasterises a geometric shape between two data-coordinate cells
    into gewebe.feld using the current range. DrawTool is the
    dispatcher invoked from the mouse-release path once the user
    finishes a drag with a non-POINT tool selected.

    DrawDragTool / DeleteDragTool (legacy XOR rubber-band overlay)
    are not separately ported; PatternCanvas::paintEvent draws the
    preview directly using the tool_i0/j0/i1/j1 anchors on the
    TDBWFRM, so no XOR erase is needed.                            */

#include "mainwindow.h"
#include "datamodule.h"
#include "undoredo.h"
#include "felddef.h"

#include <QImage>
#include <QPainter>

#include <algorithm>
#include <cstdlib>

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawTool (int _i, int _j, int _i1, int _j1)
{
	switch (tool) {
	case TOOL_POINT:
		return;
	case TOOL_LINE:
		DrawToolLine(_i, _j, _i1, _j1);
		break;
	case TOOL_RECTANGLE:
		DrawToolRectangle(_i, _j, _i1, _j1, false);
		break;
	case TOOL_FILLEDRECTANGLE:
		DrawToolRectangle(_i, _j, _i1, _j1, true);
		break;
	case TOOL_ELLIPSE:
		DrawToolEllipse(_i, _j, _i1, _j1, false);
		break;
	case TOOL_FILLEDELLIPSE:
		DrawToolEllipse(_i, _j, _i1, _j1, true);
		break;
	}
	RecalcAll();
	CalcRangeSchuesse();
	CalcRangeKette();
	CalcRapport();
	refresh();
	if (undo) undo->Snapshot();
}

/*-----------------------------------------------------------------*/
/*  Bresenham-ish line rasteriser, verbatim port of the legacy
    anonymous DrawLine helper in tools.cpp. Preserves the legacy's
    axis-flip + diagonal-swap logic so the rasterisation pattern
    matches pixel-for-pixel. */
static void drawLine (FeldGewebe& _gewebe, int _i, int _j, int _i1, int _j1, char _range)
{
	if (_i >= _i1) return;

	const int tx = _i;
	_i1 -= tx; _i -= tx;
	const int ty = _j;
	_j1 -= ty; _j -= ty;

	int dy = _j1 - _j;

	bool spiegelnv = false;
	if (dy < 0) { _j1 = -_j1; dy = _j1 - _j; spiegelnv = true; }

	bool spiegelnd = false;
	if (std::abs(dy) > _i1) {
		std::swap(_i1, _j1);
		spiegelnd = true;
	}

	const int dxx = _i1 - _i;
	const int dyy = _j1 - _j;

	int d      = 2*dyy - dxx;
	const int incrE  = 2*dyy;
	const int incrNE = 2*(dyy - dxx);

	int x = _i;
	int y = _j;

	auto set = [&](int px, int py) {
		if (spiegelnv) _gewebe.feld.Set(tx + px, ty - py, _range);
		else           _gewebe.feld.Set(tx + px, ty + py, _range);
	};

	if (!spiegelnd) set(x, y);
	else            set(y, x);

	while (x < _i1) {
		if (d <= 0) { d += incrE;  x++; }
		else        { d += incrNE; x++; y++; }
		if (!spiegelnd) set(x, y);
		else            set(y, x);
	}
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawToolLine (int _i, int _j, int _i1, int _j1)
{
	if (_i == _i1) {
		/*  Vertical. */
		if (_j > _j1) std::swap(_j, _j1);
		for (int j = scroll_y2 + _j; j <= scroll_y2 + _j1; j++)
			gewebe.feld.Set(scroll_x1 + _i, j, currentrange);
	} else if (_j == _j1) {
		/*  Horizontal. */
		if (_i > _i1) std::swap(_i, _i1);
		for (int i = scroll_x1 + _i; i <= scroll_x1 + _i1; i++)
			gewebe.feld.Set(i, scroll_y2 + _j, currentrange);
	} else if (std::abs(_i1 - _i) == std::abs(_j1 - _j)) {
		/*  45° diagonal. */
		if (_i > _i1) {
			std::swap(_i, _i1);
			std::swap(_j, _j1);
		}
		if (_j < _j1)
			for (int i = _i; i <= _i1; i++)
				gewebe.feld.Set(scroll_x1 + i, scroll_y2 + _j + i - _i, currentrange);
		else
			for (int i = _i; i <= _i1; i++)
				gewebe.feld.Set(scroll_x1 + i, scroll_y2 + _j - i + _i, currentrange);
	} else {
		if (_i > _i1) {
			std::swap(_i, _i1);
			std::swap(_j, _j1);
		}
		drawLine(gewebe, scroll_x1 + _i, scroll_y2 + _j,
		                  scroll_x1 + _i1, scroll_y2 + _j1, char(currentrange));
	}
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawToolRectangle (int _i, int _j, int _i1, int _j1, bool _filled)
{
	if (_i > _i1) std::swap(_i, _i1);
	if (_j > _j1) std::swap(_j, _j1);
	if (_filled) {
		for (int i = scroll_x1 + _i; i <= scroll_x1 + _i1; i++)
			for (int j = scroll_y2 + _j; j <= scroll_y2 + _j1; j++)
				gewebe.feld.Set(i, j, currentrange);
	} else {
		for (int i = scroll_x1 + _i; i <= scroll_x1 + _i1; i++) {
			gewebe.feld.Set(i, scroll_y2 + _j,  currentrange);
			gewebe.feld.Set(i, scroll_y2 + _j1, currentrange);
		}
		for (int j = scroll_y2 + _j; j <= scroll_y2 + _j1; j++) {
			gewebe.feld.Set(scroll_x1 + _i,  j, currentrange);
			gewebe.feld.Set(scroll_x1 + _i1, j, currentrange);
		}
	}
}

/*-----------------------------------------------------------------*/
/*  Ellipse: legacy renders the shape into an offscreen TBitmap and
    samples pixel centres to decide which grid cells to set. The
    QImage equivalent is identical in spirit — QPainter::drawArc
    for outline, drawEllipse for filled. */
void __fastcall TDBWFRM::DrawToolEllipse (int _i, int _j, int _i1, int _j1, bool _filled)
{
	if (_i > _i1) std::swap(_i, _i1);
	if (_j > _j1) std::swap(_j, _j1);
	const int w = _i1 - _i;
	const int h = _j1 - _j;
	const int mult = _filled ? 13 : 1;
	const int iw = (w + 1) * mult;
	const int ih = (h + 1) * mult;
	if (iw <= 0 || ih <= 0) return;

	QImage img(iw, ih, QImage::Format_RGB32);
	img.fill(Qt::white);
	{
		QPainter p(&img);
		p.setRenderHint(QPainter::Antialiasing, false);
		p.setPen(Qt::black);
		if (_filled) {
			p.setBrush(Qt::black);
			p.drawEllipse(QRect(0, 0, iw, ih));
		} else {
			p.setBrush(Qt::NoBrush);
			/*  The legacy used Canvas->Arc(0,0,w*mult+1, h*mult+1, 0,0,0,0)
			    which is a full arc (= ellipse outline). */
			p.drawArc(QRect(0, 0, w*mult, h*mult), 0, 5760);
		}
	}

	for (int x = 0; x <= w; x++)
		for (int y = 0; y <= h; y++) {
			const QRgb pix = img.pixel(x*mult + mult/2, y*mult + mult/2);
			if (qRed(pix) == 0 && qGreen(pix) == 0 && qBlue(pix) == 0)
				gewebe.feld.Set(scroll_x1 + _i + x, scroll_y2 + _j + y, currentrange);
		}
}
