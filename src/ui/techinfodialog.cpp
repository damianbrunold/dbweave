/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "techinfodialog.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QLocale>
#include <QScreen>
#include <QStringList>
#include <QSysInfo>
#include <QTextEdit>
#include <QVBoxLayout>

TechinfoDialog::TechinfoDialog (QWidget* _parent)
	: QDialog(_parent)
{
	setWindowTitle(QStringLiteral("Technical Information"));
	setModal(true);
	resize(520, 420);

	auto* info = new QTextEdit(this);
	info->setReadOnly(true);

	/*  Build the body as HTML so bold section headers match the
	    legacy TTechinfoForm layout (GatherData adds entries with
	    an optional "bold" flag). */
	QString html;
	auto header = [&](const QString& t) {
		html += QStringLiteral("<p><b>%1</b></p>").arg(t);
	};
	auto line = [&](const QString& t) {
		html += QStringLiteral("<p style=\"margin-left:18px; margin-top:2px; "
		                       "margin-bottom:2px;\">%1</p>").arg(t);
	};

	header(QStringLiteral("DB-WEAVE version information"));
	line(QStringLiteral("Version %1").arg(QApplication::applicationVersion()));
	line(QStringLiteral("Qt port 2026 (Qt %1)").arg(QT_VERSION_STR));

	header(QStringLiteral("DB-WEAVE installation"));
	line(QStringLiteral("Installed to: %1").arg(QCoreApplication::applicationDirPath()));

	header(QStringLiteral("Operating system"));
	line(QSysInfo::prettyProductName());
	line(QStringLiteral("Kernel: %1 %2")
	        .arg(QSysInfo::kernelType(), QSysInfo::kernelVersion()));
	line(QStringLiteral("Host: %1").arg(QSysInfo::machineHostName()));
	line(QStringLiteral("Locale: %1").arg(QLocale::system().name()));

	header(QStringLiteral("Computer Equipment"));
	line(QStringLiteral("CPU architecture: %1 (built for %2)")
	        .arg(QSysInfo::currentCpuArchitecture(),
	             QSysInfo::buildCpuArchitecture()));
	line(QStringLiteral("Byte order: %1-endian")
	        .arg(QSysInfo::ByteOrder == QSysInfo::LittleEndian
	             ? QStringLiteral("little") : QStringLiteral("big")));

	if (QScreen* s = QGuiApplication::primaryScreen()) {
		const int depth = s->depth();
		const QSize geo = s->size();
		line(QStringLiteral("Screen: %1 x %2 at %3 bit")
		        .arg(geo.width()).arg(geo.height()).arg(depth));
		/*  Legacy "colors" line. For 24+ bpp legacy reported
		    "16777216 colors"; mirror that. */
		const quint64 cols = (depth >= 32) ? (quint64(1) << 32)
		                   : (depth > 0)   ? (quint64(1) << depth) : 0;
		if (cols) line(QStringLiteral("%1 colors").arg(cols));
	}

	info->setHtml(html);

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok, this);
	connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);

	auto* root = new QVBoxLayout(this);
	root->addWidget(info);
	root->addWidget(btns);
}
