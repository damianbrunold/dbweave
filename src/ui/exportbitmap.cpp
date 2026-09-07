/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Pattern export — a Qt port of legacy exportbitmap.cpp
    DoExportBitmap. Reproduces the on-screen editor layout on an
    off-screen surface with the view-option toggles honoured: the four
    core grids (einzug, aufknuepfung, trittfolge, gewebe) in the active
    darstellung, plus the warp/weft colour bars (ViewFarbe), the reed
    strip (ViewBlatteinzug), pattern-only mode (ViewOnlyGewebe) and the
    rapport highlight (RappViewRapport / Inverserepeat). Hidden bands
    collapse to nothing so the export matches what the editor shows;
    computeBands() is the single source of truth for the geometry,
    shared by patternPixelSize() (extent) and paintPattern() (drawing).
    Guide lines (Hilfslinien) and the periodic strong grid lines are
    not drawn. This mirrors the web textileplatform export.py render.

    The paintPattern() helper targets any QPaintDevice, so the same
    code drives raster output (PNG/JPEG) and vector output (SVG/PDF). */

#include "colors_compat.h"
#include "datamodule.h"
#include "draw_cell.h"
#include "mainwindow.h"
#include "palette.h"
#include "rangecolors.h"

/*-----------------------------------------------------------------*/
/*  Apply weft/warp ratio to a square base cell-size so exported
    patterns honour faktor_kette / faktor_schuss the same way the
    screen layout does. Mirrors the formula in
    patterncanvas.cpp:recomputeLayout.                             */
static void applyRatio(const TDBWFRM* _frm, int _base, int& _gw, int& _gh)
{
    _gw = _base;
    _gh = _base;
    const float fk = _frm->faktor_kette;
    const float fs = _frm->faktor_schuss;
    if (fk <= 0.0f || fs <= 0.0f)
        return;
    if (fs > fk)
        _gh = int(double(_base) * fs / fk);
    else if (fk > fs)
        _gw = int(double(_base) * fk / fs);
}

#include <QBrush>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPaintEngine>
#include <QPainter>
#include <QPen>
#include <QLocale>
#include <QPdfWriter>
#include <QSvgGenerator>

#include <algorithm>

static QColor qcolorFromTColor(TColor _c)
{
    return QColor(_c & 0xFF, (_c >> 8) & 0xFF, (_c >> 16) & 0xFF);
}

static QColor paletteColor(int _idx)
{
    const COLORREF c = Data->palette->GetColor(_idx);
    return QColor(GetRValue(c), GetGValue(c), GetBValue(c));
}

/*  Decide which paper size the PDF exporter should use. Tiered:
      1. $PAPERSIZE environment variable ("a4", "letter", ...).
         Set explicitly by the user or the session, highest trust.
      2. /etc/papersize on Unix -- the libpaper system default.
      3. QLocale's country: Letter-using territories (US, CA, MX,
         and the handful of Latin-American / Asian countries that
         share that standard) get Letter; everywhere else gets A4.
    Locale measurement system is deliberately NOT consulted -- a
    German user running a system with LANG=en_US.UTF-8 still wants
    A4, not Letter.                                              */
static QPageSize::PageSizeId preferredPageSize()
{
    auto fromName = [](const QString& _s) -> QPageSize::PageSizeId {
        const QString n = _s.trimmed().toLower();
        if (n == QLatin1String("a4"))
            return QPageSize::A4;
        if (n == QLatin1String("letter") || n == QLatin1String("us-letter"))
            return QPageSize::Letter;
        if (n == QLatin1String("legal"))
            return QPageSize::Legal;
        if (n == QLatin1String("a3"))
            return QPageSize::A3;
        if (n == QLatin1String("a5"))
            return QPageSize::A5;
        return QPageSize::PageSizeId(-1);
    };
    const QByteArray env = qgetenv("PAPERSIZE");
    if (!env.isEmpty()) {
        const auto id = fromName(QString::fromLocal8Bit(env));
        if (int(id) >= 0)
            return id;
    }
    QFile papersize(QStringLiteral("/etc/papersize"));
    if (papersize.exists() && papersize.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString line = QString::fromLocal8Bit(papersize.readLine()).trimmed();
        papersize.close();
        const auto id = fromName(line);
        if (int(id) >= 0)
            return id;
    }
    /*  Letter-using countries (ISO 3166 codes). Everywhere else
        gets A4. List taken from the de-facto "US Letter zone"
        observed by print vendors. */
    switch (QLocale().territory()) {
    case QLocale::UnitedStates:
    case QLocale::Canada:
    case QLocale::Mexico:
    case QLocale::Colombia:
    case QLocale::Venezuela:
    case QLocale::Chile:
    case QLocale::Philippines:
        return QPageSize::Letter;
    default:
        return QPageSize::A4;
    }
}

/*-----------------------------------------------------------------*/
/*  Visible-band geometry -- the export analogue of the web
    export.py::_bands and the print path's InitVisibility +
    CalcDimensions. Bands the user has hidden collapse to zero and are
    not drawn, so the export reflects exactly what the editor shows.
    All offsets are in logical pixels (the QPainter window space).

        [warp colour bar ..] [.................]
        [..................] [.................]
        [einzug ...........] [gap] [aufknuepfung]
        [reed .............] [.................]
        [gewebe ...........] [gap] [trittfolge .] [gap] [weft colours]

    einzugunten flips the vertical order (gewebe on top). */
namespace
{
struct ExportBands {
    int width = 0;
    int height = 0;
    int side_x = 0; // aufknuepfung / trittfolge column origin
    int weft_x = 0; // weft colour bar column origin
    int y_warpbar = 0;
    int y_einzug = 0;
    int y_reed = 0;
    int y_gewebe = 0;
    bool show_einzug = false;
    bool show_tieup = false;
    bool show_side = false;
    bool show_reed = false;
    bool show_warpbar = false;
    bool show_weftbar = false;
    bool pattern_only = false;
};

ExportBands computeBands(int _gw, int _gh, int _dx, int _dy, int _shafts, int _treadles,
                         bool _einzugunten, bool _showFarbe, bool _showReed, bool _patternOnly)
{
    const int gw = _gw;
    const int gh = _gh;

    ExportBands b;
    b.show_einzug = _shafts != 0;
    b.show_side = _treadles != 0;
    b.show_tieup = _shafts != 0 && _treadles != 0;
    b.show_warpbar = _showFarbe && _dx > 0;
    b.show_weftbar = _showFarbe && _dy > 0;
    b.show_reed = _showReed && _dx > 0;
    b.pattern_only = _patternOnly;

    /*  Row-group heights (in cells). The gap between the threading and
        the reed/gewebe is present only when the threading is shown; the
        reed<->gewebe gap only when the reed is shown. warp_gap_h keeps
        the warp colour bar clear of the threading: legacy reserved two
        rows for the bar (clry in legacy/exportbitmap.cpp) and the editor
        layout puts a `divider` between kettfarben and einzug, so the bar
        must not sit flush against it here either. */
    const int warp_h = b.show_warpbar ? 1 : 0;
    const int einz_h = b.show_einzug ? _shafts : 0;
    const int reed_h = b.show_reed ? 1 : 0;
    const int gap_h = b.show_einzug ? 1 : 0;
    const int reed_gap_h = b.show_reed ? 1 : 0;
    const int warp_gap_h = b.show_warpbar ? 1 : 0;

    /*  Row groups, top to bottom. einzugunten puts the gewebe on top
        and the threading (with its colour bar) below it. */
    int cy = 0;
    if (!_einzugunten) {
        b.y_warpbar = cy * gh;
        cy += warp_h + warp_gap_h;
        b.y_einzug = cy * gh;
        cy += einz_h + gap_h;
        b.y_reed = cy * gh;
        cy += reed_h + reed_gap_h;
        b.y_gewebe = cy * gh;
        cy += _dy;
    } else {
        b.y_gewebe = cy * gh;
        cy += _dy + reed_gap_h;
        b.y_reed = cy * gh;
        cy += reed_h + gap_h;
        b.y_einzug = cy * gh;
        cy += einz_h + warp_gap_h;
        b.y_warpbar = cy * gh;
        cy += warp_h;
    }
    const int total_rows = cy;

    /*  Column groups: main (dx) | gap | side pane | gap | weft colours.
        The first one-cell gap separates the drawdown/threading from the
        treadling/tie-up and is present only when the side pane shows;
        the second keeps the weft colour bar clear of the trittfolge,
        mirroring legacy's clrx and the editor's schussfarben divider. */
    const int side_w = b.show_side ? _treadles : 0;
    const int weft_w = b.show_weftbar ? 1 : 0;
    const int gap_w = b.show_side ? 1 : 0;
    const int weft_gap_w = b.show_weftbar ? 1 : 0;
    b.side_x = (_dx + gap_w) * gw;
    b.weft_x = (_dx + gap_w + side_w + weft_gap_w) * gw;
    const int total_cols = _dx + gap_w + side_w + weft_gap_w + weft_w;

    b.width = gw * total_cols + 1;
    b.height = gh * total_rows + 1;
    if (b.width < 2 || b.height < 2) {
        b.width = std::max(b.width, 32);
        b.height = std::max(b.height, 32);
    }
    return b;
}
} // namespace

/*-----------------------------------------------------------------*/
void TDBWFRM::patternPixelSize(int& _w, int& _h, int _gw, int _gh, int& _shafts, int& _treadles)
{
    _shafts = 0;
    if (ViewEinzug && ViewEinzug->isChecked()) {
        for (int i = kette.a; i <= kette.b; i++) {
            const int s = einzug.feld.Get(i);
            if (s > _shafts)
                _shafts = s;
        }
    }
    _treadles = 0;
    if (ViewTrittfolge && ViewTrittfolge->isChecked()) {
        for (int i = Data->MAXX2 - 1; i >= 0; i--)
            for (int j = schuesse.a; j <= schuesse.b; j++)
                if (trittfolge.feld.Get(i, j) != 0)
                    if (_treadles < i + 1)
                        _treadles = i + 1;
    }
    const ExportBands b = computeBands(_gw, _gh, kette.count(), schuesse.count(), _shafts,
                                       _treadles, einzugunten, ViewFarbe && ViewFarbe->isChecked(),
                                       ViewBlatteinzug && ViewBlatteinzug->isChecked(),
                                       ViewOnlyGewebe && ViewOnlyGewebe->isChecked());
    _w = b.width;
    _h = b.height;
}

/*-----------------------------------------------------------------*/
void TDBWFRM::paintPattern(QPainter& p, int _gw, int _gh, int _shafts, int _treadles)
{
    const int gw = _gw;
    const int gh = _gh;
    const int shafts = _shafts;
    const int treadles = _treadles;
    const int dx = kette.count();
    const int dy = schuesse.count();
    const bool rtl = righttoleft;
    const bool ttb = toptobottom;
    const bool pegplan = ViewSchlagpatrone && ViewSchlagpatrone->isChecked();

    p.setRenderHint(QPainter::Antialiasing, false);
    /*  PaintCell() mutates the painter's brush as a side effect;
        QPainter::drawRect honours that brush and would fill each
        quadrant's outline rect solid-colour. Keep the brush empty
        so the quadrant borders are strokes, not fills. */
    p.setBrush(Qt::NoBrush);

    /*  Visible-band geometry -- the single source of truth shared with
        patternPixelSize (see computeBands). Hidden bands collapse to
        nothing, so the export matches exactly what the editor shows. */
    const ExportBands b = computeBands(gw, gh, dx, dy, shafts, treadles, einzugunten,
                                       ViewFarbe && ViewFarbe->isChecked(),
                                       ViewBlatteinzug && ViewBlatteinzug->isChecked(),
                                       ViewOnlyGewebe && ViewOnlyGewebe->isChecked());
    const int einzug_y0 = b.y_einzug;
    const int gewebe_y0 = b.y_gewebe;
    const int auf_y0 = einzug_y0;
    const int tritt_y0 = gewebe_y0;
    const int side_x = b.side_x;

    /*  Warp x-position of column i, honouring right-to-left. */
    auto warpX = [&](int i) { return rtl ? (dx - i - 1) * gw : i * gw; };

    /*  Grid lines + black outline for a one-row strip over the dx main
        columns (shared by the warp colour bar and the reed strip). */
    auto hstripGrid = [&](int y0) {
        p.setPen(QColor(105, 105, 105));
        for (int i = 0; i <= dx; i++)
            p.drawLine(i * gw, y0, i * gw, y0 + gh);
        p.drawLine(0, y0, dx * gw, y0);
        p.drawLine(0, y0 + gh, dx * gw, y0 + gh);
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);
        p.drawRect(0, y0, dx * gw, gh);
    };

    /*  Vector output (PDF / SVG) antialiases each rectangle on its
        own, so two blocks that merely share an edge each cover only
        part of the pixel along the join and leave a pale hairline
        between them -- right across the fabric in Farbeffekt, and
        straight through the threads in Simulation. Outline the block
        in its own colour with a cosmetic (width 0) pen: PDF renders
        that as the thinnest line the device can draw, exactly one
        device pixel at any zoom, which covers the seam without
        changing the geometry. The raster engine lands on whole
        pixels and needs none of this -- there a cosmetic pen would
        just fatten the block by a pixel -- so it keeps fillRect. */
    const bool vectorTarget = IsVectorPainter(p);
    auto fillTile = [&p, vectorTarget](int _x, int _y, int _w, int _h, const QColor& _col) {
        if (!vectorTarget) {
            p.fillRect(_x, _y, _w, _h, _col);
            return;
        }
        p.setPen(QPen(_col, 0));
        p.setBrush(QBrush(_col));
        p.drawRect(QRectF(_x, _y, _w, _h));
        p.setBrush(Qt::NoBrush);
    };

    /*  Fill a cell's interior -- pixels [_x+1, _x+_w-1] x [_y+1,
        _y+_h-1] -- leaving the shared grid frame visible. Same rect
        DrawGewebeNormal uses for the anbindung / abbindung background
        behind their symbol. */
    auto fillInterior = [&p](int _x, int _y, int _w, int _h, const QColor& _col) {
        FillPixelBox(p, _x + 1, _y + 1, _x + _w - 1, _y + _h - 1, _col);
    };

    /*  Draw one cell in either filled or symbol mode. Every
        darstellung goes through PaintCell, AUSGEFUELLT included, so a
        filled cell keeps the same margin to the grid frame that the
        symbols leave and that the editor draws on screen -- filled
        einzug / trittfolge / aufknuepfung cells read as separate
        cells instead of merging into one solid block.
        _dontclear keeps the background untouched so the image's
        white backdrop shows through instead of the editor's btnFace
        grey. */
    auto paintExportCell
        = [&p](DARSTELLUNG _d, int _x, int _y, int _gw, int _gh, const QColor& _col) {
              PaintCell(p, _d, _x, _y, _x + _gw, _y + _gh, _col, /*_dontclear=*/true);
          };

    /*  Warp colour bar -- one row above (below, when einzugunten) the
        threading, each warp thread filled in its palette colour. */
    if (b.show_warpbar) {
        const int y0 = b.y_warpbar;
        for (int i = 0; i < dx; i++)
            fillTile(warpX(i), y0, gw, gh, paletteColor(kettfarben.feld.Get(kette.a + i)));
        hstripGrid(y0);
    }

    /*  Einzug (top-left in standard layout, bottom-left in einzugunten). */
    if (b.show_einzug) {
        const int x0 = 0;
        const int y0 = einzug_y0;
        p.setPen(QColor(105, 105, 105));
        for (int i = 0; i <= dx; i++)
            p.drawLine(x0 + i * gw, y0, x0 + i * gw, y0 + shafts * gh);
        for (int j = 0; j <= shafts; j++)
            p.drawLine(x0, y0 + j * gh, x0 + dx * gw, y0 + j * gh);
        /*  pattern_only (ViewOnlyGewebe) draws the grid frame but
            suppresses the threading symbols -- mirrors PrintEinzug. */
        if (!b.pattern_only) {
            for (int i = 0; i < dx; i++) {
                const int s = einzug.feld.Get(kette.a + i);
                if (s == 0)
                    continue;
                const int x = warpX(i);
                const int y = ttb ? y0 + (s - 1) * gh : y0 + (shafts - s) * gh;
                paintExportCell(einzug.darstellung, x, y, gw, gh, QColor(Qt::black));
            }
        }
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);
        p.drawRect(x0, y0, dx * gw, shafts * gh);
    }

    /*  Aufknuepfung (top-right in standard layout, bottom-right in einzugunten). */
    if (b.show_tieup) {
        const int x0 = side_x;
        const int y0 = auf_y0;
        p.setPen(QColor(105, 105, 105));
        for (int i = 0; i <= treadles; i++)
            p.drawLine(x0 + i * gw, y0, x0 + i * gw, y0 + shafts * gh);
        for (int j = 0; j <= shafts; j++)
            p.drawLine(x0, y0 + j * gh, x0 + treadles * gw, y0 + j * gh);
        const DARSTELLUNG darst = pegplan ? AUSGEFUELLT : aufknuepfung.darstellung;
        if (!b.pattern_only) {
            for (int j = 0; j < shafts; j++)
                for (int i = 0; i < treadles; i++) {
                    const char s = aufknuepfung.feld.Get(i, j);
                    if (s <= 0)
                        continue;
                    const int x = x0 + i * gw;
                    const int y = ttb ? y0 + j * gh : y0 + (shafts - j - 1) * gh;
                    /*  Special ranges are drawn with their per-range
                        glyph (darst_aushebung / _anbindung / _abbindung)
                        and, unlike the gewebe, no coloured background --
                        matches legacy DrawAufknuepfung, which calls
                        PaintCell with _col=black for these cells.       */
                    if (!pegplan && s == AUSHEBUNG) {
                        paintExportCell(darst_aushebung, x, y, gw, gh, QColor(Qt::black));
                    } else if (!pegplan && s == ANBINDUNG) {
                        paintExportCell(darst_anbindung, x, y, gw, gh, QColor(Qt::black));
                    } else if (!pegplan && s == ABBINDUNG) {
                        paintExportCell(darst_abbindung, x, y, gw, gh, QColor(Qt::black));
                    } else {
                        QColor col = QColor(Qt::black);
                        if (s >= 1 && s <= 9)
                            col = qcolorFromTColor(GetRangeColor(s));
                        paintExportCell(darst, x, y, gw, gh, col);
                    }
                }
        }
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);
        p.drawRect(x0, y0, treadles * gw, shafts * gh);
    }

    /*  Reed (Blatteinzug) -- one row above the gewebe; each warp column
        is half-filled black (top half when threaded through the reed,
        else bottom half). Mirrors PrintBlatteinzug. */
    if (b.show_reed) {
        const int y0 = b.y_reed;
        const int half = gh / 2;
        for (int i = 0; i < dx; i++) {
            const int x = warpX(i);
            if (blatteinzug.feld.Get(kette.a + i))
                fillTile(x, y0, gw, half, QColor(Qt::black));
            else
                fillTile(x, y0 + gh - half, gw, half, QColor(Qt::black));
        }
        hstripGrid(y0);
    }

    /*  Trittfolge (bottom-right in standard layout, top-right in einzugunten). */
    if (b.show_side) {
        const int x0 = side_x;
        const int y0 = tritt_y0;
        p.setPen(QColor(105, 105, 105));
        for (int i = 0; i <= treadles; i++)
            p.drawLine(x0 + i * gw, y0, x0 + i * gw, y0 + dy * gh);
        for (int j = 0; j <= dy; j++)
            p.drawLine(x0, y0 + j * gh, x0 + treadles * gw, y0 + j * gh);
        const DARSTELLUNG darst = pegplan ? schlagpatronendarstellung : trittfolge.darstellung;
        if (!b.pattern_only) {
            for (int j = 0; j < dy; j++)
                for (int i = 0; i < treadles; i++) {
                    const char s = trittfolge.feld.Get(i, schuesse.a + j);
                    if (s <= 0)
                        continue;
                    const int x = x0 + i * gw;
                    const int y = y0 + (dy - j - 1) * gh;
                    /*  Special ranges in pegplan view use the dedicated
                        glyphs (darst_aushebung / _anbindung / _abbindung)
                        in plain black -- same rule as the aufknuepfung
                        block above.                                     */
                    if (pegplan && s == AUSHEBUNG) {
                        paintExportCell(darst_aushebung, x, y, gw, gh, QColor(Qt::black));
                    } else if (pegplan && s == ANBINDUNG) {
                        paintExportCell(darst_anbindung, x, y, gw, gh, QColor(Qt::black));
                    } else if (pegplan && s == ABBINDUNG) {
                        paintExportCell(darst_abbindung, x, y, gw, gh, QColor(Qt::black));
                    } else {
                        QColor col = QColor(Qt::black);
                        if (s >= 1 && s <= 9)
                            col = qcolorFromTColor(GetRangeColor(s));
                        paintExportCell(darst, x, y, gw, gh, col);
                    }
                }
        }
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);
        p.drawRect(x0, y0, treadles * gw, dy * gh);
    }

    /*  Weft colour bar -- one column right of the trittfolge (or the
        gewebe, when the side pane is hidden), each weft thread filled in
        its palette colour. */
    if (b.show_weftbar) {
        const int x0 = b.weft_x;
        const int y0 = gewebe_y0;
        for (int j = 0; j < dy; j++) {
            const int y = y0 + (dy - j - 1) * gh;
            fillTile(x0, y, gw, gh, paletteColor(schussfarben.feld.Get(schuesse.a + j)));
        }
        p.setPen(QColor(105, 105, 105));
        for (int j = 0; j <= dy; j++)
            p.drawLine(x0, y0 + j * gh, x0 + gw, y0 + j * gh);
        p.drawLine(x0, y0, x0, y0 + dy * gh);
        p.drawLine(x0 + gw, y0, x0 + gw, y0 + dy * gh);
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);
        p.drawRect(x0, y0, gw, dy * gh);
    }

    /*  Gewebe (bottom-left in standard layout, top-left in einzugunten).
        Supports the same four view modes the editor uses:
          Unsichtbar  -> grid only, no cell content
          Patrone     -> range-coloured filled cells (the default)
          Farbeffekt  -> warp / weft palette colour, full cell
          Gewebe      -> simulated thread-over-thread render
        The simulation path mirrors DrawGewebeSimulation but with
        export-sized (fixed gw/gh=16) geometry.                     */
    {
        const int x0 = 0;
        const int y0 = gewebe_y0;
        const bool none = GewebeNone && GewebeNone->isChecked();
        const bool farbeffekt = GewebeFarbeffekt && GewebeFarbeffekt->isChecked();
        const bool simulation = GewebeSimulation && GewebeSimulation->isChecked();
        const bool rapportOn = RappViewRapport && RappViewRapport->isChecked();
        const bool inv = Inverserepeat && Inverserepeat->isChecked();

        /*  A cell is blank when its warp column carries no threading
            or its weft row no treadling -- nothing is woven there, so
            it stays empty and keeps its grid (legacy skipped exactly
            these in the export loop, and DrawGewebe blanks them on
            screen). */
        auto blankCell = [&](int _i, int _j) {
            return IsEmptyEinzug(kette.a + _i) || IsEmptyTrittfolge(schuesse.a + _j);
        };

        /*  Which cells end up covered edge-to-edge by Farbeffekt /
            Simulation, and so must NOT have a grid line under them --
            the grid only survives as a seam through the woven surface.
            Same rule the canvas applies in its rahmen post-pass
            (patterncanvas.cpp): Simulation cells never get a grid,
            Farbeffekt cells only with "Farbeffekt mit Raster", while
            blank, rapport-red and Patrone cells always keep theirs. */
        auto coveredCell = [&](int _i, int _j) {
            if (none || blankCell(_i, _j))
                return false;
            if (rapportOn && (IsInRapport(kette.a + _i, schuesse.a + _j) != inv))
                return false;
            if (simulation)
                return true;
            return farbeffekt && !fewithraster;
        };

        /*  Patrone / Unsichtbar keep the plain grid under everything:
            their cells are drawn inset, so the lines stay visible. The
            covering modes get per-cell frames in a post-pass after the
            content instead -- same split the canvas makes. */
        const bool perCellGrid = !none && (farbeffekt || simulation);
        if (!perCellGrid) {
            p.setPen(QColor(105, 105, 105));
            for (int i = 0; i <= dx; i++)
                p.drawLine(x0 + i * gw, y0, x0 + i * gw, y0 + dy * gh);
            for (int j = 0; j <= dy; j++)
                p.drawLine(x0, y0 + j * gh, x0 + dx * gw, y0 + j * gh);
        }
        if (!none) {
            const int dw = std::max(1, gw / 5);
            const int dh = std::max(1, gh / 5);
            for (int i = 0; i < dx; i++) {
                for (int j = 0; j < dy; j++) {
                    const char s = gewebe.feld.Get(i + kette.a, j + schuesse.a);
                    const int x = x0 + i * gw;
                    const int y = y0 + (dy - j - 1) * gh;
                    /*  Nothing is woven where the warp column has no
                        threading or the weft row no treadling; leave
                        those cells empty (their grid was drawn above)
                        instead of flooding them with weft colour.   */
                    if (blankCell(i, j))
                        continue;
                    /*  Rapport highlight takes precedence over the view
                        mode (mirrors DrawGewebeRapport): in-rapport
                        raised-warp cells are painted red as a patrone,
                        empty cells stay blank. Inverserepeat flips
                        which side gets the red treatment.            */
                    if (rapportOn && (IsInRapport(i + kette.a, j + schuesse.a) != inv)) {
                        if (s != ABBINDUNG && s > 0)
                            PaintCell(p, AUSGEFUELLT, x, y, x + gw, y + gh, QColor(Qt::red),
                                      /*_dontclear=*/true);
                        continue;
                    }
                    if (farbeffekt) {
                        bool hebung = s > 0;
                        if (sinkingshed)
                            hebung = !hebung;
                        const QColor col
                            = hebung ? paletteColor(kettfarben.feld.Get(i + kette.a))
                                     : paletteColor(schussfarben.feld.Get(j + schuesse.a));
                        fillTile(x, y, gw, gh, col);
                    } else if (simulation) {
                        const QColor kc = paletteColor(kettfarben.feld.Get(i + kette.a));
                        const QColor sc = paletteColor(schussfarben.feld.Get(j + schuesse.a));
                        bool hebung = s > 0 && s != ABBINDUNG;
                        if (sinkingshed)
                            hebung = !hebung;
                        if (hebung) {
                            /*  Warp on top: vertical warp strip in
                                kettfarben, weft stubs peeking out left
                                and right, shadow line on the right.   */
                            fillTile(x + dw, y, gw - 2 * dw, gh, kc);
                            fillTile(x, y + dh, dw, gh - 2 * dh, sc);
                            fillTile(x + gw - dw, y + dh, dw, gh - 2 * dh, sc);
                            p.setPen(QColor(Qt::black));
                            p.drawLine(x + gw - dw, y + dh, x + gw - dw, y + gh - dh);
                        } else {
                            /*  Weft on top: horizontal weft strip in
                                schussfarben, warp stubs top and bottom,
                                shadow line along the bottom.           */
                            fillTile(x, y + dh, gw, gh - 2 * dh, sc);
                            fillTile(x + dw, y, gw - 2 * dw, dh, kc);
                            fillTile(x + dw, y + gh - dh, gw - 2 * dw, dh, kc);
                            p.setPen(QColor(Qt::black));
                            p.drawLine(x + dw, y + gh - dh, x + gw - dw, y + gh - dh);
                        }
                        p.setBrush(Qt::NoBrush);
                    } else if (s == AUSHEBUNG) {
                        /*  Mirrors DrawGewebeNormal's AUSHEBUNG
                            branch: symbol glyph on btnFace background
                            when darst_aushebung != AUSGEFUELLT,
                            otherwise the GetRangeColor fallback.    */
                        if (darst_aushebung != AUSGEFUELLT) {
                            PaintCell(p, darst_aushebung, x, y, x + gw, y + gh, QColor(Qt::black),
                                      /*_dontclear=*/true);
                        } else {
                            paintExportCell(AUSGEFUELLT, x, y, gw, gh,
                                            qcolorFromTColor(GetRangeColor(s)));
                        }
                    } else if (s == ANBINDUNG) {
                        if (darst_anbindung != AUSGEFUELLT) {
                            fillInterior(x, y, gw, gh, qcolorFromTColor(col_anbindung));
                            PaintCell(p, darst_anbindung, x, y, x + gw, y + gh, QColor(Qt::black),
                                      /*_dontclear=*/true);
                        } else {
                            paintExportCell(AUSGEFUELLT, x, y, gw, gh,
                                            qcolorFromTColor(GetRangeColor(s)));
                        }
                    } else if (s == ABBINDUNG) {
                        if (darst_abbindung != AUSGEFUELLT) {
                            fillInterior(x, y, gw, gh, qcolorFromTColor(col_abbindung));
                            PaintCell(p, darst_abbindung, x, y, x + gw, y + gh, QColor(Qt::black),
                                      /*_dontclear=*/true);
                        } else {
                            paintExportCell(AUSGEFUELLT, x, y, gw, gh,
                                            qcolorFromTColor(GetRangeColor(s)));
                        }
                    } else if (s > 0) {
                        /*  Ranges 1..9: range colour with the same
                            margin to the grid frame the editor leaves
                            (DrawGewebeNormal paints these through
                            PaintCell(AUSGEFUELLT) too), so the patrone
                            reads as separate cells rather than one
                            solid block of colour.                    */
                        paintExportCell(AUSGEFUELLT, x, y, gw, gh,
                                        qcolorFromTColor(GetRangeColor(s)));
                    }
                }
            }
        }
        if (perCellGrid) {
            /*  Rahmen post-pass: frame every cell the fabric did not
                cover, on top of the finished cells (a frame drawn
                first would just be painted over by the cell fill --
                which is what "Farbeffekt mit Raster" must not do). */
            p.setPen(QColor(105, 105, 105));
            p.setBrush(Qt::NoBrush);
            for (int i = 0; i < dx; i++)
                for (int j = 0; j < dy; j++) {
                    if (coveredCell(i, j))
                        continue;
                    p.drawRect(x0 + i * gw, y0 + (dy - j - 1) * gh, gw, gh);
                }
        }
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);
        p.drawRect(x0, y0, dx * gw, dy * gh);

        /*  Rapport extent markers -- red boundary lines around the
            rapport rectangle, drawn over the einzug strip (verticals,
            at the Kettrapport edges) and the trittfolge strip
            (horizontals, at the Schussrapport edges). Mirrors the
            on-screen DrawRapport; gated on band visibility so a hidden
            strip gets no floating line. */
        if (rapportOn) {
            const int i1 = rapport.kr.a;
            const int i2 = rapport.kr.b;
            const int j1 = rapport.sr.a;
            const int j2 = rapport.sr.b;
            if (i2 >= i1 && j2 >= j1) {
                QPen redpen{QColor(Qt::red)};
                redpen.setWidth(2);
                p.setPen(redpen);
                p.setBrush(Qt::NoBrush);
                if (b.show_einzug) {
                    const int xl = !rtl ? (i1 - kette.a) * gw : (dx - (i1 - kette.a)) * gw;
                    const int xr = !rtl ? (i2 + 1 - kette.a) * gw : (dx - (i2 + 1 - kette.a)) * gw;
                    p.drawLine(xl, einzug_y0, xl, einzug_y0 + shafts * gh);
                    p.drawLine(xr, einzug_y0, xr, einzug_y0 + shafts * gh);
                }
                if (b.show_side) {
                    const int xl = side_x;
                    const int xr = side_x + treadles * gw;
                    const int ytop = gewebe_y0 + (dy - (j2 + 1 - schuesse.a)) * gh;
                    const int ybot = gewebe_y0 + (dy - (j1 - schuesse.a)) * gh;
                    p.drawLine(xl, ytop, xr, ytop);
                    p.drawLine(xl, ybot, xr, ybot);
                }
                /*  Re-stroke the gewebe outline the red overlay may
                    have clipped. */
                p.setPen(Qt::black);
                p.setBrush(Qt::NoBrush);
                p.drawRect(x0, y0, dx * gw, dy * gh);
            }
        }
    }
}

/*-----------------------------------------------------------------*/
/*  Raster export (PNG / JPEG). Both use a white background; the
    legacy Windows btnFace grey made exports look like screenshots
    of the editor, which was rarely what users wanted.             */
static bool doExportRaster(TDBWFRM* _frm, const QString& _filename, const char* _format)
{
    int gw, gh;
    applyRatio(_frm, 16, gw, gh);
    int W = 0, H = 0, shafts = 0, treadles = 0;
    _frm->patternPixelSize(W, H, gw, gh, shafts, treadles);

    /*  Guard against a degenerate export when the pattern has no
        defined kette / schuesse range and every view is switched
        off: without this, QImage(0, 0, ...) returns a null image,
        img.save() writes a 0-byte file and the user sees "could
        not be opened" on the next try. Fall back to a minimum
        32x32 canvas so the save still produces a valid PNG/JPEG. */
    if (W < 2 || H < 2) {
        W = std::max(W, 32);
        H = std::max(H, 32);
    }

    QImage img(W, H, QImage::Format_RGB32);
    img.fill(QColor(Qt::white));

    QPainter p(&img);
    _frm->paintPattern(p, gw, gh, shafts, treadles);
    p.end();

    return img.save(_filename, _format);
}

void TDBWFRM::DoExportPng(const QString& _filename)
{
    if (!doExportRaster(this, _filename, "PNG"))
        QMessageBox::warning(this, tr("Export"),
                             tr("Failed to write PNG file:\n%1").arg(_filename));
}

void TDBWFRM::DoExportJpeg(const QString& _filename)
{
    if (!doExportRaster(this, _filename, "JPEG"))
        QMessageBox::warning(this, tr("Export"),
                             tr("Failed to write JPEG file:\n%1").arg(_filename));
}

/*-----------------------------------------------------------------*/
void TDBWFRM::DoExportSvg(const QString& _filename)
{
    int gw, gh;
    applyRatio(this, 16, gw, gh);
    int W = 0, H = 0, shafts = 0, treadles = 0;
    patternPixelSize(W, H, gw, gh, shafts, treadles);

    QSvgGenerator gen;
    gen.setFileName(_filename);
    gen.setSize(QSize(W, H));
    gen.setViewBox(QRect(0, 0, W, H));
    gen.setTitle(QFileInfo(filename).fileName());
    gen.setDescription(QStringLiteral("DB-WEAVE weaving pattern"));

    QPainter p(&gen);
    p.fillRect(QRect(0, 0, W, H), QColor(Qt::white));
    paintPattern(p, gw, gh, shafts, treadles);
    const bool ok = p.end();
    if (!ok)
        QMessageBox::warning(this, tr("Export"),
                             tr("Failed to write SVG file:\n%1").arg(_filename));
}

/*-----------------------------------------------------------------*/
void TDBWFRM::DoExportPdf(const QString& _filename)
{
    int gw, gh;
    applyRatio(this, 16, gw, gh);
    int Wpx = 0, Hpx = 0, shafts = 0, treadles = 0;
    patternPixelSize(Wpx, Hpx, gw, gh, shafts, treadles);

    QPdfWriter pdf(_filename);
    pdf.setCreator(QStringLiteral("DB-WEAVE"));
    pdf.setTitle(QFileInfo(filename).fileName());
    pdf.setPageSize(QPageSize(preferredPageSize()));
    pdf.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);

    QPainter p(&pdf);
    /*  Lay the pattern out at a fixed physical cell size (matches
        the print path's 2.0-2.5 mm per cell) instead of stretching
        to fill the whole page -- a small pattern shouldn't blow up
        to cover an A4 sheet. If the pattern at that natural size is
        larger than the paintable area, fall back to a proportional
        shrink-fit; otherwise place it at the top-left of the
        margin rect so the rest of the page stays blank.           */
    constexpr double MM_PER_CELL = 2.5;
    const QRect page = p.viewport();
    const double dev_per_mm = pdf.resolution() / 25.4;
    /*  Scale so the smaller-factor cell axis equals MM_PER_CELL; a
        non-unit weft/warp ratio stretches the other axis naturally
        (gw != gh in Wpx/Hpx already).                               */
    const int base = std::min(gw, gh);
    const double dev_per_logical_px = (MM_PER_CELL / base) * dev_per_mm;
    double w = Wpx * dev_per_logical_px;
    double h = Hpx * dev_per_logical_px;
    if (w > page.width() || h > page.height()) {
        const double s = qMin(page.width() / w, page.height() / h);
        w *= s;
        h *= s;
    }
    p.setViewport(page.x(), page.y(), int(w), int(h));
    p.setWindow(0, 0, Wpx, Hpx);

    p.fillRect(QRect(0, 0, Wpx, Hpx), QColor(Qt::white));
    paintPattern(p, gw, gh, shafts, treadles);
    const bool ok = p.end();
    if (!ok)
        QMessageBox::warning(this, tr("Export"),
                             tr("Failed to write PDF file:\n%1").arg(_filename));
}
