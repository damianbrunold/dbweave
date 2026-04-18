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

#include <QActionGroup>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>
#include <QToolButton>

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

	/*  Edit menu: clipboard + selection transforms. All operate on
	    the currently-active selection; they're no-ops if nothing is
	    selected. The rubber-band selection is built by click+drag in
	    PatternCanvas.                                            */
	QMenu* editMenu = menuBar()->addMenu(QStringLiteral("&Edit"));
	QAction* actUndo             = editMenu->addAction(QStringLiteral("&Undo"));
	QAction* actRedo             = editMenu->addAction(QStringLiteral("&Redo"));
	editMenu->addSeparator();
	QAction* actCut              = editMenu->addAction(QStringLiteral("Cu&t"));
	QAction* actCopy             = editMenu->addAction(QStringLiteral("&Copy"));
	QAction* actPaste            = editMenu->addAction(QStringLiteral("&Paste"));
	QAction* actPasteTransparent = editMenu->addAction(QStringLiteral("Paste T&ransparent"));
	QAction* actDelete           = editMenu->addAction(QStringLiteral("&Delete"));
	editMenu->addSeparator();
	QAction* actInvert        = editMenu->addAction(QStringLiteral("&Invert"));
	QAction* actMirrorH       = editMenu->addAction(QStringLiteral("Mirror &Horizontal"));
	QAction* actMirrorV       = editMenu->addAction(QStringLiteral("Mirror &Vertical"));
	QAction* actRotate        = editMenu->addAction(QStringLiteral("R&otate 90\xc2\xb0"));
	QAction* actCentralsym    = editMenu->addAction(QStringLiteral("Central &symmetry"));
	editMenu->addSeparator();
	QAction* actRollUp    = editMenu->addAction(QStringLiteral("Roll &Up"));
	QAction* actRollDown  = editMenu->addAction(QStringLiteral("Roll &Down"));
	QAction* actRollLeft  = editMenu->addAction(QStringLiteral("Roll &Left"));
	QAction* actRollRight = editMenu->addAction(QStringLiteral("Roll &Right"));
	actUndo ->setShortcut(QKeySequence::Undo);
	actRedo ->setShortcut(QKeySequence::Redo);
	actCut  ->setShortcut(QKeySequence::Cut);
	actCopy ->setShortcut(QKeySequence::Copy);
	actPaste->setShortcut(QKeySequence::Paste);
	actPasteTransparent->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V));
	actDelete->setShortcut(QKeySequence::Delete);
	connect(actUndo,             &QAction::triggered, this, [this] { if (undo && undo->Undo()) refresh(); });
	connect(actRedo,             &QAction::triggered, this, [this] { if (undo && undo->Redo()) refresh(); });
	connect(actCut,              &QAction::triggered, this, [this] { CutSelection();         });
	connect(actCopy,             &QAction::triggered, this, [this] { CopySelection();        });
	connect(actPaste,            &QAction::triggered, this, [this] { PasteSelection(false);  });
	connect(actPasteTransparent, &QAction::triggered, this, [this] { PasteSelection(true);   });
	connect(actDelete,           &QAction::triggered, this, [this] { DeleteSelection();      });
	connect(actInvert,           &QAction::triggered, this, [this] { InvertSelection();      });
	connect(actMirrorH,          &QAction::triggered, this, [this] { MirrorHorzSelection();  });
	connect(actMirrorV,          &QAction::triggered, this, [this] { MirrorVertSelection();  });
	connect(actRotate,           &QAction::triggered, this, [this] { RotateSelection();      });
	connect(actCentralsym,       &QAction::triggered, this, [this] { CentralsymSelection();  });
	connect(actRollUp,           &QAction::triggered, this, [this] { RollUpSelection();      });
	connect(actRollDown,         &QAction::triggered, this, [this] { RollDownSelection();    });
	connect(actRollLeft,         &QAction::triggered, this, [this] { RollLeftSelection();    });
	connect(actRollRight,        &QAction::triggered, this, [this] { RollRightSelection();   });

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

	/*  View-toggle items reuse the QActions that already live on
	    TDBWFRM (allocated earlier in the ctor). Put them into the
	    menu and wire them to refresh the canvas when toggled. */
	viewMenu->addSeparator();
	viewMenu->addAction(ViewEinzug);        ViewEinzug       ->setText(QStringLiteral("&Threading"));
	viewMenu->addAction(ViewTrittfolge);    ViewTrittfolge   ->setText(QStringLiteral("T&readling"));
	viewMenu->addAction(ViewSchlagpatrone); ViewSchlagpatrone->setText(QStringLiteral("&Pegplan"));
	viewMenu->addSeparator();
	viewMenu->addAction(RappViewRapport);   RappViewRapport  ->setText(QStringLiteral("&Rapport markers"));
	viewMenu->addAction(ViewHlines);        ViewHlines       ->setText(QStringLiteral("&Guide lines"));
	viewMenu->addSeparator();
	QMenu* gewebeMenu = viewMenu->addMenu(QStringLiteral("&Cloth display"));
	auto* gewebeGroup = new QActionGroup(this);
	gewebeGroup->setExclusive(true);
	GewebeNormal    ->setText(QStringLiteral("&Normal"));
	GewebeFarbeffekt->setText(QStringLiteral("Colour &effect"));
	GewebeSimulation->setText(QStringLiteral("&Simulation"));
	GewebeNone      ->setText(QStringLiteral("N&one"));
	for (QAction* a : { GewebeNormal, GewebeFarbeffekt, GewebeSimulation, GewebeNone }) {
		a->setCheckable(true);
		gewebeGroup->addAction(a);
		gewebeMenu->addAction(a);
	}
	/*  Any toggle should repaint the canvas. */
	for (QAction* a : { ViewEinzug, ViewTrittfolge, ViewSchlagpatrone,
	                    RappViewRapport, ViewHlines,
	                    GewebeNormal, GewebeFarbeffekt,
	                    GewebeSimulation, GewebeNone })
		connect(a, &QAction::triggered, this, [this] { refresh(); });

	/*  --- Toolbars -----------------------------------------------
	    Qt standard icons give us passable visuals without shipping
	    custom art. File + Edit + Transform + Zoom on the main
	    toolbar; the 1..9 range picker lives on its own toolbar so
	    it can be hidden independently.                          */
	auto icon = [this](QStyle::StandardPixmap sp) {
		return this->style()->standardIcon(sp);
	};
	actOpen  ->setIcon(icon(QStyle::SP_DialogOpenButton));
	actSave  ->setIcon(icon(QStyle::SP_DialogSaveButton));
	actUndo  ->setIcon(icon(QStyle::SP_ArrowBack));
	actRedo  ->setIcon(icon(QStyle::SP_ArrowForward));
	actCut   ->setIcon(icon(QStyle::SP_DialogDiscardButton));
	actCopy  ->setIcon(icon(QStyle::SP_FileIcon));
	actPaste ->setIcon(icon(QStyle::SP_FileDialogNewFolder));
	actDelete->setIcon(icon(QStyle::SP_TrashIcon));

	QToolBar* mainBar = addToolBar(QStringLiteral("Main"));
	mainBar->setObjectName(QStringLiteral("mainToolBar"));
	mainBar->addAction(actOpen);
	mainBar->addAction(actSave);
	mainBar->addSeparator();
	mainBar->addAction(actUndo);
	mainBar->addAction(actRedo);
	mainBar->addSeparator();
	mainBar->addAction(actCut);
	mainBar->addAction(actCopy);
	mainBar->addAction(actPaste);
	mainBar->addAction(actDelete);
	mainBar->addSeparator();
	mainBar->addAction(actInvert);
	mainBar->addAction(actMirrorH);
	mainBar->addAction(actMirrorV);
	mainBar->addAction(actRotate);
	mainBar->addSeparator();
	mainBar->addAction(actZoomIn);
	mainBar->addAction(actZoomOut);
	mainBar->addAction(actZoomNormal);

	/*  Range picker: nine checkable buttons driving currentrange.
	    Clicking also fills the current selection if one is active
	    (same behaviour as the digit keys). The swatch colour comes
	    from GetRangeColor; see rangecolors.cpp. */
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
			if (selection.Valid()) ApplyRangeToSelection(r);
			refresh();
		});
	}
	/*  Keep the toolbar in sync when currentrange is changed from
	    other paths (digit keys, mouse). */
	if (currentrange >= 1 && currentrange <= 9)
		rangeActions[currentrange - 1]->setChecked(true);

	/*  Status-bar panels (right-aligned permanent widgets). */
	sbField   = new QLabel(this);
	sbSelect  = new QLabel(this);
	sbRange   = new QLabel(this);
	sbRapport = new QLabel(this);
	sbZoom    = new QLabel(this);
	statusBar()->addWidget(sbField, 1);
	statusBar()->addWidget(sbSelect, 1);
	statusBar()->addPermanentWidget(sbRange);
	statusBar()->addPermanentWidget(sbRapport);
	statusBar()->addPermanentWidget(sbZoom);
	UpdateStatusBar();

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

void TDBWFRM::refresh()
{
	/*  Repaint the pattern canvas. QWidget::update() on the main
	    window only invalidates the window chrome; children paint
	    independently in Qt. */
	if (pattern_canvas) pattern_canvas->update();
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

void __fastcall TDBWFRM::zoomIn()
{
	if (currentzoom >= 9) return;
	++currentzoom;
	if (pattern_canvas) pattern_canvas->recomputeLayout();
	refresh();
	SetModified();
}

void __fastcall TDBWFRM::zoomOut()
{
	if (currentzoom <= 0) return;
	--currentzoom;
	if (pattern_canvas) pattern_canvas->recomputeLayout();
	refresh();
	SetModified();
}

void __fastcall TDBWFRM::zoomNormal()
{
	/*  Legacy default currentzoom = 3 (11 px/cell). */
	if (currentzoom == 3) return;
	currentzoom = 3;
	if (pattern_canvas) pattern_canvas->recomputeLayout();
	refresh();
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
	refresh();
}
void __fastcall TDBWFRM::SetModified(bool)    {}
void __fastcall TDBWFRM::SetCursor(int, int)  {}
void __fastcall TDBWFRM::SetAppTitle()        {}
void __fastcall TDBWFRM::UpdateScrollbars()   {}
void __fastcall TDBWFRM::InvalidateFeld(const GRIDPOS&) { refresh(); }

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
