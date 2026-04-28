/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Cell-level rendering for the print subsystem — per-cell symbol
    (PrPaintCell) and the five gewebe-rendering modes (None,
    Normal, Rapport, Farbeffekt, Simulation). Verbatim port of
    legacy printdraw.cpp.                                         */

#include "print.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "palette.h"
#include "rangecolors.h"
#include "colors_compat.h"

#include <QFont>
#include <QPainter>

/*-----------------------------------------------------------------*/
#define XSCALE(x) (int(x_scale * (x)))
#define YSCALE(y) (int(y_scale * (y)))

static QColor qcolFromTColor(TColor _c)
{
    return QColor(_c & 0xFF, (_c >> 8) & 0xFF, (_c >> 16) & 0xFF);
}

static QColor palCol(int _idx)
{
    const COLORREF c = Data->palette->GetColor(_idx);
    return QColor(GetRValue(c), GetGValue(c), GetBValue(c));
}

/*-----------------------------------------------------------------*/
void PrPrinter::PrPaintCell(DARSTELLUNG _d, int _x, int _y, int _xx, int _yy, TColor _col,
                            int _number)
{
    int penwidth = 0;
    QPen pen(qcolFromTColor(_col));
    if (_d != AUSGEFUELLT) {
        penwidth = dpiX() * 5 / 254; /* 0.5 mm */
        static constexpr int zoomTable[10] = { 5, 7, 9, 11, 13, 15, 17, 19, 21, 23 };
        penwidth = penwidth * zoomTable[mainfrm->currentzoom] / zoomTable[3];
        pen.setWidth(XSCALE(penwidth));
    }
    /*  Round caps soften the stroke ends for STRICH / KREUZ / PUNKT /
        STEIGEND / FALLEND / SMALLKREUZ when printing, avoiding the
        sharp rectangular tips that otherwise read as harsh on paper.
        Screen rendering (draw_cell.cpp) keeps flat caps so small
        on-screen symbols stay crisp.                                */
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    canvas->setPen(pen);
    canvas->setBrush(QBrush(qcolFromTColor(_col)));

    const int dx = penwidth;
    const int dy = penwidth;

    switch (_d) {
    case AUSGEFUELLT:
        canvas->fillRect(QRect(XSCALE(_x + dx), YSCALE(_y + dy),
                               XSCALE(_xx - dx + 1) - XSCALE(_x + dx),
                               YSCALE(_yy - dy + 1) - YSCALE(_y + dy)),
                         qcolFromTColor(_col));
        break;

    case STRICH:
        canvas->drawLine(XSCALE((_x + _xx) / 2), YSCALE(_yy - dy), XSCALE((_x + _xx) / 2),
                         YSCALE(_y + dy + 1));
        canvas->drawLine(XSCALE((_x + _xx) / 2 + 1), YSCALE(_yy - dy), XSCALE((_x + _xx) / 2 + 1),
                         YSCALE(_y + dy + 1));
        break;

    case KREUZ:
        canvas->drawLine(XSCALE(_x + dx), YSCALE(_yy - dy), XSCALE(_xx - dx), YSCALE(_y + dy));
        canvas->drawLine(XSCALE(_x + dx), YSCALE(_y + dy), XSCALE(_xx - dx), YSCALE(_yy - dy));
        break;

    case PUNKT: {
        const int x = (_x + _xx) / 2;
        const int y = (_y + _yy) / 2;
        canvas->drawLine(XSCALE(x), YSCALE(y), XSCALE(x + 1), YSCALE(y));
        canvas->drawLine(XSCALE(x), YSCALE(y + 1), XSCALE(x + 1), YSCALE(y + 1));
        break;
    }
    case KREIS:
        canvas->setBrush(Qt::NoBrush);
        canvas->drawEllipse(XSCALE(_x + dx), YSCALE(_y + dy),
                            XSCALE(_xx - dx + 1) - XSCALE(_x + dx),
                            YSCALE(_yy - dy + 1) - YSCALE(_y + dy));
        break;

    case STEIGEND:
        canvas->drawLine(XSCALE(_x + dx), YSCALE(_yy - dy), XSCALE(_xx - dx), YSCALE(_y + dy));
        break;

    case FALLEND:
        canvas->drawLine(XSCALE(_x + dx), YSCALE(_y + dy), XSCALE(_xx - dx), YSCALE(_yy - dy));
        break;

    case SMALLKREIS: {
        QPen small = canvas->pen();
        if (small.width() > 2)
            small.setWidth(small.width() / 2);
        canvas->setPen(small);
        canvas->setBrush(Qt::NoBrush);
        const int sdx = dx + (_xx - _x) / 10;
        const int sdy = dy + (_yy - _y) / 10;
        canvas->drawEllipse(XSCALE(_x + sdx), YSCALE(_y + sdy),
                            XSCALE(_xx - sdx + 1) - XSCALE(_x + sdx),
                            YSCALE(_yy - sdy + 1) - YSCALE(_y + sdy));
        break;
    }

    case SMALLKREUZ: {
        QPen small = canvas->pen();
        if (small.width() > 2)
            small.setWidth(small.width() / 2);
        canvas->setPen(small);
        const int sdx = dx + (_xx - _x) / 10;
        const int sdy = dy + (_yy - _y) / 10;
        canvas->drawLine(XSCALE(_x + sdx), YSCALE(_yy - sdy), XSCALE(_xx - sdx), YSCALE(_y + sdy));
        canvas->drawLine(XSCALE(_x + sdx), YSCALE(_y + sdy), XSCALE(_xx - sdx), YSCALE(_yy - sdy));
        break;
    }

    case NUMBER:
        if (_number >= 0) {
            QFont f = canvas->font();
            f.setPixelSize(std::max(1, einzug.gh * 8 / 10));
            canvas->setFont(f);
            canvas->setPen(qcolFromTColor(_col));
            const QString s = QString::number(_number + 1);
            canvas->drawText(QRect(XSCALE(_x + dx), YSCALE(_y + dy),
                                   XSCALE(_xx - dx) - XSCALE(_x + dx),
                                   YSCALE(_yy - dy) - YSCALE(_y + dy)),
                             Qt::AlignCenter, s);
        } else {
            canvas->fillRect(QRect(XSCALE(_x + dx), YSCALE(_y + dy),
                                   XSCALE(_xx - dx + 1) - XSCALE(_x + dx),
                                   YSCALE(_yy - dy + 1) - YSCALE(_y + dy)),
                             qcolFromTColor(_col));
        }
        break;

    case HDASH:
        canvas->drawLine(XSCALE(_x + dx), YSCALE((_y + _yy) / 2), XSCALE(_xx - dx + 1),
                         YSCALE((_y + _yy) / 2));
        canvas->drawLine(XSCALE(_x + dx), YSCALE((_y + _yy) / 2 + 1), XSCALE(_xx - dx + 1),
                         YSCALE((_y + _yy) / 2 + 1));
        break;

    case PLUS:
        canvas->drawLine(XSCALE((_x + _xx) / 2), YSCALE(_yy - dy), XSCALE((_x + _xx) / 2),
                         YSCALE(_y + dy + 1));
        canvas->drawLine(XSCALE((_x + _xx) / 2 + 1), YSCALE(_yy - dy), XSCALE((_x + _xx) / 2 + 1),
                         YSCALE(_y + dy + 1));
        canvas->drawLine(XSCALE(_x + dx), YSCALE((_y + _yy) / 2), XSCALE(_xx - dx + 1),
                         YSCALE((_y + _yy) / 2));
        canvas->drawLine(XSCALE(_x + dx), YSCALE((_y + _yy) / 2 + 1), XSCALE(_xx - dx + 1),
                         YSCALE((_y + _yy) / 2 + 1));
        break;
    }
}

/*-----------------------------------------------------------------*/
/*  gewebe rendering modes. */
void PrPrinter::PrintGewebeNone(int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    canvas->setPen(Qt::black);
    canvas->setBrush(Qt::NoBrush);
    canvas->drawRect(XSCALE(_x), YSCALE(_y), XSCALE(_xx) - XSCALE(_x), YSCALE(_yy) - YSCALE(_y));
}

void PrPrinter::PrintGewebeNormal(int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    const char s = mainfrm->gewebe.feld.Get(_i + kette.a, _j + schuesse.a);
    if (s == AUSHEBUNG)
        PrPaintCell(mainfrm->darst_aushebung, _x, _y, _xx, _yy);
    else if (s == ANBINDUNG)
        PrPaintCell(mainfrm->darst_anbindung, _x, _y, _xx, _yy);
    else if (s == ABBINDUNG)
        PrPaintCell(mainfrm->darst_abbindung, _x, _y, _xx, _yy);
    else if (s > 0) {
        canvas->setPen(qcolFromTColor(GetRangeColor(s)));
        canvas->setBrush(qcolFromTColor(GetRangeColor(s)));
        canvas->fillRect(QRect(XSCALE(_x), YSCALE(_y), XSCALE(_xx + 1) - XSCALE(_x),
                               YSCALE(_yy + 1) - YSCALE(_y)),
                         qcolFromTColor(GetRangeColor(s)));
    } else {
        canvas->setPen(Qt::black);
        canvas->setBrush(Qt::white);
        canvas->drawRect(XSCALE(_x), YSCALE(_y), XSCALE(_xx + 1) - XSCALE(_x),
                         YSCALE(_yy + 1) - YSCALE(_y));
    }
}

void PrPrinter::PrintGewebeRapport(int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    const char s = mainfrm->gewebe.feld.Get(_i + kette.a, _j + schuesse.a);
    QColor fill = Qt::white;
    if (s > 0 && s != ABBINDUNG)
        fill = blackandwhite ? QColor(Qt::gray) : QColor(Qt::red);
    canvas->setPen(Qt::black);
    canvas->setBrush(fill);
    canvas->drawRect(XSCALE(_x), YSCALE(_y), XSCALE(_xx + 1) - XSCALE(_x),
                     YSCALE(_yy + 1) - YSCALE(_y));
}

void PrPrinter::PrintGewebeFarbeffekt(int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    const char s = mainfrm->gewebe.feld.Get(_i + kette.a, _j + schuesse.a);
    bool hebung = s > 0 && s != ABBINDUNG;
    if (mainfrm->sinkingshed)
        hebung = !hebung;
    const QColor col = hebung ? palCol(mainfrm->kettfarben.feld.Get(_i + kette.a))
                              : palCol(mainfrm->schussfarben.feld.Get(_j + schuesse.a));
    canvas->fillRect(
        QRect(XSCALE(_x), YSCALE(_y), XSCALE(_xx + 1) - XSCALE(_x), YSCALE(_yy + 1) - YSCALE(_y)),
        col);
    if (mainfrm->fewithraster)
        PrintGewebeNone(_i, _j, _x, _y, _xx, _yy);
}

void PrPrinter::PrintGewebeSimulation(int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    int dw = std::abs(_xx - _x) / 4;
    int dh = std::abs(_yy - _y) / 4;
    if (dw == 0 && std::abs(_xx - _x) > 2)
        dw = 1;
    if (dh == 0 && std::abs(_yy - _y) > 2)
        dh = 1;

    const char s = mainfrm->gewebe.feld.Get(_i + kette.a, _j + schuesse.a);
    bool hebung = s > 0 && s != ABBINDUNG;
    if (mainfrm->sinkingshed)
        hebung = !hebung;

    const QColor kettCol = palCol(mainfrm->kettfarben.feld.Get(_i + kette.a));
    const QColor schuCol = palCol(mainfrm->schussfarben.feld.Get(_j + schuesse.a));

    if (hebung) {
        canvas->fillRect(QRect(XSCALE(_x + dw), YSCALE(_y), XSCALE(_xx - dw) - XSCALE(_x + dw),
                               YSCALE(_yy) - YSCALE(_y)),
                         kettCol);
        canvas->fillRect(QRect(XSCALE(_x), YSCALE(_y + dh), XSCALE(_x + dw) - XSCALE(_x),
                               YSCALE(_yy - dh) - YSCALE(_y + dh)),
                         schuCol);
        canvas->fillRect(QRect(XSCALE(_xx - dw), YSCALE(_y + dh), XSCALE(_xx) - XSCALE(_xx - dw),
                               YSCALE(_yy - dh) - YSCALE(_y + dh)),
                         schuCol);
        QPen p(Qt::black);
        p.setWidth(dw > 1 ? dw / 3 : 1);
        canvas->setPen(p);
        canvas->drawLine(XSCALE(_xx - dw), YSCALE(_y + dh), XSCALE(_xx - dw), YSCALE(_yy - dh));
    } else {
        canvas->fillRect(QRect(XSCALE(_x), YSCALE(_y + dh), XSCALE(_xx) - XSCALE(_x),
                               YSCALE(_yy - dh) - YSCALE(_y + dh)),
                         schuCol);
        canvas->fillRect(QRect(XSCALE(_x + dw), YSCALE(_y), XSCALE(_xx - dw) - XSCALE(_x + dw),
                               YSCALE(_y + dh) - YSCALE(_y)),
                         kettCol);
        canvas->fillRect(QRect(XSCALE(_x + dw), YSCALE(_yy - dh),
                               XSCALE(_xx - dw) - XSCALE(_x + dw), YSCALE(_yy) - YSCALE(_yy - dh)),
                         kettCol);
        QPen p(Qt::black);
        p.setWidth(dh > 1 ? dh / 3 : 1);
        canvas->setPen(p);
        canvas->drawLine(XSCALE(_x + dw), YSCALE(_yy - dh), XSCALE(_xx - dw), YSCALE(_yy - dh));
    }
}
