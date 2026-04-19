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

#include "vcl_compat.h"
#include "dbw3_base.h"      /* FeldBase hierarchy */
#include "hilfslinien.h"    /* Hilfslinien container */
#include "loadoptions.h"    /* LOADSTAT / LOADPARTS */

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
	FeldEinzug       einzug;
	FeldAufknuepfung aufknuepfung;
	FeldTrittfolge   trittfolge;
	FeldGewebe       gewebe;
	FeldKettfarben   kettfarben;
	FeldSchussfarben schussfarben;
	FeldBlatteinzug  blatteinzug;

	/*  Which field currently owns the keyboard cursor. */
	FELD kbd_field = GEWEBE;

	/*  Display-orientation flags. Legacy defaults: both false. */
	bool righttoleft = false;
	bool toptobottom = false;

	/*  Warp / weft ranges and the pattern rapport. */
	SZ      kette;
	SZ      schuesse;
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
	QAction* EzMinimalZ      = nullptr;
	QAction* EzMinimalS      = nullptr;
	QAction* EzGeradeZ       = nullptr;
	QAction* EzGeradeS       = nullptr;
	QAction* EzChorig2       = nullptr;
	QAction* EzChorig3       = nullptr;
	QAction* EzBelassen      = nullptr;
	QAction* EzFixiert       = nullptr;

	QAction* TfMinimalZ      = nullptr;
	QAction* TfMinimalS      = nullptr;
	QAction* TfGeradeZ       = nullptr;
	QAction* TfGeradeS       = nullptr;
	QAction* TfGesprungen    = nullptr;
	QAction* TfBelassen      = nullptr;

	QAction* ViewSchlagpatrone = nullptr;
	QAction* ViewEinzug        = nullptr;
	QAction* ViewTrittfolge    = nullptr;
	QAction* ViewOnlyGewebe    = nullptr;
	QAction* GewebeNone        = nullptr;
	QAction* GewebeNormal      = nullptr;
	QAction* RappViewRapport   = nullptr;
	QAction* GewebeFarbeffekt  = nullptr;
	QAction* GewebeSimulation  = nullptr;
	QAction* Inverserepeat     = nullptr;
	QAction* ViewHlines        = nullptr;

	/*  ViewFarbe toggles both colour strips together (kettfarben
	    + schussfarben); ViewBlatteinzug toggles the reed-threading
	    strip. Matches legacy menu layout where the colour strips
	    share a single view toggle. */
	QAction* ViewFarbe         = nullptr;
	QAction* ViewBlatteinzug   = nullptr;

	/*  Guide lines (Hilfslinien) + the four grab-bar rectangles.
	    hlinehorz1/2 sit above/below the pattern grids, hlinevert1/2
	    to the left/right. Bar positions remain zero-sized until the
	    FormResize port lands -- the hilfslinien renderer only needs
	    them non-degenerate if the user wants the "tick into the bar"
	    segment to show. */
	Hilfslinien hlines;
	HlineBar    hlinehorz1;
	HlineBar    hlinehorz2;
	HlineBar    hlinevert1;
	HlineBar    hlinevert2;

	/*  Symbol styles for the three special ranges (AUSHEBUNG,
	    ANBINDUNG, ABBINDUNG). Legacy DrawGewebeNormal branches on
	    each of these when painting the corresponding range cells. */
	DARSTELLUNG darst_aushebung           = AUSGEFUELLT;
	DARSTELLUNG darst_anbindung           = AUSGEFUELLT;
	DARSTELLUNG darst_abbindung           = AUSGEFUELLT;
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
	bool  sinkingshed  = false;
	bool  fewithraster = false;
	float faktor_kette  = 1.0f;
	float faktor_schuss = 1.0f;
	int   currentzoom   = 4;

	/*  Strongline colour for the heavy grid lines. */
	QColor strongclr = QColor(Qt::black);

	/*  Cursor blink state. The QTimer flips this every half of
	    QApplication::cursorFlashTime(); the canvas's paintEvent
	    reads the flag to decide whether to draw the cursor
	    outline. A value of zero from cursorFlashTime() means "no
	    blink" -- we leave the cursor permanently visible. */
	bool          cursorVisible = true;
	class QTimer* cursorTimer   = nullptr;

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
	QDockWidget*   paletteDock  = nullptr;
	PalettePanel*  palettePanel = nullptr;
	QAction*       ViewFarbpalette = nullptr;

	/*  Per-shaft / per-treadle "unused" flags. Sized to Data->MAXY1 /
	    Data->MAXX2 and allocated in the ctor. Match the legacy raw
	    bool[] layout so ported code's `freieschaefte[j] = true`
	    assignments compile unchanged. */
	bool* freieschaefte = nullptr;
	bool* freietritte   = nullptr;

	/*  Scratch buffers used by RcRecalcAll: xbuf[i] marks warp i
	    as seen (non-empty) during RecalcEinzug; ybuf[j] does the
	    same for wefts during RecalcTrittfolge. Sized to MAXX1 and
	    MAXY2 respectively, owned, reallocated by AllocBuffers.  */
	char* xbuf = nullptr;
	char* ybuf = nullptr;

	/*  Undo stack + handlers, owned. */
	UrUndo*          undo           = nullptr;
	RpRapport*       rapporthandler = nullptr;
	EinzugRearrange* einzughandler  = nullptr;
	CrCursorHandler* cursorhandler  = nullptr;

	/*  Currently-open file + its path. `file` is owned and persists
	    for the document lifetime; Load() / Save() reopen it as
	    needed.                                                    */
	AnsiString filename;
	FfFile*    file = nullptr;

	/*  Loom-control brace state. Legacy stores nine numbered
	    brace entries (klammern[0..8]) plus a "current" / "last"
	    pair. The loader populates these; loom-control rendering
	    lands in Phase 11 so we only keep the array.              */
	Klammer klammern[9];

	/*  Number of visible shafts / treadles in the einzug /
	    aufknuepfung / trittfolge viewports. Loaded from files;
	    used when scroll/zoom land.                               */
	int hvisible = DEFAULT_MAXY1;
	int wvisible = DEFAULT_MAXX2;

	/*  Fixed-einzug reference for the EzFixiert style (loom-
	    control feature). 1:1 port of the legacy raw-array state. */
	short* fixeinzug = nullptr;
	short  firstfree = 1;
	int    fixsize   = 0;

	/*  Schedule a repaint of the pattern canvas. QWidget::update() on
	    the main window only invalidates the window chrome; children
	    paint independently. Every TDBWFRM method that mutates the
	    document state and wants the canvas to re-render calls this
	    instead of update().                                       */
	void refresh();

	/*  Called by SwitchLanguage(). Body is filled in when lang_main.cpp
	    is ported (that unit is a 673-line blob of LANG_C_H assignments
	    against menu items and actions that don't exist yet). */
	void __fastcall ReloadLanguage();

	/*  Stubs for the recalc / house-keeping methods every editor
	    operation dispatches through. Bodies land in later slices
	    alongside the ops that drive them (setgewebe, rapport,
	    kette/schuesse range management, ...). Defined now so ported
	    undoredo / editor code links without forward-reference
	    surgery.                                                    */
	void __fastcall RecalcGewebe();
	void __fastcall CalcRangeKette();
	void __fastcall CalcRangeSchuesse();
	void __fastcall CalcRange();
	void __fastcall SetModified (bool _modified = true);
	void __fastcall SetCursor   (int _i, int _j);
	void __fastcall SetAppTitle ();
	void __fastcall UpdateStatusBar();

	/*  Status-bar labels; created in the ctor and addPermanent-ed
	    onto statusBar() so they stay right-aligned. UpdateStatusBar
	    populates them on every refresh().                         */
	/*  Range picker: nine checkable toolbar buttons. Driven by
	    currentrange so digit-key changes stay in sync. */
	class QActionGroup* rangeGroup = nullptr;
	QAction*            rangeActions[9] = { };

	class QLabel* sbField    = nullptr;
	class QLabel* sbSelect   = nullptr;
	class QLabel* sbRange    = nullptr;
	class QLabel* sbRapport  = nullptr;
	class QLabel* sbZoom     = nullptr;

	/*  Rapport forwarding methods -- delegate to rapporthandler. */
	void __fastcall CalcRapport();
	void __fastcall UpdateRapport();
	void __fastcall ClearRapport();
	void __fastcall DrawRapport();
	bool __fastcall IsInRapport (int _i, int _j);

	/*  Drawing stubs (bodies land with the rendering slice). */
	void __fastcall DrawHilfslinien();
	void __fastcall DrawGewebe (int _i, int _j);
	void __fastcall DrawGewebeRahmen (int _i, int _j);
	void __fastcall DrawGewebeFarbeffekt  (int _i, int _j, int _x, int _y, int _xx, int _yy);
	void __fastcall DrawGewebeSimulation  (int _i, int _j, int _x, int _y, int _xx, int _yy);
	void __fastcall DrawGewebeRapport     (int _i, int _j, int _x, int _y, int _xx, int _yy);
	void __fastcall DrawEinzug (int _i, int _j);
	void __fastcall DrawAufknuepfung (int _i, int _j);
	void __fastcall DrawTrittfolge (int _i, int _j);
	void __fastcall DrawGewebeKette (int _i);
	void __fastcall DeleteGewebeKette (int _i);
	void __fastcall _ClearEinzug();
	void __fastcall _ClearAufknuepfung();
	void __fastcall _ClearSchlagpatrone();
	void __fastcall _DrawEinzug();
	void __fastcall _DrawAufknuepfung();
	void __fastcall _DrawSchlagpatrone();
	void __fastcall RecalcFreieSchaefte();
	void __fastcall RecalcFreieTritte();

	/*  --- Insert / Delete / Move menu ops ---------------------
	    Structural editing: shift warp / weft / shaft / treadle to
	    make room for a new one or remove an existing one. Ported
	    verbatim from legacy/insert.cpp, legacy/delete.cpp,
	    legacy/move.cpp.                                         */
	bool __fastcall _IsEmptySchaft  (int _j);
	bool __fastcall _IsEmptyTritt   (int _i);
	int  __fastcall GetEmptySchaft  ();
	int  __fastcall GetEmptyTritt   ();
	void __fastcall InsertKettfaden   (int _i);
	void __fastcall InsertSchussfaden (int _j);
	void __fastcall DeleteKettfaden   (int _i);
	void __fastcall DeleteSchussfaden (int _j);
	void __fastcall InsertSchaftClick ();
	void __fastcall InsertTrittClick  ();
	void __fastcall InsertKetteClick  ();
	void __fastcall InsertSchussClick ();
	void __fastcall DeleteSchaftClick ();
	void __fastcall DeleteTrittClick  ();
	void __fastcall DeleteKetteClick  ();
	void __fastcall DeleteSchussClick ();
	void __fastcall SchaftMoveUpClick   ();
	void __fastcall SchaftMoveDownClick ();
	void __fastcall TrittMoveLeftClick  ();
	void __fastcall TrittMoveRightClick ();

	/*  --- Insert binding (Koeper / Atlas) --------------------
	    Stamp a twill (Koeper _h up / _s down) or satin (Atlas
	    size _n) binding on the gewebe starting at the cursor,
	    then drive a full RecalcAll so the einzug / trittfolge /
	    aufknuepfung follow.                                   */
	void __fastcall KoeperEinfuegen (int _h, int _s);
	void __fastcall AtlasEinfuegen  (int _n);

	/*  Rapport (extend pattern) menu handlers. Rapportieren opens
	    a modal dialog to pick horizontal / vertical repeat counts;
	    Reduzieren collapses the pattern to a single rapport;
	    Override flips the manual-rapport lock using the current
	    selection as the rapport rectangle.                       */
	void __fastcall RappRapportierenClick ();
	void __fastcall RappReduzierenClick   ();
	void __fastcall RappOverrideClick     ();

	/*  Edit menu: ask the cursor handler what direction it's
	    auto-advancing in after Set/Toggle, let the user change
	    it via the cursor-direction dialog. */
	void __fastcall EditCursordirectionClick ();
	void __fastcall CursorGotoClick          ();

	/*  Full einzug / trittfolge / aufknuepfung recompute from the
	    current gewebe content. Used by bereiche, importbmp,
	    insertbindung, rapportieren, and Edit operations that
	    mutate the gewebe wholesale. Wrappers around RcRecalcAll
	    with the ViewSchlagpatrone flag threaded through.        */
	void __fastcall RecalcAll();
	void __fastcall RecalcSchlagpatrone();
	void __fastcall RecalcTrittfolgeAufknuepfung();
	void __fastcall RecalcFixEinzug();

	/*  --- Selection --------------------------------------------
	    The RANGE tracks a rubber-band rectangle on GEWEBE / EINZUG
	    / AUFKNUEPFUNG / TRITTFOLGE / BLATTEINZUG / KETTFARBEN /
	    SCHUSSFARBEN. Coordinates are absolute (include scroll). */
	RANGE selection;

	/*  Mouse-drag tracking. md stores the data-coord cell where the
	    left button went down; md_feld is the field the drag began in
	    (used so dragging out of the start field is ignored). */
	bool mousedown = false;
	FELD md_feld   = INVALID;
	PT   md;
	int  lastfarbei       = -1;  /* drag-paint de-dup, kettfarben */
	int  lastfarbej       = -1;  /* drag-paint de-dup, schussfarben */
	int  lastblatteinzugi = -1;  /* drag-paint de-dup, blatteinzug */
	/*  True when the mouse press had to wipe a prior valid
	    selection. Used on release so a click that only serves to
	    dismiss a previous rubber-band doesn't also toggle the
	    clicked cell. Matches legacy bSelectionCleared.         */
	bool bSelectionCleared = false;
	/*  Ctrl held at press time. A Ctrl-click on the main fields
	    only moves the cursor without toggling the cell, matching
	    legacy FormMouseUp's `if (!ctrl) SetX(...)` gate.         */
	bool md_ctrl           = false;

	void __fastcall ClearSelection();
	void __fastcall DrawSelection();
	void __fastcall ResizeSelection (int _i, int _j, FELD _feld, bool _square);

	/*  Apply `_range` (1..9) to every cell inside the current
	    selection using the matching SetGewebe / SetAufknuepfung /
	    SetTrittfolge op. Used by digit-key shortcuts. No-op if the
	    selection is empty or on a non-paintable field.           */
	void __fastcall ApplyRangeToSelection (int _range);

	/*  Re-colour the cells inside the selection to `_range`. Only
	    non-zero cells get rewritten; if every cell is zero the
	    whole rectangle is filled with `_range`. Matches legacy
	    SwitchRange (bound to Shift+Ctrl+digit).                  */
	void __fastcall SwitchRange (int _range);

	/*  --- Pattern utilities -----------------------------------
	    FillKoeper: given a 1-cell-wide warp (or 1-cell-tall weft)
	    selection, replicate it diagonally to the right (or up) as
	    far as space allows. Produces a twill fill from the seed
	    column / row. Selection must be on GEWEBE.
	    SwapSide: invert the tie-up (or pegplan) across every
	    in-use shaft/treadle and mirror the warp side-info
	    (einzug, kettfarben, blatteinzug) around the kette centre. */
	void __fastcall FillKoeper();
	void __fastcall SwapSide();
	void __fastcall KettLancierungClick  ();
	void __fastcall SchussLancierungClick();

	/*  Steigung ("slope") shifts each column of the selection by
	    an increasing offset, producing a diagonal twist. Valid
	    on GEWEBE / AUFKNUEPFUNG / TRITTFOLGE (last only when
	    trittfolge.einzeltritt is off).                          */
	void __fastcall SteigungInc();
	void __fastcall SteigungDec();
	void __fastcall IncrementSteigung (int _i, int _j, int _ii, int _jj, FELD _feld);
	void __fastcall DecrementSteigung (int _i, int _j, int _ii, int _jj, FELD _feld);

	/*  Range-wide schlagpatrone invert -- flips every cell in
	    trittfolge and gewebe across the used shaft range and
	    weft range. */
	void __fastcall SpInvert();

	/*  First / last used treadle indices (-1 if none used). */
	int  __fastcall GetFirstTritt();
	int  __fastcall GetLastTritt();

	/*  --- Edit-menu operations on the selection ----------------
	    All bodies live in src/ui/edit.cpp. Each op reads
	    `selection` (normalises internally), mutates the underlying
	    field, recomputes derived state (gewebe / ranges / rapport)
	    and snapshots an undo entry. No-op if the selection is
	    empty or on a non-editable field.                        */
	bool __fastcall CopySelection  (bool _movecursor = true);  /* true on success */
	void __fastcall CutSelection   ();
	void __fastcall PasteSelection (bool _transparent = false);
	void __fastcall DeleteSelection();
	void __fastcall InvertSelection();
	void __fastcall MirrorHorzSelection();
	void __fastcall MirrorVertSelection();
	void __fastcall RotateSelection();   /* 90deg, square selections only */

	/*  Shift every row / column of the selection by one cell with
	    wrap-around at the selection edge. Legacy Roll Up / Down /
	    Left / Right. */
	void __fastcall RollUpSelection();
	void __fastcall RollDownSelection();
	void __fastcall RollLeftSelection();
	void __fastcall RollRightSelection();

	/*  Rolling search for a point-symmetric arrangement of the
	    selection. Shifts the block left 0..sizex and up 0..sizey
	    cells (wrapping at the rectangle edges) and tests every
	    combination until one is centrally symmetric; on a match
	    the rolled content replaces the original. No-op if the
	    selection is already symmetric; a message box warns if no
	    symmetric roll exists. Port of EditCentralsymClick;
	    ZentralSymmChecker lives in the domain module.          */
	void __fastcall CentralsymSelection();

	/*  --- Trittfolge (weft treadling) utilities ------------------
	    Legacy helpers that read/write the trittfolge.feld and
	    aufknuepfung.feld state. Click handlers (ClearTrittfolge-
	    Click, Tf*Click) are deferred until menus are wired.    */
	bool __fastcall IsEmptyTritt (int _i);
	int  __fastcall GetFirstNonemptyTritt (int _i);
	void __fastcall RedrawTritt (int _i);
	void __fastcall RedrawAufknuepfungTritt (int _i);
	void __fastcall MoveTritt (int _von, int _nach);
	bool __fastcall AufknuepfungsspalteEqual (int _i1, int _i2);
	void __fastcall MergeTritte();
	void __fastcall EliminateEmptyTritt();
	void __fastcall SwitchTritte (int _a, int _b);
	void __fastcall RearrangeTritte();

	/*  --- Aufknuepfung helpers ----------------------------------- */
	void __fastcall MinimizeAufknuepfung();

	/*  --- Rapportieren utilities --------------------------------
	    Pattern replication along the warp / weft direction. */
	void __fastcall RapportSchuss (int _ry, bool _withcolors);
	void __fastcall RapportKette  (int _rx, bool _withcolors);
	void __fastcall CopyKettfaden   (int _von, int _nach, bool _withcolors);
	void __fastcall CopySchussfaden (int _von, int _nach, bool _withcolors);
	void __fastcall ClearKettfaden   (int _i);
	void __fastcall ClearSchussfaden (int _j);

	/*  --- Range / emptiness predicates (range.cpp) ------------- */
	bool __fastcall IsEmptySchuss (int _j);
	bool __fastcall IsEmptyKette  (int _i);
	bool __fastcall IsEmptySchussNurGewebe (int _j);
	bool __fastcall IsEmptyKetteNurGewebe  (int _i);
	void __fastcall UpdateRange (int _i, int _j, bool _set);

	/*  --- Data-level helpers (formerly spread across dbw3_form.cpp
	    / range.cpp / setgewebe.cpp / redraw.cpp) ---------------  */
	void  __fastcall ToggleGewebe       (int _i, int _j);
	void  __fastcall ToggleAufknuepfung (int _i, int _j);
	bool  __fastcall IsEmptyEinzug      (int _i);
	bool  __fastcall IsEmptyTrittfolge  (int _j);
	void  __fastcall RecalcTrittfolgeEmpty (int _j);
	bool  __fastcall KettfadenEqual     (int _a, int _b);
	short __fastcall GetFreeEinzug();
	short __fastcall GetFreeTritt();
	void  __fastcall CopyTritt          (int _von, int _nach);
	void  __fastcall ExtendSchaefte();
	void  __fastcall ExtendTritte();
	void  __fastcall _ExtendSchaefte (int _max);
	void  __fastcall _ExtendTritte   (int _max);
	void  __fastcall RearrangeSchaefte();
	void  __fastcall UpdateScrollbars();
	void  __fastcall InvalidateFeld (const GRIDPOS& _pos);

	/*  --- Redraw primitives stubs (bodies land with rendering). */
	void  __fastcall ClearGewebe       (int _i, int _j);
	void  __fastcall RedrawGewebe      (int _i, int _j);
	void  __fastcall RedrawAufknuepfung (int _i, int _j);
	void  __fastcall DrawGewebeSchuss  (int _j);
	void  __fastcall DeleteGewebeSchuss (int _j);

	/*  --- Field "Rahmen" (frame + strongline) painters ---------- */
	void __fastcall DrawEinzugRahmen       (int _i, int _j);
	void __fastcall DrawAufknuepfungRahmen (int _i, int _j);
	void __fastcall DrawTrittfolgeRahmen   (int _i, int _j);

	/*  --- State-apply operations (set*.cpp) ------------------
	    The public Set* methods wrap the DoSet* primitive with
	    an undo-snapshot + SetModified + update() repaint. */
	void __fastcall SetGewebe       (int _i, int _j, bool _set, int _range);
	void __fastcall DoSetGewebe     (int _i, int _j, bool _set, int _range);
	void __fastcall SetEinzug       (int _i, int _j);
	void __fastcall DoSetEinzug     (int _i, int _j);
	void __fastcall SetAufknuepfung   (int _i, int _j, bool _set, int _range);
	void __fastcall DoSetAufknuepfung (int _i, int _j, bool _set, int _range);
	void __fastcall SetTrittfolge   (int _i, int _j, bool _set, int _range);
	void __fastcall DoSetTrittfolge (int _i, int _j, bool _set, int _range);
	void __fastcall SetBlatteinzug   (int _i);
	void __fastcall DoSetBlatteinzug (int _i);
	void __fastcall SetKettfarben    (int _i);
	void __fastcall DoSetKettfarben  (int _i);
	void __fastcall SetSchussfarben  (int _j);
	void __fastcall DoSetSchussfarben (int _j);

	/*  --- File I/O -------------------------------------------- */
	/*  Resize every Feld to match the current Data->MAX* dimensions.
	    Called by the loader after updating MAX*. */
	void __fastcall AllocBuffers  (bool _clear);
	void __fastcall AllocBuffersX1();
	void __fastcall AllocBuffersX2();
	void __fastcall AllocBuffersY1();
	void __fastcall AllocBuffersY2();

	/*  Open `filename` as a .dbw file and populate the document
	    state. Returns true on success; _stat receives a more
	    detailed outcome in all cases. */
	bool __fastcall Load (LOADSTAT& _stat, LOADPARTS _loadparts = LOADALL);

	/*  Write the current document state to `filename` in the
	    @dbw3:file text format. Covers only the sections the
	    port currently loads (signature, version, data.size,
	    data.fields, data.palette, data.hilfslinien). Returns
	    true on success. */
	bool __fastcall Save ();

	/*  --- File menu handlers ------------------------------------ */
	void __fastcall FileOpen ();
	void __fastcall FileSave ();
	void __fastcall FileSaveAs ();

	/*  --- Most-recently-used files ------------------------------
	    Up to 6 paths stored via QSettings under
	    "Brunold Software / DB-WEAVE / mru/{0..5}". The actions and
	    the "Recent" submenu itself are built once in the ctor;
	    UpdateMRUMenu rewrites their captions/visibility from the
	    current `mru` list. */
	QStringList mru;
	class QMenu*   mruMenu = nullptr;
	class QAction* mruActions[6] = { };
	void __fastcall AddToMRU    (const QString& _filename);
	void __fastcall LoadMRU     ();
	void __fastcall SaveMRU     ();
	void __fastcall UpdateMRUMenu ();
	void __fastcall OpenFromMRU (int _index);

	/*  --- Input routing ----------------------------------------
	    Physical-to-logical hit-test: map (X, Y) pixel inside the
	    canvas to a FELD and viewport-local cell coordinates (_i,
	    _j). Sets _feld to INVALID if the point misses every grid.
	    The viewport coords do NOT include the scroll offsets, so
	    callers add scroll_x1/x2/y1/y2 as appropriate for the
	    resulting feld. */
	void __fastcall Physical2Logical (int _x, int _y, FELD& _feld, int& _i, int& _j);

	/*  Mouse-button handlers invoked from PatternCanvas mouse events.
	    Press starts a selection / paints the initial cell; Move
	    grows the rubber-band or drag-paints a 1-D strip; Release
	    clears the drag state. */
	void __fastcall handleCanvasMousePress   (int _x, int _y, bool _shift, bool _ctrl = false);
	void __fastcall handleCanvasMouseMove    (int _x, int _y, bool _shift);
	void __fastcall handleCanvasMouseRelease ();

	/*  Key-down handler invoked from PatternCanvas::keyPressEvent.
	    Drives cursor motion (arrow keys), range selection (digits),
	    field toggle (Space), and field traversal (Tab / Enter). */
	void __fastcall handleCanvasKeyPress (int _key, int _modifiers);

	/*  Build and exec a context menu at the given global screen
	    position. When a selection is active the menu offers Edit
	    ops (Cut/Copy/Paste/.../Rotate/Roll/Centralsym); otherwise
	    a View-toggle menu (Threading, Treadling, Pegplan, Rapport,
	    Guides) + a paste-at-cursor entry.                        */
	void __fastcall handleContextMenu (const QPoint& _globalPos);

	/*  --- Viewport: zoom ---------------------------------------
	    Legacy zoom[10] = { 5,7,9,11,13,15,17,19,21,23 } pixels per
	    cell; currentzoom indexes into it. These three handlers step
	    currentzoom up/down/to-default, reflow the layout, and
	    trigger a repaint. */
	void __fastcall zoomIn();
	void __fastcall zoomOut();
	void __fastcall zoomNormal();
};

/*  Matches legacy `extern PACKAGE TDBWFRM *DBWFRM;`. Populated by
    main() just like the VCL form-auto-creation pattern did. */
extern TDBWFRM* DBWFRM;

#endif
