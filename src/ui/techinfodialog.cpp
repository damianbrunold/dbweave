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
#include "language.h"

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

TechinfoDialog::TechinfoDialog(QWidget* _parent)
    : QDialog(_parent)
{
    setWindowTitle(LANG_STR("Technical Information", "Technische Informationen"));
    setModal(true);
    resize(520, 420);

    auto* info = new QTextEdit(this);
    info->setReadOnly(true);

    /*  Build the body as HTML so bold section headers match the
        legacy TTechinfoForm layout (GatherData adds entries with
        an optional "bold" flag). */
    QString html;
    auto header = [&](const QString& t) { html += QStringLiteral("<p><b>%1</b></p>").arg(t); };
    auto line = [&](const QString& t) {
        html += QStringLiteral("<p style=\"margin-left:18px; margin-top:2px; "
                               "margin-bottom:2px;\">%1</p>")
                    .arg(t);
    };

    header(LANG_STR("DB-WEAVE version information", "DB-WEAVE-Versionsinformationen"));
    line(LANG_STR("Version %1", "Version %1").arg(QApplication::applicationVersion()));
    line(LANG_STR("Qt port 2026 (Qt %1)", "Qt-Portierung 2026 (Qt %1)").arg(QT_VERSION_STR));

    header(LANG_STR("DB-WEAVE installation", "DB-WEAVE-Installation"));
    line(LANG_STR("Installed to: %1", "Installiert unter: %1")
             .arg(QCoreApplication::applicationDirPath()));

    header(LANG_STR("Operating system", "Betriebssystem"));
    line(QSysInfo::prettyProductName());
    line(LANG_STR("Kernel: %1 %2", "Kernel: %1 %2")
             .arg(QSysInfo::kernelType(), QSysInfo::kernelVersion()));
    line(LANG_STR("Host: %1", "Rechnername: %1").arg(QSysInfo::machineHostName()));
    line(LANG_STR("Locale: %1", "Gebietsschema: %1").arg(QLocale::system().name()));

    header(LANG_STR("Computer Equipment", "Computer-Ausstattung"));
    line(LANG_STR("CPU architecture: %1 (built for %2)",
                  "CPU-Architektur: %1 (gebaut für %2)")
             .arg(QSysInfo::currentCpuArchitecture(), QSysInfo::buildCpuArchitecture()));
    line(LANG_STR("Byte order: %1-endian", "Bytereihenfolge: %1-endian")
             .arg(QSysInfo::ByteOrder == QSysInfo::LittleEndian
                      ? LANG_STR("little", "little")
                      : LANG_STR("big", "big")));

    if (QScreen* s = QGuiApplication::primaryScreen()) {
        const int depth = s->depth();
        const QSize geo = s->size();
        line(LANG_STR("Screen: %1 x %2 at %3 bit",
                      "Bildschirm: %1 x %2 mit %3 Bit")
                 .arg(geo.width())
                 .arg(geo.height())
                 .arg(depth));
        const quint64 cols = (depth >= 32) ? (quint64(1) << 32)
                             : (depth > 0) ? (quint64(1) << depth)
                                           : 0;
        if (cols)
            line(LANG_STR("%1 colors", "%1 Farben").arg(cols));
    }

    info->setHtml(html);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto* root = new QVBoxLayout(this);
    root->addWidget(info);
    root->addWidget(btns);
}
