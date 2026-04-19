/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Bitmap export — a simplified Qt port of legacy exportbitmap.cpp
    DoExportBitmap. The legacy routine is a 650-line render that
    reproduces the full screen layout on an off-screen TBitmap with
    every view-option toggle honoured (Simulation, Farbeffekt,
    Hilfslinien, blatteinzug, colour strips, rapport markers). This
    port covers the four core grids (einzug, aufknuepfung,
    trittfolge, gewebe) rendered in the active darstellung — the
    common case. Richer options can be added as they are needed. */

#include "mainwindow.h"
#include "datamodule.h"
#include "palette.h"
#include "rangecolors.h"
#include "draw_cell.h"
#include "colors_compat.h"

#include <QImage>
#include <QPainter>

static QColor qcolorFromTColor (TColor _c)
{
	return QColor(_c & 0xFF, (_c >> 8) & 0xFF, (_c >> 16) & 0xFF);
}

static QColor paletteColor (int _idx)
{
	const COLORREF c = Data->palette->GetColor(_idx);
	return QColor(GetRValue(c), GetGValue(c), GetBValue(c));
}

/*-----------------------------------------------------------------*/
void TDBWFRM::DoExportBitmap (const QString& _filename)
{
	const int gw = 16;
	const int gh = 16;

	/*  Count used shafts/treadles so the exported image covers only
	    the live range, not the full MAXY1/MAXX2. */
	int shafts = 0;
	if (ViewEinzug && ViewEinzug->isChecked()) {
		for (int i = kette.a; i <= kette.b; i++) {
			const int s = einzug.feld.Get(i);
			if (s > shafts) shafts = s;
		}
	}
	int treadles = 0;
	if (ViewTrittfolge && ViewTrittfolge->isChecked()) {
		for (int i = Data->MAXX2 - 1; i >= 0; i--)
			for (int j = schuesse.a; j <= schuesse.b; j++)
				if (trittfolge.feld.Get(i, j) != 0)
					if (treadles < i + 1) treadles = i + 1;
	}

	const int sdy = shafts   != 0 ? 1 : 0;
	const int tdx = treadles != 0 ? 1 : 0;

	const int dx = kette.count();
	const int dy = schuesse.count();

	const int W = gw * (dx + tdx + treadles) + 1;
	const int H = gh * (dy + sdy + shafts)   + 1;

	QImage img(W, H, QImage::Format_RGB32);
	img.fill(QColor(212, 208, 200));

	QPainter p(&img);
	p.setRenderHint(QPainter::Antialiasing, false);

	/*  Einzug (top-left). */
	if (shafts != 0) {
		const int x0 = 0;
		const int y0 = 0;
		p.setPen(QColor(105, 105, 105));
		for (int i = 0; i <= dx;     i++) p.drawLine(x0+i*gw, y0, x0+i*gw, y0 + shafts*gh);
		for (int j = 0; j <= shafts; j++) p.drawLine(x0, y0+j*gh, x0+dx*gw, y0+j*gh);
		for (int i = 0; i < dx; i++) {
			const int s = einzug.feld.Get(kette.a + i);
			if (s == 0) continue;
			const int x = righttoleft ? x0 + (dx-i-1)*gw : x0 + i*gw;
			const int y = toptobottom ? y0 + (s-1)*gh   : y0 + (shafts-s)*gh;
			PaintCell(p, einzug.darstellung, x, y, x+gw, y+gh, QColor(Qt::black));
		}
		p.setPen(Qt::black);
		p.drawRect(x0, y0, dx*gw, shafts*gh);
	}

	/*  Aufknuepfung (top-right). */
	if (treadles != 0 && shafts != 0) {
		const int x0 = (dx + tdx) * gw;
		const int y0 = 0;
		p.setPen(QColor(105, 105, 105));
		for (int i = 0; i <= treadles; i++) p.drawLine(x0+i*gw, y0, x0+i*gw, y0+shafts*gh);
		for (int j = 0; j <= shafts;   j++) p.drawLine(x0, y0+j*gh, x0+treadles*gw, y0+j*gh);
		const DARSTELLUNG darst = (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
		    ? AUSGEFUELLT : aufknuepfung.darstellung;
		for (int j = 0; j < shafts; j++)
			for (int i = 0; i < treadles; i++) {
				const char s = aufknuepfung.feld.Get(i, j);
				if (s <= 0) continue;
				const int x = x0 + i*gw;
				const int y = toptobottom ? y0 + j*gh : y0 + (shafts-j-1)*gh;
				QColor col = QColor(Qt::black);
				if (s >= 1 && s <= 9) col = qcolorFromTColor(GetRangeColor(s));
				PaintCell(p, darst, x, y, x+gw, y+gh, col);
			}
		p.setPen(Qt::black);
		p.drawRect(x0, y0, treadles*gw, shafts*gh);
	}

	/*  Trittfolge (bottom-right). */
	if (treadles != 0) {
		const int x0 = (dx + tdx) * gw;
		const int y0 = (shafts + sdy) * gh;
		p.setPen(QColor(105, 105, 105));
		for (int i = 0; i <= treadles; i++) p.drawLine(x0+i*gw, y0, x0+i*gw, y0+dy*gh);
		for (int j = 0; j <= dy;        j++) p.drawLine(x0, y0+j*gh, x0+treadles*gw, y0+j*gh);
		const DARSTELLUNG darst = trittfolge.darstellung;
		for (int j = 0; j < dy; j++)
			for (int i = 0; i < treadles; i++) {
				const char s = trittfolge.feld.Get(i, schuesse.a + j);
				if (s <= 0) continue;
				const int x = x0 + i*gw;
				const int y = y0 + (dy-j-1)*gh;
				QColor col = QColor(Qt::black);
				if (s >= 1 && s <= 9) col = qcolorFromTColor(GetRangeColor(s));
				PaintCell(p, darst, x, y, x+gw, y+gh, col);
			}
		p.setPen(Qt::black);
		p.drawRect(x0, y0, treadles*gw, dy*gh);
	}

	/*  Gewebe (bottom-left). */
	{
		const int x0 = 0;
		const int y0 = (shafts + sdy) * gh;
		p.setPen(QColor(105, 105, 105));
		for (int i = 0; i <= dx; i++) p.drawLine(x0+i*gw, y0, x0+i*gw, y0+dy*gh);
		for (int j = 0; j <= dy; j++) p.drawLine(x0, y0+j*gh, x0+dx*gw, y0+j*gh);
		const bool farbeffekt = GewebeFarbeffekt && GewebeFarbeffekt->isChecked();
		for (int i = 0; i < dx; i++) {
			for (int j = 0; j < dy; j++) {
				const char s = gewebe.feld.Get(i + kette.a, j + schuesse.a);
				const int x = x0 + i*gw;
				const int y = y0 + (dy - j - 1)*gh;
				if (farbeffekt) {
					bool hebung = s > 0;
					if (sinkingshed) hebung = !hebung;
					const QColor col = hebung
					    ? paletteColor(kettfarben.feld.Get(i + kette.a))
					    : paletteColor(schussfarben.feld.Get(j + schuesse.a));
					p.fillRect(x, y, gw, gh, col);
				} else if (s > 0) {
					QColor col = QColor(Qt::black);
					if (s >= 1 && s <= 9) col = qcolorFromTColor(GetRangeColor(s));
					p.fillRect(x+1, y+1, gw-2, gh-2, col);
				}
			}
		}
		p.setPen(Qt::black);
		p.drawRect(x0, y0, dx*gw, dy*gh);
	}

	p.end();
	img.save(_filename);
}
