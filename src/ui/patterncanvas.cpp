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

	/*  Iterate the visible cells of each field and paint them one by
	    one via the DrawX primitives. Degenerate fields (gw/gh == 0
	    or empty width/height == "not laid out yet") are skipped.

	    Paint order matches legacy: the three symbol grids first
	    (einzug / aufknuepfung / trittfolge), then gewebe on top.
	    Colour strips (kettfarben / schussfarben), blatteinzug,
	    hilfslinien, rapport lines and the cursor land in later
	    rendering slices. */

	auto paintField = [this](FeldBase& fb, void (TDBWFRM::*draw)(int, int)) {
		if (fb.gw <= 0 || fb.gh <= 0) return;
		if (fb.pos.width <= 0 || fb.pos.height <= 0) return;
		const int cols = fb.pos.width  / fb.gw;
		const int rows = fb.pos.height / fb.gh;
		for (int i = 0; i < cols; i++)
			for (int j = 0; j < rows; j++)
				(frm->*draw)(i, j);
	};

	if (frm->ViewEinzug && frm->ViewEinzug->isChecked()) {
		paintField(frm->einzug,       &TDBWFRM::DrawEinzug);
		paintField(frm->aufknuepfung, &TDBWFRM::DrawAufknuepfung);
	}

	if (frm->ViewTrittfolge && frm->ViewTrittfolge->isChecked())
		paintField(frm->trittfolge,   &TDBWFRM::DrawTrittfolge);

	paintField(frm->gewebe,           &TDBWFRM::DrawGewebe);

	frm->currentPainter = nullptr;
}
