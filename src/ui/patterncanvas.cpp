/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "patterncanvas.h"
#include "mainwindow.h"

#include <QPainter>
#include <QPaintEvent>

PatternCanvas::PatternCanvas (TDBWFRM* _frm, QWidget* _parent)
	: QWidget(_parent)
	, frm(_frm)
{
	/*  Legacy filled the form background with the system button-face
	    colour (clBtnFace). Set the widget palette so QWidget's default
	    paint matches.                                               */
	setAutoFillBackground(true);
}

PatternCanvas::~PatternCanvas() = default;

void PatternCanvas::paintEvent (QPaintEvent* /*_e*/)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, false);

	/*  Legacy filled the whole form with clBtnFace (palette button).
	    Match that here so cells paint on top of a neutral grey.    */
	p.fillRect(rect(), palette().color(QPalette::Button));

	/*  Expose the painter to TDBWFRM so the legacy-style DrawX cell
	    primitives can paint without each one taking a painter arg.
	    Clear on exit so any stray DrawX call outside paintEvent
	    (from a menu action or undo restore) safely no-ops. */
	frm->currentPainter = &p;

	/*  Iterate the visible gewebe cells and paint each one. The
	    gewebe grid occupies
	      [frm->gewebe.pos.x0, x0 + width) x
	      [frm->gewebe.pos.y0, y0 + height)
	    with cells of gewebe.gw x gewebe.gh. A degenerate grid
	    (gw == 0 or height == 0) means "not laid out yet" -- skip. */
	if (frm->gewebe.gw > 0 && frm->gewebe.gh > 0 &&
	    frm->gewebe.pos.width > 0 && frm->gewebe.pos.height > 0)
	{
		const int cols = frm->gewebe.pos.width  / frm->gewebe.gw;
		const int rows = frm->gewebe.pos.height / frm->gewebe.gh;
		for (int i = 0; i < cols; i++)
			for (int j = 0; j < rows; j++)
				frm->DrawGewebe(i, j);
	}

	frm->currentPainter = nullptr;
}
