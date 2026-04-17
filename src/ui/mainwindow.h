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

class QPainter;
class PatternCanvas;
class UrUndo;
class RpRapport;
class EinzugRearrange;

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

	/*  Symbol styles for the three special ranges (AUSHEBUNG,
	    ANBINDUNG, ABBINDUNG). Legacy DrawGewebeNormal branches on
	    each of these when painting the corresponding range cells. */
	DARSTELLUNG darst_aushebung           = AUSGEFUELLT;
	DARSTELLUNG darst_anbindung           = AUSGEFUELLT;
	DARSTELLUNG darst_abbindung           = AUSGEFUELLT;
	DARSTELLUNG schlagpatronendarstellung = AUSGEFUELLT;

	/*  Strongline colour for the heavy grid lines. */
	QColor strongclr = QColor(Qt::black);

	/*  Active QPainter during a paintEvent. nullptr outside paint,
	    so legacy-style "frm->Canvas->..." ports can no-op safely.
	    Set by PatternCanvas::paintEvent for the duration of the
	    render, cleared afterwards.                              */
	QPainter* currentPainter = nullptr;

	/*  The central widget that renders the pattern. Owned via Qt
	    parent-child ownership -- delete when the mainwindow dies. */
	PatternCanvas* pattern_canvas = nullptr;

	/*  Per-shaft / per-treadle "unused" flags. Sized to Data->MAXY1 /
	    Data->MAXX2 and allocated in the ctor. Match the legacy raw
	    bool[] layout so ported code's `freieschaefte[j] = true`
	    assignments compile unchanged. */
	bool* freieschaefte = nullptr;
	bool* freietritte   = nullptr;

	/*  Undo stack + handlers, owned. */
	UrUndo*          undo           = nullptr;
	RpRapport*       rapporthandler = nullptr;
	EinzugRearrange* einzughandler  = nullptr;

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

	/*  Selection-management stubs (body lands with selection.cpp). */
	void __fastcall ClearSelection();
	void __fastcall ResizeSelection (int _i, int _j, FELD _feld, bool _fromMouse);

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
};

/*  Matches legacy `extern PACKAGE TDBWFRM *DBWFRM;`. Populated by
    main() just like the VCL form-auto-creation pattern did. */
extern TDBWFRM* DBWFRM;

#endif
