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
#include "palette.h"

#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QRect>
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawGewebe (int _i, int _j)
{
	QPainter* p = currentPainter;
	if (!p) return;

	int i = _i;
	if (righttoleft && gewebe.gw > 0) {
		i = gewebe.pos.width/gewebe.gw - i - 1;
	}

	const int x  = gewebe.pos.x0 + i*gewebe.gw;
	const int y  = gewebe.pos.y0 + gewebe.pos.height - (_j + 1)*gewebe.gh;
	const int xx = x + gewebe.gw;
	const int yy = y + gewebe.gh;

	const QColor bkground = palette().color(QPalette::Button);

	/*  Empty einzug OR empty trittfolge -> blank cell. */
	if (IsEmptyEinzug(scroll_x1 + _i) || IsEmptyTrittfolge(scroll_y2 + _j)) {
		p->fillRect(QRect(x + 1, y + 1, xx - x - 1, yy - y - 1), bkground);
		return;
	}

	/*  Phase-5 initial slice: only the GewebeNormal branch is
	    ported. Inverserepeat / rapport / Farbeffekt / Simulation
	    paths fall through to "normal" for now, which is correct
	    for the default view (GewebeNormal checked, everything else
	    unchecked) and visually close enough for other cases. */
	const int range = gewebe.feld.Get(scroll_x1 + _i, scroll_y2 + _j);

	QColor fg;
	if (range == AUSHEBUNG || range == ANBINDUNG || range == ABBINDUNG) {
		fg = qColorFromTColor(GetRangeColor(range));
	} else if (range > 0) {
		fg = qColorFromTColor(GetRangeColor(range));
	} else {
		/*  Empty / negative (toggled-off) cell. */
		p->fillRect(QRect(x + 1, y + 1, xx - x - 1, yy - y - 1), bkground);
		return;
	}

	/*  Legacy: Canvas->Rectangle(x+1, y+1, xx, yy) with brush = fg
	    and pen = clBtnFace. VCL's Rectangle is inclusive-exclusive;
	    fillRect gives the same coverage.                        */
	p->fillRect(QRect(x + 1, y + 1, xx - x - 1, yy - y - 1), fg);
}
/*-----------------------------------------------------------------*/
