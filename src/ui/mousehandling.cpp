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
      * Physical2Logical + HitCheck pixel-to-cell lookup -- full port
        of the legacy algorithm covering every FELD that currently
        exists in the port (BLATTEINZUG, KETTFARBEN, EINZUG, GEWEBE,
        AUFKNUEPFUNG, TRITTFOLGE, SCHUSSFARBEN). Hlinehorz/vert hit
        rectangles are deferred alongside the mouse-drag-guide-line
        slice.
      * handleCanvasMousePress -- minimal click-to-paint: left click
        sets the cursor to the clicked cell and drives the matching
        Set* state-apply op. Shift clears; no drag, no rubber-band
        selection, no tool palette, no right-click context menu.
*/

#include "mainwindow.h"
#include "cursor.h"
#include "enums.h"

static bool HitCheck (const GRIDPOS& _grid, int _x, int _y,
                      int _scrollx, int _scrolly, int& _i, int& _j,
                      int _gw, int _gh,
                      bool _righttoleft = false, bool _toptobottom = false)
{
	(void)_scrollx; (void)_scrolly;
	if (_gw <= 0 || _gh <= 0) return false;
	if (_x <= _grid.x0 || _x >= _grid.x0 + _grid.width)  return false;
	if (_y <= _grid.y0 || _y >= _grid.y0 + _grid.height) return false;
	if (_righttoleft) _i = (_grid.x0 + _grid.width  - _x) / _gw;
	else              _i = (_x - _grid.x0)                 / _gw;
	if (_toptobottom) _j = (_y - _grid.y0)                 / _gh;
	else              _j = (_grid.y0 + _grid.height - _y) / _gh;
	return true;
}

void __fastcall TDBWFRM::Physical2Logical (int _x, int _y, FELD& _feld, int& _i, int& _j)
{
	const bool rtl = righttoleft;
	const bool ttb = toptobottom;
	_feld = INVALID;
	_i = _j = 0;

	if      (HitCheck (blatteinzug.pos,   _x, _y, scroll_x1, 0,        _i, _j, blatteinzug.gw,   blatteinzug.gh,   rtl))       _feld = BLATTEINZUG;
	else if (HitCheck (kettfarben.pos,    _x, _y, scroll_x1, 0,        _i, _j, kettfarben.gw,    kettfarben.gh,    rtl))       _feld = KETTFARBEN;
	else if (HitCheck (einzug.pos,        _x, _y, scroll_x1, scroll_y1, _i, _j, einzug.gw,        einzug.gh,        rtl, ttb)) _feld = EINZUG;
	else if (HitCheck (gewebe.pos,        _x, _y, scroll_x1, scroll_y2, _i, _j, gewebe.gw,        gewebe.gh,        rtl))      _feld = GEWEBE;
	else if (HitCheck (aufknuepfung.pos,  _x, _y, scroll_x2, scroll_y1, _i, _j, aufknuepfung.gw,  aufknuepfung.gh,  false, ttb)) _feld = AUFKNUEPFUNG;
	else if (HitCheck (trittfolge.pos,    _x, _y, scroll_x2, scroll_y2, _i, _j, trittfolge.gw,    trittfolge.gh))              _feld = TRITTFOLGE;
	else if (HitCheck (schussfarben.pos,  _x, _y, 0,         scroll_y2, _i, _j, schussfarben.gw,  schussfarben.gh))            _feld = SCHUSSFARBEN;
}

void __fastcall TDBWFRM::handleCanvasMousePress (int _x, int _y, bool _shift)
{
	FELD f;
	int  i, j;
	Physical2Logical(_x, _y, f, i, j);
	if (f == INVALID) return;

	const int r = currentrange;

	/*  (i, j) come back from Physical2Logical as viewport-local cell
	    indices. Cursor SetCursor takes data coords, so it gets
	    (i + scroll, j + scroll); Set* editor ops expect viewport
	    coords (they add scroll internally), so they see (i, j)
	    untouched.                                                 */
	switch (f) {
	case GEWEBE:
		if (cursorhandler) cursorhandler->SetCursor(f, i + scroll_x1, j + scroll_y2, true);
		SetGewebe(i, j, !_shift, r);
		break;
	case EINZUG:
		if (cursorhandler) cursorhandler->SetCursor(f, i + scroll_x1, j + scroll_y1, true);
		SetEinzug(i, j);
		break;
	case AUFKNUEPFUNG:
		if (cursorhandler) cursorhandler->SetCursor(f, i + scroll_x2, j + scroll_y1, true);
		SetAufknuepfung(i, j, !_shift, r);
		break;
	case TRITTFOLGE:
		if (cursorhandler) cursorhandler->SetCursor(f, i + scroll_x2, j + scroll_y2, true);
		SetTrittfolge(i, j, !_shift, r);
		break;
	case KETTFARBEN:
		if (cursorhandler) cursorhandler->SetCursor(f, i + scroll_x1, 0, true);
		SetKettfarben(i);
		break;
	case SCHUSSFARBEN:
		if (cursorhandler) cursorhandler->SetCursor(f, 0, j + scroll_y2, true);
		SetSchussfarben(j);
		break;
	case BLATTEINZUG:
		if (cursorhandler) cursorhandler->SetCursor(f, i + scroll_x1, 0, true);
		SetBlatteinzug(i);
		break;
	default:
		break;
	}
	update();
}
