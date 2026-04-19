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
#include <QFileInfo>
#include <QLocale>
#include <QMessageBox>

#include "datamodule.h"
#include "language.h"
#include "mainwindow.h"
#include "loadoptions.h"
#include "settings.h"

int main(int argc, char* argv[])
{
    /*  The icons.qrc resource lives in the dbweave_ui static
        library, so its registrar isn't automatically pulled in by
        the linker. Force initialisation before any QIcon is built
        from ":/icons/*".                                        */
    Q_INIT_RESOURCE(icons);

    QApplication app(argc, argv);

    /*  Legacy DB-WEAVE always showed icons next to menu entries.
        Qt defaults to hiding them on platforms whose HIG discourages
        menu icons (GNOME, macOS): force them on so the ported
        layout matches the VCL original.                          */
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);

    QApplication::setOrganizationName("Brunold Software");
    QApplication::setOrganizationDomain("brunoldsoftware.ch");
    QApplication::setApplicationName("DB-WEAVE");
    QApplication::setApplicationVersion("0.1.0");

    /*  Pick the UI language. Precedence matches legacy
        EnvOptionsDialog logic: an explicit preference saved under
        Environment/Language wins, otherwise fall back to the OS
        locale (GE for de-*, EN for everything else). Must run
        before any QString is built from LANG_STR -- concretely
        before TDBWFRM's ctor wires up menu text or print headers. */
    {
        Settings settings;
        settings.SetCategory(AnsiString("Environment"));
        int lang = settings.Load(AnsiString("Language"), -1);
        if (lang < 0) {
            const QString tag = QLocale::system().name().toLower();
            lang = tag.startsWith(QStringLiteral("de")) ? 1 : 0;
        }
        active_language = (lang == 1) ? GE : EN;
    }

    /*  Reconstruct the legacy VCL form-auto-creation pattern: both
        `Data` and `DBWFRM` are globals referenced throughout the
        ported editor code. Initialise them once at startup. */
    Data = new TData();
    DBWFRM = new TDBWFRM();

    /*  If the user passed a .dbw path on the command line, load it.
        Otherwise start with an empty pattern -- File | Open is now
        available from the menu. */
    const QStringList args = QApplication::arguments();
    if (args.size() >= 2) {
        const QString path = args.at(1);
        if (QFileInfo::exists(path)) {
            DBWFRM->filename = path;
            LOADSTAT stat = UNKNOWN_FAILURE;
            if (!DBWFRM->Load(stat, LOADALL)) {
                QMessageBox::warning(
                    DBWFRM, QStringLiteral("DB-WEAVE"),
                    QStringLiteral("Could not load '%1' (status %2)").arg(path).arg(int(stat)));
            }
        }
    }
    DBWFRM->resize(1024, 768);

    DBWFRM->show();

    const int rc = app.exec();

    delete DBWFRM;
    DBWFRM = nullptr;
    delete Data;
    Data = nullptr;
    return rc;
}
