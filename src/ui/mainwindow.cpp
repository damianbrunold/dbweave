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
#include "overviewdialog.h"
#include "undoredo.h"
#include "rapport.h"
#include "einzug.h"
#include "cursor.h"
#include "datamodule.h"
#include "fileformat.h"
#include "language.h"
#include "settings.h"
#include "assert_compat.h"

#include <cstring>

#include <QActionGroup>
#include <QApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPixmap>
#include <QScrollArea>
#include <QScrollBar>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

#include "rangecolors.h"
#include "colors_compat.h"
#include "draw_cell.h"
#include "dbw3_base.h"

namespace
{
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

/*  Build a solid-colour swatch icon with a thin black border, used
    for the range-picker dock (one entry per range colour). */
QIcon swatchIcon(const QColor& _c)
{
    QPixmap pm(16, 16);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.fillRect(1, 1, 14, 14, _c);
    p.setPen(Qt::black);
    p.drawRect(1, 1, 13, 13);
    return QIcon(pm);
}

/*  Build a range-picker icon that mirrors exactly what
    DrawGewebeNormal paints for the given special range. The caller
    passes:
      _d  -- the active darst_* (STEIGEND / KREUZ / KREIS / ... or
             AUSGEFUELLT)
      _bg -- background-fill when _d != AUSGEFUELLT (green for
             anbindung, yellow for abbindung, transparent for
             aushebung)
      _fill -- colour used when _d == AUSGEFUELLT, i.e. the plain
             GetRangeColor(range) the "Patrone" view falls back to.
*/
QIcon rangeSymbolIcon(DARSTELLUNG _d, const QColor& _bg, const QColor& _fill)
{
    constexpr int SZ = 18;
    QPixmap pm(SZ, SZ);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    if (_d == AUSGEFUELLT) {
        /*  Filled-cell fallback: paint the AUSGEFUELLT range
            colour with a 1-pixel margin, matching DrawGewebeNormal
            (AUSGEFUELLT, GetRangeColor(range)).                  */
        p.fillRect(QRect(1, 1, SZ - 2, SZ - 2), _fill);
    } else {
        if (_bg.alpha() > 0)
            p.fillRect(QRect(1, 1, SZ - 2, SZ - 2), _bg);
        PaintCell(p, _d, 0, 0, SZ - 1, SZ - 1, QColor(Qt::black), /*_dontclear=*/true);
    }
    return QIcon(pm);
}
}

void TDBWFRM::updateRangeDockIcons()
{
    if (rangeAushebungAction)
        rangeAushebungAction->setIcon(rangeSymbolIcon(darst_aushebung, QColor(Qt::transparent),
                                                      qColorFromTColor(GetRangeColor(AUSHEBUNG))));
    if (rangeAnbindungAction)
        rangeAnbindungAction->setIcon(rangeSymbolIcon(darst_anbindung,
                                                      qColorFromTColor(col_anbindung),
                                                      qColorFromTColor(GetRangeColor(ANBINDUNG))));
    if (rangeAbbindungAction)
        rangeAbbindungAction->setIcon(rangeSymbolIcon(darst_abbindung,
                                                      qColorFromTColor(col_abbindung),
                                                      qColorFromTColor(GetRangeColor(ABBINDUNG))));
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

    /*  Scratch buffers for RcRecalcAll. */
    xbuf = new char[Data->MAXX1];
    ybuf = new char[Data->MAXY2];
    /*  Fixeinzug snapshot buffer (legacy allocated this in
        AllocBuffersX1, which the Qt port only runs via the file
        loader). Without it, EditFixeinzug bails out on the startup
        document because fixeinzug == nullptr.                   */
    fixeinzug = new short[Data->MAXX1];
    std::memset(fixeinzug, 0, Data->MAXX1 * sizeof(short));

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

    /*  Central layout: pattern canvas on the left (stretching), a
        fixed right-hand side panel on the right that hosts the palette
        / range / tools strips. The previous QDockWidget setup let
        users undock and lose the panels; this is intentionally
        unmovable so the layout is predictable.                      */
    pattern_canvas = new PatternCanvas(this, this);
    QWidget* central = new QWidget(this);
    auto* centralRow = new QHBoxLayout(central);
    centralRow->setContentsMargins(0, 0, 0, 0);
    centralRow->setSpacing(0);
    centralRow->addWidget(pattern_canvas, /*stretch=*/1);

    /*  Side panel: horizontal row laid out left-to-right as
            [ range bar | tools bar | palette (full height) ]
        Range and tools are top-aligned vertical toolbars so they
        don't stretch; the palette claims the full vertical extent
        and gets a scrollbar when the window is short.            */
    sidePanel = new QFrame(central);
    sidePanel->setFrameShape(QFrame::NoFrame);
    auto* sideRow = new QHBoxLayout(sidePanel);
    sideRow->setContentsMargins(2, 2, 2, 2);
    sideRow->setSpacing(4);

    /*  Palette inside a scroll area so a tall palette stays usable on
        a short window. The palette widget itself is fixed-size, so
        the scroll area gives a vertical scrollbar when the side panel
        runs out of room.                                              */
    palettePanel = new PalettePanel(this, sidePanel);
    paletteScroll = new QScrollArea(sidePanel);
    paletteScroll->setWidget(palettePanel);
    paletteScroll->setWidgetResizable(false);
    paletteScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    paletteScroll->setFrameShape(QFrame::NoFrame);
    /*  Width = palette swatches + scrollbar gutter. */
    const int paletteW
        = palettePanel->sizeHint().width() + paletteScroll->verticalScrollBar()->sizeHint().width();
    paletteScroll->setFixedWidth(paletteW);
    /*  Palette is appended last (right-most) after the range / tools
        toolbars are constructed below.                                */

    /*  ViewFarbpalette: regular checkable action -- no dock to bind
        a toggleViewAction to. Wired to show/hide the scroll area. */
    ViewFarbpalette = new QAction(this);
    ViewFarbpalette->setCheckable(true);
    ViewFarbpalette->setChecked(false);
    paletteScroll->hide();
    ViewFarbpalette->setText(QStringLiteral("&Palette"));
    connect(ViewFarbpalette, &QAction::toggled, this, [this](bool on) {
        paletteScroll->setVisible(on);
        updateSidePanelVisibility();
    });

    /*  Legacy menu structure -- see legacy/dbw3_form.dfm. Every
        caption, shortcut and icon mirrors the original VCL form so
        the new Qt application keeps the look and feel users know.
        Entries whose handlers haven't been ported yet appear as
        disabled stubs -- the menu layout is visually complete.   */

    /*  Bilingual helpers. `mkAct` creates a QAction and registers
        its EN/DE captions with langEntries so SwitchLanguage() can
        swap them live. `addStub` does the same for disabled stubs.
        `addMenu` / `addSubmenu` wrap QMenu creation + registration. */
    auto mkAct = [this](const char* en, const char* de, const char* icon = nullptr,
                        const char* en_tip = "", const char* de_tip = "") {
        QAction* a = new QAction(this);
        if (icon)
            a->setIcon(legacyIcon(icon));
        registerLang(a, QString::fromUtf8(en), QString::fromUtf8(de), QString::fromUtf8(en_tip),
                     QString::fromUtf8(de_tip));
        return a;
    };
    auto addStub
        = [this, &mkAct](QMenu* m, const char* en, const char* de, const char* icon = nullptr) {
              QAction* a = mkAct(en, de, icon);
              a->setEnabled(false);
              m->addAction(a);
              return a;
          };
    auto addMenu = [this](QMenuBar* bar, const char* en, const char* de) {
        QMenu* m = bar->addMenu(QString());
        registerLangMenu(m, QString::fromUtf8(en), QString::fromUtf8(de));
        return m;
    };
    auto addSubmenu = [this](QMenu* parent, const char* en, const char* de) {
        QMenu* m = parent->addMenu(QString());
        registerLangMenu(m, QString::fromUtf8(en), QString::fromUtf8(de));
        return m;
    };
    auto menuAct
        = [this, &mkAct](QMenu* m, const char* en, const char* de, const char* icon = nullptr,
                         const char* en_tip = "", const char* de_tip = "") {
              QAction* a = mkAct(en, de, icon, en_tip, de_tip);
              m->addAction(a);
              return a;
          };

    /*  ---------- &File (MenuDatei) --------------------------- */
    QMenu* fileMenu = addMenu(menuBar(), "&File", "&Datei");
    QAction* actFileNew = menuAct(fileMenu, "&New", "&Neu", "sb_new", "Creates a new pattern",
                                  "Erstellt ein neues Muster");
    actFileNew->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    connect(actFileNew, &QAction::triggered, this, [this] { FileNewClick(); });
    QAction* actFileNewTpl = menuAct(fileMenu, "New &from template...", "Neu ge&mäss Vorlage...",
                                     nullptr, "Creates a new pattern based upon a template",
                                     "Erstellt ein neues Muster anhand einer Vorlage");
    connect(actFileNewTpl, &QAction::triggered, this, [this] { FileNewTemplateClick(); });
    QAction* actOpen = menuAct(fileMenu, "&Open...", "Ö&ffnen...", "sb_open", "Opens a pattern",
                               "Öffnet ein Muster");
    actOpen->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    QAction* actOpenNewWin
        = menuAct(fileMenu, "Open in new &window...", "In neuem &Fenster öffnen...", nullptr,
                  "Opens a pattern in a separate DB-WEAVE window",
                  "Öffnet ein Muster in einem eigenen DB-WEAVE-Fenster");
    connect(actOpenNewWin, &QAction::triggered, this, [this] { FileOpenInNewWindow(); });
    QAction* actLoadParts = menuAct(fileMenu, "&Load parts...", "&Teile laden...", nullptr,
                                    "Loads only parts of a pattern into the current pattern",
                                    "Lädt Teile eines Musters in das aktuelle Muster");
    QAction* actSave = menuAct(fileMenu, "&Save", "&Speichern", "sb_save", "Saves the pattern",
                               "Speichert das Muster");
    actSave->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    QAction* actSaveAs = menuAct(fileMenu, "Save &as...", "Speichern &unter...", nullptr,
                                 "Saves the pattern to a new file",
                                 "Speichert das Muster unter einem neuen Namen");
    QAction* actFileRevert
        = menuAct(fileMenu, "Revert &changes", "Änderungen ver&werfen", nullptr,
                  "Discards any changes and restores the last saved state of the pattern",
                  "Verwirft alle Änderungen und kehrt zum letzten gespeicherten Zustand zurück");
    connect(actFileRevert, &QAction::triggered, this, [this] { FileRevertClick(); });
    /*  Mirror legacy idle.cpp: Revert is only meaningful when there
        is a backing file AND unsaved edits. Update the enabled state
        each time the File menu opens. */
    connect(fileMenu, &QMenu::aboutToShow, this,
            [this, actFileRevert] { actFileRevert->setEnabled(!filename.isEmpty() && modified); });
    fileMenu->addSeparator();
    QMenu* importMenu = addSubmenu(fileMenu, "I&mport", "I&mport");
    QAction* actImportWIF
        = menuAct(importMenu, "&WIF file...", "&WIF Datei...", nullptr,
                  "Imports the pattern from a WIF file", "Importiert das Muster einer WIF Datei");
    QMenu* exportMenu = addSubmenu(fileMenu, "&Export", "E&xportieren");
    QAction* actExportPng
        = menuAct(exportMenu, "&PNG file...", "&PNG Datei...", nullptr,
                  "Exports the pattern as a PNG image", "Exportiert das Muster als PNG-Bild");
    QAction* actExportJpeg
        = menuAct(exportMenu, "&JPEG file...", "&JPEG Datei...", nullptr,
                  "Exports the pattern as a JPEG image", "Exportiert das Muster als JPEG-Bild");
    QAction* actExportSvg = menuAct(exportMenu, "&SVG file...", "&SVG Datei...", nullptr,
                                    "Exports the pattern as an SVG vector file",
                                    "Exportiert das Muster als SVG-Vektordatei");
    QAction* actExportPdf
        = menuAct(exportMenu, "PD&F file...", "PD&F Datei...", nullptr,
                  "Exports the pattern as a PDF file", "Exportiert das Muster als PDF-Datei");
    exportMenu->addSeparator();
    QAction* actExportWIF
        = menuAct(exportMenu, "&WIF file...", "&WIF Datei...", nullptr,
                  "Exports the pattern to a WIF file", "Exportiert das Muster in eine WIF Datei");
    fileMenu->addSeparator();
    QAction* actFileProps = menuAct(fileMenu, "Propert&ies...", "&Eigenschaften...", nullptr,
                                    "Edits the properties of the pattern",
                                    "Bearbeitet die Eigenschaften des Musters");
    fileMenu->addSeparator();
    QAction* actPageSetup = menuAct(fileMenu, "Page se&tup...", "Se&ite einrichten...", nullptr,
                                    "Sets the page margins and headers and footers",
                                    "Richtet die Seitenränder und Kopf- und Fusszeilen ein");
    QAction* actPreview
        = menuAct(fileMenu, "Print pre&view", "Druckvorsc&hau", "sb_preview",
                  "Shows how the pattern will be printed", "Zeigt wie das Muster gedruckt wird");
    QAction* actPrint = menuAct(fileMenu, "&Print...", "&Drucken...", "sb_print",
                                "Prints the pattern", "Druckt das Muster");
    actPrint->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    QAction* actPrintRange
        = menuAct(fileMenu, "P&rint part...", "Teil dru&cken...", nullptr,
                  "Prints only a part of the pattern", "Druckt nur einen Teil des Musters");
    actPrintRange->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(actPrintRange, &QAction::triggered, this, [this] { FilePrintRangeClick(); });
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
    QAction* actQuit = menuAct(fileMenu, "E&xit", "&Beenden", nullptr, "Exits the program",
                               "Beendet das Programm");

    connect(actOpen, &QAction::triggered, this, [this] { FileOpen(); });
    connect(actSave, &QAction::triggered, this, [this] { FileSave(); });
    connect(actSaveAs, &QAction::triggered, this, [this] { FileSaveAs(); });
    connect(actLoadParts, &QAction::triggered, this, [this] { LoadPartsClick(); });
    connect(actFileProps, &QAction::triggered, this, [this] { FilePropsClick(); });
    connect(actPageSetup, &QAction::triggered, this, [this] { FileSetPageClick(); });
    connect(actPrint, &QAction::triggered, this, [this] { FilePrintClick(); });
    connect(actPreview, &QAction::triggered, this, [this] { FilePrintpreviewClick(); });
    connect(actImportWIF, &QAction::triggered, this, [this] { ImportWIFClick(); });
    connect(actExportPng, &QAction::triggered, this, [this] { DateiExportPngClick(); });
    connect(actExportJpeg, &QAction::triggered, this, [this] { DateiExportJpegClick(); });
    connect(actExportSvg, &QAction::triggered, this, [this] { DateiExportSvgClick(); });
    connect(actExportPdf, &QAction::triggered, this, [this] { DateiExportPdfClick(); });
    connect(actExportWIF, &QAction::triggered, this, [this] { DateiExportWifClick(); });
    connect(actQuit, &QAction::triggered, this, &TDBWFRM::close);

    /*  ---------- &Edit (MenuBearbeiten) --------------------- */
    QMenu* editMenu = addMenu(menuBar(), "&Edit", "&Bearbeiten");
    QAction* actUndo = menuAct(editMenu, "&Undo", "&Rückgängig", "mn_undo",
                               "Undoes the last action", "Macht die letzte Änderung rückgängig");
    actUndo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
    QAction* actRedo
        = menuAct(editMenu, "&Redo", "&Wiederholen", "mn_redo", "Redoes the last undone action",
                  "Führt die letzte rückgängig gemachte Änderung durch");
    actRedo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
    editMenu->addSeparator();
    QAction* actCut = menuAct(editMenu, "Cu&t", "A&usschneiden", "mn_cut",
                              "Cuts the selection to the clipboard",
                              "Schneidet die Selektion aus und kopiert sie in die Zwischenablage");
    actCut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));
    QAction* actCopy = menuAct(editMenu, "&Copy", "&Kopieren", "mn_copy",
                               "Copies the selection to the clipboard",
                               "Kopiert die Selektion in die Zwischenablage");
    actCopy->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));
    QAction* actPaste
        = menuAct(editMenu, "&Paste", "&Einfügen", "mn_paste",
                  "Pastes the contents of the clipboard at the current cursor location",
                  "Fügt den Inhalt der Zwischenablage an der aktuellen Cursorposition ein");
    actPaste->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_V));
    QAction* actPasteTransparent = menuAct(
        editMenu, "P&aste transparently", "&Transparent einfügen", nullptr,
        "Pastes the contents of the clipboard transparently at the current cursor location",
        "Fügt den Inhalt der Zwischenablage transparent an der aktuellen Cursorposition ein");
    actPasteTransparent->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    editMenu->addSeparator();
    QAction* actInvert = menuAct(editMenu, "&Invert", "&Invertieren", "sb_invert",
                                 "Inverts the selection", "Invertiert die Selektion");
    actInvert->setShortcut(QKeySequence(Qt::Key_I));
    QAction* actMirrorH = menuAct(editMenu, "Mirror &horizontal", "Spiegeln &horizontal",
                                  "sb_mirrorh", "Mirrors the selection in horizontal direction",
                                  "Spiegelt die Selektion in horizontaler Richtung");
    actMirrorH->setShortcut(QKeySequence(Qt::Key_H));
    QAction* actMirrorV = menuAct(editMenu, "Mirror &vertical", "Spiegeln &vertikal", "sb_mirrorv",
                                  "Mirrors the selection in vertical direction",
                                  "Spiegelt die Selektion in vertikaler Richtung");
    actMirrorV->setShortcut(QKeySequence(Qt::Key_V));
    QAction* actRotate = menuAct(editMenu, "Rotat&e", "R&otieren", "sb_rotate",
                                 "Rotates the square selection in clockwise direction",
                                 "Rotiert die quadratische Selektion im Uhrzeigersinn");
    actRotate->setShortcut(QKeySequence(Qt::Key_R));
    QAction* actCentralsym
        = menuAct(editMenu, "Ma&ke central symmetric", "&Zentralsymmetrisch", "sb_centralsym",
                  "Tries to make the selection central symmetric",
                  "Versucht die Selektion zentralsymmetrisch zu machen");
    actCentralsym->setShortcut(QKeySequence(Qt::Key_Z));
    QAction* actDelete = menuAct(editMenu, "&Delete", "&Löschen", nullptr, "Deletes the selection",
                                 "Löscht die Selektion");
    actDelete->setShortcut(QKeySequence(Qt::Key_Delete));

    QMenu* rollMenu = addSubmenu(editMenu, "R&oll", "Rolle&n");
    QAction* actRollUp = menuAct(rollMenu, "Roll &up", "Nach &oben", nullptr,
                                 "Rolls the selection upwards", "Rollt die Selektion aufwärts");
    /*  Legacy shortcuts are Ctrl+6/7/8/9 (see legacy/dbw3_form.dfm);
        Ctrl+Arrow is reserved for strongline-step cursor navigation. */
    actRollUp->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_6));
    QAction* actRollDown = menuAct(rollMenu, "Roll &down", "Nach &unten", nullptr,
                                   "Rolls the selection downwards", "Rollt die Selektion abwärts");
    actRollDown->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_7));
    QAction* actRollLeft
        = menuAct(rollMenu, "Roll &left", "Nach &links", nullptr, "Rolls the selection to the left",
                  "Rollt die Selektion nach links");
    actRollLeft->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_8));
    QAction* actRollRight
        = menuAct(rollMenu, "Roll &right", "Nach &rechts", nullptr,
                  "Rolls the selection to the right", "Rollt die Selektion nach rechts");
    actRollRight->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_9));

    QMenu* slopeMenu = addSubmenu(editMenu, "&Slope", "&Steigung");
    QAction* actSteigInc
        = menuAct(slopeMenu, "&Increase", "&Erhöhen", nullptr,
                  "Increases the slope of the selection", "Erhöht die Steigung der Selektion");
    actSteigInc->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
    QAction* actSteigDec
        = menuAct(slopeMenu, "&Decrease", "&Vermindern", nullptr,
                  "Decreases the slope of the selection", "Vermindert die Steigung der Selektion");
    actSteigDec->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_J));
    editMenu->addSeparator();

    QMenu* moveMenu = addSubmenu(editMenu, "&Move", "Ver&schieben");
    QAction* actSchaftUp
        = menuAct(moveMenu, "Harness &up", "Schaft nach &oben", "sb_schaftup",
                  "Moves the selected harness up", "Verschiebt den aktuellen Schaft nach oben");
    actSchaftUp->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F7));
    QAction* actSchaftDown
        = menuAct(moveMenu, "Harness &down", "Schaft nach &unten", "sb_schaftdown",
                  "Moves the selected harness down", "Verschiebt den aktuellen Schaft nach unten");
    actSchaftDown->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F8));
    QAction* actTrittLeft
        = menuAct(moveMenu, "Treadle &left", "Tritt nach &links", "sb_trittleft",
                  "Moves the selected treadle left", "Verschiebt den aktuellen Tritt nach links");
    actTrittLeft->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F5));
    QAction* actTrittRight
        = menuAct(moveMenu, "Treadle &right", "Tritt nach &rechts", "sb_trittright",
                  "Moves the selected treadle right", "Verschiebt den aktuellen Tritt nach rechts");
    actTrittRight->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F6));

    QMenu* insMenu = addSubmenu(editMenu, "I&nsert", "Ein&fügen");
    QAction* actInsSchaft = menuAct(insMenu, "&Harness", "&Schaft", nullptr,
                                    "Inserts a harness at the current cursor position",
                                    "Fügt einen Schaft an der aktuellen Cursorposition ein");
    actInsSchaft->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_S));
    QAction* actInsTritt = menuAct(insMenu, "&Treadle", "&Tritt", nullptr,
                                   "Inserts a treadle at the current cursor position",
                                   "Fügt einen Tritt an der aktuellen Cursorposition ein");
    actInsTritt->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_T));
    QAction* actInsKette = menuAct(insMenu, "&End, Warp thread", "&Kettfaden", nullptr,
                                   "Inserts a warp thread at the current cursor position",
                                   "Fügt einen Kettfaden an der aktuellen Cursorposition ein");
    actInsKette->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_K));
    QAction* actInsSchuss = menuAct(insMenu, "&Pick, Weft thread", "Schuss&faden", nullptr,
                                    "Inserts a weft thread at the current cursor position",
                                    "Fügt einen Schussfaden an der aktuellen Cursorposition ein");
    actInsSchuss->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F));

    QMenu* delMenu = addSubmenu(editMenu, "De&lete", "Lös&chen");
    QAction* actDelSchaft = menuAct(delMenu, "&Harness", "&Schaft", nullptr,
                                    "Deletes the harness at the current cursor position",
                                    "Löscht den Schaft an der aktuellen Cursorposition");
    actDelSchaft->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    QAction* actDelTritt = menuAct(delMenu, "&Treadle", "&Tritt", nullptr,
                                   "Deletes the treadle at the current cursor position",
                                   "Löscht den Tritt an der aktuellen Cursorposition");
    actDelTritt->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
    QAction* actDelKette = menuAct(delMenu, "&End, Warp thread", "&Kettfaden", nullptr,
                                   "Deletes the warp thread at the current cursor position",
                                   "Löscht den Kettfaden an der aktuellen Cursorposition");
    actDelKette->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_K));
    QAction* actDelSchuss = menuAct(delMenu, "&Pick, Weft thread", "Schuss&faden", nullptr,
                                    "Deletes the weft thread at the current cursor position",
                                    "Löscht den Schussfaden an der aktuellen Cursorposition");
    actDelSchuss->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    editMenu->addSeparator();

    QMenu* lanceeMenu = addSubmenu(editMenu, "La&ncee", "&Lancierung");
    QAction* actKettLanc = menuAct(lanceeMenu, "&Warp lancee...", "&Kettlancierung...", nullptr,
                                   "Inserts warp threads in order to create a warp lancee",
                                   "Fügt Kettfäden ein um eine Kettlancierung zu machen");
    QAction* actSchussLanc = menuAct(lanceeMenu, "W&eft lancee...", "&Schusslancierung...", nullptr,
                                     "Inserts weft threads in order to create a weft lancee",
                                     "Fügt Schussfäden ein um eine Schusslancierung zu machen");

    QAction* actFillKoeper
        = menuAct(editMenu, "T&will completion", "&Köper ergänzen", nullptr,
                  "Complements the currently selected thread to a complete twill",
                  "Ergänzt den selektierten Faden zu einem vollständigen Köper");
    actFillKoeper->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_K));
    QAction* actSwapSide = menuAct(editMenu, "S&witch side", "S&eite wechseln", nullptr,
                                   "Switches to the other side of the pattern",
                                   "Wechselt auf die andere Seite des Musters");
    actSwapSide->setShortcut(QKeySequence(Qt::Key_F11));

    /*  After stepping the undo/redo history, sync the modified flag
        from the clean-state marker so undoing all the way back to a
        just-loaded / just-saved snapshot drops the asterisk in the
        title bar (and re-enables it when stepping away again).     */
    auto syncModifiedFromUndo = [this] {
        if (undo)
            SetModified(!undo->IsAtCleanState());
    };
    connect(actUndo, &QAction::triggered, this, [this, syncModifiedFromUndo] {
        if (undo && undo->Undo()) {
            syncModifiedFromUndo();
            refresh();
        }
    });
    connect(actRedo, &QAction::triggered, this, [this, syncModifiedFromUndo] {
        if (undo && undo->Redo()) {
            syncModifiedFromUndo();
            refresh();
        }
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
    QMenu* viewMenu = addMenu(menuBar(), "&View", "&Ansicht");
    registerLang(ViewBlatteinzug, QStringLiteral("R&eed threading"), QStringLiteral("&Blatteinzug"),
                 QStringLiteral("Toggles reed threading"),
                 QStringLiteral("Blatteinzug sichtbar/unsichtbar"));
    viewMenu->addAction(ViewBlatteinzug);
    registerLang(ViewFarbe, QStringLiteral("&Color"), QStringLiteral("&Farbe"),
                 QStringLiteral("Toggles color"), QStringLiteral("Farbe sichtbar/unsichtbar"));
    ViewFarbe->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
    viewMenu->addAction(ViewFarbe);
    registerLang(ViewEinzug, QStringLiteral("&Threading"), QStringLiteral("&Einzug"),
                 QStringLiteral("Toggles threading"), QStringLiteral("Einzug sichtbar/unsichtbar"));
    ViewEinzug->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    viewMenu->addAction(ViewEinzug);
    registerLang(ViewTrittfolge, QStringLiteral("Tre&adling"), QStringLiteral("&Trittfolge"),
                 QStringLiteral("Toggles treadling"),
                 QStringLiteral("Trittfolge sichtbar/unsichtbar"));
    ViewTrittfolge->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    viewMenu->addAction(ViewTrittfolge);
    registerLang(ViewHlines, QStringLiteral("&Support lines"), QStringLiteral("&Hilfslinien"),
                 QStringLiteral("Toggles support-lines"),
                 QStringLiteral("Hilfslinien sichtbar/unsichtbar"));
    viewMenu->addAction(ViewHlines);
    registerLang(ViewOnlyGewebe, QStringLiteral("&Only pattern"), QStringLiteral("&Nur Bindung"),
                 QStringLiteral("Displays only the pattern"),
                 QStringLiteral("Zeigt nur die Bindung an"));
    viewMenu->addAction(ViewOnlyGewebe);
    viewMenu->addSeparator();
    QMenu* gewebeMenu = addSubmenu(viewMenu, "&View", "&Ansicht");
    auto* gewebeGroup = new QActionGroup(this);
    gewebeGroup->setExclusive(true);
    registerLang(GewebeNone, QStringLiteral("&Invisible"), QStringLiteral("&Unsichtbar"),
                 QStringLiteral("Suppresses the pattern"),
                 QStringLiteral("Unterdrückt die Bindung"));
    GewebeNone->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Slash));
    registerLang(GewebeNormal, QStringLiteral("&Draft"), QStringLiteral("&Patrone"),
                 QStringLiteral("Draft mode"), QStringLiteral("Patronenmodus"));
    GewebeNormal->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));
    registerLang(GewebeFarbeffekt, QStringLiteral("&Color"), QStringLiteral("&Farbeffekt"),
                 QStringLiteral("Color mode"), QStringLiteral("Farbeffektmodus"));
    GewebeFarbeffekt->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_2));
    registerLang(GewebeSimulation, QStringLiteral("&Fabric"), QStringLiteral("&Gewebe"),
                 QStringLiteral("Fabric mode"), QStringLiteral("Gewebesimulationsmodus"));
    GewebeSimulation->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_3));
    for (QAction* a : { GewebeNone, GewebeNormal, GewebeFarbeffekt, GewebeSimulation }) {
        a->setCheckable(true);
        gewebeGroup->addAction(a);
        gewebeMenu->addAction(a);
    }
    viewMenu->addSeparator();
    QAction* actZoomIn = menuAct(viewMenu, "&Zoom in", "&Vergrössern", "sb_zoomin", "Zoom in",
                                 "Vergrössert die Ansicht");
    actZoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    QAction* actZoomNormal
        = menuAct(viewMenu, "&Normal", "&Normal", nullptr, "Sets magnification to default value",
                  "Stellt die Standardgrösse ein");
    QAction* actZoomOut = menuAct(viewMenu, "Zoo&m out", "Ver&kleinern", "sb_zoomout", "Zoom out",
                                  "Verkleinert die Ansicht");
    actZoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_U));
    /*  Toolbar toggles -- appended below once the side-panel children
        have been constructed; each is a checkable QAction wired to
        show/hide its widget and re-evaluate side-panel visibility. */
    viewMenu->addSeparator();
    QAction* actOverview
        = menuAct(viewMenu, "Overv&iew", "Übe&rsicht", nullptr, "Enters an overview-mode",
                  "Schaltet in einen Übersichtsmodus um");
    actOverview->setShortcut(QKeySequence(Qt::Key_F4));
    connect(actOverview, &QAction::triggered, this, [this] {
        OverviewDialog dlg(this, this);
        dlg.exec();
    });

    connect(actZoomIn, &QAction::triggered, this, [this] { zoomIn(); });
    connect(actZoomOut, &QAction::triggered, this, [this] { zoomOut(); });
    connect(actZoomNormal, &QAction::triggered, this, [this] { zoomNormal(); });
    auto relayout = [this] {
        if (pattern_canvas)
            pattern_canvas->recomputeLayout();
        refresh();
    };
    for (QAction* a :
         { ViewEinzug, ViewTrittfolge, ViewBlatteinzug, ViewFarbe, ViewHlines, ViewOnlyGewebe })
        connect(a, &QAction::triggered, this, relayout);
    for (QAction* a : { GewebeNormal, GewebeFarbeffekt, GewebeSimulation, GewebeNone })
        connect(a, &QAction::triggered, this, [this] { refresh(); });

    /*  ---------- &Threading (MenuEinzug) -------------------- */
    QMenu* threadMenu = addMenu(menuBar(), "&Threading", "&Einzug");
    QAction* actEzAssist = menuAct(threadMenu, "Threading assistant...", "Einzugassistent...");
    threadMenu->addSeparator();
    QAction* actEzSpiegeln = menuAct(threadMenu, "&Mirror all", "&Spiegeln", nullptr,
                                     "Mirrors the threading in horizontal direction",
                                     "Spiegelt den Einzug in horizontaler Richtung");
    connect(actEzSpiegeln, &QAction::triggered, this, [this] { EzSpiegelnClick(); });
    QAction* actClearEinzug = menuAct(threadMenu, "&Delete", "&Löschen", nullptr,
                                      "Deletes the threading", "Löscht den Einzug");
    threadMenu->addSeparator();
    auto* ezGroup = new QActionGroup(this);
    ezGroup->setExclusive(true);
    registerLang(EzMinimalZ, QStringLiteral("Normal &rising"), QStringLiteral("&Normal steigend"),
                 QStringLiteral("Arranges the threading in a rising manner"),
                 QStringLiteral("Ordnet den Einzug steigend an"));
    registerLang(EzMinimalS, QStringLiteral("Normal &falling"), QStringLiteral("Normal &fallend"),
                 QStringLiteral("Arranges the threading in a falling manner"),
                 QStringLiteral("Ordnet den Einzug fallend an"));
    registerLang(EzGeradeZ, QStringLiteral("&Straight rising"), QStringLiteral("&Gerade steigend"),
                 QStringLiteral("Arranges the threading straight rising"),
                 QStringLiteral("Ordnet den Einzug gerade steigend an"));
    registerLang(EzGeradeS, QStringLiteral("S&traight falling"), QStringLiteral("G&erade fallend"),
                 QStringLiteral("Arranges the threading straight falling"),
                 QStringLiteral("Ordnet den Einzug gerade fallend an"));
    registerLang(EzChorig2, QStringLiteral("&2 choirs"), QStringLiteral("&2-chorig"),
                 QStringLiteral("Arranges the threading in a simple 2-choir scheme"),
                 QStringLiteral("Ordnet den Einzug in einem einfachen Zweichorsystem an"));
    registerLang(EzChorig3, QStringLiteral("&3 choirs"), QStringLiteral("&3-chorig"),
                 QStringLiteral("Arranges the threading in a simple 3-choir scheme"),
                 QStringLiteral("Ordnet den Einzug in einem einfachen Dreichorsystem"));
    /*  EzBelassen is retained as internal state (still in the radio
        action group and written to .dbw) so file round-trips don't
        alter the saved style, but it is deliberately not shown in the
        menu: in practice it produces the same result as EzMinimalZ
        after a fresh RecalcAll (see Rearrange dispatch in
        einzug.cpp), and its presence alongside "Benutzerdefiniert"
        was a known source of user confusion.                     */
    registerLang(
        EzFixiert, QStringLiteral("&User defined..."), QStringLiteral("&Benutzerdefiniert..."),
        QStringLiteral("Arranges the threading according to a user-supplied template"),
        QStringLiteral("Ordnet den Einzug gemäss einem vom Benutzer vorgegebenen Schema an"));
    ezGroup->addAction(EzBelassen);
    for (QAction* a :
         { EzMinimalZ, EzMinimalS, EzGeradeZ, EzGeradeS, EzChorig2, EzChorig3, EzFixiert }) {
        ezGroup->addAction(a);
        threadMenu->addAction(a);
    }
    threadMenu->addSeparator();
    actCopyEzTf = menuAct(threadMenu, "&Copy from treadling", "&Kopieren von Trittfolge", nullptr,
                          "Copies the treadling into the threading",
                          "Kopiert die Trittfolge in den Einzug");
    connect(actEzAssist, &QAction::triggered, this, [this] { EinzugAssistentClick(); });
    connect(actClearEinzug, &QAction::triggered, this, [this] { refresh(); });
    connect(actCopyEzTf, &QAction::triggered, this, [this] { CopyEinzugTrittfolgeClick(); });
    connect(EzFixiert, &QAction::triggered, this, [this] { EditFixeinzug(); });

    /*  Einzug style radio items -- rebuild everything from the
        current gewebe via RecalcAll(). The per-style Rearrange
        helpers alone don't always produce a clean result when
        switching between e.g. NormalZ and Chorig2, so we let the
        full recalc handle it (same path as a gewebe edit).       */
    auto runRecalc = [this] {
        RecalcAll();
        CalcRapport();
        UpdateRapport();
        UpdateStatusBar();
        refresh();
        SetModified();
        if (undo)
            undo->Snapshot();
    };
    for (QAction* a : { EzMinimalZ, EzMinimalS, EzGeradeZ, EzGeradeS, EzChorig2, EzChorig3 })
        connect(a, &QAction::triggered, this, runRecalc);

    /*  ---------- Tre&adling (MenuTrittfolge) ---------------- */
    treadMenu = addMenu(menuBar(), "Tre&adling", "&Trittfolge");
    QAction* actTfSpiegeln = menuAct(treadMenu, "&Mirror", "&Spiegeln", nullptr,
                                     "Mirrors the treadling in vertical direction",
                                     "Spiegelt die Trittfolge in vertikaler Richtung");
    QAction* actClearTf = menuAct(treadMenu, "&Delete", "&Löschen", nullptr,
                                  "Deletes the treadling", "Löscht die Trittfolge");
    treadMenu->addSeparator();
    auto* tfGroup = new QActionGroup(this);
    tfGroup->setExclusive(true);
    registerLang(TfMinimalZ, QStringLiteral("Normal &rising"), QStringLiteral("&Normal steigend"),
                 QStringLiteral("Arranges the treadling rising"),
                 QStringLiteral("Ordnet die Trittfolge steigend an"));
    registerLang(TfMinimalS, QStringLiteral("Normal &falling"), QStringLiteral("Normal &fallend"),
                 QStringLiteral("Arranges the treadling falling"),
                 QStringLiteral("Ordnet die Trittfolge fallend an"));
    registerLang(TfGesprungen, QStringLiteral("Cr&ossed"), QStringLiteral("Gesp&rungen"),
                 QStringLiteral("Arranges the treadling crossed"),
                 QStringLiteral("Ordnet die Trittfolge gesprungen an"));
    /*  TfBelassen is hidden from the menu for the same reason as
        EzBelassen (see the einzug block above): in the current
        architecture RearrangeTritte always runs after a fresh
        RecalcTrittfolge, and MinimalZ's sort loop becomes a no-op
        there -- TfBelassen's early-return therefore produces the same
        visible result. The action is kept in the radio group as
        internal state so saved-style round-trips are preserved. */
    tfGroup->addAction(TfBelassen);
    for (QAction* a : { TfMinimalZ, TfMinimalS, TfGesprungen }) {
        tfGroup->addAction(a);
        treadMenu->addAction(a);
    }
    treadMenu->addSeparator();
    QAction* actCopyTfEz = menuAct(treadMenu, "C&opy from threading", "&Kopieren von Einzug",
                                   nullptr, "Copies the threading into the treadling",
                                   "Kopiert den Einzug in die Trittfolge");
    connect(actTfSpiegeln, &QAction::triggered, this, [this] { TfSpiegelnClick(); });
    connect(actClearTf, &QAction::triggered, this, [this] { ClearTrittfolgeClick(); });
    connect(actCopyTfEz, &QAction::triggered, this, [this] { CopyTrittfolgeEinzugClick(); });

    /*  Trittfolge style radio items -- same full-recalc path as
        einzug (see above).                                        */
    for (QAction* a : { TfMinimalZ, TfMinimalS, TfGesprungen })
        connect(a, &QAction::triggered, this, runRecalc);

    /*  ---------- &Pegplan (MenuSchlagpatrone) --------------- */
    spMenu = addMenu(menuBar(), "&Pegplan", "&Schlagpatrone");
    QAction* actSpInvert = menuAct(spMenu, "&Invert", "&Invertieren", "sb_invert",
                                   "Inverts the pegplan", "Invertiert die gesamte Schlagpatrone");
    QAction* actClearSp = menuAct(spMenu, "&Delete", "&Löschen", nullptr, "Deletes the pegplan",
                                  "Löscht die Schlagpatrone");
    QAction* actSpSpiegeln = menuAct(spMenu, "&Mirror", "&Spiegeln", "sb_mirrorv",
                                     "Mirrors the pegplan in vertical direction",
                                     "Spiegelt die Schlagpatrone in vertikaler Richtung");
    connect(actSpInvert, &QAction::triggered, this, [this] { SpInvert(); });
    connect(actClearSp, &QAction::triggered, this, [this] { ClearSchlagpatroneClick(); });
    connect(actSpSpiegeln, &QAction::triggered, this, [this] { SpSpiegelnClick(); });

    /*  ---------- Tie-&up (MenuAufknuepfung) ----------------- */
    QMenu* aufMenu = addMenu(menuBar(), "Tie-&up", "A&ufknüpfung");
    QAction* actAufInvert = menuAct(aufMenu, "&Invert", "&Invertieren", "sb_invert",
                                    "Inverts the tie-up", "Invertiert die Aufknüpfung");
    connect(actAufInvert, &QAction::triggered, this, [this] { AufInvertClick(); });
    QAction* actClearAuf = menuAct(aufMenu, "&Delete", "&Löschen", nullptr, "Deletes the tie-up",
                                   "Löscht die Aufknüpfung");
    connect(actClearAuf, &QAction::triggered, this, [this] { ClearAufknuepfungClick(); });
    QMenu* aufRollMenu = addSubmenu(aufMenu, "&Roll", "&Rollen");
    QAction* actAufRollUp = menuAct(aufRollMenu, "&Up", "Nach &oben", nullptr,
                                    "Rolls the tie-up up", "Rollt die Aufknüpfung nach oben");
    /*  Legacy AufRoll shortcuts are Ctrl+Shift+6/7/8/9; keeps Ctrl+Shift+Arrow
        free for future use and matches legacy/dbw3_form.dfm. */
    actAufRollUp->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_6));
    connect(actAufRollUp, &QAction::triggered, this, [this] { AufRollUpClick(); });
    QAction* actAufRollDown = menuAct(aufRollMenu, "&Down", "Nach &unten", nullptr,
                                      "Rolls the tie-up down", "Rollt die Aufknüpfung nach unten");
    actAufRollDown->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_7));
    connect(actAufRollDown, &QAction::triggered, this, [this] { AufRollDownClick(); });
    QAction* actAufRollLeft = menuAct(aufRollMenu, "&Left", "Nach &links", nullptr,
                                      "Rolls the tie-up left", "Rollt die Aufknüpfung nach links");
    actAufRollLeft->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_8));
    connect(actAufRollLeft, &QAction::triggered, this, [this] { AufRollLeftClick(); });
    QAction* actAufRollRight
        = menuAct(aufRollMenu, "&Right", "Nach &rechts", nullptr, "Rolls the tie-up right",
                  "Rollt die Aufknüpfung nach rechts");
    actAufRollRight->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_9));
    connect(actAufRollRight, &QAction::triggered, this, [this] { AufRollRightClick(); });
    QMenu* aufSlopeMenu = addSubmenu(aufMenu, "&Slope", "&Steigung");
    QAction* actAufSlopeInc
        = menuAct(aufSlopeMenu, "&Increase", "&Erhöhen", nullptr,
                  "Increases the slope of the tie-up", "Erhöht die Steigung der Aufknüpfung");
    actAufSlopeInc->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
    connect(actAufSlopeInc, &QAction::triggered, this, [this] { AufSteigungIncClick(); });
    QAction* actAufSlopeDec
        = menuAct(aufSlopeMenu, "&Decrease", "&Vermindern", nullptr,
                  "Decreases the slope of the tie-up", "Vermindert die Steigung der Aufknüpfung");
    actAufSlopeDec->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_J));
    connect(actAufSlopeDec, &QAction::triggered, this, [this] { AufSteigungDecClick(); });
    QAction* actAufZentralsymm
        = menuAct(aufMenu, "Ma&ke central symmetric", "&Zentralsymmetrisch", "sb_centralsym",
                  "Tries to make the tie-up central symmetric",
                  "Versucht die Aufknüpfung zentralsymmetrisch zu machen");
    connect(actAufZentralsymm, &QAction::triggered, this, [this] { AufZentralsymmClick(); });

    /*  ---------- &Insert (MenuEinfuegen) -------------------- */
    QMenu* insertMenu = addMenu(menuBar(), "&Insert", "Ei&nfügen");
    QMenu* atlasMenu = addSubmenu(insertMenu, "&Satin", "&Atlas");
    for (int n = 5; n <= 10; n++) {
        const QByteArray en = QStringLiteral("Satin &%1").arg(n).toUtf8();
        const QByteArray de = QStringLiteral("Atlas &%1").arg(n).toUtf8();
        QAction* a = menuAct(atlasMenu, en.constData(), de.constData());
        connect(a, &QAction::triggered, this, [this, n] { AtlasEinfuegen(n); });
    }
    QMenu* koepMenu = addSubmenu(insertMenu, "&Twill", "&Köper");
    QMenu* koepBalMenu = addSubmenu(koepMenu, "&Balanced", "&Gleichseitig");
    auto addKoeperItem
        = [this, &mkAct, &menuAct](QMenu* m, const char* en, const char* de, int h, int s) {
              QAction* a = menuAct(m, en, de);
              connect(a, &QAction::triggered, this, [this, h, s] { KoeperEinfuegen(h, s); });
              return a;
          };
    addKoeperItem(koepBalMenu, "Twill &2/2", "Köper &2/2", 2, 2);
    addKoeperItem(koepBalMenu, "Twill &3/3", "Köper &3/3", 3, 3);
    addKoeperItem(koepBalMenu, "Twill &4/4", "Köper &4/4", 4, 4);
    addKoeperItem(koepBalMenu, "Twill &5/5", "Köper &5/5", 5, 5);
    QMenu* kettkoepMenu = addSubmenu(koepMenu, "&Warp sided", "&Kettseitig");
    addKoeperItem(kettkoepMenu, "&1  Twill 2/1", "&1  Köper 2/1", 2, 1);
    addKoeperItem(kettkoepMenu, "&2  Twill 3/1", "&2  Köper 3/1", 3, 1);
    addKoeperItem(kettkoepMenu, "&3  Twill 4/1", "&3  Köper 4/1", 4, 1);
    addKoeperItem(kettkoepMenu, "&4  Twill 5/1", "&4  Köper 5/1", 5, 1);
    addKoeperItem(kettkoepMenu, "&5  Twill 3/2", "&5  Köper 3/2", 3, 2);
    addKoeperItem(kettkoepMenu, "&6  Twill 4/2", "&6  Köper 4/2", 4, 2);
    addKoeperItem(kettkoepMenu, "&7  Twill 5/2", "&7  Köper 5/2", 5, 2);
    addKoeperItem(kettkoepMenu, "&8  Twill 4/3", "&8  Köper 4/3", 4, 3);
    addKoeperItem(kettkoepMenu, "&9  Twill 5/3", "&9  Köper 5/3", 5, 3);

    /*  ---------- &Additional (MenuWeitere) ------------------ */
    /*  Legacy layout: "Weitere" is a submenu of "Einfügen" holding the
        named user-pattern slots. "Verwaltung" sits at the Einfügen level
        so the user can reach it to create the first pattern even while
        Weitere is hidden (it hides itself when no slots have data). */
    QMenu* additionalMenu = addSubmenu(insertMenu, "&Additional", "&Weitere");
    MenuWeitere = additionalMenu;
    for (int i = 0; i < MAXUSERDEF; i++) {
        UserdefAct[i] = additionalMenu->addAction(QString());
        UserdefAct[i]->setVisible(false);
        connect(UserdefAct[i], &QAction::triggered, this, [this, i]() {
            const bool transp = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
            InsertUserdef(i, transp);
        });
    }
    QMenu* udCfg = addSubmenu(insertMenu, "A&dministration", "&Verwaltung");
    QAction* actUdAdd = menuAct(udCfg, "&Add current pattern...", "&Muster hinzufügen...");
    QAction* actUdAddSel = menuAct(udCfg, "Add &selection...", "&Selektion hinzufügen...");
    QAction* actUdDel = menuAct(udCfg, "&Remove...", "&Löschen...");
    connect(actUdAdd, &QAction::triggered, this, [this] { UserdefAddClick(); });
    connect(actUdAddSel, &QAction::triggered, this, [this] { UserdefAddSelClick(); });
    connect(actUdDel, &QAction::triggered, this, [this] { UserdefRemoveClick(); });

    /*  ---------- &Repeat (MenuRapport) ---------------------- */
    QMenu* repeatMenu = addMenu(menuBar(), "&Repeat", "&Rapport");
    registerLang(RappViewRapport, QStringLiteral("&Repeat visible"),
                 QStringLiteral("&Rapport sichtbar"), QStringLiteral("Toggles repeat"),
                 QStringLiteral("Rapport sichtbar/unsichtbar"));
    RappViewRapport->setIcon(legacyIcon("mn_repeatvisible"));
    RappViewRapport->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    repeatMenu->addAction(RappViewRapport);
    repeatMenu->addSeparator();
    QAction* actRappExtend = menuAct(repeatMenu, "R&epeat...", "R&apportieren...", "mn_repeat",
                                     "Repeats the pattern", "Rapportiert das Muster");
    actRappExtend->setShortcut(QKeySequence(Qt::Key_F8));
    QAction* actRappReduce
        = menuAct(repeatMenu, "Re&duce", "Re&duzieren", nullptr,
                  "Reduces the pattern to 1x1 repeat", "Reduziert das Muster auf einen Rapport");
    actRappReduce->setShortcut(QKeySequence(Qt::Key_F7));
    registerLang(Inverserepeat, QStringLiteral("&Invert repeat colors"),
                 QStringLiteral("Rapportfarben &umgekehrt"));
    Inverserepeat->setIcon(legacyIcon("mn_inverserepeat"));
    repeatMenu->addAction(Inverserepeat);
    connect(actRappExtend, &QAction::triggered, this, [this] { RappRapportierenClick(); });
    connect(actRappReduce, &QAction::triggered, this, [this] { RappReduzierenClick(); });
    connect(RappViewRapport, &QAction::triggered, this, relayout);
    /*  CursorLocked is only meaningful while the repeat is visible;
        mirror legacy's dbw3_form.cpp:865 which auto-unchecks it when
        the repeat is hidden.                                         */
    connect(RappViewRapport, &QAction::toggled, this, [this](bool on) {
        if (!CursorLocked)
            return;
        if (!on && CursorLocked->isChecked())
            CursorLocked->setChecked(false);
        CursorLocked->setEnabled(on);
    });
    connect(Inverserepeat, &QAction::triggered, this, [this] { refresh(); });

    /*  ---------- &Color (MenuFarbe) ------------------------- */
    QMenu* colorMenu = addMenu(menuBar(), "&Color", "&Farbe");
    if (ViewFarbpalette) {
        registerLang(ViewFarbpalette, QStringLiteral("&Palette"), QStringLiteral("&Palette"),
                     QStringLiteral("Toggles color palette"),
                     QStringLiteral("Farbpalette sichtbar/unsichtbar"));
        ViewFarbpalette->setIcon(legacyIcon("mn_palette"));
        ViewFarbpalette->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
        colorMenu->addAction(ViewFarbpalette);
    }
    QAction* actDefColors
        = menuAct(colorMenu, "&Edit colors...", "&Farben bearbeiten...", nullptr,
                  "Edits the colors of the palette", "Bearbeitet die Paletten-Farben");
    QAction* actFarbverlauf
        = menuAct(colorMenu, "Color &blending...", "Farb&verlauf...", "mn_colorblend",
                  "Creates a color blending", "Erstellt einen Farbverlauf");
    colorMenu->addSeparator();
    QAction* actSetKettf = menuAct(colorMenu, "Set &warp color...", "&Kettfarbe setzen...", nullptr,
                                   "Sets the warp color", "Setzt die Kettfarbe");
    QAction* actSetSchuf = menuAct(colorMenu, "Set w&eft color...", "&Schussfarbe setzen...",
                                   nullptr, "Sets the weft color", "Setzt die Schussfarbe");
    QAction* actReplColor
        = menuAct(colorMenu, "&Replace color...", "Farbe e&rsetzen...", "mn_colorreplace",
                  "Replaces the color at the cursor position globally",
                  "Ersetzt die Farbe an der Cursorposition global");
    colorMenu->addSeparator();
    QAction* actKwieS = menuAct(colorMenu, "Warp colors as weft colors", "Kett- wie Schussfarben");
    connect(actKwieS, &QAction::triggered, this, [this] { KettfarbenWieSchussfarbenClick(); });
    QAction* actSwieK = menuAct(colorMenu, "Weft colors as warp colors", "Schuss- wie Kettfarben");
    connect(actSwieK, &QAction::triggered, this, [this] { SchussfarbenWieKettfarbenClick(); });
    QAction* actSwapCols
        = menuAct(colorMenu, "&Switch colors", "F&arben vertauschen", nullptr,
                  "Switches warp and weft colors", "Vertauscht Kett- und Schussfarben");
    connect(actDefColors, &QAction::triggered, this, [this] { DefineColorsClick(); });
    connect(actFarbverlauf, &QAction::triggered, this, [this] { FarbverlaufClick(); });
    connect(actSetKettf, &QAction::triggered, this, [this] { SetKettfarbeClick(); });
    connect(actSetSchuf, &QAction::triggered, this, [this] { SetSchussfarbeClick(); });
    connect(actReplColor, &QAction::triggered, this, [this] { ReplaceColorClick(); });
    connect(actSwapCols, &QAction::triggered, this, [this] { SwitchColorsClick(); });

    /*  ---------- Ra&nge (MenuBereich) ----------------------- */
    QMenu* rangeMenu = addMenu(menuBar(), "Ra&nge", "Bere&iche");
    QAction* actImportBmp = menuAct(rangeMenu, "&Import bitmap...", "&Bild importieren...", nullptr,
                                    "Imports a bitmap", "Importiert ein Bild");
    QMenu* currentRangeMenu = addSubmenu(rangeMenu, "&Current range", "&Aktueller Bereich");
    auto* rangeMenuGroup = new QActionGroup(this);
    rangeMenuGroup->setExclusive(true);
    QAction* menuRangeActs[9];
    for (int r = 1; r <= 9; r++) {
        const QByteArray en = QStringLiteral("Range &%1").arg(r).toUtf8();
        const QByteArray de = QStringLiteral("Bereich &%1").arg(r).toUtf8();
        const QByteArray en_tip = QStringLiteral("Activates range %1").arg(r).toUtf8();
        const QByteArray de_tip = QStringLiteral("Aktiviert Bereich %1").arg(r).toUtf8();
        QAction* a = menuAct(currentRangeMenu, en.constData(), de.constData(), nullptr,
                             en_tip.constData(), de_tip.constData());
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
    QAction* actRAush = menuAct(currentRangeMenu, "&Lift out", "&Aushebung", nullptr,
                                "Activates lift out range", "Aktiviert Aushebungs-Bereich");
    actRAush->setCheckable(true);
    actRAush->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_0));
    QAction* actRAnb = menuAct(currentRangeMenu, "&Binding", "A&nbindung", nullptr,
                               "Activates binding point range", "Aktiviert Anbindungs-Bereich");
    actRAnb->setCheckable(true);
    actRAnb->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    QAction* actRAbb = menuAct(currentRangeMenu, "&Unbinding", "A&bbindung", nullptr,
                               "Activates unbinding point range", "Aktiviert Abbindungs-Bereich");
    actRAbb->setCheckable(true);
    actRAbb->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_0));
    rangeMenuGroup->addAction(actRAush);
    rangeMenuGroup->addAction(actRAnb);
    rangeMenuGroup->addAction(actRAbb);
    connect(actRAush, &QAction::triggered, this, [this] {
        currentrange = AUSHEBUNG;
        refresh();
    });
    connect(actRAnb, &QAction::triggered, this, [this] {
        currentrange = ANBINDUNG;
        refresh();
    });
    connect(actRAbb, &QAction::triggered, this, [this] {
        currentrange = ABBINDUNG;
        refresh();
    });
    QAction* actBlockmuster
        = menuAct(rangeMenu, "&Block substitution...", "&Blockmusterung...", nullptr,
                  "Creates a block substitution", "Erstellt eine Blockmusterung");
    QAction* actRangeSubst
        = menuAct(rangeMenu, "&Range substitution...", "B&ereichsmusterung...", nullptr,
                  "Creates a range substitution", "Erstellt eine Bereichsmusterung");
    connect(actImportBmp, &QAction::triggered, this, [this] { ImportBitmapClick(); });
    connect(actBlockmuster, &QAction::triggered, this, [this] { EditBlockmusterClick(); });
    connect(actRangeSubst, &QAction::triggered, this, [this] { RangePatternsClick(); });

    /*  ---------- E&xtras (MenuExtras) ----------------------- */
    QMenu* extrasMenu = addMenu(menuBar(), "E&xtras", "E&xtras");
    OptionsLockGewebe = menuAct(extrasMenu, "Pattern &locked", "&Bindung gesperrt", nullptr,
                                "Locks the pattern; No changes can be made anymore",
                                "Sperrt die Bindung; Keine Änderungen mehr möglich");
    OptionsLockGewebe->setCheckable(true);
    connect(OptionsLockGewebe, &QAction::toggled, this, [this](bool) {
        /*  Status-bar shows "Bindung gesperrt" while locked. If the
            cursor is currently sitting on gewebe when the lock turns
            on, move it to the next traversable field -- gewebe is
            now skipped by Tab / Shift-Tab and by mouse clicks.     */
        if (OptionsLockGewebe->isChecked() && kbd_field == GEWEBE && cursorhandler)
            cursorhandler->GotoNextField();
        UpdateStatusBar();
        refresh();
    });
    QMenu* cursorMenu = addSubmenu(extrasMenu, "&Cursor", "&Cursor");
    CursorLocked = menuAct(cursorMenu, "&Contained", "&In Rapport", nullptr,
                           "Restricts the cursor position within the repeat",
                           "Beschränkt die Cursorposition auf den Rapport");
    CursorLocked->setCheckable(true);
    CursorLocked->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    /*  Legacy gate: only meaningful when the repeat is visible, so the
        toggle follows RappViewRapport's checked state.                */
    CursorLocked->setEnabled(RappViewRapport && RappViewRapport->isChecked());
    connect(CursorLocked, &QAction::toggled, this, [this](bool on) {
        if (cursorhandler)
            cursorhandler->SetCursorLocked(on);
        refresh();
    });
    QAction* actCursorDir = menuAct(cursorMenu, "&Movement...", "&Bewegung...", nullptr,
                                    "Configures the automatic cursor movement",
                                    "Konfiguriert die automatische Cursorbewegung");
    QAction* actCursorGoto = menuAct(cursorMenu, "&Go to...", "&Gehe zu...", nullptr,
                                     "Sets the cursor to a specific field",
                                     "Setzt den Cursor in ein spezifisches Feld");
    actCursorGoto->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    connect(actCursorDir, &QAction::triggered, this, [this] { EditCursordirectionClick(); });
    connect(actCursorGoto, &QAction::triggered, this, [this] { CursorGotoClick(); });
    QAction* actVerhaeltnis
        = menuAct(extrasMenu, "&Weft/warp-ratio...", "Schuss/&Kett-Verhältnis...", nullptr,
                  "Sets the weft/warp-ratio", "Setzt das Schuss/Kett-Verhältnis");
    connect(actVerhaeltnis, &QAction::triggered, this, [this] { VerhaeltnisClick(); });
    registerLang(ViewSchlagpatrone, QStringLiteral("&Pegplan"), QStringLiteral("&Schlagpatrone"),
                 QStringLiteral("Toggles pegplan mode"),
                 QStringLiteral("Wechselt zwischen Schlagpatrone und Aufknüpfung/Trittfolge"));
    ViewSchlagpatrone->setShortcut(QKeySequence(Qt::Key_F9));
    extrasMenu->addAction(ViewSchlagpatrone);
    connect(ViewSchlagpatrone, &QAction::triggered, this, [this] { ToggleSchlagpatrone(); });
    QAction* actWeave = menuAct(extrasMenu, "&Weave", "&Weben", nullptr,
                                "Switches into the weaving mode", "Wechselt in den Weben-Modus");
    actWeave->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));
    connect(actWeave, &QAction::triggered, this, [this] { LoomControlClick(); });
    QAction* actViewInfos = menuAct(extrasMenu, "&Information...", "&Informationen...", nullptr,
                                    "Displays information about the current pattern",
                                    "Zeigt Informationen über das aktuelle Muster an");
    connect(actViewInfos, &QAction::triggered, this, [this] { ViewInfosClick(); });
    extrasMenu->addSeparator();
    QMenu* baseMenu = addSubmenu(extrasMenu, "&Base settings", "&Grundeinstellung");
    QAction* actOptAm
        = menuAct(baseMenu, "&American", "&Amerikanisch", nullptr,
                  "Activates the settings commonly used in the united states",
                  "Aktiviert die Einstellungen, die gewöhnlich in den USA verwendet werden");
    connect(actOptAm, &QAction::triggered, this, [this] { OptAmericanClick(); });
    QAction* actOptSk = menuAct(baseMenu, "S&candinavian", "&Skandinavisch", nullptr,
                                "Activates the settings commonly used in the scandinavian region",
                                "Aktiviert die Einstellungen, die gewöhnlich in der "
                                "skandinavischen Region verwendet werden");
    connect(actOptSk, &QAction::triggered, this, [this] { OptSkandinavischClick(); });
    QAction* actOptSw = menuAct(baseMenu, "&German/Swiss", "&Deutsch/Schweizerisch", nullptr,
                                "Activates the settings commonly used in Germany and Switzerland",
                                "Aktiviert die Einstellungen, die gewöhnlich in Deutschland und "
                                "der Schweiz verwendet werden");
    connect(actOptSw, &QAction::triggered, this, [this] { OptSwissClick(); });
    QMenu* optMenu = addSubmenu(extrasMenu, "&Options", "&Optionen");
    QAction* actXOpt
        = menuAct(optMenu, "&For this pattern...", "&Für dieses Muster...", nullptr,
                  "Edits options for this pattern", "Bearbeitet Optionen für dieses Muster");
    QAction* actXOptG = menuAct(optMenu, "&Global...", "&Global...", nullptr,
                                "Edits options for this and all new patterns",
                                "Bearbeitet Optionen für dieses und alle neuen Muster");
    optMenu->addSeparator();
    QAction* actEnvOpt = menuAct(optMenu, "&Environment...", "&Umgebung...", nullptr,
                                 "Edits environment options", "Bearbeitet Umgebungsoptionen");
    connect(actXOpt, &QAction::triggered, this, [this] { XOptionsClick(); });
    connect(actXOptG, &QAction::triggered, this, [this] { XOptionsGlobalClick(); });
    connect(actEnvOpt, &QAction::triggered, this, [this] { OptEnvironmentClick(); });

    /*  ---------- &? (MenuHelp) ------------------------------ */
    QMenu* helpMenu = addMenu(menuBar(), "&?", "&?");
    QAction* actHilfe = menuAct(helpMenu, "&Help topics", "&Hilfethemen");
    actHilfe->setShortcut(QKeySequence(Qt::Key_F1));
    connect(actHilfe, &QAction::triggered, this, [] {
        const QString url = (active_language == GE)
                                ? QStringLiteral("https://www.brunoldsoftware.ch/hilfe")
                                : QStringLiteral("https://www.brunoldsoftware.ch/help");
        QDesktopServices::openUrl(QUrl(url));
    });
    QAction* actBsoftOnline
        = menuAct(helpMenu, "&Brunold Software Online", "&Brunold Software Online", nullptr,
                  "Brings you to the website of Brunold Software",
                  "Bringt Sie auf die Website von Brunold Software");
    connect(actBsoftOnline, &QAction::triggered, this, [] {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://www.brunoldsoftware.ch/")));
    });
    helpMenu->addSeparator();
    QAction* actTechInfo
        = menuAct(helpMenu, "&Technical information...", "&Technische Informationen...", nullptr,
                  "Displays technical information about DB-WEAVE, your computer and the operating "
                  "system used",
                  "Zeigt technische Informationen über DB-WEAVE, Ihren Computer und das verwendete "
                  "Betriebssystem an");
    QAction* actAbout
        = menuAct(helpMenu, "About &DB-WEAVE...", "Über &DB-WEAVE...", nullptr,
                  "Displays information about DB-WEAVE", "Zeigt Informationen über DB-WEAVE an");
    connect(actTechInfo, &QAction::triggered, this, [this] { TechinfoDialog(this).exec(); });
    connect(actAbout, &QAction::triggered, this, [this] { AboutDialog(this).exec(); });

    /*  ---------- Toolbar (legacy TToolBar) ------------------
        24 buttons matching legacy/dbw3_form.dfm's toolbar layout:
        File (New/Open/Save), Print (Preview/Print),
        History (Undo/Redo), Clipboard (Cut/Copy/Paste),
        Zoom (In/Out), Move (ShaftUp/Down, TrittLeft/Right),
        Transforms (Invert/MirrorH/MirrorV/Rotate/Centralsym/
        Highlight).                                             */
    QToolBar* mainBar = addToolBar(QString());
    mainBar->setObjectName(QStringLiteral("mainToolBar"));
    registerLangWidget(mainBar, QStringLiteral("Toolbar"), QStringLiteral("Symbolleiste"));
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
    QAction* actHighlight
        = new QAction(legacyIcon("sb_highlight"), QStringLiteral("Highlight"), this);
    actHighlight->setShortcut(QKeySequence(Qt::Key_F12));
    actHighlight->setToolTip(QStringLiteral("Highlight (F12) -- Highlights the dependent parts"));
    actHighlight->setCheckable(true);
    connect(actHighlight, &QAction::toggled, this, [this](bool _on) {
        highlight = _on;
        refresh();
    });
    mainBar->addAction(actHighlight);

    /*  Range picker: nine checkable range buttons + three special
        (L/B/U) buttons, hosted in a vertical QToolBar pinned to the
        side panel. Icon-only: a colour swatch per numbered range, a
        bold letter per special range. Toggled via the View menu.    */
    QToolBar* rangeBar = new QToolBar(sidePanel);
    rangeBar->setOrientation(Qt::Vertical);
    rangeBar->setIconSize(QSize(16, 16));
    rangeBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    rangeBar->setMovable(false);
    rangeBar->setFloatable(false);
    sideRow->addWidget(rangeBar, /*stretch=*/0, Qt::AlignTop);
    rangeBar->hide();
    rangeToolbar = rangeBar;
    rangeGroup = new QActionGroup(this);
    rangeGroup->setExclusive(true);
    for (int r = 1; r <= 9; r++) {
        QAction* a = new QAction(this);
        a->setIcon(swatchIcon(qColorFromTColor(GetRangeColor(r))));
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
    auto addSpecial = [&](int r, const QString& tip) {
        QAction* a = new QAction(this);
        a->setCheckable(true);
        a->setToolTip(tip);
        rangeGroup->addAction(a);
        rangeBar->addAction(a);
        connect(a, &QAction::triggered, this, [this, r] {
            currentrange = r;
            refresh();
        });
        return a;
    };
    rangeAushebungAction = addSpecial(AUSHEBUNG, QStringLiteral("Lift out (Aushebung)"));
    rangeAnbindungAction = addSpecial(ANBINDUNG, QStringLiteral("Binding (Anbindung)"));
    rangeAbbindungAction = addSpecial(ABBINDUNG, QStringLiteral("Unbinding (Abbindung)"));
    updateRangeDockIcons();
    ViewRanges = new QAction(this);
    QAction* actViewRanges = ViewRanges;
    actViewRanges->setCheckable(true);
    actViewRanges->setChecked(false);
    registerLang(actViewRanges, QStringLiteral("&Ranges"), QStringLiteral("&Bereiche"),
                 QStringLiteral("Toggles the ranges toolbar"),
                 QStringLiteral("Bereiche-Leiste sichtbar/unsichtbar"));
    connect(actViewRanges, &QAction::toggled, this, [this, rangeBar](bool on) {
        rangeBar->setVisible(on);
        updateSidePanelVisibility();
    });
    viewMenu->addAction(actViewRanges);

    /*  Drawing tools -- icon-only vertical toolbar pinned to the side
        panel below the range strip. Toggle from the View menu. Tools
        are radio-checkable so the active tool is visually highlighted. */
    QToolBar* toolsBar = new QToolBar(sidePanel);
    toolsBar->setOrientation(Qt::Vertical);
    toolsBar->setIconSize(QSize(16, 16));
    toolsBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolsBar->setMovable(false);
    toolsBar->setFloatable(false);
    sideRow->addWidget(toolsBar, /*stretch=*/0, Qt::AlignTop);
    toolsBar->hide();
    toolsToolbar = toolsBar;
    auto* toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);
    auto addTool = [&](const QString& tip, const char* icon, TOOL _t) {
        QAction* a = new QAction(this);
        a->setIcon(legacyIcon(icon));
        a->setToolTip(tip);
        a->setCheckable(true);
        toolGroup->addAction(a);
        toolsBar->addAction(a);
        if (_t == TOOL_POINT)
            a->setChecked(true);
        connect(a, &QAction::triggered, this, [this, _t] { tool = _t; });
    };
    addTool(QStringLiteral("Point"), "tool_cursor", TOOL_POINT);
    addTool(QStringLiteral("Line"), "tool_line", TOOL_LINE);
    addTool(QStringLiteral("Rectangle"), "tool_rectangle", TOOL_RECTANGLE);
    addTool(QStringLiteral("Filled rectangle"), "tool_filledrectangle", TOOL_FILLEDRECTANGLE);
    addTool(QStringLiteral("Ellipse"), "tool_ellipse", TOOL_ELLIPSE);
    addTool(QStringLiteral("Filled ellipse"), "tool_filledellipse", TOOL_FILLEDELLIPSE);
    ViewTools = new QAction(this);
    QAction* actViewTools = ViewTools;
    actViewTools->setCheckable(true);
    actViewTools->setChecked(false);
    registerLang(actViewTools, QStringLiteral("&Tools"), QStringLiteral("&Werkzeuge"),
                 QStringLiteral("Toggles the tools toolbar"),
                 QStringLiteral("Werkzeugleiste sichtbar/unsichtbar"));
    connect(actViewTools, &QAction::toggled, this, [this, toolsBar](bool on) {
        toolsBar->setVisible(on);
        updateSidePanelVisibility();
    });
    viewMenu->addAction(actViewTools);
    /*  Palette toggle lives on the &Color menu already but is handy
        in View too, alongside the other toolbar toggles.          */
    if (ViewFarbpalette)
        viewMenu->addAction(ViewFarbpalette);

    /*  Append the palette as the rightmost item so toolbars render to
        its left. The scroll area takes stretch=0 (its width is fixed)
        but expands vertically because QHBoxLayout fills the row height
        for items without an explicit alignment.                      */
    sideRow->addWidget(paletteScroll, /*stretch=*/0);
    centralRow->addWidget(sidePanel, /*stretch=*/0);
    setCentralWidget(central);
    updateSidePanelVisibility();

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

    /*  Apply the initial pegplan-vs-trittfolge menu layout. The saved
        PegplanMode preference will be honoured when the file loader
        / startup path toggles ViewSchlagpatrone; calling this now
        ensures the "Pegplan" top-level menu is hidden in the default
        (trittfolge) state.                                          */
    UpdateSchlagpatroneMode();

    /*  Seed the runtime with the user's saved Grundeinstellung so a
        fresh launch starts in the user's preferred region-defaults.
        A pattern load overwrites these from the file.              */
    ApplyBaseStyleFromSettings();

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

    /*  Seed the undo stack with a snapshot of the fresh-document
        state so the user's first action on the startup document is
        undoable. Without this, UrUndo::Undo() returns false because
        current->Prev() is still empty (ResetDocument takes the same
        snapshot on File > New).                                  */
    if (undo)
        undo->Snapshot();
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

void TDBWFRM::updateSidePanelVisibility()
{
    if (!sidePanel)
        return;
    /*  Collapse the side panel when none of its children are visible
        so the canvas reclaims the column. */
    const bool any = (paletteScroll && paletteScroll->isVisibleTo(sidePanel))
                     || (rangeToolbar && rangeToolbar->isVisibleTo(sidePanel))
                     || (toolsToolbar && toolsToolbar->isVisibleTo(sidePanel));
    sidePanel->setVisible(any);
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
    if (pattern_canvas) {
        /*  Keyboard viewport scrolls (Home/End/PageUp/PageDown) and
            other paths mutate frm->scroll_* directly; push those
            values into the scrollbar thumb so the UI stays in sync. */
        pattern_canvas->syncScrollbarsFromFrm();
        pattern_canvas->update();
    }
    /*  Palette contents / alt-palette toggle can change via the
        Options dialog, the Define Colors dialog, or a file load;
        those paths all end in refresh(), so repainting the dock
        here keeps its swatches and selection in sync. */
    if (palettePanel)
        palettePanel->update();
    /*  The three special-range dock icons mirror darst_aushebung /
        _anbindung / _abbindung, which the xoptions dialog can
        change. Regenerate the icons here so the dock preview
        always matches how the gewebe paints the same cell.      */
    updateRangeDockIcons();
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

void TDBWFRM::registerLang(QAction* _a, const QString& _en, const QString& _de,
                           const QString& _en_tip, const QString& _de_tip)
{
    if (!_a)
        return;
    LangEntry e;
    e.action = _a;
    e.en_text = _en;
    e.de_text = _de;
    e.en_tip = _en_tip;
    e.de_tip = _de_tip;
    langEntries.push_back(e);
    const bool de = (active_language == GE);
    _a->setText(de ? _de : _en);
    if (!_en_tip.isEmpty() || !_de_tip.isEmpty())
        _a->setToolTip(de ? _de_tip : _en_tip);
}

void TDBWFRM::registerLangMenu(QMenu* _m, const QString& _en, const QString& _de)
{
    if (!_m)
        return;
    LangEntry e;
    e.menu = _m;
    e.en_text = _en;
    e.de_text = _de;
    langEntries.push_back(e);
    _m->setTitle(active_language == GE ? _de : _en);
}

void TDBWFRM::registerLangWidget(QWidget* _w, const QString& _en, const QString& _de)
{
    if (!_w)
        return;
    LangEntry e;
    e.widget = _w;
    e.en_text = _en;
    e.de_text = _de;
    langEntries.push_back(e);
    _w->setWindowTitle(active_language == GE ? _de : _en);
}

void TDBWFRM::ReloadLanguage()
{
    const bool de = (active_language == GE);
    for (const LangEntry& e : langEntries) {
        if (e.action) {
            e.action->setText(de ? e.de_text : e.en_text);
            if (!e.en_tip.isEmpty() || !e.de_tip.isEmpty())
                e.action->setToolTip(de ? e.de_tip : e.en_tip);
        } else if (e.menu) {
            e.menu->setTitle(de ? e.de_text : e.en_text);
        } else if (e.widget) {
            e.widget->setWindowTitle(de ? e.de_text : e.en_text);
        }
    }
    /*  Window title follows the active pattern; just re-run the
        usual app-title builder. */
    SetAppTitle();
    /*  The View > Trittfolge caption depends on pegplan mode, which
        the langEntries registry doesn't know about. Let the dedicated
        helper fix it up (and the menu visibility along with it).    */
    UpdateSchlagpatroneMode();
}

/*-----------------------------------------------------------------*/
/*  Pegplan mode entry / exit. Port of legacy ToggleSchlagpatrone +
    ViewSchlagpatroneClick from dbw3_form.cpp.                     */
void TDBWFRM::ToggleSchlagpatrone()
{
    if (!ViewSchlagpatrone)
        return;
    /*  Qt already flipped isChecked() before emitting triggered().
        Read the new state and act on it.                           */
    const bool pegplan = ViewSchlagpatrone->isChecked();
    trittfolge.einzeltritt = !pegplan;
    if (!pegplan) {
        /*  Leaving pegplan: split the schlagpatrone back into a
            diagonal einzug-aligned trittfolge plus a compact
            aufknuepfung. Legacy also re-checks the saved trittfolge
            style radio; MinimalZ is the ctor default, so reassert
            that when we can't recover the previous one.             */
        if (TfMinimalZ)
            TfMinimalZ->setChecked(true);
        RecalcTrittfolgeAufknuepfung();
    } else {
        /*  Entering pegplan: project (einzug × aufknuepfung × trittfolge)
            into a single Schlagpatrone in the trittfolge field.     */
        RecalcSchlagpatrone();
    }
    SetModified();
    /*  The aufknuepfung field is gone while pegplan is active; jump
        the kbd cursor out of it if it's parked there.               */
    if (kbd_field == AUFKNUEPFUNG && cursorhandler) {
        cursorhandler->GotoNextField();
    }
    if (undo)
        undo->Snapshot();

    UpdateSchlagpatroneMode();
    if (pattern_canvas)
        pattern_canvas->recomputeLayout();
    refresh();

    /*  Persist the preference so the next session starts in the same
        mode. Matches legacy "Divers / PegplanMode".                  */
    Settings settings;
    settings.SetCategory(QString("Divers"));
    settings.Save(QString("PegplanMode"), pegplan ? 1 : 0);
}

/*-----------------------------------------------------------------*/
void TDBWFRM::UpdateSchlagpatroneMode()
{
    const bool pegplan = ViewSchlagpatrone && ViewSchlagpatrone->isChecked();
    const bool de = (active_language == GE);
    /*  View > Trittfolge caption switches to Pegplan/Schlagpatrone in
        pegplan mode. Overrides whatever ReloadLanguage set.          */
    if (ViewTrittfolge) {
        if (pegplan) {
            ViewTrittfolge->setText(de ? QStringLiteral("Sch&lagpatrone")
                                       : QStringLiteral("Pegpl&an"));
            ViewTrittfolge->setToolTip(de ? QStringLiteral("Schlagpatrone sichtbar/unsichtbar")
                                          : QStringLiteral("Toggles pegplan"));
        } else {
            ViewTrittfolge->setText(de ? QStringLiteral("&Trittfolge")
                                       : QStringLiteral("Tre&adling"));
            ViewTrittfolge->setToolTip(de ? QStringLiteral("Trittfolge sichtbar/unsichtbar")
                                          : QStringLiteral("Toggles treadling"));
        }
    }
    /*  Top-level Treadling / Pegplan menus swap visibility with the
        active mode. */
    if (treadMenu && treadMenu->menuAction())
        treadMenu->menuAction()->setVisible(!pegplan);
    if (spMenu && spMenu->menuAction())
        spMenu->menuAction()->setVisible(pegplan);
    /*  "Copy from treadling" only makes sense with a separate
        trittfolge field -- hide it while pegplan is active. */
    if (actCopyEzTf)
        actCopyEzTf->setVisible(!pegplan);
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
void TDBWFRM::SetModified(bool _modified)
{
    modified = _modified;
    SetAppTitle();
}

/*-----------------------------------------------------------------*/
/*  Grundeinstellung presets. Two-level update: the choice is always
    persisted globally (so File > New picks it up and the next
    session starts with it) and also applied to the in-memory view
    state. Only when the current document has non-trivial content
    does the preset change ALSO mark the pattern as modified and
    snapshot undo — the idea being that the user just changed a
    viewable property of the loaded pattern. For an empty freshly-
    launched session, the preset just updates the global defaults
    without leaving a bogus "unsaved changes" state.

    A subsequent file load overrides the in-memory view with the
    file's own settings (LoadViewGeneral in fileload.cpp) — this
    helper doesn't change that. The user's saved Grundeinstellung
    only wins over file-stored values when File > New creates a
    fresh document.                                               */
namespace
{
void saveBaseStyle(DARSTELLUNG ezD, DARSTELLUNG aufD, DARSTELLUNG tfD, DARSTELLUNG spD,
                   bool einzugunten, bool righttoleft, bool toptobottom, bool sinkingshed,
                   bool reedVisible)
{
    Settings settings;
    settings.SetCategory(QString("Display"));
    settings.Save(QString("Threading"), int(ezD));
    settings.Save(QString("Tie-up"), int(aufD));
    settings.Save(QString("Treadling"), int(tfD));
    settings.Save(QString("Pegplan"), int(spD));
    settings.SetCategory(QString("View"));
    settings.Save(QString("ThreadingDown"), einzugunten ? 1 : 0);
    settings.Save(QString("RightToLeft"), righttoleft ? 1 : 0);
    settings.Save(QString("TopToBottom"), toptobottom ? 1 : 0);
    settings.SetCategory(QString("Settings"));
    settings.Save(QString("SinkingShed"), sinkingshed ? 1 : 0);
    settings.SetCategory(QString("Divers"));
    settings.Save(QString("ViewReedthreading"), reedVisible ? 1 : 0);
}
} /* anonymous namespace */

bool TDBWFRM::HasNonTrivialContent()
{
    if (kette.b != -1 || schuesse.b != -1)
        return true;
    for (int j = 0; j < Data->MAXY1; j++)
        if (!IsFreeSchaft(j))
            return true;
    for (int i = 0; i < Data->MAXX2; i++)
        if (!IsFreeTritt(i))
            return true;
    return false;
}

void TDBWFRM::ApplyBaseStyleFromSettings()
{
    Settings settings;
    settings.SetCategory(QString("Display"));
    einzug.darstellung = DARSTELLUNG(settings.Load(QString("Threading"), int(STRICH)));
    aufknuepfung.darstellung = DARSTELLUNG(settings.Load(QString("Tie-up"), int(KREUZ)));
    trittfolge.darstellung = DARSTELLUNG(settings.Load(QString("Treadling"), int(PUNKT)));
    schlagpatronendarstellung = DARSTELLUNG(settings.Load(QString("Pegplan"), int(AUSGEFUELLT)));
    darst_aushebung = DARSTELLUNG(settings.Load(QString("LiftOut"), int(STEIGEND)));
    darst_anbindung = DARSTELLUNG(settings.Load(QString("Binding"), int(KREUZ)));
    darst_abbindung = DARSTELLUNG(settings.Load(QString("Unbinding"), int(KREIS)));
    settings.SetCategory(QString("View"));
    einzugunten = (settings.Load(QString("ThreadingDown"), 0) != 0);
    righttoleft = (settings.Load(QString("RightToLeft"), 0) != 0);
    toptobottom = (settings.Load(QString("TopToBottom"), 0) != 0);
    settings.SetCategory(QString("Settings"));
    sinkingshed = (settings.Load(QString("SinkingShed"), 0) != 0);
    settings.SetCategory(QString("Divers"));
    if (ViewBlatteinzug)
        ViewBlatteinzug->setChecked(settings.Load(QString("ViewReedthreading"), 1) != 0);
}

void TDBWFRM::OptSwissClick()
{
    einzug.darstellung = STRICH;
    aufknuepfung.darstellung = KREUZ;
    trittfolge.darstellung = PUNKT;
    schlagpatronendarstellung = AUSGEFUELLT;
    righttoleft = false;
    toptobottom = false;
    sinkingshed = false;
    einzugunten = false;
    if (ViewBlatteinzug)
        ViewBlatteinzug->setChecked(true);
    saveBaseStyle(einzug.darstellung, aufknuepfung.darstellung, trittfolge.darstellung,
                  schlagpatronendarstellung, einzugunten, righttoleft, toptobottom, sinkingshed,
                  true);
    if (pattern_canvas)
        pattern_canvas->recomputeLayout();
    if (HasNonTrivialContent()) {
        if (undo)
            undo->Snapshot();
        SetModified();
    }
    refresh();
}

void TDBWFRM::OptSkandinavischClick()
{
    einzug.darstellung = AUSGEFUELLT;
    aufknuepfung.darstellung = AUSGEFUELLT;
    trittfolge.darstellung = AUSGEFUELLT;
    schlagpatronendarstellung = AUSGEFUELLT;
    righttoleft = true;
    toptobottom = true;
    sinkingshed = true;
    einzugunten = true;
    if (ViewBlatteinzug)
        ViewBlatteinzug->setChecked(false);
    saveBaseStyle(einzug.darstellung, aufknuepfung.darstellung, trittfolge.darstellung,
                  schlagpatronendarstellung, einzugunten, righttoleft, toptobottom, sinkingshed,
                  false);
    if (pattern_canvas)
        pattern_canvas->recomputeLayout();
    if (HasNonTrivialContent()) {
        if (undo)
            undo->Snapshot();
        SetModified();
    }
    refresh();
}

void TDBWFRM::OptAmericanClick()
{
    einzug.darstellung = AUSGEFUELLT;
    aufknuepfung.darstellung = AUSGEFUELLT;
    trittfolge.darstellung = AUSGEFUELLT;
    schlagpatronendarstellung = AUSGEFUELLT;
    righttoleft = true;
    toptobottom = false;
    sinkingshed = false;
    einzugunten = false;
    if (ViewBlatteinzug)
        ViewBlatteinzug->setChecked(true);
    saveBaseStyle(einzug.darstellung, aufknuepfung.darstellung, trittfolge.darstellung,
                  schlagpatronendarstellung, einzugunten, righttoleft, toptobottom, sinkingshed,
                  true);
    if (pattern_canvas)
        pattern_canvas->recomputeLayout();
    if (HasNonTrivialContent()) {
        if (undo)
            undo->Snapshot();
        SetModified();
    }
    refresh();
}
void TDBWFRM::SetCursor(int _i, int _j)
{
    if (cursorhandler)
        cursorhandler->SetCursor(kbd_field, _i, _j, /*clearselection=*/true);
}
void TDBWFRM::SetAppTitle()
{
    const QString untitled = LANG_STR("Untitled", "Unbenannt");
    QString shown;
    if (filename.isEmpty())
        shown = untitled;
    else
        shown = QFileInfo((QString)filename).fileName();
    if (modified)
        shown += QStringLiteral("*");
    setWindowTitle(shown + QStringLiteral(" - DB-WEAVE"));
}
void TDBWFRM::UpdateScrollbars()
{
    if (pattern_canvas)
        pattern_canvas->syncScrollbarsFromFrm();
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
    DrawGewebeSimulation all live in draw.cpp. */
