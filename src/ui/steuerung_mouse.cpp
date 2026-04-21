/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port of legacy steuerung_mouse.cpp. Mouse coordinates come from
    QMouseEvent->position() in canvas-local pixels; the legacy
    FormMouseDown / Move / Up handlers worked in form-local pixels
    with left==0, so the same arithmetic applies.                 */

#include "steuerung.h"

#include "datamodule.h"
#include "steuerungcanvas.h"

#include <QMouseEvent>
#include <QPoint>

/*-----------------------------------------------------------------*/
void TSTRGFRM::FormMouseDown(QMouseEvent* _e)
{
    if (!_e || gridsize <= 0 || !data)
        return;
    const QPoint pos = _e->pos();
    const int X = pos.x();
    const int Y = pos.y();

    bool updatestatus = false;
    bool repaint = false;

    /*  --- Click in the schlagpatrone strip -------------------- */
    if (X > left + (x1 + 1) * gridsize && X < left + maxi * gridsize && Y < bottom
        && Y > bottom - maxj * gridsize) {
        const int schuss = scrolly + (bottom - Y) / gridsize;
        weave_position = schuss;
        if (weave_position >= scrolly + maxj)
            weave_position = scrolly + maxj - 1;
        ValidateWeavePosition();
        schussselected = true;
        updatestatus = true;
        repaint = true;
    }

    /*  --- Click in the klammer strip -------------------------- */
    if (X > left + maxi * gridsize + dx
        && X < left + maxi * gridsize + dx + MAXKLAMMERN * 11 && Y < bottom
        && Y > bottom - maxj * gridsize) {
        const int klammer = (X - left - maxi * gridsize - dx) / 11;
        if (klammer >= 0 && klammer < MAXKLAMMERN) {
            int start, stop;
            if (klammern[klammer].first < scrolly)
                start = 0;
            else
                start = klammern[klammer].first - scrolly;
            if (klammern[klammer].last > scrolly + maxj)
                stop = maxj;
            else
                stop = klammern[klammer].last - scrolly;
            const int j = (bottom - Y) / gridsize;

            schussselected = false;
            current_klammer = klammer;
            updatestatus = true;
            repaint = true;

            if (j >= start && j <= stop) {
                dragging = true;
                drag_klammer = klammer;
                drag_j = j;
                if (j == start)
                    drag_style = DRAG_BOTTOM;
                else if (j == stop)
                    drag_style = DRAG_TOP;
                else
                    drag_style = DRAG_MIDDLE;
            }
        }
    }

    if (updatestatus)
        UpdateStatusbar();
    if (repaint && canvas)
        canvas->update();
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::FormMouseMove(QMouseEvent* _e)
{
    if (!dragging || gridsize <= 0 || !data || !_e)
        return;
    const int j = (bottom - _e->pos().y()) / gridsize;
    if (j == drag_j)
        return;

    Klammer& k = klammern[drag_klammer];
    switch (drag_style) {
    case DRAG_TOP:
        k.last = scrolly + j;
        if (k.last < k.first + 1)
            k.last = k.first + 1;
        break;
    case DRAG_BOTTOM:
        k.first = scrolly + j;
        if (k.first > k.last - 1)
            k.first = k.last - 1;
        break;
    case DRAG_MIDDLE: {
        const int diff = j - drag_j;
        k.first += diff;
        k.last += diff;
        if (k.last >= data->MAXY2)
            k.last = data->MAXY2 - 1;
        if (k.first >= data->MAXY2)
            k.first = data->MAXY2 - 1;
        if (k.last < 0)
            k.last = 0;
        if (k.first < 0)
            k.first = 0;
        break;
    }
    }
    drag_j = j;
    modified = true;
    /*  Repetitions == 0 means "empty" in the file format; a
        just-resized klammer should be made non-empty if the user
        interacted with it.                                       */
    if (k.repetitions == 0)
        k.repetitions = 1;
    UpdateStatusbar();
    refreshGotoActions();
    if (canvas)
        canvas->update();
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::FormMouseUp(QMouseEvent* /*_e*/)
{
    dragging = false;
}
