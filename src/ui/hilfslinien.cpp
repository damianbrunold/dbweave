/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope:
      * Hilfslinien container -- full algorithm port, unchanged logic.
      * TDBWFRM::DrawHilfslinien / _DrawHilfslinie -- Qt paintEvent
        port. The legacy clBtnFace "erase" branch is collapsed: Qt
        repaints wholesale on update(), so "delete" mode is never
        needed -- DrawHilfslinie always draws, the "erase" simply
        omits the call on the next paintEvent.
      * DrawHlineBars is NOT ported in this slice. Legacy drew the
        four HlineBar rectangles outside the field grids where the
        user could grab-drag a guide line; that UI affordance depends
        on mouse-event routing which lands in a later slice. The bar
        positions (hlinehorz1/2, hlinevert1/2) default to zero-sized
        rectangles so the "tick into the bar" line segment in
        _DrawHilfslinie simply draws a zero-length line (no-op).    */

#include "hilfslinien.h"
#include "mainwindow.h"
#include "assert_compat.h"

#include <QAction>
#include <QColor>
#include <QPainter>
#include <QPen>

#define INITIALMAX 20

Hilfslinien::Hilfslinien()
{
    list = nullptr;
    max = 0;
    last = -1;
    Reallocate(INITIALMAX);
}

Hilfslinien::~Hilfslinien()
{
    delete[] list;
}

void Hilfslinien::Reallocate(int _newmax)
{
    if (_newmax <= max)
        return;
    try {
        Hilfslinie* newlist = new Hilfslinie[_newmax];
        if (list && last >= 0) {
            for (int i = 0; i <= last && i < max; i++)
                newlist[i] = list[i];
        }
        delete[] list;
        list = newlist;
        max = _newmax;
    } catch (...) {
        list = nullptr;
    }
}

void Hilfslinien::SetData(Hilfslinie* _list, int _count)
{
    delete[] list;
    list = _list;
    max = _count;
    last = _count - 1;
}

bool Hilfslinien::Add(HLTYP _typ, HLFELD _feld, int _pos)
{
    dbw3_assert(list);
    if (last >= max - 1)
        Reallocate(max + 10);
    if (last >= max - 1)
        return false;
    last++;
    list[last].typ = _typ;
    list[last].feld = _feld;
    list[last].pos = _pos;
    return true;
}

void Hilfslinien::Update(int _index, HLFELD _feld, int _pos)
{
    dbw3_assert(list);
    dbw3_assert(_index <= last && _index < max);
    if (_index > last)
        return;
    list[_index].feld = _feld;
    list[_index].pos = _pos;
}

void Hilfslinien::Delete(int _index)
{
    dbw3_assert(list);
    dbw3_assert(_index <= last && _index < max);
    if (_index > last)
        return;
    for (int i = _index + 1; i <= last; i++)
        list[i - 1] = list[i];
    last--;
}

void Hilfslinien::Delete(Hilfslinie* _hline)
{
    dbw3_assert(_hline);
    if (!_hline)
        return;
    for (int i = 0; i <= last && i < max; i++) {
        if (list[i].typ == _hline->typ && list[i].feld == _hline->feld
            && list[i].pos == _hline->pos) {
            Delete(i);
            break;
        }
    }
}

void Hilfslinien::DeleteAll()
{
    last = -1;
}

int Hilfslinien::GetCount() const
{
    return last + 1;
}

Hilfslinie* Hilfslinien::GetLine(int _index)
{
    dbw3_assert(list);
    if (_index > last)
        return nullptr;
    return &list[_index];
}

Hilfslinie* Hilfslinien::GetLine(HLTYP _typ, HLFELD _feld, int _pos)
{
    if (!list)
        return nullptr;
    for (int i = 0; i <= last && i < max; i++) {
        if (list[i].typ == _typ && list[i].feld == _feld && list[i].pos == _pos)
            return &list[i];
    }
    return nullptr;
}

/*-----------------------------------------------------------------*/
static inline bool checked(QAction* _a)
{
    return _a && _a->isChecked();
}

static void _DrawHilfslinie(TDBWFRM* frm, Hilfslinie* _hline)
{
    QPainter* p = frm->currentPainter;
    if (!p)
        return;

    const QColor HLINECOLOR(Qt::blue);
    p->setPen(QPen(HLINECOLOR));

    const bool viewEinzug = checked(frm->ViewEinzug);
    const bool viewTrittfolge = checked(frm->ViewTrittfolge);
    const bool viewSchlagpat = checked(frm->ViewSchlagpatrone);
    const bool gewebeNormal = checked(frm->GewebeNormal);
    const bool gewebeNone = checked(frm->GewebeNone);

    switch (_hline->typ) {
    case HL_HORZ: {
        if (_hline->feld == HL_TOP) {
            if (!viewEinzug)
                break;
            if (frm->einzug.gh <= 0)
                break;
            if (_hline->pos <= frm->scroll_y1 || _hline->pos >= frm->scroll_y1 + frm->hvisible)
                break;
            int y;
            if (frm->toptobottom) {
                y = frm->einzug.pos.y0 + (_hline->pos - frm->scroll_y1) * frm->einzug.gh;
            } else {
                y = frm->einzug.pos.y0 + frm->einzug.pos.height
                    - (_hline->pos - frm->scroll_y1) * frm->einzug.gh;
            }
            p->drawLine(frm->einzug.pos.x0 + 1, y, frm->einzug.pos.x0 + frm->einzug.pos.width, y);
            if (viewTrittfolge && !viewSchlagpat) {
                p->drawLine(frm->aufknuepfung.pos.x0 + 1, y,
                            frm->aufknuepfung.pos.x0 + frm->aufknuepfung.pos.width, y);
            }
            p->drawLine(frm->hlinevert1.x0 + 5, y, frm->hlinevert1.x0 + frm->hlinevert1.width, y);
        } else {
            if (frm->gewebe.gh <= 0)
                break;
            if (_hline->pos <= frm->scroll_y2
                || _hline->pos >= frm->scroll_y2 + frm->gewebe.pos.height / frm->gewebe.gh)
                break;
            int y = frm->gewebe.pos.y0 + frm->gewebe.pos.height
                    - (_hline->pos - frm->scroll_y2) * frm->gewebe.gh;
            if (gewebeNormal || gewebeNone) {
                p->drawLine(frm->gewebe.pos.x0 + 1, y, frm->gewebe.pos.x0 + frm->gewebe.pos.width,
                            y);
            }
            if (viewTrittfolge) {
                p->drawLine(frm->trittfolge.pos.x0 + 1, y,
                            frm->trittfolge.pos.x0 + frm->trittfolge.pos.width, y);
            }
            p->drawLine(frm->hlinevert2.x0 + 5, y, frm->hlinevert2.x0 + frm->hlinevert2.width, y);
        }
        break;
    }
    case HL_VERT: {
        if (_hline->feld == HL_LEFT) {
            if (frm->gewebe.gw <= 0)
                break;
            if (_hline->pos <= frm->scroll_x1
                || _hline->pos >= frm->scroll_x1 + frm->gewebe.pos.width / frm->gewebe.gw)
                break;
            int x;
            if (frm->righttoleft) {
                x = frm->gewebe.pos.x0 + frm->gewebe.pos.width
                    - (_hline->pos - frm->scroll_x1) * frm->gewebe.gw;
            } else {
                x = frm->gewebe.pos.x0 + (_hline->pos - frm->scroll_x1) * frm->gewebe.gw;
            }
            if (gewebeNormal || gewebeNone) {
                p->drawLine(x, frm->gewebe.pos.y0 + 1, x,
                            frm->gewebe.pos.y0 + frm->gewebe.pos.height);
            }
            if (viewEinzug) {
                p->drawLine(x, frm->einzug.pos.y0 + 1, x,
                            frm->einzug.pos.y0 + frm->einzug.pos.height);
            }
            p->drawLine(x, frm->hlinehorz1.y0 + 1, x,
                        frm->hlinehorz1.y0 + frm->hlinehorz1.height - 4);
        } else {
            if (!viewTrittfolge)
                break;
            if (frm->trittfolge.gw <= 0)
                break;
            if (_hline->pos <= frm->scroll_x2 || _hline->pos >= frm->scroll_x2 + frm->wvisible)
                break;
            int x = frm->trittfolge.pos.x0 + (_hline->pos - frm->scroll_x2) * frm->trittfolge.gw;
            p->drawLine(x, frm->trittfolge.pos.y0 + 1, x,
                        frm->trittfolge.pos.y0 + frm->trittfolge.pos.height);
            if (viewEinzug && !viewSchlagpat) {
                p->drawLine(x, frm->aufknuepfung.pos.y0 + 1, x,
                            frm->aufknuepfung.pos.y0 + frm->aufknuepfung.pos.height);
            }
            p->drawLine(x, frm->hlinehorz2.y0 + 1, x,
                        frm->hlinehorz2.y0 + frm->hlinehorz2.height - 4);
        }
        break;
    }
    case HL_NONE:
        break;
    }
}

void TDBWFRM::DrawHilfslinien()
{
    const int count = hlines.GetCount();
    for (int i = 0; i < count; i++) {
        Hilfslinie* line = hlines.GetLine(i);
        if (line)
            _DrawHilfslinie(this, line);
    }
}
