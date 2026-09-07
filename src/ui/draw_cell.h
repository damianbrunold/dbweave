/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Primitive per-cell rendering. These are the leaf drawing routines of
    the pattern editor: every higher-level Draw* method (DrawEinzug,
    DrawAufknuepfung, DrawTrittfolge, ...) ultimately dispatches here
    to paint or clear a single grid cell with a darstellung symbol.

    Deliberate structural change vs. the legacy VCL versions in
    dbw3_form.h: the signature now takes a QPainter& and QColor values
    instead of a TCanvas* and TColor. This decouples the leaf renderer
    from the main window (the legacy NUMBER branch also reached into
    DBWFRM->currentzoom -- here the caller passes the font height
    explicitly) and makes it unit-testable against an off-screen
    QImage.

    Coordinates follow VCL's inclusive-exclusive convention: the cell
    occupies pixels in [_x, _xx) x [_y, _yy).
*/

#ifndef DBWEAVE_UI_DRAW_CELL_H
#define DBWEAVE_UI_DRAW_CELL_H

#include <QColor>
#include <QPainter>

#include "enums.h" /* DARSTELLUNG */

/*  True when the painter draws onto a vector device (PDF, SVG, a
    printer) rather than a pixel grid. The two need different
    coordinate handling: on a raster device an integer coordinate
    names a whole pixel, on a vector one it names a geometric
    position with the grid-frame strokes centred on it. */
bool IsVectorPainter(const QPainter& _p);

/*  Fill the pixel box [_x1, _x2] x [_y1, _y2], both ends inclusive.
    On a vector device the box is mapped to the continuous span
    [_x1-0.5, _x2+0.5] so it stays centred between the grid frame
    lines instead of drifting half a pixel down and right. */
void FillPixelBox(QPainter& _p, int _x1, int _y1, int _x2, int _y2, const QColor& _color);

void ClearCell(QPainter& _p, int _x, int _y, int _xx, int _yy, const QColor& _bkground);

void PaintCell(QPainter& _p, DARSTELLUNG _darstellung, int _x, int _y, int _xx, int _yy,
               const QColor& _color = QColor(Qt::black), bool _dontclear = false, int _number = -1,
               const QColor& _bkground = QColor(212, 208, 200), int _fontHeight = 0);

#endif /* DBWEAVE_UI_DRAW_CELL_H */
