/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope for this slice:
      * full algorithm half -- IsInRapport, CalcRapport (CalcKetts and
        CalcSchussrapport), the two *Equal helpers;
      * STUB rendering half -- ClearRapport, DrawRapport,
        DrawDifferences. Those paint red rectangles and two-pixel
        dashed lines on the main window canvas; the code is in the
        port source with the Canvas-dependent body commented out so it
        is easy to fill in when the rendering slice lands.
*/

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "rapportimpl.h"
#include "mainwindow.h"
#include "datamodule.h"
#include <QCursor>
#include <QPainter>
#include <QPen>
/*-----------------------------------------------------------------*/
RpRapportImpl::RpRapportImpl(TDBWFRM* _frm, TData* _data)
    : frm(_frm)
    , data(_data)
{
}
/*-----------------------------------------------------------------*/
RpRapportImpl::~RpRapportImpl() { }
/*-----------------------------------------------------------------*/
bool RpRapportImpl::IsInRapport(int _i, int _j)
{
    return _i >= frm->rapport.kr.a && _i <= frm->rapport.kr.b && _j >= frm->rapport.sr.a
           && _j <= frm->rapport.sr.b;
}
/*-----------------------------------------------------------------*/
void RpRapportImpl::UpdateRapport()
{
    if (frm->RappViewRapport && frm->RappViewRapport->isChecked())
        ClearRapport();
    RAPPORT oldrapport = frm->rapport;
    CalcRapport();
    if (frm->RappViewRapport && frm->RappViewRapport->isChecked()) {
        DrawRapport();
        DrawDifferences(oldrapport, frm->rapport);
    }
    frm->UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void RpRapportImpl::ClearRapport()
{
    /*  Qt no-op. Legacy's VCL retained-mode canvas needed an
        explicit clBtnShadow overpaint to erase a previously-drawn
        red rapport marker. Under Qt the widget repaints wholesale
        on update(), so the next paintEvent simply omits the marker
        and it's gone. DrawHilfslinien redraw that lived here also
        falls out of the paintEvent flow. */
}
/*-----------------------------------------------------------------*/
void RpRapportImpl::DrawRapport()
{
    QPainter* p = frm->currentPainter;
    if (!p)
        return;

    if (frm->gewebe.gw <= 0 || frm->gewebe.gh <= 0)
        return;
    const int w = frm->gewebe.pos.width / frm->gewebe.gw;
    const int h = frm->gewebe.pos.height / frm->gewebe.gh;

    p->setPen(QPen(QColor(Qt::red)));

    /*  Kettrapport: two vertical red lines at the left edge of the
        first warp thread and the right edge of the last, spanning
        the einzug strip (not the gewebe -- the legacy put the
        marker OUTSIDE the gewebe area so it's visible even when
        the pattern is dark).                                     */
    int i1 = frm->rapport.kr.a;
    int i2 = frm->rapport.kr.b;
    if (i2 >= frm->scroll_x1 && i1 <= frm->scroll_x1 + w) {
        if (i1 < frm->scroll_x1 && i2 >= frm->scroll_x1)
            i1 = frm->scroll_x1;
        if (i2 > frm->scroll_x1 + w - 1 && i1 <= frm->scroll_x1 + w - 1)
            i2 = frm->scroll_x1 + w - 1;
        int x = frm->gewebe.pos.x0 + (i1 - frm->scroll_x1) * frm->gewebe.gw;
        int xx = frm->gewebe.pos.x0 + (i2 - frm->scroll_x1 + 1) * frm->gewebe.gw;
        if (frm->righttoleft) {
            x = frm->gewebe.pos.width - x + 2 * frm->gewebe.pos.x0;
            xx = frm->gewebe.pos.width - xx + 2 * frm->gewebe.pos.x0;
        }
        p->drawLine(x, frm->einzug.pos.y0, x, frm->einzug.pos.y0 + frm->einzug.pos.height);
        p->drawLine(xx, frm->einzug.pos.y0, xx, frm->einzug.pos.y0 + frm->einzug.pos.height);
    }

    /*  Schussrapport: two horizontal red lines spanning the
        trittfolge strip. */
    int j1 = frm->rapport.sr.a;
    int j2 = frm->rapport.sr.b;
    if (j2 >= frm->scroll_y2 && j1 <= frm->scroll_y2 + h) {
        if (j1 < frm->scroll_y2 && j2 >= frm->scroll_y2)
            j1 = frm->scroll_y2;
        if (j2 > frm->scroll_y2 + h - 1 && j1 <= frm->scroll_y2 + h - 1)
            j2 = frm->scroll_y2 + h - 1;
        const int y
            = frm->gewebe.pos.y0 + frm->gewebe.pos.height - (j1 - frm->scroll_y2) * frm->gewebe.gh;
        const int yy = frm->gewebe.pos.y0 + frm->gewebe.pos.height
                       - (j2 - frm->scroll_y2 + 1) * frm->gewebe.gh;
        p->drawLine(frm->trittfolge.pos.x0, y, frm->trittfolge.pos.x0 + frm->trittfolge.pos.width,
                    y);
        p->drawLine(frm->trittfolge.pos.x0, yy, frm->trittfolge.pos.x0 + frm->trittfolge.pos.width,
                    yy);
    }
}
/*-----------------------------------------------------------------*/
void RpRapportImpl::DrawDifferences(const RAPPORT& _old, const RAPPORT& _new)
{
    /*  STUB: when the rapport expands, the newly-revealed gewebe
        cells need redrawing (DrawGewebe / DrawGewebeRahmen). Legacy
        code iterates the expanded rectangle; body restored when
        rendering lands.                                           */
    (void)_old;
    (void)_new;
}
/*-----------------------------------------------------------------*/
void RpRapportImpl::CalcKettrapport()
{
    // Rapport loeschen
    frm->rapport.kr = SZ(0, -1);

    int i;
    int i1 = frm->kette.a;
    int i2 = frm->kette.b;
    if (i1 == -1 || i2 == -1)
        return;

    // Kettrapport bestimmen
    frm->rapport.kr.a = i1;
    frm->rapport.kr.b = i1;
g_again:
    // potentiellen Rapportanfang finden
    for (i = frm->rapport.kr.b + 1; i <= i2; i++) {
        if (EinzugEqual(frm->rapport.kr.a, i))
            break;
    }
    frm->rapport.kr.b = i - 1;
    // Pruefen ob es tatsaechlich Rapport ist
    for (i = frm->rapport.kr.b + 1; i <= i2; i++) {
        if (!EinzugEqual(frm->rapport.kr.a
                             + (i - frm->rapport.kr.b - 1)
                                   % (frm->rapport.kr.b - frm->rapport.kr.a + 1),
                         i)) {
            frm->rapport.kr.b++;
            goto g_again;
        }
    }
}
/*-----------------------------------------------------------------*/
void RpRapportImpl::CalcSchussrapport()
{
    // Rapport loeschen
    frm->rapport.sr = SZ(0, -1);

    int j;
    int j1 = frm->schuesse.a;
    int j2 = frm->schuesse.b;
    if (j1 == -1 || j2 == -1)
        return;

    // Schussrapport bestimmen
    frm->rapport.sr.a = j1;
    frm->rapport.sr.b = j1;
g_again:
    // potentiellen Rapportanfang finden
    for (j = frm->rapport.sr.b + 1; j <= j2; j++) {
        if (TrittfolgeEqual(frm->rapport.sr.a, j))
            break;
    }
    frm->rapport.sr.b = j - 1;
    // Pruefen ob es tatsaechlich Rapport ist
    for (j = frm->rapport.sr.b + 1; j <= j2; j++) {
        if (!TrittfolgeEqual(frm->rapport.sr.a
                                 + (j - frm->rapport.sr.b - 1)
                                       % (frm->rapport.sr.b - frm->rapport.sr.a + 1),
                             j)) {
            frm->rapport.sr.b++;
            goto g_again;
        }
    }
}
/*-----------------------------------------------------------------*/
void RpRapportImpl::CalcRapport()
{
    if (frm->rapport.overridden) {
        return;
    }

    QCursor old = frm->cursor();
    frm->setCursor(Qt::WaitCursor);

    CalcKettrapport();
    CalcSchussrapport();

    frm->setCursor(old);
}
/*-----------------------------------------------------------------*/
bool RpRapportImpl::TrittfolgeEqual(int _j1, int _j2)
{
    for (int i = 0; i < data->MAXX2; i++) {
        char s1 = frm->trittfolge.feld.Get(i, _j1);
        char s2 = frm->trittfolge.feld.Get(i, _j2);
        if (s1 <= 0 && s2 > 0)
            return false;
        if (s1 > 0 && s2 <= 0)
            return false;
        if (s1 > 0 && s2 > 0 && s1 != s2)
            return false;
    }
    if (frm->kette.a != -1 && frm->kette.b != -1)
        for (int i = frm->kette.a; i <= frm->kette.b; i++) {
            char s1 = frm->gewebe.feld.Get(i, _j1);
            char s2 = frm->gewebe.feld.Get(i, _j2);
            if (s1 <= 0 && s2 > 0)
                return false;
            if (s1 > 0 && s2 <= 0)
                return false;
            if (s1 > 0 && s2 > 0 && s1 != s2)
                return false;
        }
    return true;
}
/*-----------------------------------------------------------------*/
bool RpRapportImpl::EinzugEqual(int _i1, int _i2)
{
    if (frm->einzug.feld.Get(_i1) != frm->einzug.feld.Get(_i2))
        return false;
    if (frm->schuesse.a != -1 && frm->schuesse.b != -1)
        for (int j = frm->schuesse.a; j <= frm->schuesse.b; j++) {
            char s1 = frm->gewebe.feld.Get(_i1, j);
            char s2 = frm->gewebe.feld.Get(_i2, j);
            if (s1 <= 0 && s2 > 0)
                return false;
            if (s1 > 0 && s2 <= 0)
                return false;
            if (s1 > 0 && s2 > 0 && s1 != s2)
                return false;
        }
    return true;
}
/*-----------------------------------------------------------------*/
RpRapport* RpRapport::CreateInstance(TDBWFRM* _frm, TData* _data)
{
    RpRapport* p = NULL;
    try {
        p = new RpRapportImpl(_frm, _data);
    } catch (...) {
    }
    return p;
}
/*-----------------------------------------------------------------*/
void RpRapport::ReleaseInstance(RpRapport* _rapport)
{
    delete _rapport;
}
/*-----------------------------------------------------------------*/
