/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Palette swatch picker. Lays out Data->palette entries in a fixed
    COLS x ceil(N/COLS) grid; total widget size is fixed so a parent
    QScrollArea can scroll vertically when the surrounding panel is
    too short. The currently-selected palette index gets a thick
    highlight outline. Lives in the right-hand side panel of TDBWFRM;
    toggled from the View > Palette menu entry.                      */

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

private:
    TDBWFRM* frm;

    static constexpr int CELL = 18; /* swatch edge in px */
    static constexpr int COLS = 4;  /* fixed column count */

    int entryCount() const; /* MAX_PAL_ENTRY */
    int columns() const;    /* COLS */
    int rows() const;       /* ceil(N/COLS) */
    bool cellAt(const QPoint& _pos, int& _idx) const;
};

#endif
