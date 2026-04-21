/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "steuerung.h"

#include "datamodule.h"
#include "language.h"
#include "mainwindow.h"
#include "steuerungcanvas.h"

#include <QAction>
#include <QActionGroup>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QScrollBar>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

/*-----------------------------------------------------------------*/
TSTRGFRM::TSTRGFRM(TDBWFRM* _main, QWidget* _parent)
    : QDialog(_parent)
    , frm(_main)
    , data(Data)
{
    setWindowTitle(LANG_STR("Weaving - DB-WEAVE", "Weben - DB-WEAVE"));
    setModal(true);

    /*  Let the dialog take real estate like a secondary window;
        legacy TSTRGFRM opened at the main window's size.         */
    if (frm)
        resize(frm->size());
    else
        resize(800, 600);

    /*  Non-owning field pointers. Caller (LoomControlClick) has
        populated these before show() but guard anyway.         */
    if (frm) {
        einzug = &frm->einzug;
        aufknuepfung = &frm->aufknuepfung;
        trittfolge = &frm->trittfolge;
        gewebe = &frm->gewebe;
        kettfarben = &frm->kettfarben;
        schussfarben = &frm->schussfarben;
    }

    buildMenus();
    buildToolbar();
    buildCentralArea();
    buildStatusbar();

    pullStateFromMain();
}

/*-----------------------------------------------------------------*/
TSTRGFRM::~TSTRGFRM()
{
    /*  Stage 7f will stop / terminate the controller here. */
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::buildMenus()
{
    menubar = new QMenuBar(this);

    auto disabledAct = [this](QMenu* _m, const QString& _en, const QString& _ge,
                              const QKeySequence& _shortcut = QKeySequence()) {
        QAction* a = _m->addAction(LANG_STR(_en, _ge));
        if (!_shortcut.isEmpty())
            a->setShortcut(_shortcut);
        a->setEnabled(false);
        return a;
    };

    /*  --- &File (legacy had WebenBeenden at top level) --------- */
    QMenu* fileMenu = menubar->addMenu(LANG_STR("&File", "&Datei"));
    actBeenden
        = fileMenu->addAction(LANG_STR("&End weaving", "Weben be&enden"), this, &QDialog::close);
    actBeenden->setShortcut(QKeySequence(Qt::Key_Escape));

    /*  --- &Weave (MenuWeave) ---------------------------------- */
    QMenu* weaveMenu = menubar->addMenu(LANG_STR("&Weave", "&Weben"));
    actStart = disabledAct(weaveMenu, "&Start", "&Start", QKeySequence(Qt::Key_Space));
    actStop = disabledAct(weaveMenu, "S&top", "S&top", QKeySequence(Qt::Key_Escape));
    weaveMenu->addSeparator();
    actReverse = disabledAct(weaveMenu, "Wea&ve backwards", "&Rückwärts weben");
    actReverse->setCheckable(true);

    /*  --- &Options -------------------------------------------- */
    QMenu* optionsMenu = menubar->addMenu(LANG_STR("&Options", "&Optionen"));
    actOptionsLoom = disabledAct(optionsMenu, "&Loom...", "&Webstuhl...");
    actLoop = disabledAct(optionsMenu, "L&oop", "&Endlos");
    actLoop->setCheckable(true);
    actReverseSchaft = disabledAct(optionsMenu, "Rev&erse shafts", "Schäfte &umgekehrt");
    actReverseSchaft->setCheckable(true);
    optionsMenu->addSeparator();
    QMenu* schaftsMenu = optionsMenu->addMenu(LANG_STR("Number of &shafts", "Anzahl &Schäfte"));
    QActionGroup* schaftGroup = new QActionGroup(this);
    const int schaftCounts[8] = { 4, 8, 12, 16, 20, 24, 28, 32 };
    for (int i = 0; i < 8; i++) {
        actSchafts[i] = schaftsMenu->addAction(QString::number(schaftCounts[i]));
        actSchafts[i]->setCheckable(true);
        actSchafts[i]->setEnabled(false);
        schaftGroup->addAction(actSchafts[i]);
    }

    /*  --- &View (MenuAnsicht) --------------------------------- */
    QMenu* viewMenu = menubar->addMenu(LANG_STR("&View", "&Ansicht"));
    actViewPatrone
        = disabledAct(viewMenu, "Pa&ttern", "&Patrone", QKeySequence(Qt::CTRL | Qt::Key_1));
    actViewPatrone->setCheckable(true);
    actViewFarbeffekt
        = disabledAct(viewMenu, "C&oloured", "&Farbeffekt", QKeySequence(Qt::CTRL | Qt::Key_2));
    actViewFarbeffekt->setCheckable(true);
    actViewGewebesimulation = disabledAct(viewMenu, "&Simulation", "Gewebe&simulation",
                                          QKeySequence(Qt::CTRL | Qt::Key_3));
    actViewGewebesimulation->setCheckable(true);
    QActionGroup* viewGroup = new QActionGroup(this);
    viewGroup->addAction(actViewPatrone);
    viewGroup->addAction(actViewFarbeffekt);
    viewGroup->addAction(actViewGewebesimulation);
    viewMenu->addSeparator();
    actZoomIn = disabledAct(viewMenu, "Zoom &in", "Zoom ver&grössern",
                            QKeySequence(Qt::CTRL | Qt::Key_I));
    actZoomNormal = disabledAct(viewMenu, "Zoom &normal", "Zoom &normal",
                                QKeySequence(Qt::CTRL | Qt::Key_N));
    actZoomOut
        = disabledAct(viewMenu, "Zoom &out", "Zoom ver&kleinern", QKeySequence(Qt::CTRL | Qt::Key_U));

    /*  --- &Position ------------------------------------------- */
    QMenu* posMenu = menubar->addMenu(LANG_STR("&Position", "&Position"));
    actSetCurrentPos = disabledAct(posMenu, "&Set current position", "&Aktuelle Position setzen",
                                   QKeySequence(Qt::CTRL | Qt::Key_S));
    actGotoLastPos = disabledAct(posMenu, "&Go to last position", "Zur &letzten Position",
                                 QKeySequence(Qt::CTRL | Qt::Key_L));
    QMenu* gotoMenu
        = posMenu->addMenu(LANG_STR("Go to &klammer", "Zu &Klammer"));
    for (int i = 0; i < MAXKLAMMERN; i++) {
        actGotoKlammer[i] = gotoMenu->addAction(QString::number(i + 1));
        actGotoKlammer[i]->setShortcut(QKeySequence(Qt::CTRL | (Qt::Key_1 + i)));
        actGotoKlammer[i]->setEnabled(false);
    }
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::buildToolbar()
{
    toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setFloatable(false);
    toolbar->addAction(actStart);
    toolbar->addAction(actStop);
    toolbar->addAction(actReverse);
    toolbar->addSeparator();
    toolbar->addAction(actSetCurrentPos);
    toolbar->addAction(actGotoLastPos);
    toolbar->addSeparator();
    for (int i = 0; i < MAXKLAMMERN; i++) {
        /*  Separate goto buttons next to the main toolbar -- the
            legacy form had nine sbGoto<n> SpeedButtons that share
            behaviour with the Ctrl+<n> menu entries.              */
        actSbGoto[i] = toolbar->addAction(QString::number(i + 1));
        actSbGoto[i]->setEnabled(false);
    }
    toolbar->addSeparator();
    toolbar->addAction(actOptionsLoom);
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::buildCentralArea()
{
    canvas = new SteuerungCanvas(this);
    scrollbar = new QScrollBar(Qt::Vertical, this);
    scrollbar->setEnabled(false); /* wired in 7b */
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::buildStatusbar()
{
    statusbar = new QStatusBar(this);
    labPosition = new QLabel(statusbar);
    labKlammer = new QLabel(statusbar);
    labRepetition = new QLabel(statusbar);
    statusbar->addWidget(labPosition, 1);
    statusbar->addWidget(labKlammer, 1);
    statusbar->addWidget(labRepetition, 1);

    /*  Root layout: menu bar, toolbar, canvas + scrollbar, status. */
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->setMenuBar(menubar);
    root->addWidget(toolbar);
    auto* center = new QHBoxLayout();
    center->setContentsMargins(0, 0, 0, 0);
    center->setSpacing(0);
    center->addWidget(canvas, 1);
    center->addWidget(scrollbar);
    root->addLayout(center, 1);
    root->addWidget(statusbar);
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::pullStateFromMain()
{
    if (!frm)
        return;

    /*  Port of legacy WeaveClick initialisation block (steuerung_
        form.cpp:25-102): copies the main-window klammer / zoom /
        view-mode state into TSTRGFRM so the secondary window
        starts with a consistent snapshot.                        */
    bool filldata = true;
    for (int i = 0; i < MAXKLAMMERN; i++)
        if (frm->klammern[i].repetitions > 0)
            filldata = false;
    if (filldata && frm->klammern[0].repetitions == 0) {
        /*  Default first klammer from current schuss selection. */
        if (frm->schuesse.a != -1 && frm->schuesse.b != -1) {
            frm->klammern[0].repetitions = 1;
            frm->klammern[0].first = frm->schuesse.a;
            frm->klammern[0].last = frm->schuesse.b;
        } else {
            frm->klammern[0].repetitions = 1;
            frm->klammern[0].first = 0;
            frm->klammern[0].last = 8;
        }
    }
    for (int i = 0; i < MAXKLAMMERN; i++)
        klammern[i] = frm->klammern[i];

    /*  zoom[] is initialised in-class with the same legacy values
        the main window uses (patterncanvas ZOOM_TABLE); just
        copy currentzoom. */
    currentzoom = frm->currentzoom;

    schlagpatrone = frm->ViewSchlagpatrone && frm->ViewSchlagpatrone->isChecked();
    schlagpatronendarstellung = frm->schlagpatronendarstellung;
    if (frm->rapport.kr.a >= 0)
        rapportx = frm->rapport.kr.b - frm->rapport.kr.a + 1;
    if (frm->rapport.sr.a >= 0)
        rapporty = frm->rapport.sr.b - frm->rapport.sr.a + 1;
    tritte = frm->wvisible;
    fewithraster = frm->fewithraster;
    filename = frm->filename;
    weaving = frm->weaving;

    /*  Initial view-mode radios mirror the main window's gewebe
        view toggles.                                              */
    if (actViewPatrone)
        actViewPatrone->setChecked(frm->GewebeNormal && frm->GewebeNormal->isChecked());
    if (actViewFarbeffekt)
        actViewFarbeffekt->setChecked(frm->GewebeFarbeffekt
                                      && frm->GewebeFarbeffekt->isChecked());
    if (actViewGewebesimulation)
        actViewGewebesimulation->setChecked(frm->GewebeSimulation
                                            && frm->GewebeSimulation->isChecked());
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::pushStateToMain()
{
    if (!frm)
        return;

    /*  Port of legacy WeaveClick tail (steuerung_form.cpp:83-107):
        copies the klammer state back, sets modified, and updates
        weave_position on TDBWFRM for the main-window cursor.    */
    for (int i = 0; i < MAXKLAMMERN; i++)
        frm->klammern[i] = klammern[i];
    frm->weaving = weaving;
    frm->weave_position = weave_position;
    if (modified)
        frm->SetModified();
}

/*-----------------------------------------------------------------*/
/*  Entry point from Extras > Weave / Ctrl+W. Port of legacy
    TDBWFRM::WeaveClick. Opens the steuerung window modally; on
    close, copies klammer / weave-position state back.           */
void TDBWFRM::LoomControlClick()
{
    TSTRGFRM dlg(this, this);
    dlg.exec();
    dlg.pushStateToMain();
}
