/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Per-field print methods — kettfarben, einzug, blatteinzug,
    gewebe, aufknuepfung, trittfolge, schussfarben, and hilfslinien.
    Verbatim port of legacy printprint.cpp; TCanvas calls become
    QPainter calls over the bound `canvas` painter.                */

#include "print.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "palette.h"
#include "rangecolors.h"
#include "hilfslinien.h"
#include "colors_compat.h"

#include <QPainter>
#include <QPen>

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
#define SETSTRONGWIDTH                              \
    QPen savedPen = canvas->pen();                  \
    QPen strongPen = savedPen;                      \
    strongPen.setWidth(XSCALE(dpiX() * 25 / 2540)); \
    canvas->setPen(strongPen);                      \
    const int pw = strongPen.width() / 2;
#define RESETSTRONGWIDTH canvas->setPen(savedPen);

/*-----------------------------------------------------------------*/
void PrPrinter::PrintHilfslinien()
{
    if (!mainfrm->ViewHlines || !mainfrm->ViewHlines->isChecked())
        return;
    const int count = mainfrm->hlines.GetCount();
    for (int i = 0; i < count; i++) {
        Hilfslinie* line = mainfrm->hlines.GetLine(i);
        if (line)
            PrintHilfslinie(line);
    }
}

void PrPrinter::PrintHilfslinie(Hilfslinie* _h)
{
    const QColor hlcol = blackandwhite ? QColor(Qt::black) : QColor(Qt::blue);
    switch (_h->typ) {
    case HL_HORZ:
        if (_h->feld == HL_TOP) {
            if (!einzug.visible)
                break;
            if (_h->pos <= schaefte.a || _h->pos >= schaefte.b)
                break;
            canvas->setPen(hlcol);
            int y;
            if (mainfrm->toptobottom)
                y = einzug.y0 + (_h->pos - schaefte.a) * einzug.gh;
            else
                y = einzug.y0 + einzug.height - (_h->pos - schaefte.a) * einzug.gh;
            SETSTRONGWIDTH
            strongPen.setColor(hlcol);
            canvas->setPen(strongPen);
            canvas->drawLine(XSCALE(einzug.x0 + pw), YSCALE(y),
                             XSCALE(einzug.x0 + einzug.width - pw), YSCALE(y));
            if (trittfolge.visible)
                canvas->drawLine(XSCALE(aufknuepfung.x0 + pw), YSCALE(y),
                                 XSCALE(aufknuepfung.x0 + aufknuepfung.width - pw), YSCALE(y));
            RESETSTRONGWIDTH
        } else {
            if (_h->pos <= schuesse.a || _h->pos >= schuesse.b)
                break;
            int y = gewebe.y0 + gewebe.height - (_h->pos - schuesse.a) * gewebe.gh;
            SETSTRONGWIDTH
            strongPen.setColor(hlcol);
            canvas->setPen(strongPen);
            if ((mainfrm->GewebeNormal && mainfrm->GewebeNormal->isChecked())
                || (mainfrm->GewebeNone && mainfrm->GewebeNone->isChecked()))
                canvas->drawLine(XSCALE(gewebe.x0 + pw), YSCALE(y),
                                 XSCALE(gewebe.x0 + gewebe.width - pw), YSCALE(y));
            if (trittfolge.visible)
                canvas->drawLine(XSCALE(trittfolge.x0 + pw), YSCALE(y),
                                 XSCALE(trittfolge.x0 + trittfolge.width - pw), YSCALE(y));
            RESETSTRONGWIDTH
        }
        break;
    case HL_VERT:
        if (_h->feld == HL_LEFT) {
            const bool rtl = mainfrm->righttoleft;
            if (_h->pos <= kette.a || _h->pos >= kette.b)
                break;
            int x;
            if (!rtl)
                x = gewebe.x0 + (_h->pos - kette.a) * gewebe.gw;
            else
                x = gewebe.x0 + gewebe.width - (_h->pos - kette.a) * gewebe.gw;
            SETSTRONGWIDTH
            strongPen.setColor(hlcol);
            canvas->setPen(strongPen);
            if ((mainfrm->GewebeNormal && mainfrm->GewebeNormal->isChecked())
                || (mainfrm->GewebeNone && mainfrm->GewebeNone->isChecked()))
                canvas->drawLine(XSCALE(x), YSCALE(gewebe.y0 + pw), XSCALE(x),
                                 YSCALE(gewebe.y0 + gewebe.height - pw));
            if (einzug.visible)
                canvas->drawLine(XSCALE(x), YSCALE(einzug.y0 + pw), XSCALE(x),
                                 YSCALE(einzug.y0 + einzug.height - pw));
            RESETSTRONGWIDTH
        } else {
            if (!trittfolge.visible)
                break;
            if (_h->pos <= tritte.a || _h->pos >= tritte.b)
                break;
            int x = trittfolge.x0 + (_h->pos - tritte.a) * trittfolge.gw;
            SETSTRONGWIDTH
            strongPen.setColor(hlcol);
            canvas->setPen(strongPen);
            canvas->drawLine(XSCALE(x), YSCALE(trittfolge.y0 + pw), XSCALE(x),
                             YSCALE(trittfolge.y0 + trittfolge.height - pw));
            if (einzug.visible)
                canvas->drawLine(XSCALE(x), YSCALE(aufknuepfung.y0 + pw), XSCALE(x),
                                 YSCALE(aufknuepfung.y0 + aufknuepfung.height - pw));
            RESETSTRONGWIDTH
        }
        break;
    }
}

/*-----------------------------------------------------------------*/
void PrPrinter::PrintKettfarben()
{
    if (!kettfarben.visible)
        return;
    const int x0 = kettfarben.x0;
    const int y0 = kettfarben.y0;
    const bool rtl = mainfrm->righttoleft;
    for (int i = kette.a; i <= kette.b; i++) {
        canvas->setPen(Qt::black);
        canvas->setBrush(palCol(mainfrm->kettfarben.feld.Get(i)));
        const int x = rtl ? x0 + (kette.b - i) * kettfarben.gw : x0 + (i - kette.a) * kettfarben.gw;
        canvas->drawRect(XSCALE(x), YSCALE(y0), XSCALE(x + kettfarben.gw + 1) - XSCALE(x),
                         YSCALE(y0 + kettfarben.height + 1) - YSCALE(y0));
    }
}

/*-----------------------------------------------------------------*/
void PrPrinter::PrintEinzug()
{
    if (!einzug.visible)
        return;
    const int x0 = einzug.x0;
    const int y0 = einzug.y0 + einzug.height;
    const bool rtl = mainfrm->righttoleft;
    const bool ttb = mainfrm->toptobottom;

    for (int i = 0; i < kette.count(); i++) {
        for (int j = 0; j < schaefte.count(); j++) {
            const int ii = rtl ? kette.count() - 1 - i : i;
            const int jj = ttb ? schaefte.count() - 1 - j : j;
            canvas->setPen(Qt::black);
            canvas->setBrush(Qt::NoBrush);
            canvas->drawRect(XSCALE(x0 + ii * einzug.gw), YSCALE(y0 - (jj + 1) * einzug.gh),
                             XSCALE(x0 + (ii + 1) * einzug.gw) - XSCALE(x0 + ii * einzug.gw),
                             YSCALE(y0 - jj * einzug.gh) - YSCALE(y0 - (jj + 1) * einzug.gh));
            if (!(mainfrm->ViewOnlyGewebe && mainfrm->ViewOnlyGewebe->isChecked())) {
                if (mainfrm->einzug.feld.Get(i + kette.a) == j + schaefte.a + 1) {
                    PrPaintCell(mainfrm->einzug.darstellung, x0 + ii * einzug.gw,
                                y0 - (jj + 1) * einzug.gh, x0 + (ii + 1) * einzug.gw,
                                y0 - jj * einzug.gh, TColor(0), j + schaefte.a);
                }
            }
        }
    }

    SETSTRONGWIDTH
    strongPen.setColor(Qt::black);
    canvas->setPen(strongPen);
    for (int i = 0; i < kette.count(); i++) {
        const int ii = rtl ? kette.count() - 1 - i : i;
        if (rtl) {
            if (i != 0 && mainfrm->einzug.pos.strongline_x != 0
                && ((i + kette.a) % mainfrm->einzug.pos.strongline_x) == 0)
                canvas->drawLine(XSCALE(x0 + (ii + 1) * einzug.gw), YSCALE(einzug.y0 + pw),
                                 XSCALE(x0 + (ii + 1) * einzug.gw),
                                 YSCALE(einzug.y0 + einzug.height - pw));
        } else {
            if (i != 0 && mainfrm->einzug.pos.strongline_x != 0
                && ((i + kette.a) % mainfrm->einzug.pos.strongline_x) == 0)
                canvas->drawLine(XSCALE(x0 + ii * einzug.gw), YSCALE(einzug.y0 + pw),
                                 XSCALE(x0 + ii * einzug.gw),
                                 YSCALE(einzug.y0 + einzug.height - pw));
        }
    }
    for (int j = 0; j < schaefte.count(); j++) {
        const int jj = ttb ? schaefte.count() - 1 - j : j;
        if (ttb) {
            if (j != 0 && mainfrm->einzug.pos.strongline_y != 0
                && ((j + schaefte.a) % mainfrm->einzug.pos.strongline_y) == 0)
                canvas->drawLine(XSCALE(einzug.x0 + pw), YSCALE(y0 - (jj + 1) * einzug.gh),
                                 XSCALE(einzug.x0 + einzug.width - pw),
                                 YSCALE(y0 - (jj + 1) * einzug.gh));
        } else {
            if (j != 0 && mainfrm->einzug.pos.strongline_y != 0
                && ((j + schaefte.a) % mainfrm->einzug.pos.strongline_y) == 0)
                canvas->drawLine(XSCALE(einzug.x0 + pw), YSCALE(y0 - jj * einzug.gh),
                                 XSCALE(einzug.x0 + einzug.width - pw),
                                 YSCALE(y0 - jj * einzug.gh));
        }
    }
    RESETSTRONGWIDTH
}

/*-----------------------------------------------------------------*/
void PrPrinter::PrintBlatteinzug()
{
    if (!blatteinzug.visible)
        return;
    const int x0 = blatteinzug.x0;
    const int y0 = blatteinzug.y0;
    const bool rtl = mainfrm->righttoleft;
    for (int i = kette.a; i <= kette.b; i++) {
        canvas->setPen(Qt::black);
        const bool top = mainfrm->blatteinzug.feld.Get(i);
        const int x
            = rtl ? x0 + (kette.b - i) * blatteinzug.gw : x0 + (i - kette.a) * blatteinzug.gw;
        canvas->setBrush(top ? QColor(Qt::black) : QColor(Qt::white));
        canvas->drawRect(XSCALE(x), YSCALE(y0), XSCALE(x + blatteinzug.gw + 1) - XSCALE(x),
                         YSCALE(y0 + blatteinzug.height / 2 + 1) - YSCALE(y0));
        canvas->setBrush(top ? QColor(Qt::white) : QColor(Qt::black));
        canvas->drawRect(XSCALE(x), YSCALE(y0 + blatteinzug.height / 2 - 1),
                         XSCALE(x + blatteinzug.gw + 1) - XSCALE(x),
                         YSCALE(y0 + blatteinzug.height + 1)
                             - YSCALE(y0 + blatteinzug.height / 2 - 1));
    }
}

/*-----------------------------------------------------------------*/
void PrPrinter::PrintGewebe()
{
    if (!gewebe.visible)
        return;
    const int x0 = gewebe.x0;
    const int y0 = gewebe.y0 + gewebe.height;
    const bool rtl = mainfrm->righttoleft;
    const bool normal = mainfrm->GewebeNormal && mainfrm->GewebeNormal->isChecked();
    const bool farbeffekt = mainfrm->GewebeFarbeffekt && mainfrm->GewebeFarbeffekt->isChecked();
    const bool simulation = mainfrm->GewebeSimulation && mainfrm->GewebeSimulation->isChecked();
    const bool none = mainfrm->GewebeNone && mainfrm->GewebeNone->isChecked();
    const bool rapport = mainfrm->RappViewRapport && mainfrm->RappViewRapport->isChecked();
    const bool inv = mainfrm->Inverserepeat && mainfrm->Inverserepeat->isChecked();

    for (int i = 0; i < kette.count(); i++) {
        for (int j = 0; j < schuesse.count(); j++) {
            const int ii = rtl ? kette.count() - 1 - i : i;
            if (mainfrm->IsEmptyTrittfolge(j + schuesse.a) || mainfrm->IsEmptyEinzug(i + kette.a)
                || normal) {
                canvas->setPen(Qt::black);
                canvas->setBrush(Qt::NoBrush);
                canvas->drawRect(XSCALE(x0 + ii * gewebe.gw), YSCALE(y0 - (j + 1) * gewebe.gh),
                                 XSCALE(x0 + (ii + 1) * gewebe.gw) - XSCALE(x0 + ii * gewebe.gw),
                                 YSCALE(y0 - j * gewebe.gh) - YSCALE(y0 - (j + 1) * gewebe.gh));
            }
            if (!mainfrm->IsEmptyEinzug(i + kette.a)
                && !mainfrm->IsEmptyTrittfolge(j + schuesse.a)) {
                const bool inRap = mainfrm->IsInRapport(i + kette.a, j + schuesse.a);
                const bool wantRap = rapport && (inv ? !inRap : inRap);
                if (wantRap && !none)
                    PrintGewebeRapport(i, j, x0 + ii * gewebe.gw, y0 - (j + 1) * gewebe.gh,
                                       x0 + (ii + 1) * gewebe.gw, y0 - j * gewebe.gh);
                else if (normal)
                    PrintGewebeNormal(i, j, x0 + ii * gewebe.gw, y0 - (j + 1) * gewebe.gh,
                                      x0 + (ii + 1) * gewebe.gw, y0 - j * gewebe.gh);
                else if (farbeffekt)
                    PrintGewebeFarbeffekt(i, j, x0 + ii * gewebe.gw, y0 - (j + 1) * gewebe.gh,
                                          x0 + (ii + 1) * gewebe.gw, y0 - j * gewebe.gh);
                else if (simulation)
                    PrintGewebeSimulation(i, j, x0 + ii * gewebe.gw, y0 - (j + 1) * gewebe.gh,
                                          x0 + (ii + 1) * gewebe.gw, y0 - j * gewebe.gh + 1);
                else if (none)
                    PrintGewebeNone(i, j, x0 + ii * gewebe.gw, y0 - (j + 1) * gewebe.gh,
                                    x0 + (ii + 1) * gewebe.gw, y0 - j * gewebe.gh);
            }
        }
    }

    if (normal || none) {
        SETSTRONGWIDTH
        strongPen.setColor(Qt::black);
        canvas->setPen(strongPen);
        for (int i = 0; i < kette.count(); i++) {
            const int ii = rtl ? kette.count() - 1 - i : i;
            if (rtl) {
                if (i != 0 && mainfrm->gewebe.pos.strongline_x != 0
                    && ((i + kette.a) % mainfrm->gewebe.pos.strongline_x) == 0)
                    canvas->drawLine(XSCALE(x0 + (ii + 1) * gewebe.gw), YSCALE(gewebe.y0 + pw),
                                     XSCALE(x0 + (ii + 1) * gewebe.gw),
                                     YSCALE(gewebe.y0 + gewebe.height - pw));
            } else {
                if (i != 0 && mainfrm->gewebe.pos.strongline_x != 0
                    && ((i + kette.a) % mainfrm->gewebe.pos.strongline_x) == 0)
                    canvas->drawLine(XSCALE(x0 + ii * gewebe.gw), YSCALE(gewebe.y0 + pw),
                                     XSCALE(x0 + ii * gewebe.gw),
                                     YSCALE(gewebe.y0 + gewebe.height - pw));
            }
        }
        for (int j = 0; j < schuesse.count(); j++) {
            if (j != 0 && mainfrm->gewebe.pos.strongline_y != 0
                && ((j + schuesse.a) % mainfrm->gewebe.pos.strongline_y) == 0)
                canvas->drawLine(XSCALE(gewebe.x0 + pw), YSCALE(y0 - j * gewebe.gh),
                                 XSCALE(gewebe.x0 + gewebe.width - pw), YSCALE(y0 - j * gewebe.gh));
        }
        RESETSTRONGWIDTH
    }

    canvas->setPen(Qt::black);
    canvas->setBrush(Qt::NoBrush);
    canvas->drawRect(XSCALE(gewebe.x0), YSCALE(gewebe.y0),
                     XSCALE(gewebe.x0 + kette.count() * gewebe.gw) - XSCALE(gewebe.x0),
                     YSCALE(gewebe.y0 + schuesse.count() * gewebe.gh) - YSCALE(gewebe.y0));
}

/*-----------------------------------------------------------------*/
void PrPrinter::PrintAufknuepfung()
{
    if (!aufknuepfung.visible)
        return;
    const int x0 = aufknuepfung.x0;
    const int y0 = aufknuepfung.y0 + aufknuepfung.height;
    const bool schlag = mainfrm->ViewSchlagpatrone && mainfrm->ViewSchlagpatrone->isChecked();
    const bool pegplanstyle = mainfrm->aufknuepfung.pegplanstyle;

    if (!schlag || !pegplanstyle || mainfrm->einzugunten) {
        for (int i = 0; i < tritte.count(); i++) {
            for (int j = 0; j < schaefte.count(); j++) {
                const int jj = mainfrm->toptobottom ? schaefte.count() - 1 - j : j;
                canvas->setPen(Qt::black);
                canvas->setBrush(Qt::NoBrush);
                canvas->drawRect(
                    XSCALE(x0 + i * aufknuepfung.gw), YSCALE(y0 - (jj + 1) * aufknuepfung.gh),
                    XSCALE(x0 + (i + 1) * aufknuepfung.gw) - XSCALE(x0 + i * aufknuepfung.gw),
                    YSCALE(y0 - jj * aufknuepfung.gh) - YSCALE(y0 - (jj + 1) * aufknuepfung.gh));
                if (!(mainfrm->ViewOnlyGewebe && mainfrm->ViewOnlyGewebe->isChecked())) {
                    const char s = mainfrm->aufknuepfung.feld.Get(i + tritte.a, j + schaefte.a);
                    const int _x = x0 + i * aufknuepfung.gw;
                    const int _y = y0 - (jj + 1) * aufknuepfung.gh;
                    const int _xx = x0 + (i + 1) * aufknuepfung.gw;
                    const int _yy = y0 - jj * aufknuepfung.gh;
                    if (s == AUSHEBUNG)
                        PrPaintCell(mainfrm->darst_aushebung, _x, _y, _xx, _yy);
                    else if (s == ANBINDUNG)
                        PrPaintCell(mainfrm->darst_anbindung, _x, _y, _xx, _yy);
                    else if (s == ABBINDUNG)
                        PrPaintCell(mainfrm->darst_abbindung, _x, _y, _xx, _yy);
                    else if (s > 0) {
                        if (schlag)
                            PrPaintCell(mainfrm->aufknuepfung.darstellung, _x, _y, _xx, _yy);
                        else
                            PrPaintCell(mainfrm->aufknuepfung.darstellung, _x, _y, _xx, _yy,
                                        GetRangeColor(s), j + schaefte.a);
                    }
                }
            }
        }
        SETSTRONGWIDTH
        strongPen.setColor(Qt::black);
        canvas->setPen(strongPen);
        for (int i = 0; i < tritte.count(); i++) {
            if (i != 0 && mainfrm->aufknuepfung.pos.strongline_x != 0
                && ((i + tritte.a) % mainfrm->aufknuepfung.pos.strongline_x) == 0)
                canvas->drawLine(XSCALE(x0 + i * aufknuepfung.gw), YSCALE(aufknuepfung.y0 + pw),
                                 XSCALE(x0 + i * aufknuepfung.gw),
                                 YSCALE(aufknuepfung.y0 + aufknuepfung.height - pw));
        }
        for (int j = 0; j < schaefte.count(); j++) {
            const int jj = mainfrm->toptobottom ? schaefte.count() - 1 - j : j;
            if (mainfrm->toptobottom) {
                if (j != 0 && mainfrm->aufknuepfung.pos.strongline_y != 0
                    && ((j + schaefte.a) % mainfrm->aufknuepfung.pos.strongline_y) == 0)
                    canvas->drawLine(XSCALE(aufknuepfung.x0 + pw),
                                     YSCALE(y0 - (jj + 1) * aufknuepfung.gh),
                                     XSCALE(aufknuepfung.x0 + aufknuepfung.width - pw),
                                     YSCALE(y0 - (jj + 1) * aufknuepfung.gh));
            } else {
                if (j != 0 && mainfrm->aufknuepfung.pos.strongline_y != 0
                    && ((j + schaefte.a) % mainfrm->aufknuepfung.pos.strongline_y) == 0)
                    canvas->drawLine(XSCALE(aufknuepfung.x0 + pw),
                                     YSCALE(y0 - jj * aufknuepfung.gh),
                                     XSCALE(aufknuepfung.x0 + aufknuepfung.width - pw),
                                     YSCALE(y0 - jj * aufknuepfung.gh));
            }
        }
        RESETSTRONGWIDTH
    } else {
        /*  Pegplan with stick-figure style (legacy Spezielle Darstellung). */
        canvas->setPen(Qt::black);
        canvas->setBrush(Qt::NoBrush);
        canvas->drawRect(XSCALE(x0), YSCALE(aufknuepfung.y0),
                         XSCALE(x0 + aufknuepfung.width) - XSCALE(x0),
                         YSCALE(y0) - YSCALE(aufknuepfung.y0));
        for (int i = 0; i < tritte.count(); i++) {
            int y = y0 - i * aufknuepfung.gh - aufknuepfung.gh / 2;
            if (y < aufknuepfung.y0)
                y = aufknuepfung.y0;
            if (y > y0)
                y = y0;
            canvas->drawLine(XSCALE(x0 + i * aufknuepfung.gw + aufknuepfung.gw / 2), YSCALE(y0),
                             XSCALE(x0 + i * aufknuepfung.gw + aufknuepfung.gw / 2), YSCALE(y - 1));
        }
        for (int j = 0; j < schaefte.count(); j++) {
            int x = x0 + j * aufknuepfung.gw + aufknuepfung.gw / 2;
            if (x < x0)
                x = x0;
            if (x > x0 + aufknuepfung.width)
                x = x0 + aufknuepfung.width;
            canvas->drawLine(XSCALE(x0), YSCALE(y0 - j * aufknuepfung.gh - aufknuepfung.gh / 2),
                             XSCALE(x), YSCALE(y0 - j * aufknuepfung.gh - aufknuepfung.gh / 2));
        }
    }
}

/*-----------------------------------------------------------------*/
void PrPrinter::PrintTrittfolge()
{
    if (!trittfolge.visible)
        return;
    const bool schlag = mainfrm->ViewSchlagpatrone && mainfrm->ViewSchlagpatrone->isChecked();
    const DARSTELLUNG darst
        = schlag ? mainfrm->schlagpatronendarstellung : mainfrm->trittfolge.darstellung;
    const int x0 = trittfolge.x0;
    const int y0 = trittfolge.y0 + trittfolge.height;

    for (int i = 0; i < tritte.count(); i++) {
        for (int j = 0; j < schuesse.count(); j++) {
            canvas->setPen(Qt::black);
            canvas->setBrush(Qt::NoBrush);
            canvas->drawRect(XSCALE(x0 + i * trittfolge.gw), YSCALE(y0 - (j + 1) * trittfolge.gh),
                             XSCALE(x0 + (i + 1) * trittfolge.gw) - XSCALE(x0 + i * trittfolge.gw),
                             YSCALE(y0 - j * trittfolge.gh) - YSCALE(y0 - (j + 1) * trittfolge.gh));
            if (!(mainfrm->ViewOnlyGewebe && mainfrm->ViewOnlyGewebe->isChecked())) {
                const char s = mainfrm->trittfolge.feld.Get(i + tritte.a, j + schuesse.a);
                const int _x = x0 + i * trittfolge.gw;
                const int _y = y0 - (j + 1) * trittfolge.gh;
                const int _xx = x0 + (i + 1) * trittfolge.gw;
                const int _yy = y0 - j * trittfolge.gh;
                if (s == AUSHEBUNG)
                    PrPaintCell(mainfrm->darst_aushebung, _x, _y, _xx, _yy);
                else if (s == ANBINDUNG)
                    PrPaintCell(mainfrm->darst_anbindung, _x, _y, _xx, _yy);
                else if (s == ABBINDUNG)
                    PrPaintCell(mainfrm->darst_abbindung, _x, _y, _xx, _yy);
                else if (s > 0)
                    PrPaintCell(darst, _x, _y, _xx, _yy, GetRangeColor(s), i);
            }
        }
    }

    SETSTRONGWIDTH
    strongPen.setColor(Qt::black);
    canvas->setPen(strongPen);
    for (int i = 0; i < tritte.count(); i++) {
        if (i != 0 && mainfrm->trittfolge.pos.strongline_x != 0
            && ((i + tritte.a) % mainfrm->trittfolge.pos.strongline_x) == 0)
            canvas->drawLine(XSCALE(x0 + i * trittfolge.gw), YSCALE(trittfolge.y0 + pw),
                             XSCALE(x0 + i * trittfolge.gw),
                             YSCALE(trittfolge.y0 + trittfolge.height - pw));
    }
    for (int j = 0; j < schuesse.count(); j++) {
        if (j != 0 && mainfrm->trittfolge.pos.strongline_y != 0
            && ((j + schuesse.a) % mainfrm->trittfolge.pos.strongline_y) == 0)
            canvas->drawLine(XSCALE(trittfolge.x0 + pw), YSCALE(y0 - j * trittfolge.gh),
                             XSCALE(trittfolge.x0 + trittfolge.width - pw),
                             YSCALE(y0 - j * trittfolge.gh));
    }
    RESETSTRONGWIDTH
}

/*-----------------------------------------------------------------*/
void PrPrinter::PrintSchussfarben()
{
    if (!schussfarben.visible)
        return;
    const int x0 = schussfarben.x0;
    const int y0 = schussfarben.y0 + schussfarben.height;
    for (int j = schuesse.a; j <= schuesse.b; j++) {
        canvas->setPen(Qt::black);
        canvas->setBrush(palCol(mainfrm->schussfarben.feld.Get(j)));
        canvas->drawRect(XSCALE(x0), YSCALE(y0 - (j - schuesse.a + 1) * schussfarben.gh),
                         XSCALE(x0 + schussfarben.width + 1) - XSCALE(x0),
                         YSCALE(y0 - (j - schuesse.a) * schussfarben.gh + 1)
                             - YSCALE(y0 - (j - schuesse.a + 1) * schussfarben.gh));
    }
    (void)qcolFromTColor;
}
