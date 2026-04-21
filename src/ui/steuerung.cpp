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

#include <algorithm>

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
    _ResetCurrentPos();
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::showEvent(QShowEvent* _e)
{
    QDialog::showEvent(_e);
    /*  Canvas size is only final once the dialog is mapped; run
        the first CalcSizes / UpdateStatusbar / enable-goto sweep
        here rather than in the ctor.                             */
    refresh();
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
    actLoop = optionsMenu->addAction(LANG_STR("L&oop", "&Endlos"));
    actLoop->setCheckable(true);
    actLoop->setChecked(loop);
    connect(actLoop, &QAction::toggled, this, [this](bool _on) { loop = _on; });
    actReverseSchaft = optionsMenu->addAction(LANG_STR("Rev&erse shafts", "Schäfte &umgekehrt"));
    actReverseSchaft->setCheckable(true);
    connect(actReverseSchaft, &QAction::toggled, this, [this](bool _on) { reverse = _on; });
    optionsMenu->addSeparator();
    QMenu* schaftsMenu = optionsMenu->addMenu(LANG_STR("Number of &shafts", "Anzahl &Schäfte"));
    QActionGroup* schaftGroup = new QActionGroup(this);
    const int schaftCounts[8] = { 4, 8, 12, 16, 20, 24, 28, 32 };
    for (int i = 0; i < 8; i++) {
        actSchafts[i] = schaftsMenu->addAction(QString::number(schaftCounts[i]));
        actSchafts[i]->setCheckable(true);
        schaftGroup->addAction(actSchafts[i]);
        const int n = schaftCounts[i];
        connect(actSchafts[i], &QAction::triggered, this, [this, n] { numberOfShafts = n; });
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
    actZoomIn = viewMenu->addAction(LANG_STR("Zoom &in", "Zoom ver&grössern"));
    actZoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    actZoomNormal = viewMenu->addAction(LANG_STR("Zoom &normal", "Zoom &normal"));
    actZoomNormal->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    actZoomOut = viewMenu->addAction(LANG_STR("Zoom &out", "Zoom ver&kleinern"));
    actZoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_U));
    connect(actZoomIn, &QAction::triggered, this, [this] {
        if (currentzoom < 9) {
            currentzoom++;
            refresh();
        }
    });
    connect(actZoomNormal, &QAction::triggered, this, [this] {
        currentzoom = 3;
        refresh();
    });
    connect(actZoomOut, &QAction::triggered, this, [this] {
        if (currentzoom > 0) {
            currentzoom--;
            refresh();
        }
    });

    /*  --- &Position ------------------------------------------- */
    QMenu* posMenu = menubar->addMenu(LANG_STR("&Position", "&Position"));
    actSetCurrentPos = disabledAct(posMenu, "&Set current position", "&Aktuelle Position setzen",
                                   QKeySequence(Qt::CTRL | Qt::Key_S));
    actGotoLastPos = posMenu->addAction(LANG_STR("&Go to last position", "Zur &letzten Position"));
    actGotoLastPos->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(actGotoLastPos, &QAction::triggered, this, [this] { GotoLastPosition(); });
    QMenu* gotoMenu
        = posMenu->addMenu(LANG_STR("Go to &klammer", "Zu &Klammer"));
    for (int i = 0; i < MAXKLAMMERN; i++) {
        actGotoKlammer[i] = gotoMenu->addAction(QString::number(i + 1));
        actGotoKlammer[i]->setShortcut(QKeySequence(Qt::CTRL | (Qt::Key_1 + i)));
        actGotoKlammer[i]->setEnabled(false);
        connect(actGotoKlammer[i], &QAction::triggered, this, [this, i] { GotoKlammer(i); });
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
        connect(actSbGoto[i], &QAction::triggered, this, [this, i] { GotoKlammer(i); });
    }
    toolbar->addSeparator();
    toolbar->addAction(actOptionsLoom);
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::buildCentralArea()
{
    canvas = new SteuerungCanvas(this);
    scrollbar = new QScrollBar(Qt::Vertical, this);
    scrollbar->setEnabled(false); /* enabled by UpdateScrollbar */

    /*  Port of legacy scrollbarScroll (steuerung_form.cpp:377).
        scrolly == data-space row at bottom of viewport; scrollbar
        value is the inverted complement (MAXY2 - maxj - scrolly).
        Dragging scrolls the viewport AND advances weave_position
        so the visible "current schuss" tracks the scroll motion. */
    connect(scrollbar, &QScrollBar::valueChanged, this, [this](int _v) {
        if (!data)
            return;
        const int maxPos = std::max(0, data->MAXY2 - maxj);
        const int oldscrolly = scrolly;
        scrolly = maxPos - _v;
        if (scrolly < 0)
            scrolly = 0;
        if (scrolly != oldscrolly) {
            weave_position += (scrolly - oldscrolly);
            if (weave_position < 0)
                weave_position = 0;
            if (weave_position >= data->MAXY2)
                weave_position = data->MAXY2 - 1;
            UpdateStatusbar();
            if (canvas)
                canvas->update();
        }
    });
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
/*  --- Position / layout (ported from steuerung_pos.cpp +
    steuerung_form.cpp + steuerung_draw.cpp's CalcSizes / CalcTritte)  */
void TSTRGFRM::CalcSizes()
{
    gridsize = zoom[std::clamp(currentzoom, 0, 9)];
    if (gridsize < 1)
        gridsize = 1;

    /*  The canvas widget already excludes toolbar / menu / status
        chrome, so its width/height is the legacy (right-left) /
        (bottom-top). Scrollbar is a sibling widget and lives
        outside canvas->width().                                   */
    const int w = canvas ? canvas->width() : 0;
    const int h = canvas ? canvas->height() : 0;
    const int dx = gridsize;
    klammerwidth = MAXKLAMMERN * 11;

    maxi = (w - dx - klammerwidth) / gridsize;
    if (maxi < 0)
        maxi = 0;
    maxj = (h - 1) / gridsize;
    if (maxj < 0)
        maxj = 0;

    CalcTritte();
    if (trittCols == 0)
        trittCols = 8;
    if (maxi <= trittCols)
        trittCols = 0;
    x1 = maxi - trittCols - 1;
    if (x1 < 0)
        x1 = 0;

    UpdateScrollbar();
}

void TSTRGFRM::CalcTritte()
{
    trittCols = 0;
    if (!data)
        return;
    if (schlagpatrone) {
        if (!trittfolge)
            return;
        for (int i = data->MAXX2 - 1; i >= 0; i--)
            for (int j = 0; j < data->MAXY2; j++)
                if (trittfolge->feld.Get(i, j) > 0) {
                    trittCols = i + 1;
                    return;
                }
    } else {
        if (!aufknuepfung)
            return;
        const int mj = std::min(data->MAXY1 - 1, data->MAXX2 - 1);
        for (int j = mj; j >= 0; j--)
            for (int i = 0; i < data->MAXX2; i++)
                if (aufknuepfung->feld.Get(i, j) > 0) {
                    trittCols = j + 1;
                    return;
                }
    }
}

void TSTRGFRM::UpdateScrollbar()
{
    if (!scrollbar || !data)
        return;
    /*  Legacy scrollbar.Max == Data->MAXY2-maxj; Position ==
        MAXY2-maxj-scrolly. Maps cleanly to Qt's min/max/pageStep.
        We flip the visual direction so a high position == top of
        pattern, matching legacy's "drag down = weave forward".    */
    QSignalBlocker block(scrollbar);
    const int maxy = data->MAXY2;
    const int maxPos = std::max(0, maxy - maxj);
    scrollbar->setRange(0, maxPos);
    scrollbar->setPageStep(std::max(1, maxj));
    scrollbar->setSingleStep(1);
    /*  scrolly is the "data-space schuss at the bottom of view"
        offset. Scrollbar position is the inverted complement.     */
    if (scrolly < 0)
        scrolly = 0;
    if (scrolly > maxPos)
        scrolly = maxPos;
    scrollbar->setValue(maxPos - scrolly);
    scrollbar->setEnabled(true);
}

void TSTRGFRM::AutoScroll()
{
    if (!data || maxj <= 0)
        return;
    /*  Keep weave_position at least one-tenth of the view away
        from either edge; if not, recentre on weave_position.     */
    const int onetenth = maxj / 10;
    if (weave_position - scrolly > maxj - onetenth
        || weave_position - scrolly < onetenth) {
        int ideal = weave_position - maxj / 2;
        if (ideal < 0)
            ideal = 0;
        if (ideal > data->MAXY2 - maxj)
            ideal = std::max(0, data->MAXY2 - maxj);
        if (scrolly != ideal) {
            scrolly = ideal;
            UpdateScrollbar();
            if (canvas)
                canvas->update();
        }
    }
}

void TSTRGFRM::UpdateStatusbar()
{
    if (!labPosition || !labKlammer || !labRepetition)
        return;
    if (schussselected) {
        if (!IsValidWeavePosition()) {
            labPosition->setText(LANG_STR("(invalid) Weft ", "(ungültig) Schuss ")
                                 + QString::number(weave_position + 1));
            labKlammer->clear();
            labRepetition->clear();
        } else {
            labPosition->setText(LANG_STR("Weft ", "Schuss ") + QString::number(weave_position + 1));
            labKlammer->setText(LANG_STR("Klammer ", "Klammer ")
                                + QString::number(weave_klammer + 1));
            labRepetition->setText(LANG_STR("Rep ", "Wdh. ") + QString::number(weave_repetition));
        }
    } else {
        const Klammer& k = klammern[current_klammer];
        labPosition->setText(LANG_STR("Klammer ", "Klammer ")
                             + QString::number(current_klammer + 1));
        labKlammer->setText(QString::number(k.first + 1) + QStringLiteral("..")
                            + QString::number(k.last + 1) + QStringLiteral(" (")
                            + QString::number(k.last - k.first + 1) + QStringLiteral(")"));
        labRepetition->setText(LANG_STR("Rep ", "Wdh. ") + QString::number(k.repetitions));
    }
}

void TSTRGFRM::_ResetCurrentPos()
{
    for (int i = 0; i < MAXKLAMMERN; i++) {
        if (klammern[i].repetitions > 0) {
            weave_klammer = i;
            last_klammer = current_klammer = i;
            last_position = weave_position = klammern[i].first;
            last_repetition = weave_repetition = 1;
            return;
        }
    }
    weave_klammer = last_klammer = current_klammer = 0;
    weave_position = last_position = 0;
    weave_repetition = last_repetition = 1;
}

bool TSTRGFRM::IsValidWeavePosition() const
{
    if (weave_klammer < 0 || weave_klammer >= MAXKLAMMERN)
        return false;
    const Klammer& k = klammern[weave_klammer];
    if (k.repetitions == 0)
        return false;
    if (k.repetitions < weave_repetition)
        return false;
    if (k.first > weave_position)
        return false;
    if (k.last < weave_position)
        return false;
    return true;
}

void TSTRGFRM::ValidateWeavePosition()
{
    if (IsValidWeavePosition())
        return;
    /*  Try the remaining klammern in rotation. */
    for (int i = weave_klammer + 1; i <= weave_klammer + MAXKLAMMERN; i++) {
        int wk = ((i % MAXKLAMMERN) + MAXKLAMMERN) % MAXKLAMMERN;
        const Klammer& k = klammern[wk];
        if (k.repetitions > 0 && weave_position >= k.first && weave_position <= k.last) {
            weave_klammer = wk;
            weave_repetition = 1;
            return;
        }
    }
    weave_klammer = 0;
    weave_repetition = 0;
}

void TSTRGFRM::WeaveKlammerRight()
{
    weave_klammer++;
    while (weave_klammer < MAXKLAMMERN && klammern[weave_klammer].repetitions == 0)
        weave_klammer++;
    if (weave_klammer < 0 || weave_klammer >= MAXKLAMMERN)
        weave_klammer = 0;
    if (weave_repetition > klammern[weave_klammer].repetitions)
        weave_repetition = klammern[weave_klammer].repetitions;
    ValidateWeavePosition();
}

void TSTRGFRM::WeaveKlammerLeft()
{
    weave_klammer--;
    while (weave_klammer > 0 && klammern[weave_klammer].repetitions == 0)
        weave_klammer--;
    if (weave_klammer < 0 || weave_klammer >= MAXKLAMMERN)
        weave_klammer = 0;
    if (weave_repetition > klammern[weave_klammer].repetitions)
        weave_repetition = klammern[weave_klammer].repetitions;
    ValidateWeavePosition();
}

void TSTRGFRM::WeaveRepetitionInc()
{
    weave_repetition++;
    if (weave_repetition > klammern[weave_klammer].repetitions)
        weave_repetition = klammern[weave_klammer].repetitions;
    ValidateWeavePosition();
}

void TSTRGFRM::WeaveRepetitionDec()
{
    weave_repetition--;
    if (weave_repetition < 1)
        weave_repetition = 1;
    ValidateWeavePosition();
}

void TSTRGFRM::GotoKlammer(int _klammer)
{
    if (_klammer < 0 || _klammer >= MAXKLAMMERN)
        return;
    if (klammern[_klammer].repetitions == 0)
        return;
    weave_position = klammern[_klammer].first;
    weave_repetition = 1;
    weave_klammer = _klammer;
    current_klammer = _klammer;
    schussselected = true;
    AutoScroll();
    UpdateStatusbar();
    if (canvas)
        canvas->update();
}

void TSTRGFRM::GotoLastPosition()
{
    if (last_klammer < 0)
        return;
    weave_position = last_position;
    weave_klammer = last_klammer;
    weave_repetition = last_repetition;
    current_klammer = last_klammer;
    schussselected = true;
    AutoScroll();
    UpdateStatusbar();
    if (canvas)
        canvas->update();
}

void TSTRGFRM::UpdateLastPosition()
{
    last_position = weave_position;
    last_klammer = weave_klammer;
    last_repetition = weave_repetition;
}

void TSTRGFRM::refreshGotoActions()
{
    for (int i = 0; i < MAXKLAMMERN; i++) {
        const bool on = klammern[i].repetitions != 0;
        if (actGotoKlammer[i])
            actGotoKlammer[i]->setEnabled(on);
        if (actSbGoto[i])
            actSbGoto[i]->setEnabled(on);
    }
    if (actGotoLastPos)
        actGotoLastPos->setEnabled(last_klammer >= 0);
}

void TSTRGFRM::refresh()
{
    CalcSizes();
    UpdateStatusbar();
    refreshGotoActions();
    if (canvas)
        canvas->update();
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
