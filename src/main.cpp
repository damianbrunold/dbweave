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
#include <QGuiApplication>
#include <QIcon>
#include <QLocale>
#include <QMessageBox>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>

#include "datamodule.h"
#include "language.h"
#include "mainwindow.h"
#include "loadoptions.h"
#include "print.h"
#include "settings.h"
#include "version.h"

#include <QPrinter>

#include <cstdio>
#include <cstring>

int main(int argc, char* argv[])
{
    /*  --version short-circuits before any Qt setup so it stays
        cheap and never touches the GUI subsystem. Handled by hand
        rather than via QCommandLineParser to avoid disturbing the
        legacy '/p <file>' silent-print syntax parsed below.        */
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--version") == 0) {
            std::printf("DB-WEAVE %s\n", DBWEAVE_VERSION_STRING);
            return 0;
        }
    }

    /*  The icons.qrc resource lives in the dbweave_ui static
        library, so its registrar isn't automatically pulled in by
        the linker. Force initialisation before any QIcon is built
        from ":/icons/*".                                        */
    Q_INIT_RESOURCE(icons);

    /*  Pattern rendering needs pixel-exact control: every cell
        symbol uses a 1-pixel margin inside the cell border, which
        collapses to an asymmetric 0/1-pixel margin when a
        fractional device-pixel ratio (common on Windows at 125%
        or 150% display scaling) rounds integer logical coords
        unevenly. Force integer DPR globally so one logical pixel
        always equals a whole number of device pixels. A 150%
        setting rounds up to 200% (crisp but slightly larger UI);
        125% rounds to 100% (crisp but slightly smaller).         */
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::Round);

    QApplication app(argc, argv);

    /*  Force the light color scheme unconditionally: the shipped menu
        and toolbar icons are drawn for a light background, so a
        system-level dark scheme produces unusable contrast against
        them. Revisit once the port ships dark-mode-aware icon sets
        and a real theme selector.

        We force the Fusion style plus an explicit light palette.
        styleHints()->setColorScheme() alone isn't enough on Linux:
        the native platform theme plugin (Breeze / Adwaita / ...)
        reads the desktop's color scheme directly and paints menus
        and toolbars dark regardless. Swapping in Fusion bypasses
        the platform plugin so the palette we install here is what
        actually gets drawn.                                        */
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
#endif
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    {
        /*  Chrome face is pulled a couple of shades darker than the
            Windows default (240) so it meets the lightened canvas
            (224) halfway, giving a smoother chrome-to-canvas
            transition.                                             */
        QPalette pal;
        pal.setColor(QPalette::Window, QColor(232, 232, 232));
        pal.setColor(QPalette::WindowText, Qt::black);
        pal.setColor(QPalette::Base, Qt::white);
        pal.setColor(QPalette::AlternateBase, QColor(238, 238, 238));
        pal.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
        pal.setColor(QPalette::ToolTipText, Qt::black);
        pal.setColor(QPalette::Text, Qt::black);
        pal.setColor(QPalette::Button, QColor(232, 232, 232));
        pal.setColor(QPalette::ButtonText, Qt::black);
        pal.setColor(QPalette::BrightText, Qt::red);
        pal.setColor(QPalette::Link, QColor(0, 0, 238));
        pal.setColor(QPalette::Highlight, QColor(0, 120, 215));
        pal.setColor(QPalette::HighlightedText, Qt::white);
        pal.setColor(QPalette::PlaceholderText, QColor(120, 120, 120));
        pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(120, 120, 120));
        pal.setColor(QPalette::Disabled, QPalette::Text, QColor(120, 120, 120));
        pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 120, 120));
        QApplication::setPalette(pal);
    }

    /*  Legacy DB-WEAVE always showed icons next to menu entries.
        Qt defaults to hiding them on platforms whose HIG discourages
        menu icons (GNOME, macOS): force them on so the ported
        layout matches the VCL original.                          */
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);

    QApplication::setOrganizationName("Brunold Software");
    QApplication::setOrganizationDomain("brunoldsoftware.ch");
    QApplication::setApplicationName("DB-WEAVE");
    QApplication::setApplicationVersion(QStringLiteral(DBWEAVE_VERSION_STRING));

    /*  Application icon. Ship both the 16/32 legacy PNGs (extracted
        from DBW.ICO) and the modern SVG so Qt can pick the best
        match for the platform's chrome (window titlebar, task
        switcher, dock).

        Skipped on macOS: setWindowIcon() there calls
        [NSApp setApplicationIconImage:], which would override the
        bundle's CFBundleIconFile (the squircle-clipped .icns) for
        the Dock while the app is running. macOS window titlebars
        don't show app icons, so there's nothing to gain.          */
#ifndef Q_OS_MACOS
    {
        QIcon appIcon;
        appIcon.addFile(QStringLiteral(":/icons/app/dbweave-16.png"), QSize(16, 16));
        appIcon.addFile(QStringLiteral(":/icons/app/dbweave-32.png"), QSize(32, 32));
        appIcon.addFile(QStringLiteral(":/icons/app/dbweave.svg"));
        QApplication::setWindowIcon(appIcon);
    }
#endif

    /*  Pick the UI language. Precedence matches legacy
        EnvOptionsDialog logic: an explicit preference saved under
        Environment/Language wins, otherwise fall back to the OS
        locale (GE for de-*, EN for everything else). Must run
        before any QString is built from LANG_STR -- concretely
        before TDBWFRM's ctor wires up menu text or print headers. */
    {
        Settings settings;
        settings.SetCategory(QString("Environment"));
        int lang = settings.Load(QString("Language"), -1);
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

    /*  Command-line:
          dbweave                      -> empty pattern
          dbweave <file.dbw>           -> open the file
          dbweave /p <file.dbw>        -> silent-print mode (legacy
                                          HandleCommandlinePrint): load
                                          the file, send it to the
                                          default printer without any
                                          dialog, then exit.          */
    const QStringList args = QApplication::arguments();

    if (args.size() >= 3 && args.at(1) == QStringLiteral("/p")) {
        const QString path = args.at(2);
        if (!QFileInfo::exists(path)) {
            delete DBWFRM;
            delete Data;
            return 2;
        }
        DBWFRM->filename = path;
        LOADSTAT stat = UNKNOWN_FAILURE;
        if (!DBWFRM->Load(stat, LOADALL)) {
            delete DBWFRM;
            delete Data;
            return 3;
        }
        /*  Default QPrinter -> system default printer, no dialog. */
        QPrinter printer(QPrinter::HighResolution);
        PrPrinterPrint job(DBWFRM, Data, &printer);
        job.Print();

        delete DBWFRM;
        DBWFRM = nullptr;
        delete Data;
        Data = nullptr;
        return 0;
    }

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
    /*  Restore saved geometry + dock layout when available; fall
        back to a sensible default on first run.                   */
    if (!DBWFRM->LoadWindowState())
        DBWFRM->resize(1024, 768);
    DBWFRM->SetAppTitle();

    DBWFRM->show();

    const int rc = app.exec();

    delete DBWFRM;
    DBWFRM = nullptr;
    delete Data;
    Data = nullptr;
    return rc;
}
