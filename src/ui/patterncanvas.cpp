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
#include <QApplication>

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

    /*  Scrollbar wiring mirrors legacy scrolling.cpp:
          sb_horz1 (kettfarben/einzug/gewebe/blatteinzug along warp)
            -- flips on righttoleft so the thumb visually tracks the
            near edge (right side when righttoleft is on).
          sb_horz2 (aufknuepfung/trittfolge along treadle) -- no flip.
          sb_vert1 (einzug/aufknuepfung along shaft) -- flips unless
            toptobottom is on (cell j=0 is at the BOTTOM by default;
            the scrollbar thumb at the TOP should show the highest j).
          sb_vert2 (gewebe/trittfolge along weft) -- always flips;
            legacy ignores toptobottom here because the gewebe /
            trittfolge grids stay bottom-up regardless.            */
    connect(sbHorz1, &QScrollBar::valueChanged, this, [this](int v) {
        const int inv = frm->righttoleft ? sbHorz1->maximum() - v : v;
        if (frm->scroll_x1 == inv)
            return;
        frm->scroll_x1 = inv;
        update();
    });
    connect(sbHorz2, &QScrollBar::valueChanged, this, [this](int v) {
        if (frm->scroll_x2 == v)
            return;
        frm->scroll_x2 = v;
        update();
    });
    connect(sbVert1, &QScrollBar::valueChanged, this, [this](int v) {
        const int inv = frm->toptobottom ? v : sbVert1->maximum() - v;
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

void PatternCanvas::syncScrollbarsFromFrm()
{
    /*  Inverse of the valueChanged lambdas above: compute the slider
        value that corresponds to the desired frm->scroll_* value.
        QAbstractSlider::setValue is a no-op when the value is
        unchanged, so this is safe to call unconditionally.         */
    if (sbHorz1) {
        const int v = frm->righttoleft ? sbHorz1->maximum() - frm->scroll_x1 : frm->scroll_x1;
        sbHorz1->setValue(v);
    }
    if (sbHorz2) {
        sbHorz2->setValue(frm->scroll_x2);
    }
    if (sbVert1) {
        const int v = frm->toptobottom ? frm->scroll_y1 : sbVert1->maximum() - frm->scroll_y1;
        sbVert1->setValue(v);
    }
    if (sbVert2) {
        sbVert2->setValue(sbVert2->maximum() - frm->scroll_y2);
    }
}

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
    /*  Weft/warp ratio: the larger of faktor_kette / faktor_schuss
        keeps its axis at ZOOM_TABLE[zi]; the smaller-factor axis is
        stretched by the ratio. Formula matches legacy dbw3_form.cpp
        CalcGrid (lines 412-427). Result is non-square cells whenever
        the two factors differ.                                       */
    int gw_def = ZOOM_TABLE[zi];
    int gh_def = ZOOM_TABLE[zi];
    const float fk = frm->faktor_kette;
    const float fs = frm->faktor_schuss;
    if (fk > 0.0f && fs > 0.0f) {
        if (fs > fk)
            gh_def = int(double(ZOOM_TABLE[zi]) * fs / fk);
        else if (fk > fs)
            gw_def = int(double(ZOOM_TABLE[zi]) * fk / fs);
    }
    const int GW = _gw > 0 ? _gw : gw_def;
    const int GH = _gh > 0 ? _gh : gh_def;

    constexpr int MARGIN = 10;
    constexpr int SB_SIZE = 16; /* thickness of a scrollbar channel */
    /*  Inter-strip gap: zoom-scaled, matches legacy RecalcDimensions
        (`divider = zoom[min(currentzoom,3)] * 3 / 4`). For cell sizes
        up to zoom[3]=11 this is ~3/4 of a cell; capped at zoom 3 so
        very large cells don't produce huge gaps. Used for every
        inner gap between adjacent visible fields / strips.         */
    const int divider = (std::min(GW, ZOOM_TABLE[3]) * 3) / 4;
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

    /*  Port of legacy dbw3_form.cpp:RecalcDimensions. Two layout
        modes selected by frm->einzugunten:

        einzugunten = false (default, "threading on top"):
          left column top-to-bottom = kettfarben, einzug,
              blatteinzug, gewebe;
          right column = aufknuepfung above (with einzug),
              trittfolge below (with gewebe).

        einzugunten = true ("threading below pattern"):
          left column top-to-bottom = gewebe, blatteinzug, einzug,
              kettfarben;
          right column = trittfolge above (with gewebe),
              aufknuepfung below (with einzug).

        schussfarben sits on the RIGHT side of trittfolge (between
        trittfolge and the right scrollbar) in either mode.       */

    const bool showFarbe = !frm->ViewFarbe || frm->ViewFarbe->isChecked();
    const bool showEinzug = !frm->ViewEinzug || frm->ViewEinzug->isChecked();
    const bool showBlatteinzug = !frm->ViewBlatteinzug || frm->ViewBlatteinzug->isChecked();
    const bool showTrittfolge = !frm->ViewTrittfolge || frm->ViewTrittfolge->isChecked();
    const bool showHlines = frm->ViewHlines && frm->ViewHlines->isChecked();
    const bool einzugunten = frm->einzugunten;
    const int BAR = GW; /* hilfslinien bar thickness, zoom-scaled */

    const int schussfarbenW = showFarbe ? GW : 0;
    const int trittfolgeW = showTrittfolge ? af_w : 0;

    /*  Width available for the einzug/gewebe/blatteinzug/kettfarben
        column. The right column eats aufknuepfung width + schuss-
        farben + trittfolge + inter-strip gaps + hilfslinien bar.  */
    const int rightW = trittfolgeW + (showFarbe ? schussfarbenW + divider : 0)
                       + (showHlines ? BAR + divider : 0);
    /*  Left outer margin (colX) + gewebe↔trittfolge gap (divider)
        + right outer margin. */
    const int ez_area_w = usableW - rightW - 2 * MARGIN - divider;
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
        `divider` pixel gap between each pair of visible strips.
        Meaning of each div depends on einzugunten:
          !einzugunten: fh_div between kettfarben↔einzug,
                        e_div  between einzug↔blatteinzug,
                        b_div  between blatteinzug↔gewebe.
          einzugunten:  g_div  between gewebe↔blatteinzug,
                        b_div  between blatteinzug↔einzug,
                        fh_div between einzug↔kettfarben.
        Total reserved space is the same in either mode.          */
    const int fh_div = showFarbe ? divider : 0;
    const int e_div = (showEinzug && !einzugunten) ? divider : 0;
    const int g_div = (showEinzug && einzugunten) ? divider : 0;
    const int b_div = showBlatteinzug ? divider : 0;

    /*  Legacy places the two horizontal hilfslinien bars at the top
        of the client area; every strip below is pushed down by
        BAR+divider when ViewHlines is active. The two vertical bars
        sit far-right, between schussfarben and the vertical
        scrollbar (that width is reserved via rightW above).        */
    const int hlh_div = showHlines ? BAR + divider : 0;

    /*  Strip heights are independent of einzugunten. Positions are
        not -- compute heights first, then place each strip according
        to the mode. */
    const int kettfarbenH = showFarbe ? GH : 0;
    const int einzugH = showEinzug ? af_h : 0;
    const int blatteinzugH = showBlatteinzug ? GH : 0;
    const int stripsBesideGewebeH
        = kettfarbenH + fh_div + einzugH + e_div + blatteinzugH + b_div + g_div;
    /*  hlh_div is added to the top column; the gewebe bottom must
        stay above the horizontal scrollbar channel, so we subtract
        hlh_div from the available height. Without this the gewebe
        slid down under the scrollbar when ViewHlines was active
        with large cells.                                            */
    const int tf_area_h = usableH - 2 * MARGIN - stripsBesideGewebeH - hlh_div;
    /*  Cap the vertical extent at Data->MAXY2 so the schussfarben
        / gewebe row loops don't read past the backing
        schussfarben.feld.                                            */
    const int tf_rows = std::max(0, std::min(tf_area_h / GH, Data ? Data->MAXY2 : 0));
    const int gewebeH = tf_rows * GH;

    frm->kettfarben.gw = GW;
    frm->kettfarben.gh = GH;
    frm->kettfarben.pos.x0 = colX;
    frm->kettfarben.pos.width = showFarbe ? colW : 0;
    frm->kettfarben.pos.height = kettfarbenH;

    frm->einzug.gw = GW;
    frm->einzug.gh = GH;
    frm->einzug.pos.x0 = colX;
    frm->einzug.pos.width = showEinzug ? colW : 0;
    frm->einzug.pos.height = einzugH;

    frm->blatteinzug.gw = GW;
    frm->blatteinzug.gh = GH;
    frm->blatteinzug.pos.x0 = colX;
    frm->blatteinzug.pos.width = showBlatteinzug ? colW : 0;
    frm->blatteinzug.pos.height = blatteinzugH;

    frm->gewebe.gw = GW;
    frm->gewebe.gh = GH;
    frm->gewebe.pos.x0 = colX;
    frm->gewebe.pos.width = colW;
    frm->gewebe.pos.height = gewebeH;

    if (!einzugunten) {
        /*  Top-to-bottom: kettfarben, einzug, blatteinzug, gewebe. */
        frm->kettfarben.pos.y0 = MARGIN + hlh_div;
        frm->einzug.pos.y0 = frm->kettfarben.pos.y0 + kettfarbenH + fh_div;
        frm->blatteinzug.pos.y0 = frm->einzug.pos.y0 + einzugH + e_div;
        frm->gewebe.pos.y0 = frm->blatteinzug.pos.y0 + blatteinzugH + b_div;
    } else {
        /*  Top-to-bottom: gewebe, blatteinzug, einzug, kettfarben. */
        frm->gewebe.pos.y0 = MARGIN + hlh_div;
        frm->blatteinzug.pos.y0 = frm->gewebe.pos.y0 + gewebeH + g_div;
        frm->einzug.pos.y0 = frm->blatteinzug.pos.y0 + blatteinzugH + b_div;
        frm->kettfarben.pos.y0 = frm->einzug.pos.y0 + einzugH + fh_div;
    }

    /*  Right column: aufknuepfung above (aligned vertically with
        einzug), trittfolge below (aligned with gewebe),
        schussfarben further right (aligned with gewebe). */
    frm->trittfolge.gw = GW;
    frm->trittfolge.gh = GH;
    frm->trittfolge.pos.x0 = colX + colW + divider;
    frm->trittfolge.pos.y0 = frm->gewebe.pos.y0;
    frm->trittfolge.pos.width = trittfolgeW;
    frm->trittfolge.pos.height = showTrittfolge ? frm->gewebe.pos.height : 0;

    frm->schussfarben.gw = GW;
    frm->schussfarben.gh = GH;
    frm->schussfarben.pos.x0
        = frm->trittfolge.pos.x0 + frm->trittfolge.pos.width + (showFarbe ? divider : 0);
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
        frm->schussfarben.pos.x0 = hlvX - divider - frm->schussfarben.pos.width;

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

    /*  `invert` mirrors the legacy scrolling.cpp convention: if set,
        bar value 0 maps to the highest scroll_* index. Per-bar policy:
          sbHorz1: invert when righttoleft
          sbHorz2: never invert
          sbVert1: invert unless toptobottom
          sbVert2: always invert                                    */
    auto setupHorz = [&](QScrollBar* sb, int x, int y, int w, int maxScroll, int pageCells,
                         int& scrollVal, bool invert) {
        if (w <= 0 || pageCells <= 0) {
            sb->hide();
            return;
        }
        sb->setGeometry(x, y, w, SB_SIZE);
        sb->blockSignals(true);
        sb->setRange(0, maxScroll);
        sb->setPageStep(pageCells);
        sb->setSingleStep(1);
        sb->setValue(invert ? maxScroll - scrollVal : scrollVal);
        sb->blockSignals(false);
        sb->setVisible(maxScroll > 0);
    };

    auto setupVert = [&](QScrollBar* sb, int x, int y, int h, int maxScroll, int pageCells,
                         int& scrollVal, bool invert) {
        if (h <= 0 || pageCells <= 0) {
            sb->hide();
            return;
        }
        sb->setGeometry(x, y, SB_SIZE, h);
        sb->blockSignals(true);
        sb->setRange(0, maxScroll);
        sb->setPageStep(pageCells);
        sb->setSingleStep(1);
        sb->setValue(invert ? maxScroll - scrollVal : scrollVal);
        sb->blockSignals(false);
        sb->setVisible(maxScroll > 0);
    };

    setupHorz(sbHorz1, frm->gewebe.pos.x0, H - SB_SIZE, frm->gewebe.pos.width, maxX1, gewebe_cols,
              frm->scroll_x1, frm->righttoleft);
    setupHorz(sbHorz2, frm->trittfolge.pos.x0, H - SB_SIZE, frm->trittfolge.pos.width, maxX2,
              tf_cols, frm->scroll_x2, false);
    setupVert(sbVert1, W - SB_SIZE, frm->aufknuepfung.pos.y0, frm->aufknuepfung.pos.height, maxY1,
              ezaf_rows, frm->scroll_y1, !frm->toptobottom);
    setupVert(sbVert2, W - SB_SIZE, frm->gewebe.pos.y0, frm->gewebe.pos.height, maxY2, gewebe_rows,
              frm->scroll_y2, true);
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
        narrow pixel gap that separates gewebe from the adjacent
        strip stack. In default mode that gap is above gewebe
        (between blatteinzug/einzug/kettfarben.bottom and
        gewebe.top). In einzugunten mode gewebe is on top, so the
        gap is below gewebe (between gewebe.bottom and the
        blatteinzug/einzug/kettfarben.top below it). Hit-testing
        only this narrow band keeps clicks on the strips
        themselves from being swallowed by the drag handle.       */
    int top, bottom;
    if (!frm->einzugunten) {
        bottom = frm->gewebe.pos.y0;
        top = bottom;
        if (frm->blatteinzug.pos.height > 0)
            top = frm->blatteinzug.pos.y0 + frm->blatteinzug.pos.height;
        else if (frm->einzug.pos.height > 0)
            top = frm->einzug.pos.y0 + frm->einzug.pos.height;
        else if (frm->kettfarben.pos.height > 0)
            top = frm->kettfarben.pos.y0 + frm->kettfarben.pos.height;
    } else {
        top = frm->gewebe.pos.y0 + frm->gewebe.pos.height;
        bottom = top;
        if (frm->blatteinzug.pos.height > 0)
            bottom = frm->blatteinzug.pos.y0;
        else if (frm->einzug.pos.height > 0)
            bottom = frm->einzug.pos.y0;
        else if (frm->kettfarben.pos.height > 0)
            bottom = frm->kettfarben.pos.y0;
    }
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

void PatternCanvas::cancelPendingMouseState()
{
    if (dragKind != DragNone) {
        dragKind = DragNone;
        unsetCursor();
    }
    /*  Mirror handleCanvasMouseRelease's tail without dispatching any
        Set* op -- we don't know if the user wanted the half-finished
        click to take effect, and silently committing one on focus
        loss would surprise more than it would help. */
    if (frm->mousedown || frm->md_feld != INVALID || frm->tool_active) {
        frm->mousedown = false;
        frm->md_feld = INVALID;
        frm->tool_active = false;
        frm->lastfarbei = -1;
        frm->lastfarbej = -1;
        frm->lastblatteinzugi = -1;
        frm->bSelectionCleared = false;
        frm->md_ctrl = false;
        frm->refresh();
    }
}

void PatternCanvas::focusOutEvent(QFocusEvent* _e)
{
    cancelPendingMouseState();
    QWidget::focusOutEvent(_e);
}

void PatternCanvas::leaveEvent(QEvent* _e)
{
    /*  Only cancel if no button is held: a legitimate drag past the
        widget edge keeps the left button down and we want to ride it
        out, not abort. */
    if (!(QApplication::mouseButtons() & Qt::LeftButton))
        cancelPendingMouseState();
    QWidget::leaveEvent(_e);
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
    /*  Render into an off-screen QImage at DPR 1.0 (one logical
        pixel in our coord system = one image pixel), then blit the
        whole image into the widget's painter in one pass. This
        keeps every PaintCell / DrawLine coordinate landing on
        integer pixels inside the buffer, so AUSGEFUELLT's 1-pixel
        margin stays symmetric even when the widget is drawn at a
        fractional device-pixel ratio (typical on Windows at 125%
        or 150% scaling). The final drawImage upscales uniformly to
        the screen's device resolution.                            */
    if (width() <= 0 || height() <= 0)
        return;
    if (backbuffer.size() != size() || backbuffer.isNull()) {
        backbuffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);
        /*  DPR stays at the default 1.0 -- logical coords in the
            image equal pixel coords.                              */
    }

    QPainter p(&backbuffer);
    p.setRenderHint(QPainter::Antialiasing, false);

    /*  Legacy filled the whole form with clBtnFace. Use the hard-
        coded legacy grey instead of the theme's QPalette::Button
        so the canvas stays readable on dark desktops.          */
    p.fillRect(backbuffer.rect(), kLegacyBtnFace);

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
    auto paintField = [this, &p](FeldBase& fb, void (TDBWFRM::*draw)(int, int),
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
        paintField(frm->aufknuepfung, &TDBWFRM::DrawAufknuepfung, &TDBWFRM::DrawAufknuepfungRahmen,
                   onlyGewebe);
    }

    if (frm->ViewTrittfolge && frm->ViewTrittfolge->isChecked())
        paintField(frm->trittfolge, &TDBWFRM::DrawTrittfolge, &TDBWFRM::DrawTrittfolgeRahmen,
                   onlyGewebe);

    /*  Gewebe rahmen is special-cased: cells rendered by
        DrawGewebeSimulation never get a grid overlay; cells rendered
        by DrawGewebeFarbeffekt only get one when fewithraster is
        set. Empty / rapport-red / Normal cells always get the grid.
        When GewebeNone is active, the rahmen-only blank pass already
        handles every cell uniformly, so paintField is enough there. */
    const bool farbeffekt = frm->GewebeFarbeffekt && frm->GewebeFarbeffekt->isChecked();
    const bool simulation = frm->GewebeSimulation && frm->GewebeSimulation->isChecked();
    if (gewebeNone || (!farbeffekt && !simulation)) {
        paintField(frm->gewebe, &TDBWFRM::DrawGewebe, &TDBWFRM::DrawGewebeRahmen, gewebeNone);
    } else {
        paintField(frm->gewebe, &TDBWFRM::DrawGewebe, nullptr, /*blankContent=*/false);
        const bool show_rapport = frm->RappViewRapport && frm->RappViewRapport->isChecked();
        const bool inv = frm->Inverserepeat && frm->Inverserepeat->isChecked();
        const FeldBase& fb = frm->gewebe;
        const int cols = fb.gw > 0 ? fb.pos.width / fb.gw : 0;
        const int rows = fb.gh > 0 ? fb.pos.height / fb.gh : 0;
        for (int i = 0; i < cols; i++) {
            for (int j = 0; j < rows; j++) {
                const int gi = frm->scroll_x1 + i;
                const int gj = frm->scroll_y2 + j;
                const bool empty = frm->IsEmptyEinzug(gi) || frm->IsEmptyTrittfolge(gj);
                const bool rapportCell
                    = !empty && show_rapport && (frm->IsInRapport(gi, gj) != inv);
                /*  A cell renders as Farbeffekt/Simulation only when
                    none of the earlier branches in DrawGewebe took it. */
                const bool fxCell = !empty && !rapportCell && farbeffekt;
                const bool simCell = !empty && !rapportCell && simulation;
                if (simCell)
                    continue;
                if (fxCell && !frm->fewithraster)
                    continue;
                frm->DrawGewebeRahmen(i, j);
            }
        }
    }

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
            const int idx = frm->righttoleft ? frm->scroll_x1 + (cols - 1 - i) : frm->scroll_x1 + i;
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
            const int idx = frm->righttoleft ? frm->scroll_x1 + (cols - 1 - i) : frm->scroll_x1 + i;
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

    /*  Highlight overlay (F12): translucent red on every cell
        dependent on the current kbd cursor position. Drawn before
        the cursor so the cursor outline stays visible on top.    */
    frm->DrawHighlight();

    /*  Cursor outline sits on top of everything so it's visible
        no matter which view mode is active. Skipped during the
        "off" phase of the blink cycle. */
    if (frm->cursorhandler && frm->cursorVisible)
        frm->cursorhandler->DrawCursor();

    frm->currentPainter = nullptr;
    p.end();

    /*  Blit the backbuffer to the widget in one pass. Any DPI
        scaling Qt applies happens here, over the whole image, and
        therefore cannot introduce the per-primitive rounding
        asymmetry we were seeing on fractional-DPR Windows.      */
    QPainter widgetPainter(this);
    widgetPainter.drawImage(0, 0, backbuffer);
}
