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
#include "patterncanvas.h"
#include "palettepanel.h"
#include "aboutdialog.h"
#include "techinfodialog.h"
#include "undoredo.h"
#include "rapport.h"
#include "einzug.h"
#include "cursor.h"
#include "datamodule.h"
#include "fileformat.h"
#include "assert_compat.h"

#include <QDockWidget>

#include <cstring>

#include <QActionGroup>
#include <QApplication>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

namespace {
/*  Load the dual-resolution (16 + 24 px) icon for `name`, which
    matches a basename under resources/icons/. Returns an empty
    QIcon if the resource isn't present -- used for menu entries
    whose legacy glyph was the "blank" placeholder.            */
QIcon legacyIcon(const char* name)
{
    QIcon icon;
    const QString s16 = QStringLiteral(":/icons/16/%1.png").arg(QLatin1String(name));
    const QString s24 = QStringLiteral(":/icons/24/%1.png").arg(QLatin1String(name));
    icon.addFile(s16, QSize(16, 16));
    icon.addFile(s24, QSize(24, 24));
    return icon;
}
}

TDBWFRM* DBWFRM = nullptr;

TDBWFRM::TDBWFRM(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("DB-WEAVE"));
    /*  Window sizing is left to main.cpp -- tests that probe rendered
        pixels need to control PatternCanvas's size directly, and a
        resize() here triggers the central-widget layout to stomp any
        subsequent explicit pattern_canvas->resize().               */

    /*  Create all menu-toggle actions up-front so the undo-item
        snapshotting logic sees non-null pointers. They are made
        checkable so ->isChecked()/->setChecked() semantics match
        the legacy TMenuItem. Exactly one einzug style and one
        trittfolge style is selected at any time; the legacy default
        is EzBelassen / TfBelassen.                                */
    auto mk = [this](bool checked = false) {
        QAction* a = new QAction(this);
        a->setCheckable(true);
        a->setChecked(checked);
        return a;
    };
    /*  Default einzug / trittfolge rearrangement style is MinimalZ
        (left-leaning minimum-shaft arrangement). Matches legacy:
        under EzBelassen / TfBelassen the Rearrange passes are
        no-ops, which leaves stale shafts / treadles behind after
        toggle-clears. The "Minimal" variants run MergeSchaefte /
        MergeTritte on every SetGewebe and keep the loom state
        clean.                                                    */
    EzMinimalZ = mk(/*checked=*/true);
    EzMinimalS = mk();
    EzGeradeZ = mk();
    EzGeradeS = mk();
    EzChorig2 = mk();
    EzChorig3 = mk();
    EzBelassen = mk();
    EzFixiert = mk();

    TfMinimalZ = mk(/*checked=*/true);
    TfMinimalS = mk();
    TfGeradeZ = mk();
    TfGeradeS = mk();
    TfGesprungen = mk();
    TfBelassen = mk();

    ViewSchlagpatrone = mk();
    ViewEinzug = mk(/*checked=*/true);
    ViewTrittfolge = mk(/*checked=*/true);
    ViewOnlyGewebe = mk();
    GewebeNone = mk();
    GewebeNormal = mk(/*checked=*/true);
    RappViewRapport = mk();
    GewebeFarbeffekt = mk();
    GewebeSimulation = mk();
    Inverserepeat = mk();
    ViewHlines = mk(/*checked=*/true);
    ViewFarbe = mk(/*checked=*/true);
    ViewBlatteinzug = mk(/*checked=*/true);

    /*  Allocate the per-shaft / per-treadle availability arrays. Both
        are initialised to true (all free); RecalcFreieSchaefte() /
        RecalcFreieTritte() repopulate them during pattern load and
        after editing operations. */
    freieschaefte = new bool[Data->MAXY1];
    freietritte = new bool[Data->MAXX2];
    for (int j = 0; j < Data->MAXY1; j++)
        freieschaefte[j] = true;
    for (int i = 0; i < Data->MAXX2; i++)
        freietritte[i] = true;

    /*  Scratch buffers for RcRecalcAll. */
    xbuf = new char[Data->MAXX1];
    ybuf = new char[Data->MAXY2];

    /*  Blockmuster undo ring. Allocated once per mainwindow; the
        Muster array it references lives as member state so no
        ownership transfer is needed. */
    blockundo = new BlockUndo(&blockmuster, currentbm);
    bereichundo = new BlockUndo(&bereichmuster, currentbm);

    /*  Page layout defaults (margins + header/footer). */
    InitBorders();
    std::memset(xbuf, 0, Data->MAXX1);
    std::memset(ybuf, 0, Data->MAXY2);

    /*  Seed a blank document with the same defaults the legacy
        Clear methods produce: blatteinzug -> 2-2 reed pattern,
        kettfarben / schussfarben -> default warp / weft colour.
        A subsequent file load overwrites these from disk.    */
    blatteinzug.Clear();
    kettfarben.Clear();
    schussfarben.Clear();

    /*  Undo stack is constructed after the fields so UrUndoItem's
        Allocate() can read field dimensions via `this`.            */
    undo = new UrUndo(this);
    rapporthandler = RpRapport::CreateInstance(this, Data);
    einzughandler = EinzugRearrange::CreateInstance(this, Data);
    cursorhandler = CrCursorHandler::CreateInstance(this, Data);
    file = new FfFile();

    /*  The pattern canvas is the central widget. Ownership is via Qt
        parent-child; Qt will delete it with the window. */
    pattern_canvas = new PatternCanvas(this, this);
    setCentralWidget(pattern_canvas);

    /*  Palette dock -- floats on the right by default. The
        enclosing QDockWidget owns the panel; ViewFarbpalette is a
        checkable QAction bound to toggleViewAction so the menu
        state stays in sync with the dock's visibility. */
    paletteDock = new QDockWidget(QStringLiteral("Palette"), this);
    paletteDock->setObjectName(QStringLiteral("paletteDock"));
    palettePanel = new PalettePanel(this, paletteDock);
    paletteDock->setWidget(palettePanel);
    addDockWidget(Qt::RightDockWidgetArea, paletteDock);
    ViewFarbpalette = paletteDock->toggleViewAction();
    ViewFarbpalette->setText(QStringLiteral("&Palette"));

    /*  Legacy menu structure -- see legacy/dbw3_form.dfm. Every
        caption, shortcut and icon mirrors the original VCL form so
        the new Qt application keeps the look and feel users know.
        Entries whose handlers haven't been ported yet appear as
        disabled stubs -- the menu layout is visually complete.   */

    auto mkAct = [this](const QString& text, const char* icon = nullptr) {
        QAction* a = new QAction(text, this);
        if (icon)
            a->setIcon(legacyIcon(icon));
        return a;
    };
    auto addStub = [this, &mkAct](QMenu* m, const QString& text, const char* icon = nullptr) {
        QAction* a = mkAct(text, icon);
        a->setEnabled(false);
        m->addAction(a);
        return a;
    };

    /*  ---------- &File (MenuDatei) --------------------------- */
    QMenu* fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
    QAction* actFileNew = addStub(fileMenu, QStringLiteral("&New"), "sb_new");
    actFileNew->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    addStub(fileMenu, QStringLiteral("New &from template..."));
    QAction* actOpen = mkAct(QStringLiteral("&Open..."), "sb_open");
    actOpen->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    fileMenu->addAction(actOpen);
    QAction* actLoadParts = mkAct(QStringLiteral("&Load parts..."));
    fileMenu->addAction(actLoadParts);
    QAction* actSave = mkAct(QStringLiteral("&Save"), "sb_save");
    actSave->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    fileMenu->addAction(actSave);
    QAction* actSaveAs = mkAct(QStringLiteral("Save &as..."));
    fileMenu->addAction(actSaveAs);
    addStub(fileMenu, QStringLiteral("Revert &changes"));
    fileMenu->addSeparator();
    QMenu* importMenu = fileMenu->addMenu(QStringLiteral("I&mport"));
    QAction* actImportWIF = importMenu->addAction(QStringLiteral("&WIF Datei..."));
    QMenu* exportMenu = fileMenu->addMenu(QStringLiteral("&Export"));
    QAction* actExportBmp = exportMenu->addAction(QStringLiteral("&Bitmap Datei..."));
    QAction* actExportWIF = exportMenu->addAction(QStringLiteral("&WIF Datei..."));
    addStub(exportMenu, QStringLiteral("&DB-WEAVE 3.5 Datei..."));
    fileMenu->addSeparator();
    QAction* actFileProps = mkAct(QStringLiteral("Propert&ies..."));
    fileMenu->addAction(actFileProps);
    fileMenu->addSeparator();
    QAction* actPageSetup = mkAct(QStringLiteral("Page se&tup..."));
    fileMenu->addAction(actPageSetup);
    QAction* actPreview = mkAct(QStringLiteral("Print pre&view"), "sb_preview");
    fileMenu->addAction(actPreview);
    QAction* actPrint = mkAct(QStringLiteral("&Print..."), "sb_print");
    actPrint->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    fileMenu->addAction(actPrint);
    QAction* actPrintRange = mkAct(QStringLiteral("Print &range..."));
    actPrintRange->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    actPrintRange->setEnabled(false);
    fileMenu->addAction(actPrintRange);
    addStub(fileMenu, QStringLiteral("Printer se&tup..."));
    fileMenu->addSeparator();
    /*  MRU 1..6 in-line (legacy style -- not a submenu). */
    mruMenu = nullptr;
    for (int i = 0; i < 6; i++) {
        QAction* a = new QAction(this);
        a->setVisible(false);
        fileMenu->addAction(a);
        mruActions[i] = a;
        connect(a, &QAction::triggered, this, [this, i] { OpenFromMRU(i); });
    }
    fileMenu->addSeparator();
    QAction* actQuit = mkAct(QStringLiteral("E&xit"));
    fileMenu->addAction(actQuit);

    connect(actOpen, &QAction::triggered, this, [this] { FileOpen(); });
    connect(actSave, &QAction::triggered, this, [this] { FileSave(); });
    connect(actSaveAs, &QAction::triggered, this, [this] { FileSaveAs(); });
    connect(actLoadParts, &QAction::triggered, this, [this] { LoadPartsClick(); });
    connect(actFileProps, &QAction::triggered, this, [this] { FilePropsClick(); });
    connect(actPageSetup, &QAction::triggered, this, [this] { FileSetPageClick(); });
    connect(actPrint, &QAction::triggered, this, [this] { FilePrintClick(); });
    connect(actPreview, &QAction::triggered, this, [this] { FilePrintpreviewClick(); });
    connect(actImportWIF, &QAction::triggered, this, [this] { ImportWIFClick(); });
    connect(actExportBmp, &QAction::triggered, this, [this] { DateiExportClick(); });
    connect(actExportWIF, &QAction::triggered, this, [this] { DateiExportClick(); });
    connect(actQuit, &QAction::triggered, this, &TDBWFRM::close);

    /*  ---------- &Edit (MenuBearbeiten) --------------------- */
    QMenu* editMenu = menuBar()->addMenu(QStringLiteral("&Edit"));
    QAction* actUndo = mkAct(QStringLiteral("&Undo"), "mn_undo");
    actUndo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
    editMenu->addAction(actUndo);
    QAction* actRedo = mkAct(QStringLiteral("&Redo"), "mn_redo");
    actRedo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
    editMenu->addAction(actRedo);
    editMenu->addSeparator();
    QAction* actCut = mkAct(QStringLiteral("Cu&t"), "mn_cut");
    actCut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));
    editMenu->addAction(actCut);
    QAction* actCopy = mkAct(QStringLiteral("&Copy"), "mn_copy");
    actCopy->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));
    editMenu->addAction(actCopy);
    QAction* actPaste = mkAct(QStringLiteral("&Paste"), "mn_paste");
    actPaste->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_V));
    editMenu->addAction(actPaste);
    QAction* actPasteTransparent = mkAct(QStringLiteral("P&aste transparently"));
    actPasteTransparent->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    editMenu->addAction(actPasteTransparent);
    editMenu->addSeparator();
    QAction* actInvert = mkAct(QStringLiteral("&Invert"), "sb_invert");
    actInvert->setShortcut(QKeySequence(Qt::Key_I));
    editMenu->addAction(actInvert);
    QAction* actMirrorH = mkAct(QStringLiteral("Mirror &horizontal"), "sb_mirrorh");
    actMirrorH->setShortcut(QKeySequence(Qt::Key_H));
    editMenu->addAction(actMirrorH);
    QAction* actMirrorV = mkAct(QStringLiteral("Mirror &vertical"), "sb_mirrorv");
    actMirrorV->setShortcut(QKeySequence(Qt::Key_V));
    editMenu->addAction(actMirrorV);
    QAction* actRotate = mkAct(QStringLiteral("Rotat&e"), "sb_rotate");
    actRotate->setShortcut(QKeySequence(Qt::Key_R));
    editMenu->addAction(actRotate);
    QAction* actCentralsym = mkAct(QStringLiteral("Ma&ke central symmetric"), "sb_centralsym");
    actCentralsym->setShortcut(QKeySequence(Qt::Key_Z));
    editMenu->addAction(actCentralsym);
    QAction* actDelete = mkAct(QStringLiteral("&Delete"));
    actDelete->setShortcut(QKeySequence(Qt::Key_Delete));
    editMenu->addAction(actDelete);

    QMenu* rollMenu = editMenu->addMenu(QStringLiteral("R&oll"));
    QAction* actRollUp = rollMenu->addAction(QStringLiteral("&Up"));
    actRollUp->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Up));
    QAction* actRollDown = rollMenu->addAction(QStringLiteral("&Down"));
    actRollDown->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Down));
    QAction* actRollLeft = rollMenu->addAction(QStringLiteral("&Left"));
    actRollLeft->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Left));
    QAction* actRollRight = rollMenu->addAction(QStringLiteral("&Right"));
    actRollRight->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Right));

    QMenu* slopeMenu = editMenu->addMenu(QStringLiteral("&Slope"));
    QAction* actSteigInc = slopeMenu->addAction(QStringLiteral("&Increase"));
    actSteigInc->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
    QAction* actSteigDec = slopeMenu->addAction(QStringLiteral("&Decrease"));
    actSteigDec->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_J));
    editMenu->addSeparator();

    QMenu* moveMenu = editMenu->addMenu(QStringLiteral("&Move"));
    QAction* actSchaftUp = moveMenu->addAction(QStringLiteral("Shaft &up"));
    actSchaftUp->setIcon(legacyIcon("sb_schaftup"));
    actSchaftUp->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F7));
    QAction* actSchaftDown = moveMenu->addAction(QStringLiteral("Shaft &down"));
    actSchaftDown->setIcon(legacyIcon("sb_schaftdown"));
    actSchaftDown->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F8));
    QAction* actTrittLeft = moveMenu->addAction(QStringLiteral("Treadle &left"));
    actTrittLeft->setIcon(legacyIcon("sb_trittleft"));
    actTrittLeft->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F5));
    QAction* actTrittRight = moveMenu->addAction(QStringLiteral("Treadle &right"));
    actTrittRight->setIcon(legacyIcon("sb_trittright"));
    actTrittRight->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F6));

    QMenu* insMenu = editMenu->addMenu(QStringLiteral("I&nsert"));
    QAction* actInsSchaft = insMenu->addAction(QStringLiteral("&Shaft"));
    actInsSchaft->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_S));
    QAction* actInsTritt = insMenu->addAction(QStringLiteral("&Treadle"));
    actInsTritt->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_T));
    QAction* actInsKette = insMenu->addAction(QStringLiteral("&End, Warp thread"));
    actInsKette->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_K));
    QAction* actInsSchuss = insMenu->addAction(QStringLiteral("&Pick, Weft thread"));
    actInsSchuss->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F));

    QMenu* delMenu = editMenu->addMenu(QStringLiteral("De&lete"));
    QAction* actDelSchaft = delMenu->addAction(QStringLiteral("&Shaft"));
    actDelSchaft->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    QAction* actDelTritt = delMenu->addAction(QStringLiteral("&Treadle"));
    actDelTritt->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
    QAction* actDelKette = delMenu->addAction(QStringLiteral("&End, Warp thread"));
    actDelKette->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_K));
    QAction* actDelSchuss = delMenu->addAction(QStringLiteral("&Pick, Weft thread"));
    actDelSchuss->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    editMenu->addSeparator();

    QMenu* lanceeMenu = editMenu->addMenu(QStringLiteral("La&ncee"));
    QAction* actKettLanc = lanceeMenu->addAction(QStringLiteral("&Warp lancee..."));
    QAction* actSchussLanc = lanceeMenu->addAction(QStringLiteral("W&eft lancee..."));

    QAction* actFillKoeper = mkAct(QStringLiteral("T&will completion"));
    actFillKoeper->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_K));
    editMenu->addAction(actFillKoeper);
    QAction* actSwapSide = mkAct(QStringLiteral("S&witch side"));
    actSwapSide->setShortcut(QKeySequence(Qt::Key_F11));
    editMenu->addAction(actSwapSide);

    connect(actUndo, &QAction::triggered, this, [this] {
        if (undo && undo->Undo())
            refresh();
    });
    connect(actRedo, &QAction::triggered, this, [this] {
        if (undo && undo->Redo())
            refresh();
    });
    connect(actCut, &QAction::triggered, this, [this] { CutSelection(); });
    connect(actCopy, &QAction::triggered, this, [this] { CopySelection(); });
    connect(actPaste, &QAction::triggered, this, [this] { PasteSelection(false); });
    connect(actPasteTransparent, &QAction::triggered, this, [this] { PasteSelection(true); });
    connect(actDelete, &QAction::triggered, this, [this] { DeleteSelection(); });
    connect(actInvert, &QAction::triggered, this, [this] { InvertSelection(); });
    connect(actMirrorH, &QAction::triggered, this, [this] { MirrorHorzSelection(); });
    connect(actMirrorV, &QAction::triggered, this, [this] { MirrorVertSelection(); });
    connect(actRotate, &QAction::triggered, this, [this] { RotateSelection(); });
    connect(actCentralsym, &QAction::triggered, this, [this] { CentralsymSelection(); });
    connect(actRollUp, &QAction::triggered, this, [this] { RollUpSelection(); });
    connect(actRollDown, &QAction::triggered, this, [this] { RollDownSelection(); });
    connect(actRollLeft, &QAction::triggered, this, [this] { RollLeftSelection(); });
    connect(actRollRight, &QAction::triggered, this, [this] { RollRightSelection(); });
    connect(actSteigInc, &QAction::triggered, this, [this] { SteigungInc(); });
    connect(actSteigDec, &QAction::triggered, this, [this] { SteigungDec(); });
    connect(actSchaftUp, &QAction::triggered, this, [this] { SchaftMoveUpClick(); });
    connect(actSchaftDown, &QAction::triggered, this, [this] { SchaftMoveDownClick(); });
    connect(actTrittLeft, &QAction::triggered, this, [this] { TrittMoveLeftClick(); });
    connect(actTrittRight, &QAction::triggered, this, [this] { TrittMoveRightClick(); });
    connect(actInsKette, &QAction::triggered, this, [this] { InsertKetteClick(); });
    connect(actInsSchuss, &QAction::triggered, this, [this] { InsertSchussClick(); });
    connect(actInsSchaft, &QAction::triggered, this, [this] { InsertSchaftClick(); });
    connect(actInsTritt, &QAction::triggered, this, [this] { InsertTrittClick(); });
    connect(actDelKette, &QAction::triggered, this, [this] { DeleteKetteClick(); });
    connect(actDelSchuss, &QAction::triggered, this, [this] { DeleteSchussClick(); });
    connect(actDelSchaft, &QAction::triggered, this, [this] { DeleteSchaftClick(); });
    connect(actDelTritt, &QAction::triggered, this, [this] { DeleteTrittClick(); });
    connect(actKettLanc, &QAction::triggered, this, [this] { KettLancierungClick(); });
    connect(actSchussLanc, &QAction::triggered, this, [this] { SchussLancierungClick(); });
    connect(actFillKoeper, &QAction::triggered, this, [this] { FillKoeper(); });
    connect(actSwapSide, &QAction::triggered, this, [this] { SwapSide(); });

    /*  ---------- &View (MenuAnsicht) ------------------------- */
    QMenu* viewMenu = menuBar()->addMenu(QStringLiteral("&View"));
    ViewBlatteinzug->setText(QStringLiteral("R&eed threading"));
    viewMenu->addAction(ViewBlatteinzug);
    ViewFarbe->setText(QStringLiteral("&Color"));
    ViewFarbe->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
    viewMenu->addAction(ViewFarbe);
    ViewEinzug->setText(QStringLiteral("&Threading"));
    ViewEinzug->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    viewMenu->addAction(ViewEinzug);
    ViewTrittfolge->setText(QStringLiteral("T&readling"));
    ViewTrittfolge->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    viewMenu->addAction(ViewTrittfolge);
    ViewHlines->setText(QStringLiteral("&Support lines"));
    viewMenu->addAction(ViewHlines);
    ViewOnlyGewebe->setText(QStringLiteral("&Only pattern visible"));
    viewMenu->addAction(ViewOnlyGewebe);
    viewMenu->addSeparator();
    QMenu* gewebeMenu = viewMenu->addMenu(QStringLiteral("&View"));
    auto* gewebeGroup = new QActionGroup(this);
    gewebeGroup->setExclusive(true);
    GewebeNone->setText(QStringLiteral("&Invisible"));
    GewebeNone->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Slash));
    GewebeNormal->setText(QStringLiteral("&Draft"));
    GewebeNormal->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));
    GewebeFarbeffekt->setText(QStringLiteral("&Color"));
    GewebeFarbeffekt->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_2));
    GewebeSimulation->setText(QStringLiteral("&Fabric"));
    GewebeSimulation->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_3));
    for (QAction* a : { GewebeNone, GewebeNormal, GewebeFarbeffekt, GewebeSimulation }) {
        a->setCheckable(true);
        gewebeGroup->addAction(a);
        gewebeMenu->addAction(a);
    }
    viewMenu->addSeparator();
    QAction* actZoomIn = mkAct(QStringLiteral("&Zoom in"), "sb_zoomin");
    actZoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    viewMenu->addAction(actZoomIn);
    QAction* actZoomNormal = mkAct(QStringLiteral("&Normal"));
    viewMenu->addAction(actZoomNormal);
    QAction* actZoomOut = mkAct(QStringLiteral("Zoo&m out"), "sb_zoomout");
    actZoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_U));
    viewMenu->addAction(actZoomOut);
    viewMenu->addSeparator();
    QAction* actToolPalette = addStub(viewMenu, QStringLiteral("Tool pa&lette"));
    actToolPalette->setShortcut(QKeySequence(Qt::Key_F10));
    QAction* actRedraw = addStub(viewMenu, QStringLiteral("Re&draw"));
    actRedraw->setShortcut(QKeySequence(Qt::Key_F5));
    QAction* actOverview = addStub(viewMenu, QStringLiteral("Overv&iew"));
    actOverview->setShortcut(QKeySequence(Qt::Key_F4));

    connect(actZoomIn, &QAction::triggered, this, [this] { zoomIn(); });
    connect(actZoomOut, &QAction::triggered, this, [this] { zoomOut(); });
    connect(actZoomNormal, &QAction::triggered, this, [this] { zoomNormal(); });
    auto relayout = [this] {
        if (pattern_canvas)
            pattern_canvas->recomputeLayout();
        refresh();
    };
    for (QAction* a : { ViewEinzug, ViewTrittfolge, ViewBlatteinzug, ViewFarbe, ViewHlines,
                        ViewOnlyGewebe })
        connect(a, &QAction::triggered, this, relayout);
    for (QAction* a : { GewebeNormal, GewebeFarbeffekt, GewebeSimulation, GewebeNone })
        connect(a, &QAction::triggered, this, [this] { refresh(); });

    /*  ---------- &Threading (MenuEinzug) -------------------- */
    QMenu* threadMenu = menuBar()->addMenu(QStringLiteral("&Threading"));
    QAction* actEzAssist = threadMenu->addAction(QStringLiteral("Threading assistant..."));
    threadMenu->addSeparator();
    QAction* actEzSpiegeln = threadMenu->addAction(QStringLiteral("&Mirror"));
    QAction* actClearEinzug = threadMenu->addAction(QStringLiteral("&Delete"));
    threadMenu->addSeparator();
    auto* ezGroup = new QActionGroup(this);
    ezGroup->setExclusive(true);
    EzMinimalZ->setText(QStringLiteral("Normal &rising"));
    EzMinimalS->setText(QStringLiteral("Normal &falling"));
    EzGeradeZ->setText(QStringLiteral("&Straight rising"));
    EzGeradeS->setText(QStringLiteral("S&traight falling"));
    EzChorig2->setText(QStringLiteral("&2 choirs"));
    EzChorig3->setText(QStringLiteral("&3 choirs"));
    EzBelassen->setText(QStringLiteral("Fi&xed"));
    EzFixiert->setText(QStringLiteral("&User defined..."));
    for (QAction* a : { EzMinimalZ, EzMinimalS, EzGeradeZ, EzGeradeS, EzChorig2, EzChorig3,
                        EzBelassen, EzFixiert }) {
        ezGroup->addAction(a);
        threadMenu->addAction(a);
    }
    threadMenu->addSeparator();
    QAction* actCopyEzTf = threadMenu->addAction(QStringLiteral("&Copy from treadling"));
    connect(actEzAssist, &QAction::triggered, this, [this] { EinzugAssistentClick(); });
    connect(actEzSpiegeln, &QAction::triggered, this, [this] {
        /*  Legacy EzSpiegeln: mirror entire threading -- not yet
            ported as a standalone op. Stubbed for now.          */
    });
    actEzSpiegeln->setEnabled(false);
    connect(actClearEinzug, &QAction::triggered, this, [this] { _ClearEinzug(); refresh(); });
    connect(actCopyEzTf, &QAction::triggered, this, [this] { CopyEinzugTrittfolgeClick(); });
    connect(EzFixiert, &QAction::triggered, this, [this] { EditFixeinzug(); });

    /*  ---------- Tre&adling (MenuTrittfolge) ---------------- */
    QMenu* treadMenu = menuBar()->addMenu(QStringLiteral("Tre&adling"));
    QAction* actTfSpiegeln = treadMenu->addAction(QStringLiteral("&Mirror"));
    QAction* actClearTf = treadMenu->addAction(QStringLiteral("&Delete"));
    treadMenu->addSeparator();
    auto* tfGroup = new QActionGroup(this);
    tfGroup->setExclusive(true);
    TfMinimalZ->setText(QStringLiteral("Normal &rising"));
    TfMinimalS->setText(QStringLiteral("Normal &falling"));
    TfGeradeZ->setText(QStringLiteral("&Straight rising"));
    TfGeradeS->setText(QStringLiteral("S&traight falling"));
    TfGesprungen->setText(QStringLiteral("Cr&ossed"));
    TfBelassen->setText(QStringLiteral("Fi&xed"));
    for (QAction* a : { TfMinimalZ, TfMinimalS, TfGeradeZ, TfGeradeS, TfGesprungen, TfBelassen }) {
        tfGroup->addAction(a);
        treadMenu->addAction(a);
    }
    treadMenu->addSeparator();
    QAction* actCopyTfEz = treadMenu->addAction(QStringLiteral("&Copy from threading"));
    connect(actTfSpiegeln, &QAction::triggered, this, [this] { TfSpiegelnClick(); });
    connect(actClearTf, &QAction::triggered, this, [this] { ClearTrittfolgeClick(); });
    connect(actCopyTfEz, &QAction::triggered, this, [this] { CopyTrittfolgeEinzugClick(); });

    /*  ---------- &Pegplan (MenuSchlagpatrone) --------------- */
    QMenu* spMenu = menuBar()->addMenu(QStringLiteral("&Pegplan"));
    QAction* actSpInvert = spMenu->addAction(QStringLiteral("&Invert"));
    actSpInvert->setIcon(legacyIcon("sb_invert"));
    QAction* actClearSp = spMenu->addAction(QStringLiteral("&Delete"));
    QAction* actSpSpiegeln = spMenu->addAction(QStringLiteral("&Mirror"));
    actSpSpiegeln->setIcon(legacyIcon("sb_mirrorv"));
    connect(actSpInvert, &QAction::triggered, this, [this] { SpInvert(); });
    connect(actClearSp, &QAction::triggered, this, [this] { ClearSchlagpatroneClick(); });
    connect(actSpSpiegeln, &QAction::triggered, this, [this] { SpSpiegelnClick(); });

    /*  ---------- Tie-&up (MenuAufknuepfung) ----------------- */
    QMenu* aufMenu = menuBar()->addMenu(QStringLiteral("Tie-&up"));
    QAction* actAufInvert = aufMenu->addAction(QStringLiteral("&Invert"));
    actAufInvert->setIcon(legacyIcon("sb_invert"));
    actAufInvert->setEnabled(false);
    QAction* actClearAuf = aufMenu->addAction(QStringLiteral("&Delete"));
    actClearAuf->setEnabled(false);
    QMenu* aufRollMenu = aufMenu->addMenu(QStringLiteral("&Roll"));
    QAction* actAufRollUp = aufRollMenu->addAction(QStringLiteral("&Up"));
    actAufRollUp->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Up));
    actAufRollUp->setEnabled(false);
    QAction* actAufRollDown = aufRollMenu->addAction(QStringLiteral("&Down"));
    actAufRollDown->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Down));
    actAufRollDown->setEnabled(false);
    QAction* actAufRollLeft = aufRollMenu->addAction(QStringLiteral("&Left"));
    actAufRollLeft->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Left));
    actAufRollLeft->setEnabled(false);
    QAction* actAufRollRight = aufRollMenu->addAction(QStringLiteral("&Right"));
    actAufRollRight->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Right));
    actAufRollRight->setEnabled(false);
    QMenu* aufSlopeMenu = aufMenu->addMenu(QStringLiteral("&Slope"));
    QAction* actAufSlopeInc = aufSlopeMenu->addAction(QStringLiteral("&Increase"));
    actAufSlopeInc->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
    actAufSlopeInc->setEnabled(false);
    QAction* actAufSlopeDec = aufSlopeMenu->addAction(QStringLiteral("&Decrease"));
    actAufSlopeDec->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_J));
    actAufSlopeDec->setEnabled(false);
    QAction* actAufZentralsymm = aufMenu->addAction(QStringLiteral("Ma&ke central symmetric"));
    actAufZentralsymm->setIcon(legacyIcon("sb_centralsym"));
    actAufZentralsymm->setEnabled(false);

    /*  ---------- &Insert (MenuEinfuegen) -------------------- */
    QMenu* insertMenu = menuBar()->addMenu(QStringLiteral("&Insert"));
    QMenu* atlasMenu = insertMenu->addMenu(QStringLiteral("&Satin"));
    for (int n = 5; n <= 10; n++) {
        QAction* a = atlasMenu->addAction(QStringLiteral("Satin &%1").arg(n));
        connect(a, &QAction::triggered, this, [this, n] { AtlasEinfuegen(n); });
    }
    QMenu* koepMenu = insertMenu->addMenu(QStringLiteral("&Twill"));
    QMenu* koepBalMenu = koepMenu->addMenu(QStringLiteral("&Balanced twill"));
    auto addKoeperItem = [this](QMenu* m, const QString& label, int h, int s) {
        QAction* a = m->addAction(label);
        connect(a, &QAction::triggered, this, [this, h, s] { KoeperEinfuegen(h, s); });
        return a;
    };
    addKoeperItem(koepBalMenu, QStringLiteral("Twill &2/2"), 2, 2);
    addKoeperItem(koepBalMenu, QStringLiteral("Twill &3/3"), 3, 3);
    addKoeperItem(koepBalMenu, QStringLiteral("Twill &4/4"), 4, 4);
    addKoeperItem(koepBalMenu, QStringLiteral("Twill &5/5"), 5, 5);
    QMenu* kettkoepMenu = koepMenu->addMenu(QStringLiteral("&Warp twill"));
    addKoeperItem(kettkoepMenu, QStringLiteral("&1  Twill 2/1"), 2, 1);
    addKoeperItem(kettkoepMenu, QStringLiteral("&2  Twill 3/1"), 3, 1);
    addKoeperItem(kettkoepMenu, QStringLiteral("&3  Twill 4/1"), 4, 1);
    addKoeperItem(kettkoepMenu, QStringLiteral("&4  Twill 5/1"), 5, 1);
    addKoeperItem(kettkoepMenu, QStringLiteral("&5  Twill 3/2"), 3, 2);
    addKoeperItem(kettkoepMenu, QStringLiteral("&6  Twill 4/2"), 4, 2);
    addKoeperItem(kettkoepMenu, QStringLiteral("&7  Twill 5/2"), 5, 2);
    addKoeperItem(kettkoepMenu, QStringLiteral("&8  Twill 4/3"), 4, 3);
    addKoeperItem(kettkoepMenu, QStringLiteral("&9  Twill 5/3"), 5, 3);

    /*  ---------- &Additional (MenuWeitere) ------------------ */
    QMenu* additionalMenu = menuBar()->addMenu(QStringLiteral("&Additional"));
    MenuWeitere = additionalMenu;
    for (int i = 0; i < MAXUSERDEF; i++) {
        UserdefAct[i] = additionalMenu->addAction(QString());
        UserdefAct[i]->setVisible(false);
        connect(UserdefAct[i], &QAction::triggered, this, [this, i]() {
            const bool transp = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
            InsertUserdef(i, transp);
        });
    }
    additionalMenu->addSeparator();
    QMenu* udCfg = additionalMenu->addMenu(QStringLiteral("&Configuration"));
    QAction* actUdAdd = udCfg->addAction(QStringLiteral("&Add current pattern..."));
    QAction* actUdAddSel = udCfg->addAction(QStringLiteral("A&dd selection..."));
    QAction* actUdDel = udCfg->addAction(QStringLiteral("&Remove..."));
    connect(actUdAdd, &QAction::triggered, this, [this] { UserdefAddClick(); });
    connect(actUdAddSel, &QAction::triggered, this, [this] { UserdefAddSelClick(); });
    connect(actUdDel, &QAction::triggered, this, [this] { UserdefRemoveClick(); });

    /*  ---------- &Repeat (MenuRapport) ---------------------- */
    QMenu* repeatMenu = menuBar()->addMenu(QStringLiteral("&Repeat"));
    RappViewRapport->setText(QStringLiteral("&Repeat visible"));
    RappViewRapport->setIcon(legacyIcon("mn_repeatvisible"));
    RappViewRapport->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    repeatMenu->addAction(RappViewRapport);
    repeatMenu->addSeparator();
    QAction* actRappExtend = mkAct(QStringLiteral("Repe&at..."), "mn_repeat");
    actRappExtend->setShortcut(QKeySequence(Qt::Key_F8));
    repeatMenu->addAction(actRappExtend);
    QAction* actRappReduce = mkAct(QStringLiteral("R&educe"));
    actRappReduce->setShortcut(QKeySequence(Qt::Key_F7));
    repeatMenu->addAction(actRappReduce);
    QAction* actRappOverride = mkAct(QStringLiteral("&Set size to selection"));
    repeatMenu->addAction(actRappOverride);
    Inverserepeat->setText(QStringLiteral("&Invert repeat colors"));
    Inverserepeat->setIcon(legacyIcon("mn_inverserepeat"));
    repeatMenu->addAction(Inverserepeat);
    connect(actRappExtend, &QAction::triggered, this, [this] { RappRapportierenClick(); });
    connect(actRappReduce, &QAction::triggered, this, [this] { RappReduzierenClick(); });
    connect(actRappOverride, &QAction::triggered, this, [this] { RappOverrideClick(); });
    connect(RappViewRapport, &QAction::triggered, this, relayout);
    connect(Inverserepeat, &QAction::triggered, this, [this] { refresh(); });

    /*  ---------- &Color (MenuFarbe) ------------------------- */
    QMenu* colorMenu = menuBar()->addMenu(QStringLiteral("&Color"));
    if (ViewFarbpalette) {
        ViewFarbpalette->setText(QStringLiteral("&Palette"));
        ViewFarbpalette->setIcon(legacyIcon("mn_palette"));
        ViewFarbpalette->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
        colorMenu->addAction(ViewFarbpalette);
    }
    QAction* actDefColors = mkAct(QStringLiteral("Edit &colors..."));
    colorMenu->addAction(actDefColors);
    QAction* actFarbverlauf = mkAct(QStringLiteral("Color &blending..."), "mn_colorblend");
    colorMenu->addAction(actFarbverlauf);
    colorMenu->addSeparator();
    QAction* actSetKettf = mkAct(QStringLiteral("Set &warp color..."));
    colorMenu->addAction(actSetKettf);
    QAction* actSetSchuf = mkAct(QStringLiteral("Set w&eft color..."));
    colorMenu->addAction(actSetSchuf);
    QAction* actReplColor = mkAct(QStringLiteral("&Replace color..."), "mn_colorreplace");
    colorMenu->addAction(actReplColor);
    colorMenu->addSeparator();
    QAction* actKwieS = mkAct(QStringLiteral("C&opy warp colors from weft colors"));
    colorMenu->addAction(actKwieS);
    QAction* actSwieK = mkAct(QStringLiteral("Copy we&ft colors from warp colors"));
    colorMenu->addAction(actSwieK);
    QAction* actSwapCols = mkAct(QStringLiteral("&Switch warp/weft colors"));
    colorMenu->addAction(actSwapCols);
    connect(actDefColors, &QAction::triggered, this, [this] { DefineColorsClick(); });
    connect(actFarbverlauf, &QAction::triggered, this, [this] { FarbverlaufClick(); });
    connect(actSetKettf, &QAction::triggered, this, [this] { SetKettfarbeClick(); });
    connect(actSetSchuf, &QAction::triggered, this, [this] { SetSchussfarbeClick(); });
    connect(actReplColor, &QAction::triggered, this, [this] { ReplaceColorClick(); });
    connect(actSwapCols, &QAction::triggered, this, [this] { SwitchColorsClick(); });
    actKwieS->setEnabled(false);
    actSwieK->setEnabled(false);

    /*  ---------- Ra&nge (MenuBereich) ----------------------- */
    QMenu* rangeMenu = menuBar()->addMenu(QStringLiteral("Ra&nge"));
    QAction* actImportBmp = rangeMenu->addAction(QStringLiteral("Import &bitmap..."));
    QMenu* currentRangeMenu = rangeMenu->addMenu(QStringLiteral("&Current range"));
    auto* rangeMenuGroup = new QActionGroup(this);
    rangeMenuGroup->setExclusive(true);
    QAction* menuRangeActs[9];
    for (int r = 1; r <= 9; r++) {
        QAction* a = currentRangeMenu->addAction(QStringLiteral("Range &%1").arg(r));
        a->setCheckable(true);
        a->setShortcut(QKeySequence(Qt::SHIFT | (Qt::Key_0 + r)));
        rangeMenuGroup->addAction(a);
        menuRangeActs[r - 1] = a;
        connect(a, &QAction::triggered, this, [this, r] {
            currentrange = r;
            if (selection.Valid())
                ApplyRangeToSelection(r);
            refresh();
        });
    }
    if (currentrange >= 1 && currentrange <= 9)
        menuRangeActs[currentrange - 1]->setChecked(true);
    currentRangeMenu->addSeparator();
    QAction* actRAush = currentRangeMenu->addAction(QStringLiteral("&Lift out"));
    actRAush->setCheckable(true);
    actRAush->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_0));
    QAction* actRAnb = currentRangeMenu->addAction(QStringLiteral("&Binding"));
    actRAnb->setCheckable(true);
    actRAnb->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    QAction* actRAbb = currentRangeMenu->addAction(QStringLiteral("&Un-binding"));
    actRAbb->setCheckable(true);
    actRAbb->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_0));
    rangeMenuGroup->addAction(actRAush);
    rangeMenuGroup->addAction(actRAnb);
    rangeMenuGroup->addAction(actRAbb);
    connect(actRAush, &QAction::triggered, this, [this] { currentrange = AUSHEBUNG; refresh(); });
    connect(actRAnb, &QAction::triggered, this, [this] { currentrange = ANBINDUNG; refresh(); });
    connect(actRAbb, &QAction::triggered, this, [this] { currentrange = ABBINDUNG; refresh(); });
    QAction* actBlockmuster = rangeMenu->addAction(QStringLiteral("&Block substitution..."));
    QAction* actRangeSubst = rangeMenu->addAction(QStringLiteral("&Range substitution..."));
    connect(actImportBmp, &QAction::triggered, this, [this] { ImportBitmapClick(); });
    connect(actBlockmuster, &QAction::triggered, this, [this] { EditBlockmusterClick(); });
    connect(actRangeSubst, &QAction::triggered, this, [this] { RangePatternsClick(); });

    /*  ---------- E&xtras (MenuExtras) ----------------------- */
    QMenu* extrasMenu = menuBar()->addMenu(QStringLiteral("E&xtras"));
    QAction* actLockGewebe = addStub(extrasMenu, QStringLiteral("&Lock pattern"));
    Q_UNUSED(actLockGewebe);
    QMenu* cursorMenu = extrasMenu->addMenu(QStringLiteral("&Cursor"));
    QAction* actCursorLocked = cursorMenu->addAction(QStringLiteral("&Contain"));
    actCursorLocked->setCheckable(true);
    actCursorLocked->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    actCursorLocked->setEnabled(false);
    QAction* actCursorDir = cursorMenu->addAction(QStringLiteral("&Movement..."));
    QAction* actCursorGoto = cursorMenu->addAction(QStringLiteral("&Goto..."));
    actCursorGoto->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    connect(actCursorDir, &QAction::triggered, this, [this] { EditCursordirectionClick(); });
    connect(actCursorGoto, &QAction::triggered, this, [this] { CursorGotoClick(); });
    QAction* actVerhaeltnis = addStub(extrasMenu, QStringLiteral("Weft/warp &ratio..."));
    Q_UNUSED(actVerhaeltnis);
    ViewSchlagpatrone->setText(QStringLiteral("&Pegplan"));
    ViewSchlagpatrone->setShortcut(QKeySequence(Qt::Key_F9));
    extrasMenu->addAction(ViewSchlagpatrone);
    connect(ViewSchlagpatrone, &QAction::triggered, this, relayout);
    QAction* actWeave = addStub(extrasMenu, QStringLiteral("&Weave"));
    actWeave->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));
    QAction* actViewInfos = extrasMenu->addAction(QStringLiteral("&Information..."));
    connect(actViewInfos, &QAction::triggered, this, [this] { ViewInfosClick(); });
    extrasMenu->addSeparator();
    QMenu* baseMenu = extrasMenu->addMenu(QStringLiteral("&Base settings"));
    addStub(baseMenu, QStringLiteral("&American"));
    addStub(baseMenu, QStringLiteral("S&candinavian"));
    addStub(baseMenu, QStringLiteral("&German/Swiss"));
    QMenu* optMenu = extrasMenu->addMenu(QStringLiteral("&Options"));
    QAction* actXOpt = optMenu->addAction(QStringLiteral("&For this pattern..."));
    QAction* actXOptG = optMenu->addAction(QStringLiteral("F&or every new pattern..."));
    optMenu->addSeparator();
    QAction* actEnvOpt = optMenu->addAction(QStringLiteral("&Environment..."));
    connect(actXOpt, &QAction::triggered, this, [this] { XOptionsClick(); });
    connect(actXOptG, &QAction::triggered, this, [this] { XOptionsGlobalClick(); });
    connect(actEnvOpt, &QAction::triggered, this, [this] { OptEnvironmentClick(); });

    /*  ---------- &? (MenuHelp) ------------------------------ */
    QMenu* helpMenu = menuBar()->addMenu(QStringLiteral("&?"));
    QAction* actHilfe = addStub(helpMenu, QStringLiteral("&Help topics"));
    actHilfe->setShortcut(QKeySequence(Qt::Key_F1));
    addStub(helpMenu, QStringLiteral("&Brunold Software Online"));
    helpMenu->addSeparator();
    QAction* actTechInfo = helpMenu->addAction(QStringLiteral("&Technical information..."));
    QAction* actAbout = helpMenu->addAction(QStringLiteral("About &DB-WEAVE..."));
    connect(actTechInfo, &QAction::triggered, this, [this] { TechinfoDialog(this).exec(); });
    connect(actAbout, &QAction::triggered, this, [this] { AboutDialog(this).exec(); });

    /*  ---------- Toolbar (legacy TToolBar) ------------------
        24 buttons matching legacy/dbw3_form.dfm's toolbar layout:
        File (New/Open/Save), Print (Preview/Print),
        History (Undo/Redo), Clipboard (Cut/Copy/Paste),
        Zoom (In/Out), Move (ShaftUp/Down, TrittLeft/Right),
        Transforms (Invert/MirrorH/MirrorV/Rotate/Centralsym/
        Highlight).                                             */
    QToolBar* mainBar = addToolBar(QStringLiteral("Toolbar"));
    mainBar->setObjectName(QStringLiteral("mainToolBar"));
    mainBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mainBar->setIconSize(QSize(16, 16));
    mainBar->addAction(actFileNew);
    mainBar->addAction(actOpen);
    mainBar->addAction(actSave);
    mainBar->addSeparator();
    mainBar->addAction(actPreview);
    mainBar->addAction(actPrint);
    mainBar->addSeparator();
    mainBar->addAction(actUndo);
    mainBar->addAction(actRedo);
    mainBar->addSeparator();
    mainBar->addAction(actCut);
    mainBar->addAction(actCopy);
    mainBar->addAction(actPaste);
    mainBar->addSeparator();
    mainBar->addAction(actZoomIn);
    mainBar->addAction(actZoomOut);
    mainBar->addSeparator();
    mainBar->addAction(actSchaftUp);
    mainBar->addAction(actSchaftDown);
    mainBar->addAction(actTrittLeft);
    mainBar->addAction(actTrittRight);
    mainBar->addSeparator();
    mainBar->addAction(actInvert);
    mainBar->addAction(actMirrorH);
    mainBar->addAction(actMirrorV);
    mainBar->addAction(actRotate);
    mainBar->addAction(actCentralsym);
    QAction* actHighlight = new QAction(legacyIcon("sb_highlight"), QStringLiteral("Highlight"),
                                        this);
    actHighlight->setShortcut(QKeySequence(Qt::Key_F12));
    actHighlight->setToolTip(QStringLiteral("Highlight (F12) -- Highlights the dependent parts"));
    actHighlight->setEnabled(false);
    mainBar->addAction(actHighlight);

    /*  Range picker: nine checkable buttons driving currentrange.
        Lives on its own toolbar so it can be hidden independently. */
    QToolBar* rangeBar = addToolBar(QStringLiteral("Ranges"));
    rangeBar->setObjectName(QStringLiteral("rangeToolBar"));
    rangeGroup = new QActionGroup(this);
    rangeGroup->setExclusive(true);
    for (int r = 1; r <= 9; r++) {
        QAction* a = new QAction(QString::number(r), this);
        a->setCheckable(true);
        a->setToolTip(QStringLiteral("Range %1").arg(r));
        rangeGroup->addAction(a);
        rangeBar->addAction(a);
        rangeActions[r - 1] = a;
        connect(a, &QAction::triggered, this, [this, r] {
            currentrange = r;
            if (selection.Valid())
                ApplyRangeToSelection(r);
            refresh();
        });
    }
    if (currentrange >= 1 && currentrange <= 9)
        rangeActions[currentrange - 1]->setChecked(true);
    rangeBar->addSeparator();
    auto addSpecial = [&](const QString& label, int r, const QString& tip) {
        QAction* a = new QAction(label, this);
        a->setCheckable(true);
        a->setToolTip(tip);
        rangeGroup->addAction(a);
        rangeBar->addAction(a);
        connect(a, &QAction::triggered, this, [this, r] {
            currentrange = r;
            refresh();
        });
    };
    addSpecial(QStringLiteral("L"), AUSHEBUNG, QStringLiteral("Lift out (Aushebung)"));
    addSpecial(QStringLiteral("B"), ANBINDUNG, QStringLiteral("Binding (Anbindung)"));
    addSpecial(QStringLiteral("U"), ABBINDUNG, QStringLiteral("Unbinding (Abbindung)"));

    /*  Loom menu — Phase 11 dummy simulator (not part of the
        legacy top-level menu but kept so the feature remains
        reachable while a more complete port is pending).    */
    QMenu* loomMenu = menuBar()->addMenu(QStringLiteral("&Loom"));
    QAction* actLoomControl = loomMenu->addAction(QStringLiteral("&Loom control..."));
    connect(actLoomControl, &QAction::triggered, this, [this] { LoomControlClick(); });

    /*  Drawing-tools menu (no legacy counterpart -- the old UI
        uses a floating tool palette). Kept here for now.       */
    QMenu* toolsMenu = menuBar()->addMenu(QStringLiteral("&Tools"));
    auto* toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);
    auto addTool = [&](const QString& label, const char* icon, TOOL _t) {
        QAction* a = toolsMenu->addAction(legacyIcon(icon), label);
        a->setCheckable(true);
        toolGroup->addAction(a);
        if (_t == TOOL_POINT)
            a->setChecked(true);
        connect(a, &QAction::triggered, this, [this, _t] { tool = _t; });
    };
    addTool(QStringLiteral("&Point"), "tool_cursor", TOOL_POINT);
    addTool(QStringLiteral("&Line"), "tool_line", TOOL_LINE);
    addTool(QStringLiteral("&Rectangle"), "tool_rectangle", TOOL_RECTANGLE);
    addTool(QStringLiteral("&Filled rectangle"), "tool_filledrectangle", TOOL_FILLEDRECTANGLE);
    addTool(QStringLiteral("&Ellipse"), "tool_ellipse", TOOL_ELLIPSE);
    addTool(QStringLiteral("Fille&d ellipse"), "tool_filledellipse", TOOL_FILLEDELLIPSE);

    /*  Status-bar panels (right-aligned permanent widgets). */
    sbField = new QLabel(this);
    sbSelect = new QLabel(this);
    sbRange = new QLabel(this);
    sbRapport = new QLabel(this);
    sbZoom = new QLabel(this);
    statusBar()->addWidget(sbField, 1);
    statusBar()->addWidget(sbSelect, 1);
    statusBar()->addPermanentWidget(sbRange);
    statusBar()->addPermanentWidget(sbRapport);
    statusBar()->addPermanentWidget(sbZoom);
    UpdateStatusBar();

    /*  Populate the recent-files submenu from QSettings. */
    LoadMRU();
    LoadUserdefMenu();

    /*  Cursor blink. QApplication::cursorFlashTime is the total
        flash period in ms; fire the timer at half that so one tick
        corresponds to a show/hide flip. */
    const int flash = QApplication::cursorFlashTime();
    if (flash > 0) {
        cursorTimer = new QTimer(this);
        cursorTimer->setInterval(flash / 2);
        connect(cursorTimer, &QTimer::timeout, this, [this] {
            cursorVisible = !cursorVisible;
            if (pattern_canvas)
                pattern_canvas->update();
        });
        cursorTimer->start();
    }
}

TDBWFRM::~TDBWFRM()
{
    CrCursorHandler::Release(cursorhandler);
    cursorhandler = nullptr;
    EinzugRearrange::ReleaseInstance(einzughandler);
    einzughandler = nullptr;
    RpRapport::ReleaseInstance(rapporthandler);
    rapporthandler = nullptr;
    delete undo;
    delete file;
    file = nullptr;
    delete[] freieschaefte;
    freieschaefte = nullptr;
    delete[] freietritte;
    freietritte = nullptr;
    delete[] xbuf;
    xbuf = nullptr;
    delete[] ybuf;
    ybuf = nullptr;
    delete[] fixeinzug;
    fixeinzug = nullptr;
    delete blockundo;
    blockundo = nullptr;
    delete bereichundo;
    bereichundo = nullptr;
    /*  QAction members are owned by `this` via QObject parenting. */
}

void TDBWFRM::refresh()
{
    /*  Reset the cursor blink so it's visible immediately after any
        mutation (cursor move, paint op, zoom, ...) rather than
        appearing off mid-flash. */
    cursorVisible = true;
    if (cursorTimer)
        cursorTimer->start();

    /*  Repaint the pattern canvas. QWidget::update() on the main
        window only invalidates the window chrome; children paint
        independently in Qt. */
    if (pattern_canvas)
        pattern_canvas->update();
    /*  Rebuild the status-bar labels so the cursor position, range,
        selection size and zoom stay current on every refresh.     */
    UpdateStatusBar();
    /*  Keep the range-picker toolbar in sync with currentrange. */
    if (rangeActions[0]) {
        const int r = currentrange;
        for (int i = 0; i < 9; i++)
            rangeActions[i]->setChecked(r == i + 1);
    }
}

void TDBWFRM::zoomIn()
{
    if (currentzoom >= 9)
        return;
    ++currentzoom;
    if (pattern_canvas)
        pattern_canvas->recomputeLayout();
    refresh();
    SetModified();
}

void TDBWFRM::zoomOut()
{
    if (currentzoom <= 0)
        return;
    --currentzoom;
    if (pattern_canvas)
        pattern_canvas->recomputeLayout();
    refresh();
    SetModified();
}

void TDBWFRM::zoomNormal()
{
    /*  Legacy default currentzoom = 3 (11 px/cell). */
    if (currentzoom == 3)
        return;
    currentzoom = 3;
    if (pattern_canvas)
        pattern_canvas->recomputeLayout();
    refresh();
    SetModified();
}

void TDBWFRM::ReloadLanguage()
{
    /*  Placeholder until lang_main.cpp is ported. */
}

void TDBWFRM::RecalcGewebe()
{
    /*  Port of legacy recalc.cpp RecalcGewebe. Rebuilds gewebe from
        einzug x aufknuepfung x trittfolge (default view) or from
        einzug x trittfolge (pegplan view).                         */
    QCursor old = cursor();
    setCursor(Qt::WaitCursor);

    gewebe.feld.Init((char)0);

    if (!ViewSchlagpatrone || !ViewSchlagpatrone->isChecked()) {
        for (int i = 0; i < Data->MAXX1; i++) {
            const short n = einzug.feld.Get(i);
            if (n == 0)
                continue;
            for (int k = 0; k < Data->MAXX2; k++) {
                const char s = aufknuepfung.feld.Get(k, n - 1);
                if (s > 0) {
                    for (int m = 0; m < Data->MAXY2; m++)
                        if (trittfolge.feld.Get(k, m) > 0)
                            gewebe.feld.Set(i, m, s);
                }
            }
        }
    } else {
        for (int i = 0; i < Data->MAXX1; i++) {
            const short n = einzug.feld.Get(i);
            if (n == 0)
                continue;
            for (int k = 0; k < Data->MAXY2; k++) {
                const char s = trittfolge.feld.Get(n - 1, k);
                if (s > 0)
                    gewebe.feld.Set(i, k, s);
            }
        }
    }

    setCursor(old);
    refresh();
}
void TDBWFRM::SetModified(bool) { }
void TDBWFRM::SetCursor(int, int) { }
void TDBWFRM::SetAppTitle() { }
void TDBWFRM::UpdateScrollbars() { }
void TDBWFRM::InvalidateFeld(const GRIDPOS&)
{
    refresh();
}

void TDBWFRM::RearrangeSchaefte()
{
    dbw3_assert(einzughandler);
    if (einzughandler)
        einzughandler->Rearrange();
}

void TDBWFRM::CalcRapport()
{
    if (rapporthandler)
        rapporthandler->CalcRapport();
}
void TDBWFRM::UpdateRapport()
{
    if (rapporthandler)
        rapporthandler->UpdateRapport();
}
void TDBWFRM::ClearRapport()
{
    if (rapporthandler)
        rapporthandler->ClearRapport();
}
void TDBWFRM::DrawRapport()
{
    if (rapporthandler)
        rapporthandler->DrawRapport();
}
bool TDBWFRM::IsInRapport(int _i, int _j)
{
    return rapporthandler ? rapporthandler->IsInRapport(_i, _j) : false;
}

/*  DrawGewebe / DrawEinzug / DrawAufknuepfung / DrawTrittfolge,
    the four *Rahmen painters, DrawGewebeFarbeffekt /
    DrawGewebeSimulation, and DrawGewebeKette / Schuss /
    DeleteGewebeKette / Schuss all live in draw.cpp. */
void TDBWFRM::_ClearEinzug() { }
void TDBWFRM::_ClearAufknuepfung() { }
void TDBWFRM::_ClearSchlagpatrone() { }
void TDBWFRM::_DrawEinzug() { }
void TDBWFRM::_DrawAufknuepfung() { }
void TDBWFRM::_DrawSchlagpatrone() { }
void TDBWFRM::ClearGewebe(int, int) { }
void TDBWFRM::RedrawGewebe(int, int) { }
void TDBWFRM::RedrawAufknuepfung(int, int) { }
