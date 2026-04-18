/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  16x16 swatch grid rendering Data->palette. Click a swatch to
    set Data->color; the current choice gets a white outline. Port
    of legacy farbpalette_form.cpp; lives inside a QDockWidget
    docked on the right of TDBWFRM. */

#ifndef DBWEAVE_UI_PALETTEPANEL_H
#define DBWEAVE_UI_PALETTEPANEL_H

#include <QWidget>

class TDBWFRM;

class PalettePanel : public QWidget
{
	Q_OBJECT

public:
	explicit PalettePanel (TDBWFRM* _frm, QWidget* _parent = nullptr);

	QSize sizeHint()        const override;
	QSize minimumSizeHint() const override;

protected:
	void paintEvent    (QPaintEvent*  _e) override;
	void mousePressEvent (QMouseEvent* _e) override;
	void keyPressEvent (QKeyEvent*    _e) override;

private:
	TDBWFRM* frm;

	static constexpr int GRIDSIZE = 16;   /* 16x16 grid (256 slots) */

	int drawSize() const;     /* cell edge in px, from current widget size */
};

#endif
