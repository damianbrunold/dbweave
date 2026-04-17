/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include <QApplication>

#include "datamodule.h"
#include "mainwindow.h"

int main (int argc, char* argv[])
{
	QApplication app(argc, argv);

	QApplication::setOrganizationName("Brunold Software");
	QApplication::setOrganizationDomain("brunoldsoftware.ch");
	QApplication::setApplicationName("DB-WEAVE");
	QApplication::setApplicationVersion("0.1.0");

	/*  Reconstruct the legacy VCL form-auto-creation pattern: both
	    `Data` and `DBWFRM` are globals referenced throughout the
	    ported editor code. Initialise them once at startup. */
	Data   = new TData();
	DBWFRM = new TDBWFRM();

	DBWFRM->show();

	const int rc = app.exec();

	delete DBWFRM; DBWFRM = nullptr;
	delete Data;   Data   = nullptr;
	return rc;
}
