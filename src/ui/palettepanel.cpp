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
#include <QResizeEvent>

#include <algorithm>

PalettePanel::PalettePanel(TDBWFRM* _frm, QWidget* _parent)
    : QWidget(_parent)
    , frm(_frm)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(true);
}

int PalettePanel::entryCount() const
{
    return MAX_PAL_ENTRY;
}

int PalettePanel::columnsFor(int _h) const
{
    const int n = entryCount();
    if (_h < CELL)
        return n; /* degenerate -- one row */
    const int rowsFit = std::max(1, _h / CELL);
    return (n + rowsFit - 1) / rowsFit;
}

int PalettePanel::columns() const
{
    return columnsFor(height());
}

int PalettePanel::rows() const
{
    const int cols = std::max(1, columns());
    return (entryCount() + cols - 1) / cols;
}

QSize PalettePanel::sizeHint() const
{
    /*  Prefer a single narrow column at the natural dock height --
        the parent dock will clamp the actual height and the column
        count adapts on resize.                                      */
    return QSize(CELL, entryCount() * CELL);
}

QSize PalettePanel::minimumSizeHint() const
{
    return QSize(CELL, CELL * 8);
}

void PalettePanel::resizeEvent(QResizeEvent* _e)
{
    QWidget::resizeEvent(_e);
    update();
}

bool PalettePanel::cellAt(const QPoint& _pos, int& _idx) const
{
    const int cols = columns();
    const int r = _pos.y() / CELL;
    const int c = _pos.x() / CELL;
    if (c < 0 || r < 0 || c >= cols)
        return false;
    const int i = c * rows() + r;
    if (i < 0 || i >= entryCount())
        return false;
    _idx = i;
    return true;
}

void PalettePanel::paintEvent(QPaintEvent* /*_e*/)
{
    if (!Data || !Data->palette)
        return;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    p.fillRect(rect(), palette().color(QPalette::Button));

    const int cols = columns();
    const int nrows = rows();
    const int n = entryCount();

    p.setPen(QPen(Qt::black));
    for (int i = 0; i < n; i++) {
        const int c = i / nrows;
        const int r = i % nrows;
        const int x = c * CELL;
        const int y = r * CELL;
        p.setBrush(qColorFromTColor(Data->palette->GetColor(i)));
        p.drawRect(x, y, CELL - 1, CELL - 1);
    }

    /*  Highlight the currently-active palette slot with a thick
        contrast outline (black + inner white) so it reads clearly
        against both dark and light swatches.                      */
    const int sel = Data->color;
    if (sel >= 0 && sel < n) {
        const int c = sel / nrows;
        const int r = sel % nrows;
        const int x = c * CELL;
        const int y = r * CELL;
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(Qt::black, 3));
        p.drawRect(x + 1, y + 1, CELL - 3, CELL - 3);
        p.setPen(QPen(Qt::white, 1));
        p.drawRect(x + 2, y + 2, CELL - 5, CELL - 5);
    }
    (void)cols;
}

void PalettePanel::mousePressEvent(QMouseEvent* _e)
{
    if (_e->button() != Qt::LeftButton) {
        _e->ignore();
        return;
    }
    int idx = 0;
    if (!cellAt(_e->pos(), idx))
        return;
    Data->color = (unsigned char)idx;
    update();
    if (frm)
        frm->refresh();
    _e->accept();
}

void PalettePanel::keyPressEvent(QKeyEvent* _e)
{
    const int n = entryCount();
    const int nrows = rows();
    const int cols = std::max(1, columns());
    int sel = Data->color;
    int c = sel / nrows;
    int r = sel % nrows;
    switch (_e->key()) {
    case Qt::Key_Up:
        if (r > 0)
            --r;
        else if (c > 0) {
            --c;
            r = nrows - 1;
        }
        break;
    case Qt::Key_Down:
        if (r < nrows - 1)
            ++r;
        else if (c < cols - 1) {
            ++c;
            r = 0;
        }
        break;
    case Qt::Key_Left:
        if (c > 0)
            --c;
        break;
    case Qt::Key_Right:
        if (c < cols - 1)
            ++c;
        break;
    default:
        _e->ignore();
        return;
    }
    const int idx = c * nrows + r;
    if (idx >= 0 && idx < n && idx != Data->color) {
        Data->color = (unsigned char)idx;
        update();
        if (frm)
            frm->refresh();
    }
    _e->accept();
}
