/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "aboutdialog.h"
#include "techinfodialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AboutDialog::AboutDialog (QWidget* _parent)
	: QDialog(_parent)
{
	setWindowTitle(QStringLiteral("About DB-WEAVE"));
	setModal(true);

	/*  Title bar above the prose body. */
	auto* title = new QLabel(
	    QStringLiteral("<h2>DB-WEAVE %1</h2>"
	                   "<p>Textile CAD/CAM software<br>"
	                   "<a href=\"http://www.brunoldsoftware.ch\">"
	                   "http://www.brunoldsoftware.ch</a></p>"
	                   "<p>Copyright &copy; 1998, 2009, 2026 Damian Brunold<br>"
	                   "Qt 6 port 2026.</p>")
	        .arg(QApplication::applicationVersion()),
	    this);
	title->setTextFormat(Qt::RichText);
	title->setOpenExternalLinks(true);

	/*  Verbatim legacy prose (English text from pbInfosPaint). Two
	    columns side by side.                                     */
	auto* colLeft = new QLabel(
	    QStringLiteral(
	        "<h3>What DB-WEAVE is</h3>"
	        "<p>DB-WEAVE is a software for designing and weaving fabrics "
	        "with a dobby loom. You can design your patterns and weave "
	        "then afterwards manually. Or you can directly weave your "
	        "pattern on a computer-controlled loom.</p>"
	        "<h3>Using DB-WEAVE</h3>"
	        "<p>As DB-WEAVE is a standard Windows program, the user "
	        "interface is similar to that of other Windows-based "
	        "software. If you know Microsoft Word or Excel or CorelDraw, "
	        "you can easily use DB-WEAVE.</p>"),
	    this);
	colLeft->setTextFormat(Qt::RichText);
	colLeft->setWordWrap(true);
	colLeft->setMinimumWidth(280);

	auto* colRight = new QLabel(
	    QStringLiteral(
	        "<p>In any case, we recommend to explore the program's "
	        "capabilities by trial and error. Since DB-WEAVE has a 100 "
	        "step undo functionality you can easily correct any unwanted "
	        "changes.</p>"
	        "<h3>Further information</h3>"
	        "<p>DB-WEAVE tries to display any relevant information in "
	        "the status bar at the bottom of its window. If you need "
	        "further information please consider the manual. If you have "
	        "access to the internet, you can find more information on "
	        "our website at "
	        "<a href=\"http://www.brunoldsoftware.ch\">"
	        "http://www.brunoldsoftware.ch</a>.</p>"),
	    this);
	colRight->setTextFormat(Qt::RichText);
	colRight->setWordWrap(true);
	colRight->setOpenExternalLinks(true);
	colRight->setMinimumWidth(280);

	auto* cols = new QHBoxLayout();
	cols->addWidget(colLeft);
	cols->addSpacing(12);
	cols->addWidget(colRight);

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok, this);
	QPushButton* techBtn = btns->addButton(QStringLiteral("&Technical Info..."),
	                                       QDialogButtonBox::ActionRole);
	connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(techBtn, &QPushButton::clicked, this, [this] {
		hide();
		TechinfoDialog(this).exec();
		show();
	});

	auto* root = new QVBoxLayout(this);
	root->addWidget(title);
	root->addLayout(cols);
	root->addWidget(btns);
}
