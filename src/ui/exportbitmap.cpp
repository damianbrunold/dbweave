/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Pattern export — a simplified Qt port of legacy exportbitmap.cpp
    DoExportBitmap. The legacy routine is a 650-line render that
    reproduces the full screen layout on an off-screen TBitmap with
    every view-option toggle honoured (Simulation, Farbeffekt,
    Hilfslinien, blatteinzug, colour strips, rapport markers). This
    port covers the four core grids (einzug, aufknuepfung,
    trittfolge, gewebe) rendered in the active darstellung — the
    common case.

    The paintPattern() helper targets any QPaintDevice, so the same
    code drives raster output (PNG/JPEG) and vector output (SVG/PDF). */

#include "colors_compat.h"
#include "datamodule.h"
#include "draw_cell.h"
#include "mainwindow.h"
#include "palette.h"
#include "rangecolors.h"

#include <QFileInfo>
#include <QImage>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
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
    const int sdy = _shafts != 0 ? 1 : 0;
    const int tdx = _treadles != 0 ? 1 : 0;
    const int dx = kette.count();
    const int dy = schuesse.count();
    _w = _gw * (dx + tdx + _treadles) + 1;
    _h = _gh * (dy + sdy + _shafts) + 1;
}

/*-----------------------------------------------------------------*/
void TDBWFRM::paintPattern(QPainter& p, int _gw, int _gh, int _shafts, int _treadles)
{
    const int gw = _gw;
    const int gh = _gh;
    const int shafts = _shafts;
    const int treadles = _treadles;
    const int sdy = shafts != 0 ? 1 : 0;
    const int tdx = treadles != 0 ? 1 : 0;
    const int dx = kette.count();
    const int dy = schuesse.count();

    p.setRenderHint(QPainter::Antialiasing, false);
    /*  PaintCell() mutates the painter's brush as a side effect;
        QPainter::drawRect honours that brush and would fill each
        quadrant's outline rect solid-colour. Keep the brush empty
        so the quadrant borders are strokes, not fills. */
    p.setBrush(Qt::NoBrush);

    /*  Draw one cell in either filled or symbol mode, matching the
        print path's "filled means filled" look: AUSGEFUELLT covers
        the whole cell edge-to-edge (exactly gw x gh) so neighbouring
        filled cells merge -- the right neighbour's left edge lands
        on the shared grid line and overpaints it. Adding +1 here
        would make each cell 1 logical unit wider than its footprint,
        which is invisible at raster scale but produces obvious
        overshoot once the PDF writer scales the geometry up to the
        printable page.
        Symbol darstellungen (KREUZ, PUNKT, STRICH, KREIS, ...) leave
        the background untouched so the image's white/transparent
        backdrop shows through instead of the editor's btnFace grey. */
    auto paintExportCell = [&p](DARSTELLUNG _d, int _x, int _y, int _gw, int _gh,
                                const QColor& _col) {
        if (_d == AUSGEFUELLT) {
            p.fillRect(_x, _y, _gw, _gh, _col);
        } else {
            PaintCell(p, _d, _x, _y, _x + _gw, _y + _gh, _col, /*_dontclear=*/true);
        }
    };

    /*  Einzug (top-left). */
    if (shafts != 0) {
        const int x0 = 0;
        const int y0 = 0;
        p.setPen(QColor(105, 105, 105));
        for (int i = 0; i <= dx; i++)
            p.drawLine(x0 + i * gw, y0, x0 + i * gw, y0 + shafts * gh);
        for (int j = 0; j <= shafts; j++)
            p.drawLine(x0, y0 + j * gh, x0 + dx * gw, y0 + j * gh);
        for (int i = 0; i < dx; i++) {
            const int s = einzug.feld.Get(kette.a + i);
            if (s == 0)
                continue;
            const int x = righttoleft ? x0 + (dx - i - 1) * gw : x0 + i * gw;
            const int y = toptobottom ? y0 + (s - 1) * gh : y0 + (shafts - s) * gh;
            paintExportCell(einzug.darstellung, x, y, gw, gh, QColor(Qt::black));
        }
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);
        p.drawRect(x0, y0, dx * gw, shafts * gh);
    }

    /*  Aufknuepfung (top-right). */
    if (treadles != 0 && shafts != 0) {
        const int x0 = (dx + tdx) * gw;
        const int y0 = 0;
        p.setPen(QColor(105, 105, 105));
        for (int i = 0; i <= treadles; i++)
            p.drawLine(x0 + i * gw, y0, x0 + i * gw, y0 + shafts * gh);
        for (int j = 0; j <= shafts; j++)
            p.drawLine(x0, y0 + j * gh, x0 + treadles * gw, y0 + j * gh);
        const bool pegplan = ViewSchlagpatrone && ViewSchlagpatrone->isChecked();
        const DARSTELLUNG darst = pegplan ? AUSGEFUELLT : aufknuepfung.darstellung;
        for (int j = 0; j < shafts; j++)
            for (int i = 0; i < treadles; i++) {
                const char s = aufknuepfung.feld.Get(i, j);
                if (s <= 0)
                    continue;
                const int x = x0 + i * gw;
                const int y = toptobottom ? y0 + j * gh : y0 + (shafts - j - 1) * gh;
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
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);
        p.drawRect(x0, y0, treadles * gw, shafts * gh);
    }

    /*  Trittfolge (bottom-right). */
    if (treadles != 0) {
        const int x0 = (dx + tdx) * gw;
        const int y0 = (shafts + sdy) * gh;
        p.setPen(QColor(105, 105, 105));
        for (int i = 0; i <= treadles; i++)
            p.drawLine(x0 + i * gw, y0, x0 + i * gw, y0 + dy * gh);
        for (int j = 0; j <= dy; j++)
            p.drawLine(x0, y0 + j * gh, x0 + treadles * gw, y0 + j * gh);
        const bool pegplan = ViewSchlagpatrone && ViewSchlagpatrone->isChecked();
        const DARSTELLUNG darst = pegplan ? schlagpatronendarstellung : trittfolge.darstellung;
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
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);
        p.drawRect(x0, y0, treadles * gw, dy * gh);
    }

    /*  Gewebe (bottom-left). Supports the same four view modes the
        editor uses:
          Unsichtbar  -> grid only, no cell content
          Patrone     -> range-coloured filled cells (the default)
          Farbeffekt  -> warp / weft palette colour, full cell
          Gewebe      -> simulated thread-over-thread render
        The simulation path mirrors DrawGewebeSimulation but with
        export-sized (fixed gw/gh=16) geometry.                     */
    {
        const int x0 = 0;
        const int y0 = (shafts + sdy) * gh;
        p.setPen(QColor(105, 105, 105));
        for (int i = 0; i <= dx; i++)
            p.drawLine(x0 + i * gw, y0, x0 + i * gw, y0 + dy * gh);
        for (int j = 0; j <= dy; j++)
            p.drawLine(x0, y0 + j * gh, x0 + dx * gw, y0 + j * gh);
        const bool none = GewebeNone && GewebeNone->isChecked();
        const bool farbeffekt = GewebeFarbeffekt && GewebeFarbeffekt->isChecked();
        const bool simulation = GewebeSimulation && GewebeSimulation->isChecked();
        if (!none) {
            const int dw = std::max(1, gw / 5);
            const int dh = std::max(1, gh / 5);
            for (int i = 0; i < dx; i++) {
                for (int j = 0; j < dy; j++) {
                    const char s = gewebe.feld.Get(i + kette.a, j + schuesse.a);
                    const int x = x0 + i * gw;
                    const int y = y0 + (dy - j - 1) * gh;
                    if (farbeffekt) {
                        bool hebung = s > 0;
                        if (sinkingshed)
                            hebung = !hebung;
                        const QColor col = hebung
                            ? paletteColor(kettfarben.feld.Get(i + kette.a))
                            : paletteColor(schussfarben.feld.Get(j + schuesse.a));
                        p.fillRect(x, y, gw, gh, col);
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
                            p.fillRect(x + dw, y, gw - 2 * dw, gh, kc);
                            p.fillRect(x, y + dh, dw, gh - 2 * dh, sc);
                            p.fillRect(x + gw - dw, y + dh, dw, gh - 2 * dh, sc);
                            p.setPen(QColor(Qt::black));
                            p.drawLine(x + gw - dw, y + dh, x + gw - dw, y + gh - dh);
                        } else {
                            /*  Weft on top: horizontal weft strip in
                                schussfarben, warp stubs top and bottom,
                                shadow line along the bottom.           */
                            p.fillRect(x, y + dh, gw, gh - 2 * dh, sc);
                            p.fillRect(x + dw, y, gw - 2 * dw, dh, kc);
                            p.fillRect(x + dw, y + gh - dh, gw - 2 * dw, dh, kc);
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
                            PaintCell(p, darst_aushebung, x, y, x + gw, y + gh,
                                      QColor(Qt::black), /*_dontclear=*/true);
                        } else {
                            p.fillRect(x, y, gw, gh, qcolorFromTColor(GetRangeColor(s)));
                        }
                    } else if (s == ANBINDUNG) {
                        if (darst_anbindung != AUSGEFUELLT) {
                            p.fillRect(x, y, gw, gh, qcolorFromTColor(col_anbindung));
                            PaintCell(p, darst_anbindung, x, y, x + gw, y + gh,
                                      QColor(Qt::black), /*_dontclear=*/true);
                        } else {
                            p.fillRect(x, y, gw, gh, qcolorFromTColor(GetRangeColor(s)));
                        }
                    } else if (s == ABBINDUNG) {
                        if (darst_abbindung != AUSGEFUELLT) {
                            p.fillRect(x, y, gw, gh, qcolorFromTColor(col_abbindung));
                            PaintCell(p, darst_abbindung, x, y, x + gw, y + gh,
                                      QColor(Qt::black), /*_dontclear=*/true);
                        } else {
                            p.fillRect(x, y, gw, gh, qcolorFromTColor(GetRangeColor(s)));
                        }
                    } else if (s > 0) {
                        /*  Ranges 1..9: solid range colour, full cell. */
                        const QColor col = qcolorFromTColor(GetRangeColor(s));
                        /*  Fill the cell footprint exactly; the right
                            / bottom neighbour's own fill covers the
                            shared grid line. See paintExportCell above
                            for why +1 would break PDF output.        */
                        p.fillRect(x, y, gw, gh, col);
                    }
                }
            }
        }
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);
        p.drawRect(x0, y0, dx * gw, dy * gh);
    }
}

/*-----------------------------------------------------------------*/
/*  Raster export (PNG / JPEG). Both use a white background; the
    legacy Windows btnFace grey made exports look like screenshots
    of the editor, which was rarely what users wanted.             */
static bool doExportRaster(TDBWFRM* _frm, const QString& _filename, const char* _format)
{
    const int gw = 16;
    const int gh = 16;
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
    const int gw = 16;
    const int gh = 16;
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
    const int gw = 16;
    const int gh = 16;
    int Wpx = 0, Hpx = 0, shafts = 0, treadles = 0;
    patternPixelSize(Wpx, Hpx, gw, gh, shafts, treadles);

    QPdfWriter pdf(_filename);
    pdf.setCreator(QStringLiteral("DB-WEAVE"));
    pdf.setTitle(QFileInfo(filename).fileName());
    /*  Paper size follows the user's locale: imperial measurement
        systems (US, Liberia, Myanmar) get US Letter, everyone else
        gets ISO A4. Picking QPageSize explicitly avoids relying on
        the platform default, which is driver/printer-dependent.   */
    const QLocale::MeasurementSystem ms = QLocale().measurementSystem();
    const QPageSize::PageSizeId pageId
        = (ms == QLocale::ImperialUSSystem || ms == QLocale::ImperialUKSystem)
              ? QPageSize::Letter
              : QPageSize::A4;
    pdf.setPageSize(QPageSize(pageId));
    pdf.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);

    QPainter p(&pdf);
    /*  Scale the pattern to fit the printable page while preserving
        aspect ratio. QPdfWriter reports its logical geometry in device
        units (the pdf resolution), so viewport/window get the fit. */
    const QRect page = p.viewport();
    const double sx = double(page.width()) / Wpx;
    const double sy = double(page.height()) / Hpx;
    const double s = qMin(sx, sy);
    const int w = int(Wpx * s);
    const int h = int(Hpx * s);
    p.setViewport(page.x() + (page.width() - w) / 2, page.y() + (page.height() - h) / 2, w, h);
    p.setWindow(0, 0, Wpx, Hpx);

    p.fillRect(QRect(0, 0, Wpx, Hpx), QColor(Qt::white));
    paintPattern(p, gw, gh, shafts, treadles);
    const bool ok = p.end();
    if (!ok)
        QMessageBox::warning(this, tr("Export"),
                             tr("Failed to write PDF file:\n%1").arg(_filename));
}
