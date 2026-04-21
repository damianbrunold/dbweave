/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Palette swatch picker. Lays out Data->palette entries as a vertical
    column of swatches by default; when the available height isn't
    enough, the cells wrap into additional columns (2, 3, ...). The
    currently-selected palette index gets a thick highlight outline.
    Lives inside a QDockWidget on the right of TDBWFRM; toggled from
    the View > Palette menu entry.                                  */

#ifndef DBWEAVE_UI_PALETTEPANEL_H
#define DBWEAVE_UI_PALETTEPANEL_H

#include <QWidget>

class TDBWFRM;

class PalettePanel : public QWidget
{
    Q_OBJECT

public:
    explicit PalettePanel(TDBWFRM* _frm, QWidget* _parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    bool hasHeightForWidth() const override { return false; }

protected:
    void paintEvent(QPaintEvent* _e) override;
    void mousePressEvent(QMouseEvent* _e) override;
    void keyPressEvent(QKeyEvent* _e) override;
    void resizeEvent(QResizeEvent* _e) override;

private:
    TDBWFRM* frm;

    static constexpr int CELL = 18;  /* fixed swatch edge in px */

    int entryCount() const;       /* clamped to MAX_PAL_ENTRY */
    int columnsFor(int _h) const; /* cols needed to fit N cells in height h */
    int columns() const;          /* current column count from current height */
    int rows() const;             /* rows in the current layout */
    bool cellAt(const QPoint& _pos, int& _idx) const;
};

#endif
