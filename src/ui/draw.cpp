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
void __fastcall TDBWFRM::DrawEinzug (int _i, int _j)
{
	QPainter* p = currentPainter;
	if (!p) return;

	int i = _i;
	if (righttoleft && einzug.gw > 0) {
		i = einzug.pos.width/einzug.gw - i - 1;
	}
	int j = _j;
	if (toptobottom && einzug.gh > 0) {
		j = einzug.pos.height/einzug.gh - j - 1;
	}

	const int x  = einzug.pos.x0 + i*einzug.gw;
	const int y  = einzug.pos.y0 + einzug.pos.height - (j + 1)*einzug.gh;
	const int xx = x + einzug.gw;
	const int yy = y + einzug.gh;

	const QColor bkground = palette().color(QPalette::Button);

	/*  einzug.feld holds 1-based shaft indices; cell (_i, _j) is set
	    when einzug[scroll_x1+_i] - 1 == scroll_y1+_j. */
	if (einzug.feld.Get(scroll_x1 + _i) - 1 == (scroll_y1 + _j)) {
		PaintCell(*p, einzug.darstellung, x, y, xx, yy,
		          QColor(Qt::black), /*dontclear=*/false, scroll_y1 + _j,
		          bkground);
	} else {
		ClearCell(*p, x, y, xx, yy, bkground);
	}
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawAufknuepfung (int _i, int _j)
{
	QPainter* p = currentPainter;
	if (!p) return;

	/*  Bei Schlagpatronenansicht mit pegplanstyle wird nichts
	    gezeichnet -- aufknuepfung is rendered via trittfolge. */
	if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked() &&
	    aufknuepfung.pegplanstyle && !toptobottom) return;

	const int x = aufknuepfung.pos.x0 + _i*aufknuepfung.gw;
	int y;
	if (toptobottom) y = aufknuepfung.pos.y0 + _j*aufknuepfung.gh;
	else             y = aufknuepfung.pos.y0 + aufknuepfung.pos.height - (_j + 1)*aufknuepfung.gh;
	const int xx = x + aufknuepfung.gw;
	const int yy = y + aufknuepfung.gh;

	const QColor bkground = palette().color(QPalette::Button);

	if (!ViewSchlagpatrone || !ViewSchlagpatrone->isChecked()) {
		const char s = aufknuepfung.feld.Get(scroll_x2 + _i, scroll_y1 + _j);
		if (s == AUSHEBUNG) {
			PaintCell(*p, darst_aushebung, x, y, xx, yy,
			          (darst_aushebung == AUSGEFUELLT)
			              ? qColorFromTColor(GetRangeColor(s))
			              : QColor(Qt::black),
			          false, -1, bkground);
		} else if (s == ANBINDUNG) {
			PaintCell(*p, darst_anbindung, x, y, xx, yy,
			          (darst_aushebung == AUSGEFUELLT)
			              ? qColorFromTColor(GetRangeColor(s))
			              : QColor(Qt::black),
			          false, -1, bkground);
		} else if (s == ABBINDUNG) {
			PaintCell(*p, darst_abbindung, x, y, xx, yy,
			          (darst_aushebung == AUSGEFUELLT)
			              ? qColorFromTColor(GetRangeColor(s))
			              : QColor(Qt::black),
			          false, -1, bkground);
		} else if (s > 0) {
			PaintCell(*p, aufknuepfung.darstellung, x, y, xx, yy,
			          qColorFromTColor(GetRangeColor(s)),
			          false, scroll_y1 + _j, bkground);
		} else {
			ClearCell(*p, x, y, xx, yy, bkground);
		}
	} else {
		/*  Schlagpatronen view: any non-zero fills as dark-grey filled. */
		if (aufknuepfung.feld.Get(scroll_x2 + _i, scroll_y1 + _j) > 0) {
			PaintCell(*p, AUSGEFUELLT, x, y, xx, yy,
			          QColor(128, 128, 128), false, -1, bkground);
		} else {
			ClearCell(*p, x, y, xx, yy, bkground);
		}
	}
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawTrittfolge (int _i, int _j)
{
	QPainter* p = currentPainter;
	if (!p) return;

	const int x  = trittfolge.pos.x0 + _i*trittfolge.gw;
	const int y  = trittfolge.pos.y0 + trittfolge.pos.height - (_j + 1)*trittfolge.gh;
	const int xx = x + trittfolge.gw;
	const int yy = y + trittfolge.gh;

	const QColor bkground = palette().color(QPalette::Button);
	const bool  pegplan   = ViewSchlagpatrone && ViewSchlagpatrone->isChecked();

	const char s = trittfolge.feld.Get(scroll_x2 + _i, scroll_y2 + _j);
	if (s == AUSHEBUNG) {
		if (pegplan) {
			PaintCell(*p, darst_aushebung, x, y, xx, yy,
			          (darst_aushebung == AUSGEFUELLT)
			              ? qColorFromTColor(GetRangeColor(s))
			              : QColor(Qt::black),
			          false, -1, bkground);
		} else {
			PaintCell(*p, trittfolge.darstellung, x, y, xx, yy,
			          QColor(Qt::black), false, -1, bkground);
		}
	} else if (s == ANBINDUNG) {
		if (pegplan) {
			PaintCell(*p, darst_anbindung, x, y, xx, yy,
			          (darst_aushebung == AUSGEFUELLT)
			              ? qColorFromTColor(GetRangeColor(s))
			              : QColor(Qt::black),
			          false, -1, bkground);
		} else {
			PaintCell(*p, trittfolge.darstellung, x, y, xx, yy,
			          QColor(Qt::black), false, -1, bkground);
		}
	} else if (s == ABBINDUNG) {
		if (pegplan) {
			PaintCell(*p, darst_abbindung, x, y, xx, yy,
			          (darst_aushebung == AUSGEFUELLT)
			              ? qColorFromTColor(GetRangeColor(s))
			              : QColor(Qt::black),
			          false, -1, bkground);
		} else {
			PaintCell(*p, trittfolge.darstellung, x, y, xx, yy,
			          QColor(Qt::black), false, -1, bkground);
		}
	} else if (s > 0) {
		if (pegplan) {
			PaintCell(*p, schlagpatronendarstellung, x, y, xx, yy,
			          qColorFromTColor(GetRangeColor(s)),
			          false, scroll_x2 + _i, bkground);
		} else {
			PaintCell(*p, trittfolge.darstellung, x, y, xx, yy,
			          QColor(Qt::black), false, -1, bkground);
		}
	} else {
		ClearCell(*p, x, y, xx, yy, bkground);
	}
}
/*-----------------------------------------------------------------*/
