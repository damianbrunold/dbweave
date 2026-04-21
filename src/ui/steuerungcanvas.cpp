/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "steuerungcanvas.h"
#include "steuerung.h"
#include "legacy_colors.h"

#include <QMouseEvent>
#include <QPainter>

SteuerungCanvas::SteuerungCanvas(TSTRGFRM* _frm)
    : QWidget(nullptr)
    , frm(_frm)
{
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, legacyBtnFace());
    setPalette(pal);
    setFocusPolicy(Qt::StrongFocus);
}

void SteuerungCanvas::paintEvent(QPaintEvent* /*_e*/)
{
    QPainter p(this);
    p.fillRect(rect(), legacyBtnFace());
    if (!frm)
        return;
    frm->currentPainter = &p;
    frm->paintAll();
    frm->currentPainter = nullptr;
}

void SteuerungCanvas::resizeEvent(QResizeEvent* /*_e*/)
{
    /*  Recompute cell size / visible counts / scrollbar range
        whenever the canvas changes size.                        */
    if (frm)
        frm->CalcSizes();
}

void SteuerungCanvas::mousePressEvent(QMouseEvent* _e)
{
    if (frm)
        frm->FormMouseDown(_e);
    setFocus();
}

void SteuerungCanvas::mouseMoveEvent(QMouseEvent* _e)
{
    if (frm)
        frm->FormMouseMove(_e);
}

void SteuerungCanvas::mouseReleaseEvent(QMouseEvent* _e)
{
    if (frm)
        frm->FormMouseUp(_e);
}
