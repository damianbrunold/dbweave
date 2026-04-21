/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Phase-5 skeleton of the pattern-editor document window. In the
    legacy VCL codebase TDBWFRM is a 1200-line class declaration with
    dozens of event handlers, menu items, scrollbars, and owned Feld*
    subfields. The port reconstructs it incrementally: this first slice
    is just enough scaffolding that ported units (init.cpp, clear.cpp,
    ...) can reach for `DBWFRM->scroll_x1` etc. without breaking the
    build.

    Members are added as the units that reference them are ported.
*/

#ifndef DBWEAVE_UI_MAINWINDOW_H
#define DBWEAVE_UI_MAINWINDOW_H

#include <QAction>
#include <QColor>
#include <QMainWindow>
#include <QString>

#include <vector>

#include "vcl_compat.h"
#include "dbw3_base.h"     /* FeldBase hierarchy */
#include "hilfslinien.h"   /* Hilfslinien container */
#include "loadoptions.h"   /* LOADSTAT / LOADPARTS */
#include "blockmuster.h"   /* Muster / BlockUndo */
#include "userdef.h"       /* UserdefPattern / MAXUSERDEF */
#include "colors_compat.h" /* COLORREF */

class QPainter;
class PatternCanvas;
class UrUndo;
class RpRapport;
class EinzugRearrange;
class CrCursorHandler;
class FfFile;
class PalettePanel;
class QDockWidget;

class TDBWFRM : public QMainWindow
{
    Q_OBJECT

public:
    explicit TDBWFRM(QWidget* parent = nullptr);
    ~TDBWFRM() override;

    /*  Scroll offsets for each of the four scrollable grids. Legacy
        TDBWFRM declared them as plain int members; keeping the same
        names so the ported ScrollX()/ScrollY() bodies in init.cpp
        compile unchanged. */
    int scroll_x1 = 0;
    int scroll_x2 = 0;
    int scroll_y1 = 0;
    int scroll_y2 = 0;

    /*  The seven owned document fields. Legacy TDBWFRM declared these
        as by-value members; keeping that shape so the literal
        `DBWFRM->einzug.feld.Size()` references in ported code resolve
        without change. Defaults come from each struct's constructor
        (see src/ui/init.cpp).                                      */
    FeldEinzug einzug;
    FeldAufknuepfung aufknuepfung;
    FeldTrittfolge trittfolge;
    FeldGewebe gewebe;
    FeldKettfarben kettfarben;
    FeldSchussfarben schussfarben;
    FeldBlatteinzug blatteinzug;

    /*  Which field currently owns the keyboard cursor. */
    FELD kbd_field = GEWEBE;

    /*  Display-orientation flags. Legacy defaults: both false. */
    bool righttoleft = false;
    bool toptobottom = false;

    /*  Warp / weft ranges and the pattern rapport. */
    /*  Sentinel for "no range yet" is a = b = -1, matching legacy. The
        SZ default ctor initialises to (0, 0); override it here so a
        freshly-constructed TDBWFRM behaves the same as after File >
        New.                                                        */
    SZ kette { -1, -1 };
    SZ schuesse { -1, -1 };
    RAPPORT rapport;

    /*  Currently selected "range" (1..9 plus AUSHEBUNG/ANBINDUNG/
        ABBINDUNG). */
    int currentrange = 1;

    /*  Menu-action toggles. Allocated but left unchecked and detached
        from any menu bar until lang_main.cpp / the menu-port slice
        wires them up -- they exist now so ported code (undoredo,
        editor ops) that reads their ->isChecked() / ->setChecked()
        compiles. Owned by `this` (parent = QObject), so destroyed
        with the window. */
    QAction* EzMinimalZ = nullptr;
    QAction* EzMinimalS = nullptr;
    QAction* EzGeradeZ = nullptr;
    QAction* EzGeradeS = nullptr;
    QAction* EzChorig2 = nullptr;
    QAction* EzChorig3 = nullptr;
    QAction* EzBelassen = nullptr;
    QAction* EzFixiert = nullptr;

    QAction* TfMinimalZ = nullptr;
    QAction* TfMinimalS = nullptr;
    QAction* TfGesprungen = nullptr;
    QAction* TfBelassen = nullptr;

    QAction* OptionsLockGewebe = nullptr;

    QAction* ViewSchlagpatrone = nullptr;
    QAction* ViewEinzug = nullptr;
    QAction* ViewTrittfolge = nullptr;
    QAction* ViewOnlyGewebe = nullptr;
    QAction* GewebeNone = nullptr;
    QAction* GewebeNormal = nullptr;
    QAction* RappViewRapport = nullptr;
    QAction* GewebeFarbeffekt = nullptr;
    QAction* GewebeSimulation = nullptr;
    QAction* Inverserepeat = nullptr;
    QAction* ViewHlines = nullptr;

    /*  ViewFarbe toggles both colour strips together (kettfarben
        + schussfarben); ViewBlatteinzug toggles the reed-threading
        strip. Matches legacy menu layout where the colour strips
        share a single view toggle. */
    QAction* ViewFarbe = nullptr;
    QAction* ViewBlatteinzug = nullptr;

    /*  Guide lines (Hilfslinien) + the four grab-bar rectangles.
        hlinehorz1/2 sit above/below the pattern grids, hlinevert1/2
        to the left/right. Bar positions remain zero-sized until the
        FormResize port lands -- the hilfslinien renderer only needs
        them non-degenerate if the user wants the "tick into the bar"
        segment to show. */
    Hilfslinien hlines;
    HlineBar hlinehorz1;
    HlineBar hlinehorz2;
    HlineBar hlinevert1;
    HlineBar hlinevert2;

    /*  Symbol styles for the three special ranges (AUSHEBUNG,
        ANBINDUNG, ABBINDUNG). Legacy DrawGewebeNormal branches on
        each of these when painting the corresponding range cells. */
    /*  Legacy DB-WEAVE shipped these special-range appearances with
        symbolic glyphs (not filled cells): aushebung -> diagonal
        bottom-left-to-top-right line, anbindung -> cross on green,
        abbindung -> circle on yellow. Keeping that as the out-of-
        the-box default; the xoptions dialog can still override.    */
    DARSTELLUNG darst_aushebung = STEIGEND;
    DARSTELLUNG darst_anbindung = KREUZ;
    DARSTELLUNG darst_abbindung = KREIS;
    DARSTELLUNG schlagpatronendarstellung = AUSGEFUELLT;

    /*  GewebeFarbeffekt / GewebeSimulation state:
        sinkingshed  -- invert the "warp up" check for sinking-shed
                        looms.
        fewithraster -- in Farbeffekt mode, overlay the Rahmen after
                        the fill.
        faktor_kette / faktor_schuss -- thread-density factors for
                        Simulation (default 1.0).
        currentzoom  -- cell-size-derived zoom 0..9; picks shadow-
                        line width in Simulation.                   */
    bool sinkingshed = false;
    bool fewithraster = false;
    /*  true -> einzug renders below the gewebe (legacy
        EinzugUnten option). recomputeLayout currently ignores
        this flag; kept for option-dialog round-trip.          */
    bool einzugunten = false;
    /*  true -> use the alternative standard colour palette.  */
    bool palette2 = false;
    float faktor_kette = 1.0f;
    float faktor_schuss = 1.0f;
    int currentzoom = 4;

    /*  Strongline colour for the heavy grid lines. */
    QColor strongclr = QColor(Qt::black);

    /*  Cursor blink state. The QTimer flips this every half of
        QApplication::cursorFlashTime(); the canvas's paintEvent
        reads the flag to decide whether to draw the cursor
        outline. A value of zero from cursorFlashTime() means "no
        blink" -- we leave the cursor permanently visible. */
    bool cursorVisible = true;
    class QTimer* cursorTimer = nullptr;

    /*  Active QPainter during a paintEvent. nullptr outside paint,
        so legacy-style "frm->Canvas->..." ports can no-op safely.
        Set by PatternCanvas::paintEvent for the duration of the
        render, cleared afterwards.                              */
    QPainter* currentPainter = nullptr;

    /*  The central widget that renders the pattern. Owned via Qt
        parent-child ownership -- delete when the mainwindow dies. */
    PatternCanvas* pattern_canvas = nullptr;

    /*  Dockable 16x16 palette-picker panel. Visible toggle lives
        on the View menu + context menu.                         */
    QDockWidget* paletteDock = nullptr;
    PalettePanel* palettePanel = nullptr;
    QAction* ViewFarbpalette = nullptr;

    /*  Scratch buffers used by RcRecalcAll: xbuf[i] marks warp i
        as seen (non-empty) during RecalcEinzug; ybuf[j] does the
        same for wefts during RecalcTrittfolge. Sized to MAXX1 and
        MAXY2 respectively, owned, reallocated by AllocBuffers.  */
    char* xbuf = nullptr;
    char* ybuf = nullptr;

    /*  Undo stack + handlers, owned. */
    UrUndo* undo = nullptr;
    RpRapport* rapporthandler = nullptr;
    EinzugRearrange* einzughandler = nullptr;
    CrCursorHandler* cursorhandler = nullptr;

    /*  Currently-open file + its path. `file` is owned and persists
        for the document lifetime; Load() / Save() reopen it as
        needed.                                                    */
    AnsiString filename;
    FfFile* file = nullptr;

    /*  Loom-control brace state. Legacy stores nine numbered
        brace entries (klammern[0..8]) plus a "current" / "last"
        pair. The loader populates these; loom-control rendering
        lands in Phase 11 so we only keep the array.              */
    Klammer klammern[9];

    /*  Loom-control runtime state from the legacy STRGFRM form
        (weave position / klammer / repetition), plus the "divers"
        viewport state (schussselected, scrolly, firstschuss,
        weaving). Not wired to UI yet, but preserved in-memory so
        file round-trip doesn't lose the values.                   */
    int weave_position = 0;
    int weave_klammer = 0;
    int weave_repetition = 0;
    int last_position = 0;
    int last_klammer = 0;
    int last_repetition = 0;
    bool schussselected = true;
    int scrolly_weben = 0;
    bool firstschuss = true;
    bool weaving = false;

    /*  Print-range selection (Datei > Teil drucken...). Preserved
        via file round-trip even though the port's print path
        currently ignores them.                                    */
    SZ printkette { 0, 20 };
    SZ printschuesse { 0, 20 };
    SZ printschaefte { 0, 8 };
    SZ printtritte { 0, 8 };

    /*  Number of visible shafts / treadles in the einzug /
        aufknuepfung / trittfolge viewports. Loaded from files;
        used when scroll/zoom land.                               */
    int hvisible = DEFAULT_MAXY1;
    int wvisible = DEFAULT_MAXX2;

    /*  Fixed-einzug reference for the EzFixiert style (loom-
        control feature). 1:1 port of the legacy raw-array state. */
    short* fixeinzug = nullptr;
    short firstfree = 1;
    int fixsize = 0;

    /*  Blockmuster (substitution) editor state. The 10 Muster slots
        hold the Grundmuster (index 0) plus 9 optional patterns; the
        EditBlockmusterClick dialog opens these for editing, and on
        OK the main window replays each base-pattern cell as a block
        of the matching substitution pattern. einzugZ/trittfolgeZ
        control whether the block-expand lays out Z or S. blockundo
        is the 100-slot undo ring the dialog uses.                  */
    Muster blockmuster[10];
    int currentbm = 0;
    bool einzugZ = true;
    bool trittfolgeZ = true;
    class BlockUndo* blockundo = nullptr;

    /*  Bereichmuster (range-substitution) state. Parallel to the
        blockmuster slots but used by RangePatternsClick: the dialog
        edits these, and BereicheFillPattern then replays the selected
        bereichmuster over the selection (or the whole pattern if no
        selection is active). */
    Muster bereichmuster[10];
    class BlockUndo* bereichundo = nullptr;

    /*  User-defined patterns: 10 slots persisted via QSettings under
        Userdef0..9. LoadUserdefMenu rebuilds the menu captions from
        the slot array. */
    UserdefPattern userdef[MAXUSERDEF];
    QAction* UserdefAct[MAXUSERDEF] = {};
    class QMenu* MenuWeitere = nullptr;

    /*  Top-level menus whose visibility flips with pegplan mode.
        Legacy swapped the Trittfolge caption to "Pegplan/Schlagpatrone"
        and hid the Trittfolge operations menu in favour of the
        pegplan-specific one (SpInvert / ClearSchlagpatrone /
        SpSpiegeln).                                                    */
    class QMenu* treadMenu = nullptr;
    class QMenu* spMenu = nullptr;
    /*  "Copy from treadling" under the Threading menu. Hidden while
        ViewSchlagpatrone is checked (pegplan mode) because Trittfolge
        doesn't exist as a separate field there.                      */
    QAction* actCopyEzTf = nullptr;

    /*  Schlagpatrone (pegplan) mode toggle. Port of legacy
        ToggleSchlagpatrone + ViewSchlagpatroneClick: flips
        trittfolge.einzeltritt, rebuilds the Trittfolge/Aufknuepfung
        pair into a Schlagpatrone or back, pushes an undo snapshot,
        updates the menu layout and persists the preference.         */
    void ToggleSchlagpatrone();
    /*  Apply the visible/caption state of the Trittfolge / Pegplan
        menus and the "Copy from treadling" entry to match the current
        pegplan mode. Called from ToggleSchlagpatrone, ReloadLanguage,
        and once from the ctor after menu construction.              */
    void UpdateSchlagpatroneMode();

    /*  Schedule a repaint of the pattern canvas. QWidget::update() on
        the main window only invalidates the window chrome; children
        paint independently. Every TDBWFRM method that mutates the
        document state and wants the canvas to re-render calls this
        instead of update().                                       */
    void refresh();

    /*  Called by SwitchLanguage() when the user switches language
        live via the Environment options dialog. Walks langEntries
        and re-applies captions / tooltips for every registered
        QAction and QMenu.                                        */
    void ReloadLanguage();

    /*  Bilingual registry. Every translatable QAction (menu entry,
        toolbar button) and QMenu (top-level or submenu) goes into
        `langEntries` at construction; ReloadLanguage() walks that
        list to switch on the fly. Entries with an empty tip leave
        the tooltip untouched so icons-only actions don't pick up
        stray empty tooltips.                                     */
    struct LangEntry {
        QAction* action = nullptr;
        class QMenu* menu = nullptr;
        class QWidget* widget = nullptr; /* setWindowTitle target */
        QString en_text;
        QString de_text;
        QString en_tip;
        QString de_tip;
    };
    std::vector<LangEntry> langEntries;
    void registerLang(QAction* _a, const QString& _en, const QString& _de,
                      const QString& _en_tip = QString(), const QString& _de_tip = QString());
    void registerLangMenu(class QMenu* _m, const QString& _en, const QString& _de);
    void registerLangWidget(class QWidget* _w, const QString& _en, const QString& _de);

    /*  Stubs for the recalc / house-keeping methods every editor
        operation dispatches through. Bodies land in later slices
        alongside the ops that drive them (setgewebe, rapport,
        kette/schuesse range management, ...). Defined now so ported
        undoredo / editor code links without forward-reference
        surgery.                                                    */
    void RecalcGewebe();
    void CalcRangeKette();
    void CalcRangeSchuesse();
    void CalcRange();
    void SetModified(bool _modified = true);
    void SetCursor(int _i, int _j);
    void SetAppTitle();
    void UpdateStatusBar();

    /*  Tracks whether the current document has unsaved edits. Flipped
        on by SetModified() and off by Save() / successful Load(). The
        window title reflects this with a trailing `*`, and the close
        handler prompts the user before discarding changes.         */
    bool modified = false;

    /*  Port of legacy FormCloseQuery. Prompts Yes/No/Cancel when the
        document has unsaved changes; returns true if it's OK to
        close (either nothing to save, saved successfully, or user
        discarded changes). Called from closeEvent and also before
        FileOpen / FileNew replace the current document.             */
    bool AskSave();

protected:
    void closeEvent(class QCloseEvent* _event) override;

public:
    /*  Status-bar labels; created in the ctor and addPermanent-ed
        onto statusBar() so they stay right-aligned. UpdateStatusBar
        populates them on every refresh().                         */
    /*  Range picker: nine checkable toolbar buttons. Driven by
        currentrange so digit-key changes stay in sync. */
    class QActionGroup* rangeGroup = nullptr;
    QAction* rangeActions[9] = {};
    /*  The three special-range entries (Aushebung / Anbindung /
        Abbindung) get their dock icons regenerated on every
        refresh() so the preview tracks the live darst_* settings. */
    QAction* rangeAushebungAction = nullptr;
    QAction* rangeAnbindungAction = nullptr;
    QAction* rangeAbbindungAction = nullptr;
    void updateRangeDockIcons();

    class QLabel* sbField = nullptr;
    class QLabel* sbSelect = nullptr;
    class QLabel* sbRange = nullptr;
    class QLabel* sbRapport = nullptr;
    class QLabel* sbZoom = nullptr;

    /*  Rapport forwarding methods -- delegate to rapporthandler. */
    void CalcRapport();
    void UpdateRapport();
    void ClearRapport();
    void DrawRapport();
    bool IsInRapport(int _i, int _j);

    /*  Drawing stubs (bodies land with the rendering slice). */
    void DrawHilfslinien();
    void DrawGewebe(int _i, int _j);
    void DrawGewebeRahmen(int _i, int _j);
    void DrawGewebeFarbeffekt(int _i, int _j, int _x, int _y, int _xx, int _yy);
    void DrawGewebeSimulation(int _i, int _j, int _x, int _y, int _xx, int _yy);
    void DrawGewebeRapport(int _i, int _j, int _x, int _y, int _xx, int _yy);
    void DrawEinzug(int _i, int _j);
    void DrawAufknuepfung(int _i, int _j);
    void DrawTrittfolge(int _i, int _j);
    /*  --- Insert / Delete / Move menu ops ---------------------
        Structural editing: shift warp / weft / shaft / treadle to
        make room for a new one or remove an existing one. Ported
        verbatim from legacy/insert.cpp, legacy/delete.cpp,
        legacy/move.cpp.                                         */
    bool _IsEmptySchaft(int _j);
    bool _IsEmptyTritt(int _i);
    int GetEmptySchaft();
    int GetEmptyTritt();
    void InsertKettfaden(int _i);
    void InsertSchussfaden(int _j);
    void DeleteKettfaden(int _i);
    void DeleteSchussfaden(int _j);
    void InsertSchaftClick();
    void InsertTrittClick();
    void InsertKetteClick();
    void InsertSchussClick();
    void DeleteSchaftClick();
    void DeleteTrittClick();
    void DeleteKetteClick();
    void DeleteSchussClick();
    void SchaftMoveUpClick();
    void SchaftMoveDownClick();
    void TrittMoveLeftClick();
    void TrittMoveRightClick();

    /*  --- Insert binding (Koeper / Atlas) --------------------
        Stamp a twill (Koeper _h up / _s down) or satin (Atlas
        size _n) binding on the gewebe starting at the cursor,
        then drive a full RecalcAll so the einzug / trittfolge /
        aufknuepfung follow.                                   */
    void KoeperEinfuegen(int _h, int _s);
    void AtlasEinfuegen(int _n);

    /*  Rapport (extend pattern) menu handlers. Rapportieren opens
        a modal dialog to pick horizontal / vertical repeat counts;
        Reduzieren collapses the pattern to a single rapport;
        Override flips the manual-rapport lock using the current
        selection as the rapport rectangle.                       */
    void RappRapportierenClick();
    void RappReduzierenClick();
    void RappOverrideClick();

    /*  Edit menu: ask the cursor handler what direction it's
        auto-advancing in after Set/Toggle, let the user change
        it via the cursor-direction dialog. */
    void EditCursordirectionClick();
    void CursorGotoClick();

    /*  Full einzug / trittfolge / aufknuepfung recompute from the
        current gewebe content. Used by bereiche, importbmp,
        insertbindung, rapportieren, and Edit operations that
        mutate the gewebe wholesale. Wrappers around RcRecalcAll
        with the ViewSchlagpatrone flag threaded through.        */
    void RecalcAll();
    void RecalcSchlagpatrone();
    void RecalcTrittfolgeAufknuepfung();
    void RecalcFixEinzug();

    /*  Returns true when OptionsLockGewebe is checked. Each caller
        that would mutate gewebe should bail out (with a beep) when
        this returns true. SetGewebe() and RecalcAll() already
        short-circuit internally; this helper covers the code paths
        that write gewebe.feld directly (edit, bereiche, rapport,
        insert/delete kette/schuss, import, ...).                    */
    bool GewebeLocked();

    /*  --- Selection --------------------------------------------
        The RANGE tracks a rubber-band rectangle on GEWEBE / EINZUG
        / AUFKNUEPFUNG / TRITTFOLGE / BLATTEINZUG / KETTFARBEN /
        SCHUSSFARBEN. Coordinates are absolute (include scroll). */
    RANGE selection;

    /*  Mouse-drag tracking. md stores the data-coord cell where the
        left button went down; md_feld is the field the drag began in
        (used so dragging out of the start field is ignored). */
    bool mousedown = false;
    FELD md_feld = INVALID;
    PT md;
    int lastfarbei = -1;       /* drag-paint de-dup, kettfarben */
    int lastfarbej = -1;       /* drag-paint de-dup, schussfarben */
    int lastblatteinzugi = -1; /* drag-paint de-dup, blatteinzug */
    /*  True when the mouse press had to wipe a prior valid
        selection. Used on release so a click that only serves to
        dismiss a previous rubber-band doesn't also toggle the
        clicked cell. Matches legacy bSelectionCleared.         */
    bool bSelectionCleared = false;
    /*  Ctrl held at press time. A Ctrl-click on the main fields
        only moves the cursor without toggling the cell, matching
        legacy FormMouseUp's `if (!ctrl) SetX(...)` gate.         */
    bool md_ctrl = false;

    void ClearSelection();
    void DrawSelection();
    void ResizeSelection(int _i, int _j, FELD _feld, bool _square);

    /*  Apply `_range` (1..9) to every cell inside the current
        selection using the matching SetGewebe / SetAufknuepfung /
        SetTrittfolge op. Used by digit-key shortcuts. No-op if the
        selection is empty or on a non-paintable field.           */
    void ApplyRangeToSelection(int _range);

    /*  Re-colour the cells inside the selection to `_range`. Only
        non-zero cells get rewritten; if every cell is zero the
        whole rectangle is filled with `_range`. Matches legacy
        SwitchRange (bound to Shift+Ctrl+digit).                  */
    void SwitchRange(int _range);

    /*  --- Pattern utilities -----------------------------------
        FillKoeper: given a 1-cell-wide warp (or 1-cell-tall weft)
        selection, replicate it diagonally to the right (or up) as
        far as space allows. Produces a twill fill from the seed
        column / row. Selection must be on GEWEBE.
        SwapSide: invert the tie-up (or pegplan) across every
        in-use shaft/treadle and mirror the warp side-info
        (einzug, kettfarben, blatteinzug) around the kette centre. */
    void FillKoeper();
    void SwapSide();
    void KettLancierungClick();
    void SchussLancierungClick();

    /*  Steigung ("slope") shifts each column of the selection by
        an increasing offset, producing a diagonal twist. Valid
        on GEWEBE / AUFKNUEPFUNG / TRITTFOLGE (last only when
        trittfolge.einzeltritt is off).                          */
    void SteigungInc();
    void SteigungDec();
    void IncrementSteigung(int _i, int _j, int _ii, int _jj, FELD _feld);
    void DecrementSteigung(int _i, int _j, int _ii, int _jj, FELD _feld);

    /*  Range-wide schlagpatrone invert -- flips every cell in
        trittfolge and gewebe across the used shaft range and
        weft range. */
    void SpInvert();

    /*  First / last used treadle / shaft indices. */
    int GetFirstTritt();
    int GetLastTritt();
    int GetFirstSchaft();
    int GetLastSchaft();

    /*  --- Aufknuepfung (tie-up) menu operations. -------------------
        Act on the tie-up rectangle bounded by first/last in-use
        shaft x first/last in-use treadle. All use RecalcGewebe to
        repaint the fabric preview, never RecalcAll, so they don't
        regenerate einzug / trittfolge from gewebe and are safe under
        GewebeLocked. No-op in schlagpatrone view.                  */
    void AufInvertClick();
    void ClearAufknuepfungClick();
    void AufZentralsymmClick();
    void AufRollUpClick();
    void AufRollDownClick();
    void AufRollLeftClick();
    void AufRollRightClick();
    void AufSteigungIncClick();
    void AufSteigungDecClick();

    /*  --- Edit-menu operations on the selection ----------------
        All bodies live in src/ui/edit.cpp. Each op reads
        `selection` (normalises internally), mutates the underlying
        field, recomputes derived state (gewebe / ranges / rapport)
        and snapshots an undo entry. No-op if the selection is
        empty or on a non-editable field.                        */
    bool CopySelection(bool _movecursor = true); /* true on success */
    void CutSelection();
    void PasteSelection(bool _transparent = false);
    void DeleteSelection();
    void InvertSelection();
    void MirrorHorzSelection();
    void MirrorVertSelection();
    void RotateSelection(); /* 90deg, square selections only */

    /*  Shift every row / column of the selection by one cell with
        wrap-around at the selection edge. Legacy Roll Up / Down /
        Left / Right. */
    void RollUpSelection();
    void RollDownSelection();
    void RollLeftSelection();
    void RollRightSelection();

    /*  Rolling search for a point-symmetric arrangement of the
        selection. Shifts the block left 0..sizex and up 0..sizey
        cells (wrapping at the rectangle edges) and tests every
        combination until one is centrally symmetric; on a match
        the rolled content replaces the original. No-op if the
        selection is already symmetric; a message box warns if no
        symmetric roll exists. Port of EditCentralsymClick;
        ZentralSymmChecker lives in the domain module.          */
    void CentralsymSelection();

    /*  --- Trittfolge (weft treadling) utilities ------------------
        Legacy helpers that read/write the trittfolge.feld and
        aufknuepfung.feld state. Click handlers (ClearTrittfolge-
        Click, Tf*Click) are deferred until menus are wired.    */
    bool IsEmptyTritt(int _i);
    int GetFirstNonemptyTritt(int _i);
    void RedrawTritt(int _i);
    void RedrawAufknuepfungTritt(int _i);
    void MoveTritt(int _von, int _nach);
    bool AufknuepfungsspalteEqual(int _i1, int _i2);
    void MergeTritte();
    void EliminateEmptyTritt();
    void SwitchTritte(int _a, int _b);
    void RearrangeTritte();

    /*  --- Aufknuepfung helpers ----------------------------------- */
    void MinimizeAufknuepfung();

    /*  --- Rapportieren utilities --------------------------------
        Pattern replication along the warp / weft direction. */
    void RapportSchuss(int _ry, bool _withcolors);
    void RapportKette(int _rx, bool _withcolors);
    void CopyKettfaden(int _von, int _nach, bool _withcolors);
    void CopySchussfaden(int _von, int _nach, bool _withcolors);
    void ClearKettfaden(int _i);
    void ClearSchussfaden(int _j);

    /*  --- Range / emptiness predicates (range.cpp) ------------- */
    bool IsEmptySchuss(int _j);
    bool IsEmptyKette(int _i);
    bool IsEmptySchussNurGewebe(int _j);
    bool IsEmptyKetteNurGewebe(int _i);
    void UpdateRange(int _i, int _j, bool _set);

    /*  --- Data-level helpers (formerly spread across dbw3_form.cpp
        / range.cpp / setgewebe.cpp / redraw.cpp) ---------------  */
    void ToggleGewebe(int _i, int _j);
    void ToggleAufknuepfung(int _i, int _j);
    bool IsEmptyEinzug(int _i);
    bool IsEmptyTrittfolge(int _j);
    bool IsFreeSchaft(int _j);
    bool IsFreeTritt(int _i);
    bool KettfadenEqual(int _a, int _b);
    short GetFreeEinzug();
    short GetFreeTritt();
    void CopyTritt(int _von, int _nach);
    void ExtendSchaefte();
    void ExtendTritte();
    void _ExtendSchaefte(int _max);
    void _ExtendTritte(int _max);
    void RearrangeSchaefte();
    void UpdateScrollbars();

    /*  --- Field "Rahmen" (frame + strongline) painters ---------- */
    void DrawEinzugRahmen(int _i, int _j);
    void DrawAufknuepfungRahmen(int _i, int _j);
    void DrawTrittfolgeRahmen(int _i, int _j);

    /*  --- State-apply operations (set*.cpp) ------------------
        The public Set* methods wrap the DoSet* primitive with
        an undo-snapshot + SetModified + update() repaint. */
    void SetGewebe(int _i, int _j, bool _set, int _range);
    void DoSetGewebe(int _i, int _j, bool _set, int _range);
    void SetEinzug(int _i, int _j);
    void DoSetEinzug(int _i, int _j);
    void SetAufknuepfung(int _i, int _j, bool _set, int _range);
    void DoSetAufknuepfung(int _i, int _j, bool _set, int _range);
    void SetTrittfolge(int _i, int _j, bool _set, int _range);
    void DoSetTrittfolge(int _i, int _j, bool _set, int _range);
    void SetBlatteinzug(int _i);
    void DoSetBlatteinzug(int _i);
    void SetKettfarben(int _i);
    void DoSetKettfarben(int _i);
    void SetSchussfarben(int _j);
    void DoSetSchussfarben(int _j);

    /*  --- File I/O -------------------------------------------- */
    /*  Resize every Feld to match the current Data->MAX* dimensions.
        Called by the loader after updating MAX*. */
    void AllocBuffers(bool _clear);
    void AllocBuffersX1();
    void AllocBuffersX2();
    void AllocBuffersY1();
    void AllocBuffersY2();

    /*  Open `filename` as a .dbw file and populate the document
        state. Returns true on success; _stat receives a more
        detailed outcome in all cases. */
    bool Load(LOADSTAT& _stat, LOADPARTS _loadparts = LOADALL);

    /*  Write the current document state to `filename` in the
        @dbw3:file text format. Covers only the sections the
        port currently loads (signature, version, data.size,
        data.fields, data.palette, data.hilfslinien). Returns
        true on success. */
    bool Save();

    /*  --- File menu handlers ------------------------------------ */
    void FileOpen();
    void FileSave();
    void FileSaveAs();
    void LoadPartsClick();
    void FileNewClick();
    void FileNewTemplateClick();
    void FileRevertClick();

    /*  Extras > Grundeinstellung presets. Flip the four darstellungen,
        right-to-left / top-to-bottom orientation, sinking-shed flag,
        einzug-unten flag and reed-threading visibility to one of
        three region defaults. Ported from legacy
        OptSwiss/Skandinavisch/American click handlers.              */
    void OptSwissClick();
    void OptSkandinavischClick();
    void OptAmericanClick();

    /*  True when the current document holds any painted content or
        has a defined kette / schuesse range. Used to gate
        SetModified() inside Grundeinstellung changes so that flipping
        settings on an empty freshly-launched session doesn't leave
        the document in a "needs saving" state.                     */
    bool HasNonTrivialContent();

    /*  Read the saved Grundeinstellung from Settings and apply it to
        the current session. Called from File > New and at startup so
        a fresh document uses the user's saved region-defaults. File
        loads override these with whatever the file stored.          */
    void ApplyBaseStyleFromSettings();
    /*  Reset the document state to defaults. Shared between
        FileNewClick and FileNewTemplateClick -- after template
        loading the caller clears the filename so Save prompts for a
        fresh target.                                              */
    void ResetDocument();

    /*  Options / Environment dialogs (app settings). */
    void OptEnvironmentClick();
    void XOptionsClick();
    void XOptionsGlobalClick();
    void ShowOptions(bool _global);
    void SetAusmasse(int _x1, int _y1, int _x2, int _y2, int _vx2, int _vy1);

    /*  Document metadata dialogs. */
    void FilePropsClick();
    void ViewInfosClick();

    /*  Editing-assistant dialogs (Phase 7 batch 4). */
    void EinzugAssistentClick();
    void EditFixeinzug();
    void UpdateEinzugFixiert();
    void FarbverlaufClick();
    void EditBlockmusterClick();
    void DefineColorsClick();

    /*  User-defined patterns (legacy userdef.cpp). */
    void LoadUserdefMenu();
    void PasteUserdef(bool _transparent);
    void InsertUserdef(int _i, bool _transparent);
    void UserdefAddClick();
    void UserdefAddSelClick();
    void UserdefRemoveClick();
    int SelectUserdef(const QString& _title = QString());

    /*  Import / export (Phase 7 batch 8). */
    void ImportWIFClick();
    void ImportBitmapClick();
    bool DateiImportieren(const QString& _filename);
    void DateiExportPngClick();
    void DateiExportJpegClick();
    void DateiExportSvgClick();
    void DateiExportPdfClick();
    void DateiExportWifClick();
    void DateiExportieren(const QString& _filename);

    void patternPixelSize(int& _w, int& _h, int _gw, int _gh, int& _shafts, int& _treadles);
    void paintPattern(QPainter& _p, int _gw, int _gh, int _shafts, int _treadles);
    void DoExportPng(const QString& _filename);
    void DoExportJpeg(const QString& _filename);
    void DoExportSvg(const QString& _filename);
    void DoExportPdf(const QString& _filename);

    /*  Schlagpatrone (pegplan) / trittfolge clickhandlers. */
    void ClearSchlagpatroneClick();
    void SpSpiegelnClick();
    void SpInvertClick();
    void ClearTrittfolgeClick();
    void TfSpiegelnClick();
    void EzSpiegelnClick();

    /*  Einzug ↔ Trittfolge transfer. */
    void CopyEinzugTrittfolgeClick();
    void CopyTrittfolgeEinzugClick();

    /*  Bereiche (range-substitution) dialog + fill. */
    void RangePatternsClick();
    void BereicheFillPattern(int _x, int _y);

    /*  Palette-index selection helpers (setcolors.cpp). */
    /*  Print subsystem — page layout / header-footer state. The
        Borders / Header / Footer nested structs mirror the legacy
        layout verbatim so ported print.cpp continues to reference
        borders.range.{left,right,top,bottom}, header.{height,text},
        footer.{height,text}. */
    struct Borders {
        struct Range {
            int left = 20;
            int right = 20;
            int top = 25;
            int bottom = 25;
        } range;
    };
    struct Header {
        int height = 6;
        QString text;
    };
    struct Footer {
        int height = 6;
        QString text;
    };
    Borders borders;
    Header header;
    Footer footer;
    void InitBorders();

    /*  File menu print-stack handlers. */
    void FilePrintClick();
    void FilePrintpreviewClick();
    void FileSetPageClick();

    /*  Loom control (Phase 11). Opens LoomDialog with the current
        trittfolge / aufknuepfung / klammer state and lets the user
        run a dummy loom simulation row-by-row. */
    void LoomControlClick();

    /*  Drawing tools (tools.cpp). */
    TOOL tool = TOOL_POINT;
    /*  Tool-drag preview: when the user is dragging with a non-
        POINT tool, these record the anchor cell and the current
        hover cell, both in data coordinates. paintEvent overlays a
        rubber-band-style shape between them. */
    bool tool_active = false;
    int tool_i0 = 0, tool_j0 = 0; /* anchor */
    int tool_i1 = 0, tool_j1 = 0; /* current */

    void DrawTool(int _i, int _j, int _i1, int _j1);
    void DrawToolLine(int _i, int _j, int _i1, int _j1);
    void DrawToolRectangle(int _i, int _j, int _i1, int _j1, bool _filled);
    void DrawToolEllipse(int _i, int _j, int _i1, int _j1, bool _filled);

    int SelectColorIndex(int _index);
    bool SelectColor(COLORREF& _col);
    void SetKettfarbeClick();
    void SetSchussfarbeClick();
    void ReplaceColorClick();
    void SwitchColorsClick();
    void KettfarbenWieSchussfarbenClick();
    void SchussfarbenWieKettfarbenClick();
    void BlockExpandEinzug(int _count);
    void BlockExpandTrittfolge(int _count);
    void BlockExpandAufknuepfung(int _x, int _y);

    /*  --- Most-recently-used files ------------------------------
        Up to 6 paths stored via QSettings under
        "Brunold Software / DB-WEAVE / mru/{0..5}". The actions and
        the "Recent" submenu itself are built once in the ctor;
        UpdateMRUMenu rewrites their captions/visibility from the
        current `mru` list. */
    QStringList mru;
    class QMenu* mruMenu = nullptr;
    class QAction* mruActions[6] = {};
    void AddToMRU(const QString& _filename);
    void LoadMRU();
    void SaveMRU();
    void UpdateMRUMenu();
    void OpenFromMRU(int _index);

    /*  --- Input routing ----------------------------------------
        Physical-to-logical hit-test: map (X, Y) pixel inside the
        canvas to a FELD and viewport-local cell coordinates (_i,
        _j). Sets _feld to INVALID if the point misses every grid.
        The viewport coords do NOT include the scroll offsets, so
        callers add scroll_x1/x2/y1/y2 as appropriate for the
        resulting feld. */
    void Physical2Logical(int _x, int _y, FELD& _feld, int& _i, int& _j);

    /*  Mouse-button handlers invoked from PatternCanvas mouse events.
        Press starts a selection / paints the initial cell; Move
        grows the rubber-band or drag-paints a 1-D strip; Release
        clears the drag state. */
    void handleCanvasMousePress(int _x, int _y, bool _shift, bool _ctrl = false);
    void handleCanvasMouseMove(int _x, int _y, bool _shift);
    void handleCanvasMouseRelease();

    /*  Key-down handler invoked from PatternCanvas::keyPressEvent.
        Drives cursor motion (arrow keys), range selection (digits),
        field toggle (Space), and field traversal (Tab / Enter). */
    void handleCanvasKeyPress(int _key, int _modifiers);

    /*  Build and exec a context menu at the given global screen
        position. When a selection is active the menu offers Edit
        ops (Cut/Copy/Paste/.../Rotate/Roll/Centralsym); otherwise
        a View-toggle menu (Threading, Treadling, Pegplan, Rapport,
        Guides) + a paste-at-cursor entry.                        */
    void handleContextMenu(const QPoint& _globalPos);

    /*  --- Viewport: zoom ---------------------------------------
        Legacy zoom[10] = { 5,7,9,11,13,15,17,19,21,23 } pixels per
        cell; currentzoom indexes into it. These three handlers step
        currentzoom up/down/to-default, reflow the layout, and
        trigger a repaint. */
    void zoomIn();
    void zoomOut();
    void zoomNormal();
};

/*  Matches legacy `extern PACKAGE TDBWFRM *DBWFRM;`. Populated by
    main() just like the VCL form-auto-creation pattern did. */
extern TDBWFRM* DBWFRM;

#endif
