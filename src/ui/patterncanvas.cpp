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
#include "cursor.h"
#include "datamodule.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QScrollBar>

#include <algorithm>

PatternCanvas::PatternCanvas (TDBWFRM* _frm, QWidget* _parent)
	: QWidget(_parent)
	, frm(_frm)
{
	/*  Legacy filled the form background with the system button-face
	    colour (clBtnFace). Set the widget palette so QWidget's default
	    paint matches.                                               */
	setAutoFillBackground(true);
	/*  Accept keyboard focus so arrow-key navigation works the
	    moment the canvas is clicked. */
	setFocusPolicy(Qt::StrongFocus);

	/*  Create the four scrollbars as children. They remain hidden
	    until recomputeLayout sizes them; at that point they're
	    also show()n (only if their axis actually has off-screen
	    content).                                               */
	sbHorz1 = new QScrollBar(Qt::Horizontal, this);
	sbHorz2 = new QScrollBar(Qt::Horizontal, this);
	sbVert1 = new QScrollBar(Qt::Vertical,   this);
	sbVert2 = new QScrollBar(Qt::Vertical,   this);
	for (QScrollBar* b : { sbHorz1, sbHorz2, sbVert1, sbVert2 }) b->hide();

	connect(sbHorz1, &QScrollBar::valueChanged, this, [this](int v) {
		if (frm->scroll_x1 == v) return; frm->scroll_x1 = v; update();
	});
	connect(sbHorz2, &QScrollBar::valueChanged, this, [this](int v) {
		if (frm->scroll_x2 == v) return; frm->scroll_x2 = v; update();
	});
	connect(sbVert1, &QScrollBar::valueChanged, this, [this](int v) {
		/*  Legacy inverts sb_vert1 / sb_vert2 so that the top of the
		    bar corresponds to the top of the viewport (y grows down)
		    while scroll_y1 / y2 are cell indices growing upward. */
		const int inv = sbVert1->maximum() - v;
		if (frm->scroll_y1 == inv) return; frm->scroll_y1 = inv; update();
	});
	connect(sbVert2, &QScrollBar::valueChanged, this, [this](int v) {
		const int inv = sbVert2->maximum() - v;
		if (frm->scroll_y2 == inv) return; frm->scroll_y2 = inv; update();
	});
}

PatternCanvas::~PatternCanvas() = default;

void PatternCanvas::resizeEvent (QResizeEvent* /*_e*/)
{
	/*  Auto-layout runs only on the first resize event so tests that
	    manually place fields aren't stomped; zoom handlers re-trigger
	    the layout explicitly via recomputeLayout(). */
	if (has_laid_out) return;
	has_laid_out = true;
	recomputeLayout();
}

void PatternCanvas::recomputeLayout (int _gw, int _gh)
{
	/*  Cell size: if not forced by the caller, pick from the zoom
	    table keyed by frm->currentzoom. Legacy zoom[10] values. */
	static const int ZOOM_TABLE[10] = { 5, 7, 9, 11, 13, 15, 17, 19, 21, 23 };
	int zi = frm->currentzoom;
	if (zi < 0) zi = 0;
	if (zi > 9) zi = 9;
	const int GW = _gw > 0 ? _gw : ZOOM_TABLE[zi];
	const int GH = _gh > 0 ? _gh : ZOOM_TABLE[zi];

	constexpr int MARGIN  = 10;
	constexpr int SB_SIZE = 16;   /* thickness of a scrollbar channel */
	const int W = width();
	const int H = height();
	if (W < 4*GW || H < 4*GH) return;

	/*  Reserve SB_SIZE at the bottom + right edges for horizontal
	    and vertical scrollbars. Everything else lays out inside the
	    reduced rectangle.                                        */
	const int usableW = W - SB_SIZE;
	const int usableH = H - SB_SIZE;

	auto cap = [](int area_px, int cell) {
		const int fit = (area_px - 2*MARGIN) / cell;
		return std::max(0, fit);
	};

	/*  Aufknuepfung: 20 shafts wide, 20 shafts tall (capped to fit). */
	const int af_cells_x = std::min(20, cap(usableW / 3, GW));
	const int af_cells_y = std::min(20, cap(usableH / 3, GH));
	const int af_w = af_cells_x * GW;
	const int af_h = af_cells_y * GH;

	frm->aufknuepfung.gw = GW; frm->aufknuepfung.gh = GH;
	frm->aufknuepfung.pos.x0     = usableW - af_w - MARGIN;
	frm->aufknuepfung.pos.y0     = MARGIN;
	frm->aufknuepfung.pos.width  = af_w;
	frm->aufknuepfung.pos.height = af_h;

	const int ez_area_w = usableW - af_w - 3*MARGIN;
	const int ez_cells  = std::max(0, ez_area_w / GW);
	frm->einzug.gw = GW; frm->einzug.gh = GH;
	frm->einzug.pos.x0     = MARGIN;
	frm->einzug.pos.y0     = MARGIN;
	frm->einzug.pos.width  = ez_cells * GW;
	frm->einzug.pos.height = af_h;

	const int tf_area_h = usableH - af_h - 3*MARGIN;
	const int tf_rows   = std::max(0, tf_area_h / GH);
	frm->trittfolge.gw = GW; frm->trittfolge.gh = GH;
	frm->trittfolge.pos.x0     = frm->aufknuepfung.pos.x0;
	frm->trittfolge.pos.y0     = frm->aufknuepfung.pos.y0 + af_h + MARGIN;
	frm->trittfolge.pos.width  = af_w;
	frm->trittfolge.pos.height = tf_rows * GH;

	frm->gewebe.gw = GW; frm->gewebe.gh = GH;
	frm->gewebe.pos.x0     = frm->einzug.pos.x0;
	frm->gewebe.pos.y0     = frm->trittfolge.pos.y0;
	frm->gewebe.pos.width  = frm->einzug.pos.width;
	frm->gewebe.pos.height = frm->trittfolge.pos.height;

	/*  Clamp scroll offsets so the new viewport never shows past
	    the data extent. */
	const int gewebe_cols = frm->gewebe.pos.width  / GW;
	const int gewebe_rows = frm->gewebe.pos.height / GH;
	const int einzug_cols = frm->einzug.pos.width  / GW;
	const int ezaf_rows   = frm->einzug.pos.height / GH;
	const int tf_cols     = frm->trittfolge.pos.width / GW;

	const int maxX1 = std::max(0, Data->MAXX1 - std::max(einzug_cols, gewebe_cols));
	const int maxY1 = std::max(0, Data->MAXY1 - ezaf_rows);
	const int maxX2 = std::max(0, Data->MAXX2 - tf_cols);
	const int maxY2 = std::max(0, Data->MAXY2 - gewebe_rows);

	auto clamp = [](int& v, int maximum) {
		if (v < 0) v = 0;
		if (v > maximum) v = std::max(0, maximum);
	};
	clamp(frm->scroll_x1, maxX1);
	clamp(frm->scroll_y1, maxY1);
	clamp(frm->scroll_x2, maxX2);
	clamp(frm->scroll_y2, maxY2);

	/*  Configure and position the four scrollbars. QScrollBar emits
	    valueChanged on setValue unless the new value equals the old
	    one; use blockSignals() while we push the current scroll_*
	    back so the feedback loop doesn't re-trigger update(). */

	auto setupHorz = [&](QScrollBar* sb, int x, int y, int w,
	                     int maxScroll, int pageCells, int& scrollVal)
	{
		if (w <= 0 || pageCells <= 0) { sb->hide(); return; }
		sb->setGeometry(x, y, w, SB_SIZE);
		sb->blockSignals(true);
		sb->setRange(0, maxScroll);
		sb->setPageStep(pageCells);
		sb->setSingleStep(1);
		sb->setValue(scrollVal);
		sb->blockSignals(false);
		sb->setVisible(maxScroll > 0);
	};

	auto setupVert = [&](QScrollBar* sb, int x, int y, int h,
	                     int maxScroll, int pageCells, int& scrollVal)
	{
		if (h <= 0 || pageCells <= 0) { sb->hide(); return; }
		sb->setGeometry(x, y, SB_SIZE, h);
		sb->blockSignals(true);
		sb->setRange(0, maxScroll);
		sb->setPageStep(pageCells);
		sb->setSingleStep(1);
		/*  Invert because the bar top maps to high scroll_y values
		    in the legacy (y grows up). */
		sb->setValue(maxScroll - scrollVal);
		sb->blockSignals(false);
		sb->setVisible(maxScroll > 0);
	};

	setupHorz(sbHorz1,
	          frm->gewebe.pos.x0,
	          H - SB_SIZE,
	          frm->gewebe.pos.width,
	          maxX1, gewebe_cols, frm->scroll_x1);
	setupHorz(sbHorz2,
	          frm->trittfolge.pos.x0,
	          H - SB_SIZE,
	          frm->trittfolge.pos.width,
	          maxX2, tf_cols, frm->scroll_x2);
	setupVert(sbVert1,
	          W - SB_SIZE,
	          frm->aufknuepfung.pos.y0,
	          frm->aufknuepfung.pos.height,
	          maxY1, ezaf_rows, frm->scroll_y1);
	setupVert(sbVert2,
	          W - SB_SIZE,
	          frm->gewebe.pos.y0,
	          frm->gewebe.pos.height,
	          maxY2, gewebe_rows, frm->scroll_y2);
}

void PatternCanvas::mousePressEvent (QMouseEvent* _e)
{
	if (_e->button() != Qt::LeftButton) { _e->ignore(); return; }
	setFocus(Qt::MouseFocusReason);
	const QPoint p = _e->pos();
	const bool shift = _e->modifiers().testFlag(Qt::ShiftModifier);
	frm->handleCanvasMousePress(p.x(), p.y(), shift);
	_e->accept();
}

void PatternCanvas::mouseMoveEvent (QMouseEvent* _e)
{
	if (!(_e->buttons() & Qt::LeftButton)) { _e->ignore(); return; }
	const QPoint p = _e->pos();
	const bool shift = _e->modifiers().testFlag(Qt::ShiftModifier);
	frm->handleCanvasMouseMove(p.x(), p.y(), shift);
	_e->accept();
}

void PatternCanvas::mouseReleaseEvent (QMouseEvent* _e)
{
	if (_e->button() != Qt::LeftButton) { _e->ignore(); return; }
	frm->handleCanvasMouseRelease();
	_e->accept();
}

void PatternCanvas::contextMenuEvent (QContextMenuEvent* _e)
{
	frm->handleContextMenu(_e->globalPos());
	_e->accept();
}

void PatternCanvas::keyPressEvent (QKeyEvent* _e)
{
	frm->handleCanvasKeyPress(_e->key(), int(_e->modifiers()));
	_e->accept();
}

void PatternCanvas::wheelEvent (QWheelEvent* _e)
{
	/*  Mouse wheel: scroll the gewebe vertically by one cell per
	    notch. Shift+wheel scrolls horizontally. Ctrl+wheel zooms. */
	const QPoint p = _e->angleDelta();
	const int steps = (p.y() != 0 ? p.y() : p.x()) / 120;
	if (steps == 0) { _e->ignore(); return; }

	if (_e->modifiers().testFlag(Qt::ControlModifier)) {
		if (steps > 0) for (int i = 0; i < steps; i++)  frm->zoomIn();
		else            for (int i = 0; i < -steps; i++) frm->zoomOut();
	} else if (_e->modifiers().testFlag(Qt::ShiftModifier)) {
		/*  Route through the scrollbars so their thumb position
		    tracks the wheel-driven scroll. The valueChanged signal
		    writes back to frm->scroll_* and triggers update().     */
		if (sbHorz1->isVisible()) sbHorz1->setValue(sbHorz1->value() - steps);
		if (sbHorz2->isVisible()) sbHorz2->setValue(sbHorz2->value() - steps);
	} else {
		/*  Vertical scrollbars use inverted values, so a "wheel
		    down => content moves up" matches a value increase. */
		if (sbVert1->isVisible()) sbVert1->setValue(sbVert1->value() - steps);
		if (sbVert2->isVisible()) sbVert2->setValue(sbVert2->value() - steps);
	}
	_e->accept();
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

	/*  Hilfslinien (guide lines) drawn after rapport so they sit on
	    top of cell fills but under the cursor outline. */
	if (frm->ViewHlines && frm->ViewHlines->isChecked())
		frm->DrawHilfslinien();

	/*  Rubber-band selection rectangle sits on top of the cells
	    and guide lines; cursor paints after so it remains visible
	    even when inside a selection.                          */
	frm->DrawSelection();

	/*  Cursor outline sits on top of everything so it's visible
	    no matter which view mode is active. Skipped during the
	    "off" phase of the blink cycle. */
	if (frm->cursorhandler && frm->cursorVisible) frm->cursorhandler->DrawCursor();

	frm->currentPainter = nullptr;
}
