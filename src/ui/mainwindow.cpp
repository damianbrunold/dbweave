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
#include "undoredo.h"
#include "rapport.h"
#include "einzug.h"
#include "cursor.h"
#include "datamodule.h"
#include "fileformat.h"
#include "assert_compat.h"

#include <QMenuBar>
#include <QMenu>
#include <QKeySequence>

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
	EzMinimalZ  = mk();
	EzMinimalS  = mk();
	EzGeradeZ   = mk();
	EzGeradeS   = mk();
	EzChorig2   = mk();
	EzChorig3   = mk();
	EzBelassen  = mk(/*checked=*/true);
	EzFixiert   = mk();

	TfMinimalZ    = mk();
	TfMinimalS    = mk();
	TfGeradeZ     = mk();
	TfGeradeS     = mk();
	TfGesprungen  = mk();
	TfBelassen    = mk(/*checked=*/true);

	ViewSchlagpatrone = mk();
	ViewEinzug        = mk(/*checked=*/true);
	ViewTrittfolge    = mk(/*checked=*/true);
	ViewOnlyGewebe    = mk();
	GewebeNone        = mk();
	GewebeNormal      = mk(/*checked=*/true);
	RappViewRapport   = mk();
	GewebeFarbeffekt  = mk();
	GewebeSimulation  = mk();
	Inverserepeat     = mk();
	ViewHlines        = mk(/*checked=*/true);

	/*  Allocate the per-shaft / per-treadle availability arrays. Both
	    are initialised to true (all free); RecalcFreieSchaefte() /
	    RecalcFreieTritte() repopulate them during pattern load and
	    after editing operations. */
	freieschaefte = new bool[Data->MAXY1];
	freietritte   = new bool[Data->MAXX2];
	for (int j = 0; j < Data->MAXY1; j++) freieschaefte[j] = true;
	for (int i = 0; i < Data->MAXX2; i++) freietritte[i]   = true;

	/*  Undo stack is constructed after the fields so UrUndoItem's
	    Allocate() can read field dimensions via `this`.            */
	undo           = new UrUndo(this);
	rapporthandler = RpRapport::CreateInstance(this, Data);
	einzughandler  = EinzugRearrange::CreateInstance(this, Data);
	cursorhandler  = CrCursorHandler::CreateInstance(this, Data);
	file           = new FfFile();

	/*  The pattern canvas is the central widget. Ownership is via Qt
	    parent-child; Qt will delete it with the window. */
	pattern_canvas = new PatternCanvas(this, this);
	setCentralWidget(pattern_canvas);

	/*  Minimal File menu -- Open / Save / Save As / Quit. The rest
	    of the menu bar (Edit / View / Pattern / Loom / Help) lands
	    alongside the corresponding behaviour in later slices.    */
	QMenu* fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
	QAction* actOpen   = fileMenu->addAction(QStringLiteral("&Open..."));
	QAction* actSave   = fileMenu->addAction(QStringLiteral("&Save"));
	QAction* actSaveAs = fileMenu->addAction(QStringLiteral("Save &As..."));
	fileMenu->addSeparator();
	QAction* actQuit   = fileMenu->addAction(QStringLiteral("&Quit"));
	actOpen  ->setShortcut(QKeySequence::Open);
	actSave  ->setShortcut(QKeySequence::Save);
	actSaveAs->setShortcut(QKeySequence::SaveAs);
	actQuit  ->setShortcut(QKeySequence::Quit);
	connect(actOpen,   &QAction::triggered, this, [this] { FileOpen();   });
	connect(actSave,   &QAction::triggered, this, [this] { FileSave();   });
	connect(actSaveAs, &QAction::triggered, this, [this] { FileSaveAs(); });
	connect(actQuit,   &QAction::triggered, this, &TDBWFRM::close);

	QMenu* viewMenu = menuBar()->addMenu(QStringLiteral("&View"));
	QAction* actZoomIn     = viewMenu->addAction(QStringLiteral("Zoom &In"));
	QAction* actZoomOut    = viewMenu->addAction(QStringLiteral("Zoom &Out"));
	QAction* actZoomNormal = viewMenu->addAction(QStringLiteral("Zoom &Normal"));
	actZoomIn    ->setShortcut(QKeySequence::ZoomIn);
	actZoomOut   ->setShortcut(QKeySequence::ZoomOut);
	actZoomNormal->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
	connect(actZoomIn,     &QAction::triggered, this, [this] { zoomIn();     });
	connect(actZoomOut,    &QAction::triggered, this, [this] { zoomOut();    });
	connect(actZoomNormal, &QAction::triggered, this, [this] { zoomNormal(); });

	/*  Note: main.cpp explicitly calls seedDemo() after construction
	    so the freshly-launched app shows cloth. Tests skip it and
	    get a clean TDBWFRM.                                         */
}

TDBWFRM::~TDBWFRM()
{
	CrCursorHandler::Release(cursorhandler);         cursorhandler = nullptr;
	EinzugRearrange::ReleaseInstance(einzughandler); einzughandler = nullptr;
	RpRapport::ReleaseInstance(rapporthandler);      rapporthandler = nullptr;
	delete undo;
	delete file; file = nullptr;
	delete[] freieschaefte; freieschaefte = nullptr;
	delete[] freietritte;   freietritte   = nullptr;
	delete[] fixeinzug;     fixeinzug     = nullptr;
	/*  QAction members are owned by `this` via QObject parenting. */
}

void __fastcall TDBWFRM::zoomIn()
{
	if (currentzoom >= 9) return;
	++currentzoom;
	if (pattern_canvas) pattern_canvas->recomputeLayout();
	update();
	SetModified();
}

void __fastcall TDBWFRM::zoomOut()
{
	if (currentzoom <= 0) return;
	--currentzoom;
	if (pattern_canvas) pattern_canvas->recomputeLayout();
	update();
	SetModified();
}

void __fastcall TDBWFRM::zoomNormal()
{
	/*  Legacy default currentzoom = 3 (11 px/cell). */
	if (currentzoom == 3) return;
	currentzoom = 3;
	if (pattern_canvas) pattern_canvas->recomputeLayout();
	update();
	SetModified();
}

void __fastcall TDBWFRM::ReloadLanguage()
{
	/*  Placeholder until lang_main.cpp is ported. */
}

void __fastcall TDBWFRM::RecalcGewebe()
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
			if (n == 0) continue;
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
			if (n == 0) continue;
			for (int k = 0; k < Data->MAXY2; k++) {
				const char s = trittfolge.feld.Get(n - 1, k);
				if (s > 0) gewebe.feld.Set(i, k, s);
			}
		}
	}

	setCursor(old);
	update();
}
void __fastcall TDBWFRM::SetModified(bool)    {}
void __fastcall TDBWFRM::SetCursor(int, int)  {}
void __fastcall TDBWFRM::SetAppTitle()        {}
void __fastcall TDBWFRM::UpdateStatusBar()    {}
void __fastcall TDBWFRM::UpdateScrollbars()   {}
void __fastcall TDBWFRM::InvalidateFeld(const GRIDPOS&) { update(); }

void __fastcall TDBWFRM::RearrangeSchaefte()
{
	dbw3_assert(einzughandler);
	if (einzughandler) einzughandler->Rearrange();
}

void __fastcall TDBWFRM::CalcRapport()    { if (rapporthandler) rapporthandler->CalcRapport(); }
void __fastcall TDBWFRM::UpdateRapport()  { if (rapporthandler) rapporthandler->UpdateRapport(); }
void __fastcall TDBWFRM::ClearRapport()   { if (rapporthandler) rapporthandler->ClearRapport(); }
void __fastcall TDBWFRM::DrawRapport()    { if (rapporthandler) rapporthandler->DrawRapport(); }
bool __fastcall TDBWFRM::IsInRapport(int _i, int _j)
                                          { return rapporthandler ? rapporthandler->IsInRapport(_i, _j) : false; }

/*  DrawGewebe / DrawEinzug / DrawAufknuepfung / DrawTrittfolge,
    the four *Rahmen painters, DrawGewebeFarbeffekt /
    DrawGewebeSimulation, and DrawGewebeKette / Schuss /
    DeleteGewebeKette / Schuss all live in draw.cpp. */
void __fastcall TDBWFRM::_ClearEinzug()                            {}
void __fastcall TDBWFRM::_ClearAufknuepfung()                      {}
void __fastcall TDBWFRM::_ClearSchlagpatrone()                     {}
void __fastcall TDBWFRM::_DrawEinzug()                             {}
void __fastcall TDBWFRM::_DrawAufknuepfung()                       {}
void __fastcall TDBWFRM::_DrawSchlagpatrone()                      {}
void __fastcall TDBWFRM::RecalcFreieSchaefte()                     {}
void __fastcall TDBWFRM::RecalcFreieTritte()                       {}
void __fastcall TDBWFRM::ClearGewebe(int, int)                     {}
void __fastcall TDBWFRM::RedrawGewebe(int, int)                    {}
void __fastcall TDBWFRM::RedrawAufknuepfung(int, int)              {}

void __fastcall TDBWFRM::ClearSelection()                        {}
void __fastcall TDBWFRM::ResizeSelection(int, int, FELD, bool)   {}

void __fastcall TDBWFRM::seedDemo()
{
	/*  2/2 twill on 4 shafts x 4 treadles. Threading: straight draw
	    (shaft = i % 4 + 1). Treadling: straight (treadle = j % 4).
	    Tie-up: diagonal 1,1,0,0 pattern that produces the classic
	    diagonal-stripe cloth.                                    */
	constexpr int N    = 80;  /* warp + weft span */
	constexpr int REP  = 4;

	for (int i = 0; i < N; i++) einzug.feld.Set(i, (short)((i % REP) + 1));

	for (int shaft = 0; shaft < REP; shaft++)
		for (int treadle = 0; treadle < REP; treadle++)
			if (((shaft - treadle + REP) % REP) < 2)
				aufknuepfung.feld.Set(treadle, shaft, (char)1);

	for (int j = 0; j < N; j++) {
		trittfolge.feld.Set(j % REP, j, (char)1);
		trittfolge.isempty.Set(j, false);
	}

	/*  Compute gewebe = which warp/weft crossings come up on top. */
	for (int i = 0; i < N; i++) {
		const int shaft = einzug.feld.Get(i) - 1;
		if (shaft < 0) continue;
		for (int j = 0; j < N; j++) {
			for (int t = 0; t < REP; t++) {
				if (trittfolge.feld.Get(t, j) > 0 &&
				    aufknuepfung.feld.Get(t, shaft) > 0) {
					gewebe.feld.Set(i, j, (char)1);
					break;
				}
			}
		}
	}

	kette    = SZ(0, N - 1);
	schuesse = SZ(0, N - 1);

	/*  Mark the 4 shafts and 4 treadles as "in use" so
	    EliminateEmptyTritt / EliminateEmptySchaft don't prune them. */
	for (int s = 0; s < REP; s++) freieschaefte[s] = false;
	for (int t = 0; t < REP; t++) freietritte[t]   = false;

	/*  Recompute the rapport so the cloth view looks correct. */
	if (rapporthandler) rapporthandler->CalcRapport();
}
