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
#include "palette.h"
#include "colors_compat.h"
#include "legacy_colors.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QScrollBar>

#include <algorithm>

/*  Legacy greys live in legacy_colors.h so draw.cpp shares them. */
#define kLegacyBtnFace legacyBtnFace()
#define kLegacyBtnShadow legacyBtnShadow()

PatternCanvas::PatternCanvas(TDBWFRM* _frm, QWidget* _parent)
    : QWidget(_parent)
    , frm(_frm)
{
    /*  Fill the widget with the legacy light-grey background and pin
        the palette so the status bar / toolbar system look doesn't
        leak into the canvas through QPalette::Button etc. */
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, kLegacyBtnFace);
    pal.setColor(QPalette::Button, kLegacyBtnFace);
    pal.setColor(QPalette::Base, kLegacyBtnFace);
    pal.setColor(QPalette::Dark, kLegacyBtnShadow);
    setPalette(pal);
    /*  Accept keyboard focus so arrow-key navigation works the
        moment the canvas is clicked. */
    setFocusPolicy(Qt::StrongFocus);
    /*  Need hover events (no button held) so the cursor shape can
        change when the pointer crosses a divider gap.            */
    setMouseTracking(true);

    /*  Create the four scrollbars as children. They remain hidden
        until recomputeLayout sizes them; at that point they're
        also show()n (only if their axis actually has off-screen
        content).                                               */
    sbHorz1 = new QScrollBar(Qt::Horizontal, this);
    sbHorz2 = new QScrollBar(Qt::Horizontal, this);
    sbVert1 = new QScrollBar(Qt::Vertical, this);
    sbVert2 = new QScrollBar(Qt::Vertical, this);
    for (QScrollBar* b : { sbHorz1, sbHorz2, sbVert1, sbVert2 })
        b->hide();

    connect(sbHorz1, &QScrollBar::valueChanged, this, [this](int v) {
        if (frm->scroll_x1 == v)
            return;
        frm->scroll_x1 = v;
        update();
    });
    connect(sbHorz2, &QScrollBar::valueChanged, this, [this](int v) {
        if (frm->scroll_x2 == v)
            return;
        frm->scroll_x2 = v;
        update();
    });
    connect(sbVert1, &QScrollBar::valueChanged, this, [this](int v) {
        /*  Legacy inverts sb_vert1 / sb_vert2 so that the top of the
            bar corresponds to the top of the viewport (y grows down)
            while scroll_y1 / y2 are cell indices growing upward. */
        const int inv = sbVert1->maximum() - v;
        if (frm->scroll_y1 == inv)
            return;
        frm->scroll_y1 = inv;
        update();
    });
    connect(sbVert2, &QScrollBar::valueChanged, this, [this](int v) {
        const int inv = sbVert2->maximum() - v;
        if (frm->scroll_y2 == inv)
            return;
        frm->scroll_y2 = inv;
        update();
    });
}

PatternCanvas::~PatternCanvas() = default;

void PatternCanvas::resizeEvent(QResizeEvent* /*_e*/)
{
    /*  Re-layout on every resize so fields and scrollbars reflow
        whenever the window changes size. Tests that want to
        override field positions do so by mutating frm->*.pos
        directly without going through the widget's resize path. */
    recomputeLayout();
}

void PatternCanvas::recomputeLayout(int _gw, int _gh)
{
    /*  Cell size: if not forced by the caller, pick from the zoom
        table keyed by frm->currentzoom. Legacy zoom[10] values. */
    static const int ZOOM_TABLE[10] = { 5, 7, 9, 11, 13, 15, 17, 19, 21, 23 };
    int zi = frm->currentzoom;
    if (zi < 0)
        zi = 0;
    if (zi > 9)
        zi = 9;
    const int GW = _gw > 0 ? _gw : ZOOM_TABLE[zi];
    const int GH = _gh > 0 ? _gh : ZOOM_TABLE[zi];

    constexpr int MARGIN = 10;
    constexpr int SB_SIZE = 16; /* thickness of a scrollbar channel */
    constexpr int DIV = 3;      /* pixel gap between adjacent fields / strips */
    const int W = width();
    const int H = height();
    if (W < 4 * GW || H < 4 * GH)
        return;

    /*  Reserve SB_SIZE at the bottom + right edges for horizontal
        and vertical scrollbars. Everything else lays out inside the
        reduced rectangle.                                        */
    const int usableW = W - SB_SIZE;
    const int usableH = H - SB_SIZE;

    auto cap = [](int area_px, int cell) {
        const int fit = (area_px - 2 * MARGIN) / cell;
        return std::max(0, fit);
    };

    /*  Aufknuepfung: visible shafts x visible treadles (capped to
        fit and clamped to the field maxima). wvisible drives the
        treadle axis (width), hvisible the shaft axis (height). */
    const int fitX = cap(usableW / 3, GW);
    const int fitY = cap(usableH / 3, GH);
    const int wantW = frm->wvisible > 0 ? frm->wvisible : fitX;
    const int wantH = frm->hvisible > 0 ? frm->hvisible : fitY;
    const int af_cells_x = std::clamp(wantW, 1, fitX);
    const int af_cells_y = std::clamp(wantH, 1, fitY);
    const int af_w = af_cells_x * GW;
    const int af_h = af_cells_y * GH;

    frm->aufknuepfung.gw = GW;
    frm->aufknuepfung.gh = GH;
    /*  Aufknuepfung.y0 is finalised after showHlines is known so we
        can push it down by the same BAR + DIV the left column uses.
        Initialise with MARGIN here; the final assignment lives
        below.                                                      */
    frm->aufknuepfung.pos.y0 = MARGIN;
    frm->aufknuepfung.pos.width = af_w;
    frm->aufknuepfung.pos.height = af_h;

    /*  Port of legacy dbw3_form.cpp:RecalcDimensions (einzugunten
        = false / threading-on-top mode). Left column top-to-bottom:
          kettfarben   (GH)   -- hidden when ViewFarbe is off
          einzug       af_h   -- hidden when ViewEinzug is off
          blatteinzug  GH     -- hidden when ViewBlatteinzug is off
          gewebe       rest
        Right column: aufknuepfung top, trittfolge below (aligned
        with gewebe). schussfarben sits on the RIGHT side of
        trittfolge (between trittfolge and the right scrollbar). */

    const bool showFarbe = !frm->ViewFarbe || frm->ViewFarbe->isChecked();
    const bool showEinzug = !frm->ViewEinzug || frm->ViewEinzug->isChecked();
    const bool showBlatteinzug = !frm->ViewBlatteinzug || frm->ViewBlatteinzug->isChecked();
    const bool showTrittfolge = !frm->ViewTrittfolge || frm->ViewTrittfolge->isChecked();
    const bool showHlines = frm->ViewHlines && frm->ViewHlines->isChecked();
    const int BAR = GW; /* hilfslinien bar thickness, zoom-scaled */

    const int schussfarbenW = showFarbe ? GW : 0;
    const int trittfolgeW = showTrittfolge ? af_w : 0;

    /*  Width available for the einzug/gewebe/blatteinzug/kettfarben
        column. The right column eats aufknuepfung width + schuss-
        farben + trittfolge + gaps + hilfslinien vertical bar.     */
    const int rightW = trittfolgeW + (showFarbe ? schussfarbenW + MARGIN : 0)
                       + (showHlines ? BAR + MARGIN : 0);
    const int ez_area_w = usableW - rightW - 3 * MARGIN;
    /*  Cap the visible gewebe / strip width at Data->MAXX1 cells.
        Without this, enlarging the window produces a strip wider
        than the field's backing store -- paintEvent then reads
        kettfarben.feld.Get(i) with i > MAXX1-1 and paints whatever
        memory lies past the allocation. Legacy clamps the same way
        in RecalcDimensions.                                        */
    const int ez_cells = std::max(0, std::min(ez_area_w / GW, Data ? Data->MAXX1 : 0));
    const int colW = ez_cells * GW;
    const int colX = MARGIN;

    /*  Vertical dividers between stacked strips. Legacy inserts a
        `divider` pixel gap between each pair of visible strips
        (farbe/einzug/blatteinzug/gewebe).                       */
    const int fh_div = showFarbe ? DIV : 0;
    const int e_div = showEinzug ? DIV : 0;
    const int b_div = showBlatteinzug ? DIV : 0;

    /*  Legacy places the two horizontal hilfslinien bars at the top
        of the client area; every strip below is pushed down by
        BAR+DIV when ViewHlines is active. The two vertical bars sit
        far-right, between schussfarben and the vertical scrollbar
        (that width is already reserved via rightW above).          */
    const int hlh_div = showHlines ? BAR + DIV : 0;

    frm->kettfarben.gw = GW;
    frm->kettfarben.gh = GH;
    frm->kettfarben.pos.x0 = colX;
    frm->kettfarben.pos.y0 = MARGIN + hlh_div;
    frm->kettfarben.pos.width = showFarbe ? colW : 0;
    frm->kettfarben.pos.height = showFarbe ? GH : 0;

    frm->einzug.gw = GW;
    frm->einzug.gh = GH;
    frm->einzug.pos.x0 = colX;
    frm->einzug.pos.y0 = frm->kettfarben.pos.y0 + frm->kettfarben.pos.height + fh_div;
    frm->einzug.pos.width = showEinzug ? colW : 0;
    frm->einzug.pos.height = showEinzug ? af_h : 0;

    frm->blatteinzug.gw = GW;
    frm->blatteinzug.gh = GH;
    frm->blatteinzug.pos.x0 = colX;
    frm->blatteinzug.pos.y0 = frm->einzug.pos.y0 + frm->einzug.pos.height + e_div;
    frm->blatteinzug.pos.width = showBlatteinzug ? colW : 0;
    frm->blatteinzug.pos.height = showBlatteinzug ? GH : 0;

    const int stripsAboveGewebeH = frm->kettfarben.pos.height + fh_div + frm->einzug.pos.height
                                   + e_div + frm->blatteinzug.pos.height + b_div;
    const int tf_area_h = usableH - 2 * MARGIN - stripsAboveGewebeH;
    /*  Likewise cap the vertical extent at Data->MAXY2 so the
        schussfarben / gewebe row loops don't read past the backing
        schussfarben.feld.                                        */
    const int tf_rows = std::max(0, std::min(tf_area_h / GH, Data ? Data->MAXY2 : 0));

    frm->gewebe.gw = GW;
    frm->gewebe.gh = GH;
    frm->gewebe.pos.x0 = colX;
    frm->gewebe.pos.y0 = frm->blatteinzug.pos.y0 + frm->blatteinzug.pos.height + b_div;
    frm->gewebe.pos.width = colW;
    frm->gewebe.pos.height = tf_rows * GH;

    /*  Right column: aufknuepfung above (aligned vertically with
        einzug), trittfolge below (aligned with gewebe),
        schussfarben further right (aligned with gewebe). */
    frm->trittfolge.gw = GW;
    frm->trittfolge.gh = GH;
    frm->trittfolge.pos.x0 = colX + colW + MARGIN;
    frm->trittfolge.pos.y0 = frm->gewebe.pos.y0;
    frm->trittfolge.pos.width = trittfolgeW;
    frm->trittfolge.pos.height = showTrittfolge ? frm->gewebe.pos.height : 0;

    frm->schussfarben.gw = GW;
    frm->schussfarben.gh = GH;
    frm->schussfarben.pos.x0
        = frm->trittfolge.pos.x0 + frm->trittfolge.pos.width + (showFarbe ? MARGIN : 0);
    frm->schussfarben.pos.y0 = frm->gewebe.pos.y0;
    frm->schussfarben.pos.width = schussfarbenW;
    frm->schussfarben.pos.height = showFarbe ? frm->gewebe.pos.height : 0;

    frm->aufknuepfung.pos.x0 = frm->trittfolge.pos.x0;
    frm->aufknuepfung.pos.y0 = frm->einzug.pos.y0;
    frm->aufknuepfung.pos.width = showTrittfolge ? af_w : 0;
    frm->aufknuepfung.pos.height = showEinzug ? af_h : 0;

    /*  Shift schussfarben / hlinevert to the far right so there's
        room for the vertical bar. When ViewHlines is off the bar is
        zero-width and everything stays where it was.               */
    if (showHlines) {
        const int hlvX = usableW - BAR - MARGIN;
        frm->hlinevert1.gw = BAR;
        frm->hlinevert1.gh = GH;
        frm->hlinevert1.x0 = hlvX;
        frm->hlinevert1.y0 = frm->einzug.pos.y0;
        frm->hlinevert1.width = showEinzug ? BAR : 0;
        frm->hlinevert1.height = showEinzug ? frm->einzug.pos.height : 0;

        frm->hlinevert2.gw = BAR;
        frm->hlinevert2.gh = GH;
        frm->hlinevert2.x0 = hlvX;
        frm->hlinevert2.y0 = frm->gewebe.pos.y0;
        frm->hlinevert2.width = BAR;
        frm->hlinevert2.height = frm->gewebe.pos.height;

        /*  Pull schussfarben to the left of the vertical bar. */
        frm->schussfarben.pos.x0 = hlvX - DIV - frm->schussfarben.pos.width;

        /*  Horizontal bars sit at the very top, above their
            respective columns.                                */
        frm->hlinehorz1.gw = GW;
        frm->hlinehorz1.gh = BAR;
        frm->hlinehorz1.x0 = frm->kettfarben.pos.x0;
        frm->hlinehorz1.y0 = MARGIN;
        frm->hlinehorz1.width = frm->gewebe.pos.width;
        frm->hlinehorz1.height = BAR;

        frm->hlinehorz2.gw = GW;
        frm->hlinehorz2.gh = BAR;
        frm->hlinehorz2.x0 = frm->trittfolge.pos.x0;
        frm->hlinehorz2.y0 = MARGIN;
        frm->hlinehorz2.width = showTrittfolge ? frm->trittfolge.pos.width : 0;
        frm->hlinehorz2.height = showTrittfolge ? BAR : 0;
    } else {
        auto clearBar = [](HlineBar& b) {
            b.x0 = b.y0 = 0;
            b.width = b.height = 0;
            b.gw = b.gh = 0;
        };
        clearBar(frm->hlinehorz1);
        clearBar(frm->hlinehorz2);
        clearBar(frm->hlinevert1);
        clearBar(frm->hlinevert2);
    }

    /*  Clamp scroll offsets so the new viewport never shows past
        the data extent. */
    const int gewebe_cols = frm->gewebe.pos.width / GW;
    const int gewebe_rows = frm->gewebe.pos.height / GH;
    const int einzug_cols = frm->einzug.pos.width / GW;
    const int ezaf_rows = frm->einzug.pos.height / GH;
    const int tf_cols = frm->trittfolge.pos.width / GW;

    const int maxX1 = std::max(0, Data->MAXX1 - std::max(einzug_cols, gewebe_cols));
    const int maxY1 = std::max(0, Data->MAXY1 - ezaf_rows);
    const int maxX2 = std::max(0, Data->MAXX2 - tf_cols);
    const int maxY2 = std::max(0, Data->MAXY2 - gewebe_rows);

    auto clamp = [](int& v, int maximum) {
        if (v < 0)
            v = 0;
        if (v > maximum)
            v = std::max(0, maximum);
    };
    clamp(frm->scroll_x1, maxX1);
    clamp(frm->scroll_y1, maxY1);
    clamp(frm->scroll_x2, maxX2);
    clamp(frm->scroll_y2, maxY2);

    /*  Configure and position the four scrollbars. QScrollBar emits
        valueChanged on setValue unless the new value equals the old
        one; use blockSignals() while we push the current scroll_*
        back so the feedback loop doesn't re-trigger update(). */

    auto setupHorz
        = [&](QScrollBar* sb, int x, int y, int w, int maxScroll, int pageCells, int& scrollVal) {
              if (w <= 0 || pageCells <= 0) {
                  sb->hide();
                  return;
              }
              sb->setGeometry(x, y, w, SB_SIZE);
              sb->blockSignals(true);
              sb->setRange(0, maxScroll);
              sb->setPageStep(pageCells);
              sb->setSingleStep(1);
              sb->setValue(scrollVal);
              sb->blockSignals(false);
              sb->setVisible(maxScroll > 0);
          };

    auto setupVert
        = [&](QScrollBar* sb, int x, int y, int h, int maxScroll, int pageCells, int& scrollVal) {
              if (h <= 0 || pageCells <= 0) {
                  sb->hide();
                  return;
              }
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

    setupHorz(sbHorz1, frm->gewebe.pos.x0, H - SB_SIZE, frm->gewebe.pos.width, maxX1, gewebe_cols,
              frm->scroll_x1);
    setupHorz(sbHorz2, frm->trittfolge.pos.x0, H - SB_SIZE, frm->trittfolge.pos.width, maxX2,
              tf_cols, frm->scroll_x2);
    setupVert(sbVert1, W - SB_SIZE, frm->aufknuepfung.pos.y0, frm->aufknuepfung.pos.height, maxY1,
              ezaf_rows, frm->scroll_y1);
    setupVert(sbVert2, W - SB_SIZE, frm->gewebe.pos.y0, frm->gewebe.pos.height, maxY2, gewebe_rows,
              frm->scroll_y2);
}

/*  Divider hit-test: the 10 px MARGIN gap between gewebe and
    trittfolge (horizontal divider for wvisible) and between einzug
    and gewebe (vertical divider for hvisible). A 6 px tolerance
    lets users grab either side of the gap.                     */
bool PatternCanvas::hoverHDivider(int _x, int _y) const
{
    const int left = frm->gewebe.pos.x0 + frm->gewebe.pos.width;
    const int right = frm->trittfolge.pos.x0;
    if (left >= right)
        return false;
    if (_x < left - 2 || _x > right + 2)
        return false;
    /*  Only react when the pointer is within the combined vertical
        extent of the top and bottom rows. */
    const int top = frm->aufknuepfung.pos.y0;
    const int bottom = frm->trittfolge.pos.y0 + frm->trittfolge.pos.height;
    return _y >= top && _y <= bottom;
}

bool PatternCanvas::hoverVDivider(int _x, int _y) const
{
    /*  The vertical (einzug / gewebe sizing) divider lives in the
        narrow pixel gap immediately above gewebe. When blatteinzug
        is visible that gap is between blatteinzug.bottom and
        gewebe.top; otherwise it's the gap between einzug.bottom
        (or kettfarben.bottom when einzug is hidden) and
        gewebe.top. Hit-testing only this narrow band keeps clicks
        on the strips themselves from being swallowed by the drag
        handle.                                                   */
    const int bottom = frm->gewebe.pos.y0;
    int top = bottom;
    if (frm->blatteinzug.pos.height > 0)
        top = frm->blatteinzug.pos.y0 + frm->blatteinzug.pos.height;
    else if (frm->einzug.pos.height > 0)
        top = frm->einzug.pos.y0 + frm->einzug.pos.height;
    else if (frm->kettfarben.pos.height > 0)
        top = frm->kettfarben.pos.y0 + frm->kettfarben.pos.height;
    if (top >= bottom)
        return false;
    if (_y < top - 1 || _y > bottom + 1)
        return false;
    const int left = frm->gewebe.pos.x0;
    const int right = frm->trittfolge.pos.x0 + frm->trittfolge.pos.width;
    return _x >= left && _x <= right;
}

void PatternCanvas::mousePressEvent(QMouseEvent* _e)
{
    if (_e->button() != Qt::LeftButton) {
        _e->ignore();
        return;
    }
    setFocus(Qt::MouseFocusReason);
    const QPoint p = _e->pos();

    /*  Divider-drag has priority over click-to-select. */
    if (hoverHDivider(p.x(), p.y())) {
        dragKind = DragHDivider;
        setCursor(Qt::SplitHCursor);
        _e->accept();
        return;
    }
    if (hoverVDivider(p.x(), p.y())) {
        dragKind = DragVDivider;
        setCursor(Qt::SplitVCursor);
        _e->accept();
        return;
    }

    const bool shift = _e->modifiers().testFlag(Qt::ShiftModifier);
    const bool ctrl = _e->modifiers().testFlag(Qt::ControlModifier);
    frm->handleCanvasMousePress(p.x(), p.y(), shift, ctrl);
    _e->accept();
}

void PatternCanvas::mouseMoveEvent(QMouseEvent* _e)
{
    const QPoint p = _e->pos();
    const bool leftHeld = (_e->buttons() & Qt::LeftButton);

    if (dragKind != DragNone && leftHeld) {
        /*  Live-resize the treadle count (wvisible) or shaft count
            (hvisible) based on the drag position, and re-run the
            layout. */
        if (dragKind == DragHDivider && frm->trittfolge.gw > 0) {
            const int rightEdge = frm->trittfolge.pos.x0 + frm->trittfolge.pos.width;
            int cells = (rightEdge - p.x()) / frm->trittfolge.gw;
            cells = std::clamp(cells, 1, (int)Data->MAXX2);
            if (cells != frm->wvisible) {
                frm->wvisible = cells;
                recomputeLayout();
                frm->refresh();
            }
        } else if (dragKind == DragVDivider && frm->einzug.gh > 0) {
            const int topEdge = frm->einzug.pos.y0;
            int cells = (p.y() - topEdge) / frm->einzug.gh;
            cells = std::clamp(cells, 1, (int)Data->MAXY1);
            if (cells != frm->hvisible) {
                frm->hvisible = cells;
                recomputeLayout();
                frm->refresh();
            }
        }
        _e->accept();
        return;
    }

    if (leftHeld) {
        const bool shift = _e->modifiers().testFlag(Qt::ShiftModifier);
        frm->handleCanvasMouseMove(p.x(), p.y(), shift);
        _e->accept();
        return;
    }

    /*  Hover: adjust the cursor shape when over a divider so the
        user can see the grab target before clicking.            */
    if (hoverHDivider(p.x(), p.y()))
        setCursor(Qt::SplitHCursor);
    else if (hoverVDivider(p.x(), p.y()))
        setCursor(Qt::SplitVCursor);
    else
        unsetCursor();
    _e->ignore();
}

void PatternCanvas::mouseReleaseEvent(QMouseEvent* _e)
{
    if (_e->button() != Qt::LeftButton) {
        _e->ignore();
        return;
    }
    if (dragKind != DragNone) {
        dragKind = DragNone;
        unsetCursor();
        _e->accept();
        return;
    }
    frm->handleCanvasMouseRelease();
    _e->accept();
}

void PatternCanvas::contextMenuEvent(QContextMenuEvent* _e)
{
    frm->handleContextMenu(_e->globalPos());
    _e->accept();
}

void PatternCanvas::keyPressEvent(QKeyEvent* _e)
{
    frm->handleCanvasKeyPress(_e->key(), int(_e->modifiers()));
    _e->accept();
}

void PatternCanvas::wheelEvent(QWheelEvent* _e)
{
    /*  Mouse wheel: scroll the gewebe vertically by one cell per
        notch. Shift+wheel scrolls horizontally. Ctrl+wheel zooms. */
    const QPoint p = _e->angleDelta();
    const int steps = (p.y() != 0 ? p.y() : p.x()) / 120;
    if (steps == 0) {
        _e->ignore();
        return;
    }

    if (_e->modifiers().testFlag(Qt::ControlModifier)) {
        if (steps > 0)
            for (int i = 0; i < steps; i++)
                frm->zoomIn();
        else
            for (int i = 0; i < -steps; i++)
                frm->zoomOut();
    } else if (_e->modifiers().testFlag(Qt::ShiftModifier)) {
        /*  Route through the scrollbars so their thumb position
            tracks the wheel-driven scroll. The valueChanged signal
            writes back to frm->scroll_* and triggers update().     */
        if (sbHorz1->isVisible())
            sbHorz1->setValue(sbHorz1->value() - steps);
        if (sbHorz2->isVisible())
            sbHorz2->setValue(sbHorz2->value() - steps);
    } else {
        /*  Vertical scrollbars use inverted values, so a "wheel
            down => content moves up" matches a value increase. */
        if (sbVert1->isVisible())
            sbVert1->setValue(sbVert1->value() - steps);
        if (sbVert2->isVisible())
            sbVert2->setValue(sbVert2->value() - steps);
    }
    _e->accept();
}

void PatternCanvas::paintEvent(QPaintEvent* /*_e*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    /*  Legacy filled the whole form with clBtnFace. Use the hard-
        coded legacy grey instead of the theme's QPalette::Button
        so the canvas stays readable on dark desktops.          */
    p.fillRect(rect(), kLegacyBtnFace);

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

    /*  Matches legacy redraw.cpp: under ViewOnlyGewebe (for einzug /
        aufknuepfung / trittfolge) and under GewebeNone (for gewebe),
        the field is blanked to btnFace first, then only the Rahmen
        (grid lines + strongline + outer frame) is drawn. Cell
        symbols / fills are skipped. */
    auto paintField
        = [this, &p](FeldBase& fb, void (TDBWFRM::*draw)(int, int),
                     void (TDBWFRM::*rahmen)(int, int), bool _blankContent) {
              if (fb.gw <= 0 || fb.gh <= 0)
                  return;
              if (fb.pos.width <= 0 || fb.pos.height <= 0)
                  return;
              if (_blankContent) {
                  p.setPen(Qt::NoPen);
                  p.setBrush(kLegacyBtnFace);
                  p.drawRect(fb.pos.x0, fb.pos.y0, fb.pos.width, fb.pos.height);
              }
              const int cols = fb.pos.width / fb.gw;
              const int rows = fb.pos.height / fb.gh;
              /*  Cells first, Rahmen second so the strongline lines draw
                  on top of cell fills without getting overwritten. */
              if (!_blankContent) {
                  for (int i = 0; i < cols; i++)
                      for (int j = 0; j < rows; j++)
                          (frm->*draw)(i, j);
              }
              if (rahmen) {
                  for (int i = 0; i < cols; i++)
                      for (int j = 0; j < rows; j++)
                          (frm->*rahmen)(i, j);
              }
          };

    const bool onlyGewebe = frm->ViewOnlyGewebe && frm->ViewOnlyGewebe->isChecked();
    const bool gewebeNone = frm->GewebeNone && frm->GewebeNone->isChecked();

    if (frm->ViewEinzug && frm->ViewEinzug->isChecked()) {
        paintField(frm->einzug, &TDBWFRM::DrawEinzug, &TDBWFRM::DrawEinzugRahmen, onlyGewebe);
        paintField(frm->aufknuepfung, &TDBWFRM::DrawAufknuepfung,
                   &TDBWFRM::DrawAufknuepfungRahmen, onlyGewebe);
    }

    if (frm->ViewTrittfolge && frm->ViewTrittfolge->isChecked())
        paintField(frm->trittfolge, &TDBWFRM::DrawTrittfolge, &TDBWFRM::DrawTrittfolgeRahmen,
                   onlyGewebe);

    paintField(frm->gewebe, &TDBWFRM::DrawGewebe, &TDBWFRM::DrawGewebeRahmen, gewebeNone);

    /*  Outer frames. DrawXRahmen only paints top/left borders per
        cell, so the last column / row is unframed. Match legacy's
        plural DrawX() by closing each field with a btn-shadow
        rectangle. */
    auto frameField = [&](const FeldBase& fb) {
        if (fb.pos.width <= 0 || fb.pos.height <= 0)
            return;
        p.setPen(QPen(legacyBtnShadow()));
        p.setBrush(Qt::NoBrush);
        p.drawRect(fb.pos.x0, fb.pos.y0, fb.pos.width, fb.pos.height);
    };
    if (frm->ViewEinzug && frm->ViewEinzug->isChecked()) {
        frameField(frm->einzug);
        if (frm->ViewTrittfolge && frm->ViewTrittfolge->isChecked())
            frameField(frm->aufknuepfung);
    }
    if (frm->ViewTrittfolge && frm->ViewTrittfolge->isChecked())
        frameField(frm->trittfolge);
    frameField(frm->gewebe);

    /*  Colour strips + blatteinzug. Renderers faithful to legacy
        redraw.cpp:DrawKettfarben / DrawSchussfarben /
        DrawBlatteinzug -- each cell has a clBtnShadow boundary
        line and the outer strip is framed.                    */
    auto palCol = [](int _idx) { return qColorFromTColor((TColor)Data->palette->GetColor(_idx)); };
    auto frameRect = [&](const GRIDPOS& g) {
        p.setPen(QPen(kLegacyBtnShadow));
        p.setBrush(Qt::NoBrush);
        p.drawRect(g.x0, g.y0, g.width, g.height);
    };

    /*  Direction-aware data-index lookup. kettfarben / blatteinzug
        flow along the kette (warp) axis and share right-to-left
        handling. schussfarben flows along the weft (top-to-bottom)
        axis. Width/height are already clamped to Data->MAX* in
        recomputeLayout above, so `cols`/`rows` here never exceed
        the backing-feld size -- but we still pick the source index
        via the orientation flags so the cells draw in the same
        visual order as legacy.                                  */

    /*  kettfarben: horizontal strip, one palette-coloured cell per
        warp thread, separated by a clBtnShadow vertical hairline. */
    if (frm->kettfarben.gw > 0 && frm->kettfarben.pos.width > 0 && Data && Data->palette) {
        const int cols = frm->kettfarben.pos.width / frm->kettfarben.gw;
        const int y0 = frm->kettfarben.pos.y0;
        const int h = frm->kettfarben.pos.height;
        for (int i = 0; i < cols; i++) {
            const int idx = frm->righttoleft ? frm->scroll_x1 + (cols - 1 - i)
                                             : frm->scroll_x1 + i;
            const int x = frm->kettfarben.pos.x0 + i * frm->kettfarben.gw;
            p.setPen(Qt::NoPen);
            p.setBrush(palCol(frm->kettfarben.feld.Get(idx)));
            p.drawRect(x, y0, frm->kettfarben.gw, h);
            p.setPen(QPen(kLegacyBtnShadow));
            p.drawLine(x, y0, x, y0 + h);
        }
        frameRect(frm->kettfarben.pos);
    }

    /*  schussfarben: vertical strip right of trittfolge, one
        palette-coloured cell per weft thread. Always bottom-up
        (j=0 at the bottom) -- legacy DrawSchussfarben doesn't
        branch on toptobottom because the gewebe grid itself also
        stays bottom-up regardless of that flag. Mirroring schuss-
        farben here would misalign it with the rows of the pattern
        next to it.                                              */
    if (frm->schussfarben.gh > 0 && frm->schussfarben.pos.height > 0 && Data && Data->palette) {
        const int rows = frm->schussfarben.pos.height / frm->schussfarben.gh;
        const int x0 = frm->schussfarben.pos.x0;
        const int w = frm->schussfarben.pos.width;
        for (int j = 0; j < rows; j++) {
            const int y = frm->schussfarben.pos.y0 + frm->schussfarben.pos.height
                - (j + 1) * frm->schussfarben.gh;
            p.setPen(Qt::NoPen);
            p.setBrush(palCol(frm->schussfarben.feld.Get(frm->scroll_y2 + j)));
            p.drawRect(x0, y, w, frm->schussfarben.gh);
            p.setPen(QPen(kLegacyBtnShadow));
            p.drawLine(x0, y, x0 + w, y);
        }
        frameRect(frm->schussfarben.pos);
    }

    /*  blatteinzug: horizontal strip with a split-in-half cell per
        reed slot. value==1 -> black mark in the TOP half,
        button-face in the bottom; value==0 -> mark in the BOTTOM
        half, button-face on top. Cells separated by clBtnShadow
        hairlines. Port of legacy DrawBlatteinzug().             */
    if (frm->blatteinzug.gw > 0 && frm->blatteinzug.pos.width > 0) {
        const int cols = frm->blatteinzug.pos.width / frm->blatteinzug.gw;
        const int y0 = frm->blatteinzug.pos.y0;
        const int h = frm->blatteinzug.pos.height;
        const int half = h / 2;
        for (int i = 0; i < cols; i++) {
            const int idx = frm->righttoleft ? frm->scroll_x1 + (cols - 1 - i)
                                             : frm->scroll_x1 + i;
            const int x = frm->blatteinzug.pos.x0 + i * frm->blatteinzug.gw;
            const int gw = frm->blatteinzug.gw;
            const bool on = frm->blatteinzug.feld.Get(idx);
            p.setPen(Qt::NoPen);
            /*  Top half */
            p.setBrush(on ? QColor(Qt::black) : kLegacyBtnFace);
            p.drawRect(x, y0, gw, half);
            /*  Bottom half */
            p.setBrush(on ? kLegacyBtnFace : QColor(Qt::black));
            p.drawRect(x, y0 + half, gw, h - half);
            /*  Left-edge separator */
            p.setPen(QPen(kLegacyBtnShadow));
            p.drawLine(x, y0, x, y0 + h);
        }
        frameRect(frm->blatteinzug.pos);
    }

    /*  Rapport boundary markers overlay the einzug / trittfolge
        strips (not the gewebe). Gated on RappViewRapport.        */
    if (frm->RappViewRapport && frm->RappViewRapport->isChecked())
        frm->DrawRapport();

    /*  Hilfslinien (guide lines) drawn after rapport so they sit on
        top of cell fills but under the cursor outline. */
    if (frm->ViewHlines && frm->ViewHlines->isChecked()) {
        /*  Paint the four grab bars. Each bar is a btnFace rectangle
            with a dkGray border; horizontal bars have short ticks at
            every cell boundary pointing downwards, vertical bars
            have short ticks pointing rightwards. Port of legacy
            DrawHlineBars.                                          */
        const QColor dkGray(105, 105, 105);
        auto paintBar = [&](const HlineBar& b, bool horizontal) {
            if (b.width <= 0 || b.height <= 0)
                return;
            p.setBrush(kLegacyBtnFace);
            p.setPen(QPen(dkGray));
            p.drawRect(b.x0, b.y0, b.width, b.height);
            if (horizontal && b.gw > 0) {
                for (int x = b.x0; x < b.x0 + b.width; x += b.gw) {
                    p.drawLine(x, b.y0 + b.height - 5, x, b.y0 + b.height - 1);
                }
            } else if (!horizontal && b.gh > 0) {
                for (int y = b.y0; y < b.y0 + b.height; y += b.gh) {
                    p.drawLine(b.x0 + 1, y, b.x0 + 5, y);
                }
            }
        };
        paintBar(frm->hlinehorz1, true);
        paintBar(frm->hlinehorz2, true);
        paintBar(frm->hlinevert1, false);
        paintBar(frm->hlinevert2, false);

        frm->DrawHilfslinien();
    }

    /*  Rubber-band selection rectangle sits on top of the cells
        and guide lines; cursor paints after so it remains visible
        even when inside a selection.                          */
    frm->DrawSelection();

    /*  Tool-drag preview overlay: when the user is dragging with a
        non-POINT tool, outline the prospective shape in red between
        the anchor cell (tool_i0/j0) and the current cell
        (tool_i1/j1). Coordinates are in data space; translate back
        to viewport pixels via the gewebe field geometry. */
    if (frm->tool_active && frm->gewebe.gw > 0 && frm->gewebe.gh > 0 && frm->gewebe.pos.width > 0
        && frm->gewebe.pos.height > 0) {
        const int gw = frm->gewebe.gw;
        const int gh = frm->gewebe.gh;
        const int cols = frm->gewebe.pos.width / gw;
        const int rows = frm->gewebe.pos.height / gh;
        auto cellRect = [&](int _di, int _dj) {
            int vi = _di - frm->scroll_x1;
            int vj = _dj - frm->scroll_y2;
            if (frm->righttoleft)
                vi = cols - 1 - vi;
            if (frm->toptobottom)
                vj = rows - 1 - vj;
            const int x = frm->gewebe.pos.x0 + vi * gw;
            const int y = frm->gewebe.pos.y0 + frm->gewebe.pos.height - (vj + 1) * gh;
            return QRect(x, y, gw, gh);
        };
        const QRect r0 = cellRect(frm->tool_i0, frm->tool_j0);
        const QRect r1 = cellRect(frm->tool_i1, frm->tool_j1);
        p.setPen(QPen(Qt::red, 2));
        p.setBrush(Qt::NoBrush);
        switch (frm->tool) {
        case TOOL_LINE:
            p.drawLine(r0.center(), r1.center());
            break;
        case TOOL_RECTANGLE:
        case TOOL_FILLEDRECTANGLE: {
            const int x = std::min(r0.left(), r1.left());
            const int y = std::min(r0.top(), r1.top());
            const int xx = std::max(r0.right(), r1.right());
            const int yy = std::max(r0.bottom(), r1.bottom());
            p.drawRect(QRect(x, y, xx - x, yy - y));
            break;
        }
        case TOOL_ELLIPSE:
        case TOOL_FILLEDELLIPSE: {
            const int x = std::min(r0.left(), r1.left());
            const int y = std::min(r0.top(), r1.top());
            const int xx = std::max(r0.right(), r1.right());
            const int yy = std::max(r0.bottom(), r1.bottom());
            p.drawEllipse(QRect(x, y, xx - x, yy - y));
            break;
        }
        default:
            break;
        }
    }

    /*  Cursor outline sits on top of everything so it's visible
        no matter which view mode is active. Skipped during the
        "off" phase of the blink cycle. */
    if (frm->cursorhandler && frm->cursorVisible)
        frm->cursorhandler->DrawCursor();

    frm->currentPainter = nullptr;
}
