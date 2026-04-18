/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  The pattern-editor drawing surface. Lives inside TDBWFRM as the
    central widget and is the Qt equivalent of the legacy VCL form's
    Canvas. paintEvent() exposes its QPainter to TDBWFRM via the
    currentPainter pointer so that ported DrawX methods can keep their
    single-cell signatures; the iteration over visible cells happens
    here.

    Initial scope: paints gewebe only. Subsequent rendering slices
    extend paintEvent to cover einzug, aufknuepfung, trittfolge, the
    colour strips, the rapport lines and the cursor. */

#ifndef DBWEAVE_UI_PATTERNCANVAS_H
#define DBWEAVE_UI_PATTERNCANVAS_H

#include <QWidget>

class QScrollBar;
class TDBWFRM;

class PatternCanvas : public QWidget
{
	Q_OBJECT

public:
	explicit PatternCanvas (TDBWFRM* _frm, QWidget* _parent = nullptr);
	~PatternCanvas() override;

	/*  Lay out the four field grids inside the widget rect using
	    cell size `_gw` x `_gh`. Called by resizeEvent on the first
	    resize, and by TDBWFRM::zoomIn/zoomOut/zoomNormal thereafter.
	    A cell size of 0 picks the current zoom from frm->currentzoom.
	    Safe to call before the widget has ever been shown (tests). */
	void recomputeLayout (int _gw = 0, int _gh = 0);

protected:
	void paintEvent      (QPaintEvent*  _e) override;
	void resizeEvent     (QResizeEvent* _e) override;
	void wheelEvent      (QWheelEvent*  _e) override;
	void mousePressEvent   (QMouseEvent*         _e) override;
	void mouseMoveEvent    (QMouseEvent*         _e) override;
	void mouseReleaseEvent (QMouseEvent*         _e) override;
	void keyPressEvent     (QKeyEvent*           _e) override;
	void contextMenuEvent  (QContextMenuEvent*   _e) override;

private:
	TDBWFRM* frm;
	/*  Auto-layout runs only on the first resize; tests that manually
	    place field positions set this to true after calling resize()
	    so subsequent resize events don't stomp their layout.         */
	bool has_laid_out = false;

	/*  Four scrollbars matching legacy sb_horz1 / sb_horz2 / sb_vert1
	    / sb_vert2: one per (scroll_x1, scroll_x2, scroll_y1, scroll_y2)
	    axis. Positioned in recomputeLayout along the matching field
	    extents; their value is bound to the corresponding frm->scroll_*
	    member and a change triggers update().                       */
	QScrollBar* sbHorz1 = nullptr;   /* warp axis   (scroll_x1) */
	QScrollBar* sbHorz2 = nullptr;   /* treadle     (scroll_x2) */
	QScrollBar* sbVert1 = nullptr;   /* shaft axis  (scroll_y1) */
	QScrollBar* sbVert2 = nullptr;   /* weft axis   (scroll_y2) */
};

#endif
