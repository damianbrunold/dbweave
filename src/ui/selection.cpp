/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope:
      * RANGE::Valid / Normalize / LeftDown / TopRight value-type
        helpers (ported verbatim).
      * TDBWFRM::ClearSelection / DrawSelection / ResizeSelection
        - Qt-simplified. The legacy ClearSelection had a ~200-line
          redraw dance to erase a previously-drawn VCL rectangle by
          re-painting the Rahmen cells underneath it. Under Qt the
          canvas repaints wholesale on refresh(), so "clear" is just
          an assignment + refresh.
        - DrawSelection draws a white 1px rectangle around the
          selected cells during paintEvent.
        - ResizeSelection begins or grows the rubber-band; `_square`
          constrains to a square drag.
      * ApplyRangeToSelection walks every cell in the selection and
        drives the matching Set* op with the given range value.

    Deferred (each its own later slice):
      * CutSelection / CopySelection / PasteSelection (clipboard)
      * EditInvert / EditMirrorHorz / EditMirrorVert / EditRotate /
        EditDelete / EditCentralsym (transform ops).
      * RollUp / RollDown / RollLeft / RollRight.
*/

#include "mainwindow.h"
#include "datamodule.h"
#include "cursor.h"

#include <QPainter>
#include <QPen>
#include <algorithm>
#include <cstdlib>

/*-----------------------------------------------------------------*/
bool __fastcall RANGE::Valid()
{
	return begin.i >= 0 && begin.j >= 0 &&
	       end.i > -1   && end.j > -1   &&
	       feld != INVALID;
}

void __fastcall RANGE::Normalize()
{
	const int minx = std::min(begin.i, end.i);
	const int miny = std::min(begin.j, end.j);
	const int maxx = std::max(begin.i, end.i);
	const int maxy = std::max(begin.j, end.j);
	begin = PT(minx, miny);
	end   = PT(maxx, maxy);
}

PT __fastcall RANGE::LeftDown()  { Normalize(); return begin; }
PT __fastcall RANGE::TopRight()  { Normalize(); return end;   }

/*-----------------------------------------------------------------*/
/*  Map the (normalised) selection rectangle back to pixel coords
    on the canvas. Layout is identical to the legacy routine in
    selection.cpp::CalcSelectionRect. _x/_y are the top-left and
    _xx/_yy are the bottom-right of the rectangle (with _yy > _y
    because the y axis points down in Qt).                      */
static void calcSelectionRect (TDBWFRM* frm, int& _x, int& _y, int& _xx, int& _yy)
{
	RANGE sel = frm->selection;
	sel.Normalize();

	switch (sel.feld) {
	case EINZUG: {
		if (frm->toptobottom) {
			_y  = frm->einzug.pos.y0 + (sel.end.j   - frm->scroll_y1 + 1) * frm->einzug.gh;
			_yy = frm->einzug.pos.y0 + (sel.begin.j - frm->scroll_y1    ) * frm->einzug.gh;
		} else {
			_y  = frm->einzug.pos.y0 + frm->einzug.pos.height - (sel.end.j   - frm->scroll_y1 + 1) * frm->einzug.gh;
			_yy = frm->einzug.pos.y0 + frm->einzug.pos.height - (sel.begin.j - frm->scroll_y1    ) * frm->einzug.gh;
		}
		if (frm->righttoleft) {
			_x  = frm->einzug.pos.x0 + frm->einzug.pos.width - (sel.begin.i - frm->scroll_x1    ) * frm->einzug.gw;
			_xx = frm->einzug.pos.x0 + frm->einzug.pos.width - (sel.end.i   - frm->scroll_x1 + 1) * frm->einzug.gw;
		} else {
			_x  = frm->einzug.pos.x0 + (sel.begin.i - frm->scroll_x1    ) * frm->einzug.gw;
			_xx = frm->einzug.pos.x0 + (sel.end.i   - frm->scroll_x1 + 1) * frm->einzug.gw;
		}
		break;
	}
	case AUFKNUEPFUNG: {
		if (frm->toptobottom) {
			_y  = frm->aufknuepfung.pos.y0 + (sel.end.j   - frm->scroll_y1 + 1) * frm->aufknuepfung.gh;
			_yy = frm->aufknuepfung.pos.y0 + (sel.begin.j - frm->scroll_y1    ) * frm->aufknuepfung.gh;
		} else {
			_y  = frm->aufknuepfung.pos.y0 + frm->aufknuepfung.pos.height - (sel.end.j   - frm->scroll_y1 + 1) * frm->aufknuepfung.gh;
			_yy = frm->aufknuepfung.pos.y0 + frm->aufknuepfung.pos.height - (sel.begin.j - frm->scroll_y1    ) * frm->aufknuepfung.gh;
		}
		_x  = frm->aufknuepfung.pos.x0 + (sel.begin.i - frm->scroll_x2    ) * frm->aufknuepfung.gw;
		_xx = frm->aufknuepfung.pos.x0 + (sel.end.i   - frm->scroll_x2 + 1) * frm->aufknuepfung.gw;
		break;
	}
	case TRITTFOLGE:
		_x  = frm->trittfolge.pos.x0 + (sel.begin.i - frm->scroll_x2    ) * frm->trittfolge.gw;
		_y  = frm->trittfolge.pos.y0 + frm->trittfolge.pos.height - (sel.end.j   - frm->scroll_y2 + 1) * frm->trittfolge.gh;
		_xx = frm->trittfolge.pos.x0 + (sel.end.i   - frm->scroll_x2 + 1) * frm->trittfolge.gw;
		_yy = frm->trittfolge.pos.y0 + frm->trittfolge.pos.height - (sel.begin.j - frm->scroll_y2    ) * frm->trittfolge.gh;
		break;
	case GEWEBE: {
		if (frm->righttoleft) {
			_x  = frm->gewebe.pos.x0 + frm->gewebe.pos.width - (sel.begin.i - frm->scroll_x1    ) * frm->gewebe.gw;
			_xx = frm->gewebe.pos.x0 + frm->gewebe.pos.width - (sel.end.i   - frm->scroll_x1 + 1) * frm->gewebe.gw;
		} else {
			_x  = frm->gewebe.pos.x0 + (sel.begin.i - frm->scroll_x1    ) * frm->gewebe.gw;
			_xx = frm->gewebe.pos.x0 + (sel.end.i   - frm->scroll_x1 + 1) * frm->gewebe.gw;
		}
		_y  = frm->gewebe.pos.y0 + frm->gewebe.pos.height - (sel.end.j   - frm->scroll_y2 + 1) * frm->gewebe.gh;
		_yy = frm->gewebe.pos.y0 + frm->gewebe.pos.height - (sel.begin.j - frm->scroll_y2    ) * frm->gewebe.gh;
		break;
	}
	case BLATTEINZUG:
		_y  = frm->blatteinzug.pos.y0;
		_yy = frm->blatteinzug.pos.y0 + frm->blatteinzug.pos.height;
		if (frm->righttoleft) {
			_x  = frm->blatteinzug.pos.x0 + frm->blatteinzug.pos.width - (sel.begin.i - frm->scroll_x1    ) * frm->blatteinzug.gw;
			_xx = frm->blatteinzug.pos.x0 + frm->blatteinzug.pos.width - (sel.end.i   - frm->scroll_x1 + 1) * frm->blatteinzug.gw;
		} else {
			_x  = frm->blatteinzug.pos.x0 + (sel.begin.i - frm->scroll_x1    ) * frm->blatteinzug.gw;
			_xx = frm->blatteinzug.pos.x0 + (sel.end.i   - frm->scroll_x1 + 1) * frm->blatteinzug.gw;
		}
		break;
	case KETTFARBEN:
		_y  = frm->kettfarben.pos.y0;
		_yy = frm->kettfarben.pos.y0 + frm->kettfarben.pos.height;
		if (frm->righttoleft) {
			_x  = frm->kettfarben.pos.x0 + frm->kettfarben.pos.width - (sel.begin.i - frm->scroll_x1    ) * frm->kettfarben.gw;
			_xx = frm->kettfarben.pos.x0 + frm->kettfarben.pos.width - (sel.end.i   - frm->scroll_x1 + 1) * frm->kettfarben.gw;
		} else {
			_x  = frm->kettfarben.pos.x0 + (sel.begin.i - frm->scroll_x1    ) * frm->kettfarben.gw;
			_xx = frm->kettfarben.pos.x0 + (sel.end.i   - frm->scroll_x1 + 1) * frm->kettfarben.gw;
		}
		break;
	case SCHUSSFARBEN:
		_x  = frm->schussfarben.pos.x0;
		_y  = frm->schussfarben.pos.y0 + frm->schussfarben.pos.height - (sel.end.j   - frm->scroll_y2 + 1) * frm->schussfarben.gh;
		_xx = frm->schussfarben.pos.x0 + frm->schussfarben.pos.width;
		_yy = frm->schussfarben.pos.y0 + frm->schussfarben.pos.height - (sel.begin.j - frm->scroll_y2    ) * frm->schussfarben.gh;
		break;
	default:
		_x = _y = _xx = _yy = 0;
	}
	if (_x > _xx) std::swap(_x, _xx);
	if (_y > _yy) std::swap(_y, _yy);
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ClearSelection()
{
	selection.Clear();
	refresh();
}

void __fastcall TDBWFRM::DrawSelection()
{
	QPainter* p = currentPainter;
	if (!p) return;
	if (!selection.Valid()) return;
	if (selection.feld == GEWEBE &&
	    GewebeNone && GewebeNone->isChecked()) return;

	int x, y, xx, yy;
	calcSelectionRect(this, x, y, xx, yy);

	p->setPen(QPen(QColor(Qt::white)));
	p->setBrush(Qt::NoBrush);
	/*  Use drawLine x4 instead of drawRect so the pixel coordinates
	    exactly match the VCL MoveTo/LineTo output. drawRect would
	    inset by 1px on the bottom-right. */
	p->drawLine(x,  y,  xx, y);
	p->drawLine(xx, y,  xx, yy);
	p->drawLine(xx, yy, x,  yy);
	p->drawLine(x,  yy, x,  y);
}

void __fastcall TDBWFRM::ResizeSelection (int _i, int _j, FELD _feld, bool _square)
{
	if (!selection.Valid()) {
		selection.begin = selection.end = PT(_i, _j);
		selection.feld  = _feld;
		refresh();
		return;
	}
	if (_feld != selection.feld) return;

	if (_square) {
		const int di = std::abs(_i - selection.begin.i);
		const int dj = std::abs(_j - selection.begin.j);
		const int d  = di - dj;
		if (d > 0) {
			if (_i > selection.begin.i) _i -= d;
			else                        _i += d;
		} else if (d < 0) {
			if (_j > selection.begin.j) _j += d;
			else                        _j -= d;
		}
	}
	if (selection.end.i != _i || selection.end.j != _j) {
		selection.end = PT(_i, _j);
		refresh();
	}
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ApplyRangeToSelection (int _range)
{
	if (!selection.Valid()) return;

	RANGE sel = selection;
	sel.Normalize();

	const FELD f = sel.feld;
	/*  Set* editor ops take viewport-local coords (they add scroll
	    internally). sel.* are data coords, so subtract the matching
	    scroll offset per field.                                   */
	int sx = 0, sy = 0;
	switch (f) {
	case GEWEBE:       sx = scroll_x1; sy = scroll_y2; break;
	case EINZUG:       sx = scroll_x1; sy = scroll_y1; break;
	case AUFKNUEPFUNG: sx = scroll_x2; sy = scroll_y1; break;
	case TRITTFOLGE:   sx = scroll_x2; sy = scroll_y2; break;
	default: return;   /* strips don't use digit-range apply */
	}

	for (int i = sel.begin.i; i <= sel.end.i; i++) {
		for (int j = sel.begin.j; j <= sel.end.j; j++) {
			switch (f) {
			case GEWEBE:       SetGewebe      (i - sx, j - sy, /*_set=*/true, _range); break;
			case EINZUG:       SetEinzug      (i - sx, j - sy);                          break;
			case AUFKNUEPFUNG: SetAufknuepfung(i - sx, j - sy, /*_set=*/true, _range); break;
			case TRITTFOLGE:   SetTrittfolge  (i - sx, j - sy, /*_set=*/true, _range); break;
			default: break;
			}
		}
	}
}
