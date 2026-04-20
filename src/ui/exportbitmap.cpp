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
#include <QPdfWriter>
#include <QSvgGenerator>

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
            PaintCell(p, einzug.darstellung, x, y, x + gw, y + gh, QColor(Qt::black));
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
        const DARSTELLUNG darst = (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
                                      ? AUSGEFUELLT
                                      : aufknuepfung.darstellung;
        for (int j = 0; j < shafts; j++)
            for (int i = 0; i < treadles; i++) {
                const char s = aufknuepfung.feld.Get(i, j);
                if (s <= 0)
                    continue;
                const int x = x0 + i * gw;
                const int y = toptobottom ? y0 + j * gh : y0 + (shafts - j - 1) * gh;
                QColor col = QColor(Qt::black);
                if (s >= 1 && s <= 9)
                    col = qcolorFromTColor(GetRangeColor(s));
                PaintCell(p, darst, x, y, x + gw, y + gh, col);
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
        const DARSTELLUNG darst = trittfolge.darstellung;
        for (int j = 0; j < dy; j++)
            for (int i = 0; i < treadles; i++) {
                const char s = trittfolge.feld.Get(i, schuesse.a + j);
                if (s <= 0)
                    continue;
                const int x = x0 + i * gw;
                const int y = y0 + (dy - j - 1) * gh;
                QColor col = QColor(Qt::black);
                if (s >= 1 && s <= 9)
                    col = qcolorFromTColor(GetRangeColor(s));
                PaintCell(p, darst, x, y, x + gw, y + gh, col);
            }
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);
        p.drawRect(x0, y0, treadles * gw, dy * gh);
    }

    /*  Gewebe (bottom-left). */
    {
        const int x0 = 0;
        const int y0 = (shafts + sdy) * gh;
        p.setPen(QColor(105, 105, 105));
        for (int i = 0; i <= dx; i++)
            p.drawLine(x0 + i * gw, y0, x0 + i * gw, y0 + dy * gh);
        for (int j = 0; j <= dy; j++)
            p.drawLine(x0, y0 + j * gh, x0 + dx * gw, y0 + j * gh);
        const bool farbeffekt = GewebeFarbeffekt && GewebeFarbeffekt->isChecked();
        for (int i = 0; i < dx; i++) {
            for (int j = 0; j < dy; j++) {
                const char s = gewebe.feld.Get(i + kette.a, j + schuesse.a);
                const int x = x0 + i * gw;
                const int y = y0 + (dy - j - 1) * gh;
                if (farbeffekt) {
                    bool hebung = s > 0;
                    if (sinkingshed)
                        hebung = !hebung;
                    const QColor col = hebung ? paletteColor(kettfarben.feld.Get(i + kette.a))
                                              : paletteColor(schussfarben.feld.Get(j + schuesse.a));
                    p.fillRect(x, y, gw, gh, col);
                } else if (s > 0) {
                    QColor col = QColor(Qt::black);
                    if (s >= 1 && s <= 9)
                        col = qcolorFromTColor(GetRangeColor(s));
                    p.fillRect(x + 1, y + 1, gw - 2, gh - 2, col);
                }
            }
        }
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);
        p.drawRect(x0, y0, dx * gw, dy * gh);
    }
}

/*-----------------------------------------------------------------*/
/*  Raster export (PNG / JPEG). */
static bool doExportRaster(TDBWFRM* _frm, const QString& _filename, const char* _format)
{
    const int gw = 16;
    const int gh = 16;
    int W = 0, H = 0, shafts = 0, treadles = 0;
    _frm->patternPixelSize(W, H, gw, gh, shafts, treadles);

    QImage img(W, H, QImage::Format_RGB32);
    img.fill(QColor(212, 208, 200));

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
    /*  The SVG generator has no background; paint one so the exported
        file matches the raster output. */
    p.fillRect(QRect(0, 0, W, H), QColor(212, 208, 200));
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
    pdf.setPageSize(QPageSize(QPageSize::A4));
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

    p.fillRect(QRect(0, 0, Wpx, Hpx), QColor(212, 208, 200));
    paintPattern(p, gw, gh, shafts, treadles);
    const bool ok = p.end();
    if (!ok)
        QMessageBox::warning(this, tr("Export"),
                             tr("Failed to write PDF file:\n%1").arg(_filename));
}
