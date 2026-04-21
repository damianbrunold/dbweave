/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port of legacy steuerung_draw.cpp. Drops the VCL XOR-style
    draw-then-clear pattern in favour of a single-shot paintEvent
    (paintAll) that redraws the whole canvas on every refresh --
    Qt's double buffering means this is cheap and eliminates the
    stale-pixel hazard the legacy Clear* routines had to work
    around.                                                       */

#include "steuerung.h"

#include "colors_compat.h"
#include "datamodule.h"
#include "draw_cell.h"
#include "legacy_colors.h"
#include "mainwindow.h"
#include "palette.h"
#include "rangecolors.h"
#include "steuerungcanvas.h"

#include <QAction>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QRect>

#include <algorithm>

/*-----------------------------------------------------------------*/
static QColor rangeColor(char _s)
{
    return qColorFromTColor(GetRangeColor(_s));
}

static QColor paletteColor(int _idx)
{
    if (!Data || !Data->palette)
        return QColor(Qt::white);
    return qColorFromTColor(Data->palette->GetColor(_idx));
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::paintAll()
{
    if (!currentPainter)
        return;
    /*  Legacy FormPaint order: CalcSizes + DrawGrid + DrawData +
        DrawKlammern + DrawSelection + UpdateStatusbar.            */
    DrawGrid();
    DrawData();
    DrawKlammern();
    DrawSelection();
    UpdateStatusbar();
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::DrawGrid()
{
    QPainter& p = *currentPainter;
    p.save();
    p.setPen(legacyBtnShadow());
    /*  Gewebe columns (left of the divider). */
    for (int i = 0; i <= x1; i++) {
        const int x = left + i * gridsize;
        p.drawLine(x, bottom - 1, x, bottom - 1 - maxj * gridsize);
    }
    for (int j = 0; j <= maxj; j++) {
        const int y = bottom - j * gridsize;
        p.drawLine(left, y, left + x1 * gridsize + 1, y);
    }

    /*  Schlagpatrone (right of the divider). */
    for (int i = x1 + 1; i <= x1 + 1 + trittCols; i++) {
        const int x = left + i * gridsize;
        p.drawLine(x, bottom, x, bottom - maxj * gridsize);
    }
    for (int j = 0; j <= maxj; j++) {
        const int y = bottom - j * gridsize;
        p.drawLine(left + (x1 + 1) * gridsize, y,
                   left + (x1 + 1 + trittCols) * gridsize + 1, y);
    }

    /*  Klammer columns -- nine vertical strips, each 11 px wide. */
    for (int i = 0; i <= MAXKLAMMERN; i++) {
        const int x = left + maxi * gridsize + dx + i * 11;
        p.drawLine(x, bottom, x, bottom - maxj * gridsize);
    }
    p.drawLine(left + dx + maxi * gridsize, bottom,
               left + dx + maxi * gridsize + MAXKLAMMERN * 11 + 1, bottom);
    p.drawLine(left + dx + maxi * gridsize, bottom - maxj * gridsize,
               left + dx + maxi * gridsize + MAXKLAMMERN * 11 + 1, bottom - maxj * gridsize);

    /*  Rapport overlay (black lines across the gewebe area). */
    p.setPen(Qt::black);
    if (rapportx > 0) {
        int i = rapportx;
        while (i < x1) {
            p.drawLine(left + i * gridsize, top + 1, left + i * gridsize, bottom);
            i += rapportx;
        }
    }
    if (rapporty > 0) {
        int j = rapporty - (scrolly % rapporty);
        while (j < maxj) {
            p.drawLine(left + 1, bottom - j * gridsize, left + x1 * gridsize,
                       bottom - j * gridsize);
            j += rapporty;
        }
    }
    p.restore();
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::DrawData()
{
    if (!einzug || !aufknuepfung || !trittfolge || !gewebe || !kettfarben || !schussfarben
        || !data)
        return;

    QPainter& p = *currentPainter;
    const int mj = std::min(maxj, gewebe->feld.SizeY() - scrolly);

    /*  --- Gewebe ----------------------------------------------- */
    for (int i = 0; i < x1; i++)
        for (int j = scrolly; j < scrolly + mj; j++)
            DrawGewebe(i, j);

    /*  --- Schlagpatrone ---------------------------------------- */
    const QColor bkground = legacyBtnFace();
    if (schlagpatrone) {
        for (int i = 0; i < trittCols; i++) {
            for (int j = scrolly; j < scrolly + mj; j++) {
                const char s = trittfolge->feld.Get(i, j);
                const int px1 = left + x1 * gridsize + dx + i * gridsize;
                const int py1 = bottom - (j - scrolly + 1) * gridsize;
                const int px2 = left + x1 * gridsize + dx + (i + 1) * gridsize;
                const int py2 = bottom - (j - scrolly) * gridsize;
                if (s == AUSHEBUNG)
                    PaintCell(p, DBWFRM->darst_aushebung, px1, py1, px2, py2, Qt::black, true, -1,
                              bkground);
                else if (s == ANBINDUNG)
                    PaintCell(p, DBWFRM->darst_anbindung, px1, py1, px2, py2, Qt::black, true, -1,
                              bkground);
                else if (s == ABBINDUNG)
                    PaintCell(p, DBWFRM->darst_abbindung, px1, py1, px2, py2, Qt::black, true, -1,
                              bkground);
                else if (s > 0)
                    PaintCell(p, schlagpatronendarstellung, px1, py1, px2, py2, Qt::black, true,
                              -1, bkground);
            }
        }
    } else {
        for (int j = scrolly; j < scrolly + mj; j++) {
            for (int i = 0; i < trittfolge->feld.SizeX(); i++) {
                if (trittfolge->feld.Get(i, j) > 0) {
                    const int lim = std::min(trittCols, aufknuepfung->feld.SizeY());
                    for (int k = 0; k < lim; k++) {
                        const char s = aufknuepfung->feld.Get(i, k);
                        const int px1 = left + x1 * gridsize + dx + k * gridsize;
                        const int py1 = bottom - (j - scrolly + 1) * gridsize;
                        const int px2 = left + x1 * gridsize + dx + (k + 1) * gridsize;
                        const int py2 = bottom - (j - scrolly) * gridsize;
                        if (s == AUSHEBUNG)
                            PaintCell(p, DBWFRM->darst_aushebung, px1, py1, px2, py2, Qt::black,
                                      true, -1, bkground);
                        else if (s == ANBINDUNG)
                            PaintCell(p, DBWFRM->darst_anbindung, px1, py1, px2, py2, Qt::black,
                                      true, -1, bkground);
                        else if (s == ABBINDUNG)
                            PaintCell(p, DBWFRM->darst_abbindung, px1, py1, px2, py2, Qt::black,
                                      true, -1, bkground);
                        else if (s > 0)
                            PaintCell(p, schlagpatronendarstellung, px1, py1, px2, py2, Qt::black,
                                      true, -1, bkground);
                    }
                }
            }
        }
    }
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::DrawGewebe(int _i, int _j)
{
    if (_i >= x1)
        return;
    if (_j < scrolly)
        return;
    if (_j >= scrolly + maxj)
        return;

    const int x = left + _i * gridsize;
    const int xx = x + gridsize;
    const int y = bottom - (_j - scrolly + 1) * gridsize;
    const int yy = y + gridsize;

    if (einzug->feld.Get(_i) == 0 || (DBWFRM && DBWFRM->IsEmptyTrittfolge(_j)))
        return;

    if (actViewFarbeffekt && actViewFarbeffekt->isChecked())
        DrawGewebeFarbeffekt(_i, _j, x, y, xx, yy);
    else if (actViewGewebesimulation && actViewGewebesimulation->isChecked())
        DrawGewebeSimulation(_i, _j, x, y, xx, yy);
    else
        DrawGewebeNormal(_i, _j, x, y, xx, yy);
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::DrawGewebeNormal(int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    QPainter& p = *currentPainter;
    const QColor bkground = legacyBtnFace();
    const char s = gewebe->feld.Get(_i, _j);
    if (s == AUSHEBUNG) {
        p.fillRect(QRect(_x + 1, _y + 1, _xx - _x - 1, _yy - _y - 1), bkground);
        PaintCell(p, DBWFRM->darst_aushebung, _x, _y, _xx, _yy, Qt::black, true, -1, bkground);
    } else if (s == ANBINDUNG) {
        p.fillRect(QRect(_x + 1, _y + 1, _xx - _x - 1, _yy - _y - 1), bkground);
        PaintCell(p, DBWFRM->darst_anbindung, _x, _y, _xx, _yy, Qt::black, true, -1, bkground);
    } else if (s == ABBINDUNG) {
        p.fillRect(QRect(_x + 1, _y + 1, _xx - _x - 1, _yy - _y - 1), bkground);
        PaintCell(p, DBWFRM->darst_abbindung, _x, _y, _xx, _yy, Qt::black, true, -1, bkground);
    } else if (s > 0) {
        p.fillRect(QRect(_x + 1, _y + 1, _xx - _x - 1, _yy - _y - 1), rangeColor(s));
    }
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::DrawGewebeFarbeffekt(int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    QPainter& p = *currentPainter;
    const char s = gewebe->feld.Get(_i, _j);
    bool hebung = s > 0 && s != ABBINDUNG;
    if (DBWFRM && DBWFRM->sinkingshed)
        hebung = !hebung;
    const QColor c = hebung ? paletteColor(kettfarben->feld.Get(_i))
                            : paletteColor(schussfarben->feld.Get(_j));
    p.fillRect(QRect(_x, _y, _xx - _x, _yy - _y), c);
    if (fewithraster) {
        p.setPen(legacyBtnShadow());
        p.drawLine(_x, _y, _x, _yy);
        p.drawLine(_x, _yy, _xx, _yy);
        p.drawLine(_xx, _yy, _xx, _y);
        p.drawLine(_xx, _y, _x, _y);
    }
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::DrawGewebeSimulation(int _i, int _j, int _x, int _y, int _xx, int _yy)
{
    QPainter& p = *currentPainter;
    int dw = gridsize / 5;
    int dh = gridsize / 5;
    if (dw == 0 && gridsize > 2)
        dw = 1;
    if (dh == 0 && gridsize > 2)
        dh = 1;

    const QColor bkground = legacyBtnFace();
    p.fillRect(QRect(_x, _y, _xx - _x, _yy - _y), bkground);

    const char s = gewebe->feld.Get(_i, _j);
    bool hebung = s > 0 && s != ABBINDUNG;
    if (DBWFRM && DBWFRM->sinkingshed)
        hebung = !hebung;
    const QColor kc = paletteColor(kettfarben->feld.Get(_i));
    const QColor sc = paletteColor(schussfarben->feld.Get(_j));
    if (hebung) {
        p.fillRect(QRect(_x + dw, _y, _xx - _x - 2 * dw, _yy - _y), kc);
        p.fillRect(QRect(_x, _y + dh, dw, _yy - _y - 2 * dh), sc);
        p.fillRect(QRect(_xx - dw, _y + dh, dw, _yy - _y - 2 * dh), sc);
        p.setPen(Qt::black);
        p.drawLine(_xx - dw, _y + dh, _xx - dw, _yy - dh);
    } else {
        p.fillRect(QRect(_x, _y + dh, _xx - _x, _yy - _y - 2 * dh), sc);
        p.fillRect(QRect(_x + dw, _y, _xx - _x - 2 * dw, dh), kc);
        p.fillRect(QRect(_x + dw, _yy - dh, _xx - _x - 2 * dw, dh), kc);
        p.setPen(Qt::black);
        p.drawLine(_x + dw, _yy - dh, _xx - dw, _yy - dh);
    }
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::DrawKlammern()
{
    for (int i = 0; i < MAXKLAMMERN; i++)
        DrawKlammer(i);
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::DrawKlammer(int _i)
{
    QPainter& p = *currentPainter;
    const int x = left + (x1 + trittCols + 1) * gridsize + dx + _i * 11;
    QRect rc(x + 1, bottom - maxj * gridsize + 1, 10, maxj * gridsize - 1);
    p.fillRect(rc, legacyBtnFace());
    p.setPen((!schussselected && current_klammer == _i) ? Qt::red : Qt::black);
    _DrawKlammer(_i);
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::_DrawKlammer(int _i)
{
    QPainter& p = *currentPainter;
    bool clip_top = false;
    bool clip_bottom = false;

    int j1 = klammern[_i].first - scrolly;
    int j2 = klammern[_i].last - scrolly;

    if (j1 < 0) {
        clip_bottom = true;
        j1 = 0;
    } else if (j1 > maxj) {
        clip_bottom = true;
        j1 = maxj;
    }
    if (j2 < 0) {
        clip_top = true;
        j2 = 0;
    } else if (j2 > maxj) {
        clip_top = true;
        j2 = maxj;
    }

    const int xcol = left + (x1 + trittCols + 1) * gridsize + dx + _i * 11;

    /*  Vertical stem. */
    if (j1 < maxj && j2 > 0)
        p.drawLine(xcol + 8, bottom - j1 * gridsize - 1, xcol + 8, bottom - (j2 + 1) * gridsize);
    /*  Lower horizontal cap. */
    if (!clip_bottom)
        p.drawLine(xcol + 8, bottom - j1 * gridsize - 1, xcol + 2, bottom - j1 * gridsize - 1);
    /*  Upper horizontal cap. */
    if (!clip_top)
        p.drawLine(xcol + 8, bottom - (j2 + 1) * gridsize + 1, xcol + 2,
                   bottom - (j2 + 1) * gridsize + 1);

    /*  Repetitions number centred in the stem column. */
    if (j1 < maxj && j2 > 0) {
        const QString rep = QString::number(klammern[_i].repetitions);
        const QRect rc(xcol + 1, bottom - (j2 + 1) * gridsize, 9, (j2 - j1 + 1) * gridsize);
        p.drawText(rc, Qt::AlignLeft | Qt::AlignVCenter, rep);
    }
}

/*-----------------------------------------------------------------*/
/*  Selection rendering: the red outline around the currently
    selected schuss row (schussselected==true) or the red frame
    around the currently selected klammer (schussselected==false).
    Legacy alternated DrawSelection / ClearSelection via colour
    flips; the Qt port always redraws everything so one method is
    enough.                                                        */
void TSTRGFRM::DrawSelection()
{
    QPainter& p = *currentPainter;
    p.save();
    if (schussselected) {
        p.setPen(IsValidWeavePosition() ? QColor(Qt::blue) : QColor(Qt::red));
        _DrawPositionSelected();
        if (weave_klammer >= 0) {
            p.setPen(Qt::black);
            _DrawKlammer(weave_klammer);
        }
    } else {
        p.setPen(Qt::red);
        _DrawKlammerSelected();
        p.setPen(legacyBtnShadow());
        _DrawPositionSelected();
    }
    DrawLastPos();
    p.restore();
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::_DrawPositionSelected()
{
    QPainter& p = *currentPainter;
    /*  Legacy Rectangle is outline-only when brush style is
        bsClear; use drawRect with NoBrush.                       */
    const QPen save = p.pen();
    p.setBrush(Qt::NoBrush);
    QRect rc(left + (x1 + 1) * gridsize,
             bottom - (weave_position - scrolly + 1) * gridsize,
             trittCols * gridsize + 1,
             gridsize + 1);
    p.drawRect(rc);
    p.setPen(save);
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::_DrawKlammerSelected()
{
    _DrawKlammer(current_klammer);
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::DrawLastPos()
{
    QPainter& p = *currentPainter;
    p.save();
    p.setPen(Qt::blue);
    _DrawLastPos();
    p.restore();
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::_DrawLastPos()
{
    if (last_position < scrolly || last_position >= scrolly + maxj)
        return;
    QPainter& p = *currentPainter;
    /*  Triangle in the gap between gewebe and schlagpatrone,
        pointing right at the last-position schuss row.            */
    const int xx1 = left + (x1 + 1) * gridsize - 2;
    const int xx2 = left + (x1 + 1) * gridsize - dx + 2;
    const int yy1 = bottom - (last_position - scrolly + 1) * gridsize;
    const int yy2 = bottom - (last_position - scrolly) * gridsize + 1;
    const QPointF tip(xx1, (yy1 + yy2) / 2.0);
    const QPointF topPt(xx2, yy1);
    const QPointF botPt(xx2, yy2);
    p.drawLine(tip, topPt);
    p.drawLine(topPt, botPt);
    p.drawLine(botPt, tip);
}
