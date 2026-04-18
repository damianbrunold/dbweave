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
#include <QResizeEvent>

#include <algorithm>

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

void PatternCanvas::resizeEvent (QResizeEvent* /*_e*/)
{
	/*  Auto-layout runs only on the first resize event. Subsequent
	    resizes (QWidget::render re-fires one; a real user resize will
	    too) are a no-op so tests and custom layout code can manually
	    place fields without being stomped. When a real FormResize
	    port lands, this flag goes away and the layout re-flows. */
	if (has_laid_out) return;
	has_laid_out = true;

	/*  Placeholder layout until the legacy TDBWFRM::FormResize port
	    lands. Splits the widget into four quadrants:
	        aufknuepfung  top-right
	        einzug        top-left  (wider than aufknuepfung)
	        trittfolge    bottom-right
	        gewebe        bottom-left (the main fabric area)
	    Cell size is fixed at 12x12 px. Fields are capped at a
	    reasonable viewport (scroll offsets remain at 0).           */

	constexpr int GW     = 12;
	constexpr int GH     = 12;
	constexpr int MARGIN = 10;

	const int W = width();
	const int H = height();
	if (W < 4*GW || H < 4*GH) return;   /* too small to lay out anything */

	auto cap = [](int area_px, int cell) {
		const int fit = (area_px - 2*MARGIN) / cell;
		return std::max(0, fit);
	};

	/*  Aufknuepfung: 20 shafts wide, 20 shafts tall (capped to fit). */
	const int af_cells_x = std::min(20, cap(W / 3,   GW));
	const int af_cells_y = std::min(20, cap(H / 3,   GH));
	const int af_w = af_cells_x * GW;
	const int af_h = af_cells_y * GH;

	frm->aufknuepfung.gw = GW; frm->aufknuepfung.gh = GH;
	frm->aufknuepfung.pos.x0     = W - af_w - MARGIN;
	frm->aufknuepfung.pos.y0     = MARGIN;
	frm->aufknuepfung.pos.width  = af_w;
	frm->aufknuepfung.pos.height = af_h;

	/*  Einzug: same height as aufknuepfung, remaining width left.    */
	const int ez_area_w = W - af_w - 3*MARGIN;
	const int ez_cells  = std::max(0, ez_area_w / GW);
	frm->einzug.gw = GW; frm->einzug.gh = GH;
	frm->einzug.pos.x0     = MARGIN;
	frm->einzug.pos.y0     = MARGIN;
	frm->einzug.pos.width  = ez_cells * GW;
	frm->einzug.pos.height = af_h;

	/*  Trittfolge: same width as aufknuepfung, remaining height below. */
	const int tf_area_h = H - af_h - 3*MARGIN;
	const int tf_rows   = std::max(0, tf_area_h / GH);
	frm->trittfolge.gw = GW; frm->trittfolge.gh = GH;
	frm->trittfolge.pos.x0     = frm->aufknuepfung.pos.x0;
	frm->trittfolge.pos.y0     = frm->aufknuepfung.pos.y0 + af_h + MARGIN;
	frm->trittfolge.pos.width  = af_w;
	frm->trittfolge.pos.height = tf_rows * GH;

	/*  Gewebe: main area under einzug and left of trittfolge. */
	frm->gewebe.gw = GW; frm->gewebe.gh = GH;
	frm->gewebe.pos.x0     = frm->einzug.pos.x0;
	frm->gewebe.pos.y0     = frm->trittfolge.pos.y0;
	frm->gewebe.pos.width  = frm->einzug.pos.width;
	frm->gewebe.pos.height = frm->trittfolge.pos.height;
}

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

	auto paintField = [this](FeldBase& fb,
	                         void (TDBWFRM::*draw)  (int, int),
	                         void (TDBWFRM::*rahmen)(int, int)) {
		if (fb.gw <= 0 || fb.gh <= 0) return;
		if (fb.pos.width <= 0 || fb.pos.height <= 0) return;
		const int cols = fb.pos.width  / fb.gw;
		const int rows = fb.pos.height / fb.gh;
		/*  Cells first, Rahmen second so the strongline lines draw
		    on top of cell fills without getting overwritten. */
		for (int i = 0; i < cols; i++)
			for (int j = 0; j < rows; j++)
				(frm->*draw)(i, j);
		if (rahmen) {
			for (int i = 0; i < cols; i++)
				for (int j = 0; j < rows; j++)
					(frm->*rahmen)(i, j);
		}
	};

	if (frm->ViewEinzug && frm->ViewEinzug->isChecked()) {
		paintField(frm->einzug,       &TDBWFRM::DrawEinzug,       &TDBWFRM::DrawEinzugRahmen);
		paintField(frm->aufknuepfung, &TDBWFRM::DrawAufknuepfung, &TDBWFRM::DrawAufknuepfungRahmen);
	}

	if (frm->ViewTrittfolge && frm->ViewTrittfolge->isChecked())
		paintField(frm->trittfolge,   &TDBWFRM::DrawTrittfolge,   &TDBWFRM::DrawTrittfolgeRahmen);

	paintField(frm->gewebe,           &TDBWFRM::DrawGewebe,       &TDBWFRM::DrawGewebeRahmen);

	/*  Rapport boundary markers overlay the einzug / trittfolge
	    strips (not the gewebe). Gated on RappViewRapport.        */
	if (frm->RappViewRapport && frm->RappViewRapport->isChecked())
		frm->DrawRapport();

	frm->currentPainter = nullptr;
}
