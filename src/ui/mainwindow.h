/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Phase-5 skeleton of the pattern-editor document window. In the
    legacy VCL codebase TDBWFRM is a 1200-line class declaration with
    dozens of event handlers, menu items, scrollbars, and owned Feld*
    subfields. The port reconstructs it incrementally: this first slice
    is just enough scaffolding that ported units (init.cpp, clear.cpp,
    ...) can reach for `DBWFRM->scroll_x1` etc. without breaking the
    build.

    Members are added as the units that reference them are ported.
*/

#ifndef DBWEAVE_UI_MAINWINDOW_H
#define DBWEAVE_UI_MAINWINDOW_H

#include <QMainWindow>

#include "vcl_compat.h"

class TDBWFRM : public QMainWindow
{
	Q_OBJECT

public:
	explicit TDBWFRM(QWidget* parent = nullptr);
	~TDBWFRM() override;

	/*  Scroll offsets for each of the four scrollable grids. Legacy
	    TDBWFRM declared them as plain int members; keeping the same
	    names so the ported ScrollX()/ScrollY() bodies in init.cpp
	    compile unchanged. */
	int scroll_x1 = 0;
	int scroll_x2 = 0;
	int scroll_y1 = 0;
	int scroll_y2 = 0;
};

/*  Matches legacy `extern PACKAGE TDBWFRM *DBWFRM;`. Populated by
    main() just like the VCL form-auto-creation pattern did. */
extern TDBWFRM* DBWFRM;

#endif
