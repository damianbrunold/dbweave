/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "palettepanel.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "palette.h"
#include "colors_compat.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>

#include <algorithm>

PalettePanel::PalettePanel(TDBWFRM* _frm, QWidget* _parent)
    : QWidget(_parent)
    , frm(_frm)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(true);
}

QSize PalettePanel::sizeHint() const
{
    return QSize(16 * 14, 16 * 14);
}
QSize PalettePanel::minimumSizeHint() const
{
    return QSize(16 * 8, 16 * 8);
}

int PalettePanel::drawSize() const
{
    const int w = width() / GRIDSIZE;
    const int h = height() / GRIDSIZE;
    return std::max(1, std::min(w, h));
}

void PalettePanel::paintEvent(QPaintEvent* /*_e*/)
{
    if (!Data || !Data->palette)
        return;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    const int d = drawSize();

    p.fillRect(rect(), palette().color(QPalette::Button));
    p.setPen(QPen(Qt::black));

    for (int i = 0; i < GRIDSIZE; i++) {
        for (int j = 0; j < GRIDSIZE; j++) {
            const int idx = i * GRIDSIZE + j;
            if (idx >= MAX_PAL_ENTRY)
                return;
            p.setBrush(qColorFromTColor(Data->palette->GetColor(idx)));
            p.drawRect(i * d, j * d, d, d);
        }
    }

    /*  White cursor outline around the currently-active palette
        slot. */
    const int sel = Data->color;
    if (sel < MAX_PAL_ENTRY) {
        const int x = (sel / GRIDSIZE) * d;
        const int y = (sel % GRIDSIZE) * d;
        p.setPen(QPen(Qt::white));
        p.setBrush(Qt::NoBrush);
        p.drawRect(x, y, d, d);
    }
}

void PalettePanel::mousePressEvent(QMouseEvent* _e)
{
    if (_e->button() != Qt::LeftButton) {
        _e->ignore();
        return;
    }
    const int d = drawSize();
    if (d <= 0)
        return;
    const int i = _e->pos().x() / d;
    const int j = _e->pos().y() / d;
    if (i < 0 || j < 0 || i >= GRIDSIZE || j >= GRIDSIZE)
        return;
    const int idx = i * GRIDSIZE + j;
    if (idx >= MAX_PAL_ENTRY)
        return;
    Data->color = (unsigned char)idx;
    update();
    if (frm)
        frm->refresh();
    _e->accept();
}

void PalettePanel::keyPressEvent(QKeyEvent* _e)
{
    int x = Data->color / GRIDSIZE;
    int y = Data->color % GRIDSIZE;
    switch (_e->key()) {
    case Qt::Key_Left:
        if (x > 0)
            --x;
        break;
    case Qt::Key_Right:
        if (x < GRIDSIZE - 1)
            ++x;
        break;
    case Qt::Key_Up:
        if (y > 0)
            --y;
        else if (x > 0) {
            y = GRIDSIZE - 1;
            --x;
        }
        break;
    case Qt::Key_Down:
        if (y < GRIDSIZE - 1)
            ++y;
        else if (x < GRIDSIZE - 1) {
            y = 0;
            ++x;
        }
        break;
    default:
        _e->ignore();
        return;
    }
    const int idx = x * GRIDSIZE + y;
    if (idx < MAX_PAL_ENTRY && idx != Data->color) {
        Data->color = (unsigned char)idx;
        update();
        if (frm)
            frm->refresh();
    }
    _e->accept();
}
