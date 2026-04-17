/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "mainwindow.h"

TDBWFRM* DBWFRM = nullptr;

TDBWFRM::TDBWFRM(QWidget* parent)
	: QMainWindow(parent)
{
	setWindowTitle(QStringLiteral("DB-WEAVE"));
	resize(1024, 768);
}

TDBWFRM::~TDBWFRM() = default;
