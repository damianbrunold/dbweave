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
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
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
	/*  Default einzug / trittfolge rearrangement style is MinimalZ
	    (left-leaning minimum-shaft arrangement). Matches legacy:
	    under EzBelassen / TfBelassen the Rearrange passes are
	    no-ops, which leaves stale shafts / treadles behind after
	    toggle-clears. The "Minimal" variants run MergeSchaefte /
	    MergeTritte on every SetGewebe and keep the loom state
	    clean.                                                    */
	EzMinimalZ  = mk(/*checked=*/true);
	EzMinimalS  = mk();
	EzGeradeZ   = mk();
	EzGeradeS   = mk();
	EzChorig2   = mk();
	EzChorig3   = mk();
	EzBelassen  = mk();
	EzFixiert   = mk();

	TfMinimalZ    = mk(/*checked=*/true);
	TfMinimalS    = mk();
	TfGeradeZ     = mk();
	TfGeradeS     = mk();
	TfGesprungen  = mk();
	TfBelassen    = mk();

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
	ViewFarbe         = mk(/*checked=*/true);
	ViewBlatteinzug   = mk(/*checked=*/true);

	/*  Allocate the per-shaft / per-treadle availability arrays. Both
	    are initialised to true (all free); RecalcFreieSchaefte() /
	    RecalcFreieTritte() repopulate them during pattern load and
	    after editing operations. */
	freieschaefte = new bool[Data->MAXY1];
	freietritte   = new bool[Data->MAXX2];
	for (int j = 0; j < Data->MAXY1; j++) freieschaefte[j] = true;
	for (int i = 0; i < Data->MAXX2; i++) freietritte[i]   = true;

	/*  Scratch buffers for RcRecalcAll. */
	xbuf = new char[Data->MAXX1];
	ybuf = new char[Data->MAXY2];

	/*  Blockmuster undo ring. Allocated once per mainwindow; the
	    Muster array it references lives as member state so no
	    ownership transfer is needed. */
	blockundo   = new BlockUndo(&blockmuster,   currentbm);
	bereichundo = new BlockUndo(&bereichmuster, currentbm);
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
	undo           = new UrUndo(this);
	rapporthandler = RpRapport::CreateInstance(this, Data);
	einzughandler  = EinzugRearrange::CreateInstance(this, Data);
	cursorhandler  = CrCursorHandler::CreateInstance(this, Data);
	file           = new FfFile();

	/*  The pattern canvas is the central widget. Ownership is via Qt
	    parent-child; Qt will delete it with the window. */
	pattern_canvas = new PatternCanvas(this, this);
	setCentralWidget(pattern_canvas);

	/*  Palette dock -- floats on the right by default. The
	    enclosing QDockWidget owns the panel; ViewFarbpalette is a
	    checkable QAction bound to toggleViewAction so the menu
	    state stays in sync with the dock's visibility. */
	paletteDock  = new QDockWidget(QStringLiteral("Palette"), this);
	paletteDock->setObjectName(QStringLiteral("paletteDock"));
	palettePanel = new PalettePanel(this, paletteDock);
	paletteDock->setWidget(palettePanel);
	addDockWidget(Qt::RightDockWidgetArea, paletteDock);
	ViewFarbpalette = paletteDock->toggleViewAction();
	ViewFarbpalette->setText(QStringLiteral("&Palette"));

	/*  Minimal File menu -- Open / Save / Save As / Quit. The rest
	    of the menu bar (Edit / View / Pattern / Loom / Help) lands
	    alongside the corresponding behaviour in later slices.    */
	QMenu* fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
	QAction* actOpen   = fileMenu->addAction(QStringLiteral("&Open..."));
	QAction* actSave   = fileMenu->addAction(QStringLiteral("&Save"));
	QAction* actSaveAs = fileMenu->addAction(QStringLiteral("Save &As..."));
	QAction* actLoadParts = fileMenu->addAction(QStringLiteral("&Load parts..."));
	fileMenu->addSeparator();
	QAction* actFileProps = fileMenu->addAction(QStringLiteral("Propert&ies..."));
	connect(actFileProps, &QAction::triggered, this, [this]{ FilePropsClick(); });
	fileMenu->addSeparator();
	QAction* actImportWIF = fileMenu->addAction(QStringLiteral("Import &WIF..."));
	QAction* actImportBmp = fileMenu->addAction(QStringLiteral("Import &picture..."));
	QAction* actExport    = fileMenu->addAction(QStringLiteral("&Export..."));
	connect(actImportWIF, &QAction::triggered, this, [this]{ ImportWIFClick();    });
	connect(actImportBmp, &QAction::triggered, this, [this]{ ImportBitmapClick(); });
	connect(actExport,    &QAction::triggered, this, [this]{ DateiExportClick();  });
	fileMenu->addSeparator();
	/*  Recent-files submenu. Six QActions are built up front so
	    their captions can be rewritten from UpdateMRUMenu without
	    rebuilding the menu structure.                            */
	mruMenu = fileMenu->addMenu(QStringLiteral("&Recent"));
	for (int i = 0; i < 6; i++) {
		QAction* a = new QAction(this);
		a->setVisible(false);
		mruMenu->addAction(a);
		mruActions[i] = a;
		connect(a, &QAction::triggered, this, [this, i] { OpenFromMRU(i); });
	}
	fileMenu->addSeparator();
	QAction* actQuit   = fileMenu->addAction(QStringLiteral("&Quit"));
	actOpen  ->setShortcut(QKeySequence::Open);
	actSave  ->setShortcut(QKeySequence::Save);
	actSaveAs->setShortcut(QKeySequence::SaveAs);
	actQuit  ->setShortcut(QKeySequence::Quit);
	connect(actOpen,   &QAction::triggered, this, [this] { FileOpen();   });
	connect(actSave,   &QAction::triggered, this, [this] { FileSave();   });
	connect(actSaveAs, &QAction::triggered, this, [this] { FileSaveAs(); });
	connect(actLoadParts, &QAction::triggered, this, [this] { LoadPartsClick(); });
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
	editMenu->addSeparator();
	editMenu->addSeparator();
	QAction* actCursorDir  = editMenu->addAction(QStringLiteral("Cursor d&irection..."));
	QAction* actCursorGoto = editMenu->addAction(QStringLiteral("&Go to field..."));
	connect(actCursorDir,  &QAction::triggered, this, [this] { EditCursordirectionClick(); });
	connect(actCursorGoto, &QAction::triggered, this, [this] { CursorGotoClick();          });
	editMenu->addSeparator();
	QAction* actFillKoeper = editMenu->addAction(QStringLiteral("Fill &Twill"));
	QAction* actSwapSide   = editMenu->addAction(QStringLiteral("S&wap Side"));
	QAction* actSteigInc   = editMenu->addAction(QStringLiteral("Steigung &+"));
	QAction* actSteigDec   = editMenu->addAction(QStringLiteral("Steigung &-"));
	QAction* actSpInvert   = editMenu->addAction(QStringLiteral("Pegplan In&vert"));
	editMenu->addSeparator();
	QAction* actKettLanc   = editMenu->addAction(QStringLiteral("Warp &Lancierung..."));
	QAction* actSchussLanc = editMenu->addAction(QStringLiteral("Weft Lancie&rung..."));
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
	connect(actFillKoeper,       &QAction::triggered, this, [this] { FillKoeper();           });
	connect(actSwapSide,         &QAction::triggered, this, [this] { SwapSide();             });
	connect(actSteigInc,         &QAction::triggered, this, [this] { SteigungInc();          });
	connect(actSteigDec,         &QAction::triggered, this, [this] { SteigungDec();          });
	connect(actSpInvert,         &QAction::triggered, this, [this] { SpInvert();             });
	connect(actKettLanc,         &QAction::triggered, this, [this] { KettLancierungClick();   });
	connect(actSchussLanc,       &QAction::triggered, this, [this] { SchussLancierungClick(); });

	/*  Pattern menu: structural insert / delete of threads, shafts
	    and treadles. Each action is gated on kbd_field at invocation
	    time (inside the handler) so Qt's enabled-state doesn't need
	    live updates.                                              */
	QMenu* patternMenu = menuBar()->addMenu(QStringLiteral("&Pattern"));
	QAction* actInsKette   = patternMenu->addAction(QStringLiteral("Insert &Warp thread"));
	QAction* actInsSchuss  = patternMenu->addAction(QStringLiteral("Insert W&eft thread"));
	QAction* actInsSchaft  = patternMenu->addAction(QStringLiteral("Insert &Shaft"));
	QAction* actInsTritt   = patternMenu->addAction(QStringLiteral("Insert &Treadle"));
	patternMenu->addSeparator();
	QAction* actDelKette   = patternMenu->addAction(QStringLiteral("Delete W&arp thread"));
	QAction* actDelSchuss  = patternMenu->addAction(QStringLiteral("Delete Wef&t thread"));
	QAction* actDelSchaft  = patternMenu->addAction(QStringLiteral("Delete Sh&aft"));
	QAction* actDelTritt   = patternMenu->addAction(QStringLiteral("Delete Tre&adle"));
	patternMenu->addSeparator();
	QAction* actSchaftUp   = patternMenu->addAction(QStringLiteral("Shaft &Up"));
	QAction* actSchaftDown = patternMenu->addAction(QStringLiteral("Shaft &Down"));
	QAction* actTrittLeft  = patternMenu->addAction(QStringLiteral("Treadle &Left"));
	QAction* actTrittRight = patternMenu->addAction(QStringLiteral("Treadle &Right"));
	connect(actInsKette,   &QAction::triggered, this, [this] { InsertKetteClick();  });
	connect(actInsSchuss,  &QAction::triggered, this, [this] { InsertSchussClick(); });
	connect(actInsSchaft,  &QAction::triggered, this, [this] { InsertSchaftClick(); });
	connect(actInsTritt,   &QAction::triggered, this, [this] { InsertTrittClick();  });
	connect(actDelKette,   &QAction::triggered, this, [this] { DeleteKetteClick();  });
	connect(actDelSchuss,  &QAction::triggered, this, [this] { DeleteSchussClick(); });
	connect(actDelSchaft,  &QAction::triggered, this, [this] { DeleteSchaftClick(); });
	connect(actDelTritt,   &QAction::triggered, this, [this] { DeleteTrittClick();  });
	connect(actSchaftUp,   &QAction::triggered, this, [this] { SchaftMoveUpClick();   });
	connect(actSchaftDown, &QAction::triggered, this, [this] { SchaftMoveDownClick(); });
	connect(actTrittLeft,  &QAction::triggered, this, [this] { TrittMoveLeftClick();  });
	connect(actTrittRight, &QAction::triggered, this, [this] { TrittMoveRightClick(); });

	/*  Insert binding: stamp a Koeper (twill) or Atlas (satin)
	    weave structure into the gewebe at the cursor. 13 variants
	    matching legacy insertbindung.cpp.                       */
	patternMenu->addSeparator();
	QMenu* bindMenu  = patternMenu->addMenu(QStringLiteral("Insert &binding"));
	QMenu* koepMenu  = bindMenu   ->addMenu(QStringLiteral("&Koeper (twill)"));
	QMenu* atlasMenu = bindMenu   ->addMenu(QStringLiteral("&Atlas (satin)"));
	auto addKoeper = [this, koepMenu](const QString& label, int h, int s) {
		QAction* a = koepMenu->addAction(label);
		connect(a, &QAction::triggered, this, [this, h, s] { KoeperEinfuegen(h, s); });
	};
	addKoeper(QStringLiteral("2 / 2"), 2, 2);
	addKoeper(QStringLiteral("3 / 3"), 3, 3);
	addKoeper(QStringLiteral("4 / 4"), 4, 4);
	addKoeper(QStringLiteral("5 / 5"), 5, 5);
	koepMenu->addSeparator();
	addKoeper(QStringLiteral("2 / 1"), 2, 1);
	addKoeper(QStringLiteral("3 / 1"), 3, 1);
	addKoeper(QStringLiteral("4 / 1"), 4, 1);
	addKoeper(QStringLiteral("5 / 1"), 5, 1);
	koepMenu->addSeparator();
	addKoeper(QStringLiteral("3 / 2"), 3, 2);
	addKoeper(QStringLiteral("4 / 2"), 4, 2);
	addKoeper(QStringLiteral("5 / 2"), 5, 2);
	koepMenu->addSeparator();
	addKoeper(QStringLiteral("4 / 3"), 4, 3);
	addKoeper(QStringLiteral("5 / 3"), 5, 3);
	auto addAtlas = [this, atlasMenu](int n) {
		QAction* a = atlasMenu->addAction(QStringLiteral("Atlas &%1").arg(n));
		connect(a, &QAction::triggered, this, [this, n] { AtlasEinfuegen(n); });
	};
	for (int n = 5; n <= 10; n++) addAtlas(n);

	/*  Editing assistants. */
	patternMenu->addSeparator();
	QAction* actEzAssist    = patternMenu->addAction(QStringLiteral("Threading &wizard..."));
	QAction* actFixEinzug   = patternMenu->addAction(QStringLiteral("&User defined threading..."));
	QAction* actFarbverlauf = patternMenu->addAction(QStringLiteral("Color &blending..."));
	QAction* actDefColors   = patternMenu->addAction(QStringLiteral("&Define colors..."));
	QAction* actBlockmuster = patternMenu->addAction(QStringLiteral("&Substitute with block patterns..."));
	QAction* actRangeSubst  = patternMenu->addAction(QStringLiteral("Substitute &ranges with patterns..."));
	connect(actRangeSubst, &QAction::triggered, this, [this]{ RangePatternsClick(); });
	patternMenu->addSeparator();
	QAction* actCopyEzTf = patternMenu->addAction(QStringLiteral("Copy threading → treadling"));
	QAction* actCopyTfEz = patternMenu->addAction(QStringLiteral("Copy treadling → threading"));
	connect(actCopyEzTf, &QAction::triggered, this, [this]{ CopyEinzugTrittfolgeClick(); });
	connect(actCopyTfEz, &QAction::triggered, this, [this]{ CopyTrittfolgeEinzugClick(); });
	patternMenu->addSeparator();
	QAction* actClearTf   = patternMenu->addAction(QStringLiteral("Clear &treadling / pegplan"));
	QAction* actMirrorTf  = patternMenu->addAction(QStringLiteral("&Mirror treadling vertically"));
	QAction* actInvertSp  = patternMenu->addAction(QStringLiteral("In&vert pegplan"));
	connect(actClearTf,   &QAction::triggered, this, [this]{ ClearTrittfolgeClick(); });
	connect(actMirrorTf,  &QAction::triggered, this, [this]{ TfSpiegelnClick();      });
	connect(actInvertSp,  &QAction::triggered, this, [this]{ SpInvertClick();        });
	connect(actDefColors, &QAction::triggered, this, [this]{ DefineColorsClick(); });
	connect(actEzAssist,    &QAction::triggered, this, [this]{ EinzugAssistentClick(); });
	connect(actFixEinzug,   &QAction::triggered, this, [this]{ EditFixeinzug();        });
	connect(actFarbverlauf, &QAction::triggered, this, [this]{ FarbverlaufClick();     });
	connect(actBlockmuster, &QAction::triggered, this, [this]{ EditBlockmusterClick(); });

	/*  User-defined patterns: 10 slots plus add / add-selection /
	    remove actions. Captions are filled in by LoadUserdefMenu. */
	patternMenu->addSeparator();
	QMenu* udMenu = patternMenu->addMenu(QStringLiteral("&User-defined"));
	QAction* actUdAdd    = udMenu->addAction(QStringLiteral("&Save pattern..."));
	QAction* actUdAddSel = udMenu->addAction(QStringLiteral("Save s&election..."));
	QAction* actUdDel    = udMenu->addAction(QStringLiteral("&Delete..."));
	udMenu->addSeparator();
	MenuWeitere = udMenu->addMenu(QStringLiteral("&Other patterns"));
	for (int i = 0; i < MAXUSERDEF; i++) {
		UserdefAct[i] = MenuWeitere->addAction(QString());
		UserdefAct[i]->setVisible(false);
		connect(UserdefAct[i], &QAction::triggered, this, [this, i](){
			/*  Ctrl pressed at click → transparent paste. */
			const bool transp = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
			InsertUserdef(i, transp);
		});
	}
	MenuWeitere->menuAction()->setVisible(false);
	connect(actUdAdd,    &QAction::triggered, this, [this]{ UserdefAddClick();    });
	connect(actUdAddSel, &QAction::triggered, this, [this]{ UserdefAddSelClick(); });
	connect(actUdDel,    &QAction::triggered, this, [this]{ UserdefRemoveClick(); });

	/*  Rapport (extend pattern) entries. */
	patternMenu->addSeparator();
	QAction* actRappExtend   = patternMenu->addAction(QStringLiteral("&Extend pattern..."));
	QAction* actRappReduce   = patternMenu->addAction(QStringLiteral("Red&uce to one repeat"));
	QAction* actRappOverride = patternMenu->addAction(QStringLiteral("&Override rapport from selection"));
	connect(actRappExtend,   &QAction::triggered, this, [this] { RappRapportierenClick(); });
	connect(actRappReduce,   &QAction::triggered, this, [this] { RappReduzierenClick();   });
	connect(actRappOverride, &QAction::triggered, this, [this] { RappOverrideClick();     });

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
	QAction* actInfos = viewMenu->addAction(QStringLiteral("Pattern i&nfo..."));
	connect(actInfos, &QAction::triggered, this, [this]{ ViewInfosClick(); });
	viewMenu->addSeparator();
	if (ViewFarbpalette) viewMenu->addAction(ViewFarbpalette);
	viewMenu->addSeparator();
	viewMenu->addAction(ViewEinzug);        ViewEinzug       ->setText(QStringLiteral("&Threading"));
	viewMenu->addAction(ViewTrittfolge);    ViewTrittfolge   ->setText(QStringLiteral("T&readling"));
	viewMenu->addAction(ViewSchlagpatrone); ViewSchlagpatrone->setText(QStringLiteral("&Pegplan"));
	viewMenu->addAction(ViewBlatteinzug);   ViewBlatteinzug  ->setText(QStringLiteral("Reed t&hreading"));
	viewMenu->addAction(ViewFarbe);         ViewFarbe        ->setText(QStringLiteral("&Colour strips"));
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
	/*  Visibility toggles need a re-layout (strip widths/heights
	    collapse to zero) before the repaint. Render-mode toggles
	    only need a repaint. */
	auto relayout = [this] {
		if (pattern_canvas) pattern_canvas->recomputeLayout();
		refresh();
	};
	for (QAction* a : { ViewEinzug, ViewTrittfolge, ViewSchlagpatrone,
	                    ViewBlatteinzug, ViewFarbe,
	                    RappViewRapport, ViewHlines })
		connect(a, &QAction::triggered, this, relayout);
	for (QAction* a : { GewebeNormal, GewebeFarbeffekt,
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
	/*  Prefer system-theme icons (freedesktop names) and fall back
	    to Qt standard pixmaps when the theme doesn't carry the
	    action. Some transforms have no sensible standard icon --
	    for those the button just shows the action text.         */
	auto themed = [&](const char* name, QStyle::StandardPixmap fallback) {
		QIcon i = QIcon::fromTheme(QLatin1String(name));
		return i.isNull() ? icon(fallback) : i;
	};
	actOpen   ->setIcon(themed("document-open",  QStyle::SP_DialogOpenButton));
	actSave   ->setIcon(themed("document-save",  QStyle::SP_DialogSaveButton));
	actUndo   ->setIcon(themed("edit-undo",      QStyle::SP_ArrowBack));
	actRedo   ->setIcon(themed("edit-redo",      QStyle::SP_ArrowForward));
	actCut    ->setIcon(themed("edit-cut",       QStyle::SP_DialogDiscardButton));
	actCopy   ->setIcon(themed("edit-copy",      QStyle::SP_FileIcon));
	actPaste  ->setIcon(themed("edit-paste",     QStyle::SP_FileDialogNewFolder));
	actDelete ->setIcon(themed("edit-delete",    QStyle::SP_TrashIcon));
	actInvert ->setIcon(themed("edit-select-invert", QStyle::SP_BrowserReload));
	actMirrorH->setIcon(themed("object-flip-horizontal", QStyle::SP_ArrowLeft));
	actMirrorV->setIcon(themed("object-flip-vertical",   QStyle::SP_ArrowUp));
	actRotate ->setIcon(themed("object-rotate-right",    QStyle::SP_BrowserReload));
	actZoomIn    ->setIcon(themed("zoom-in",       QStyle::SP_FileDialogContentsView));
	actZoomOut   ->setIcon(themed("zoom-out",      QStyle::SP_FileDialogDetailedView));
	actZoomNormal->setIcon(themed("zoom-original", QStyle::SP_FileDialogListView));

	QToolBar* mainBar = addToolBar(QStringLiteral("Main"));
	mainBar->setObjectName(QStringLiteral("mainToolBar"));
	/*  Show text beside the icon so actions without a theme icon
	    still read clearly, and ones with an icon are compact. */
	mainBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
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

	/*  Three special ranges: AUSHEBUNG / ANBINDUNG / ABBINDUNG.
	    Legacy bound these to menu entries outside the Range1..9
	    group; keep them on the same toolbar but visually separated. */
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

	/*  Options menu. */
	QMenu* optMenu = menuBar()->addMenu(QStringLiteral("&Options"));
	QAction* actEnvOpt = optMenu->addAction(QStringLiteral("&Environment..."));
	QAction* actXOpt   = optMenu->addAction(QStringLiteral("&Options..."));
	QAction* actXOptG  = optMenu->addAction(QStringLiteral("Options (&global)..."));
	connect(actEnvOpt, &QAction::triggered, this, [this]{ OptEnvironmentClick(); });
	connect(actXOpt,   &QAction::triggered, this, [this]{ XOptionsClick();       });
	connect(actXOptG,  &QAction::triggered, this, [this]{ XOptionsGlobalClick(); });

	/*  Help menu. */
	QMenu* helpMenu = menuBar()->addMenu(QStringLiteral("&Help"));
	QAction* actTechInfo = helpMenu->addAction(QStringLiteral("&Technical Info..."));
	QAction* actAbout    = helpMenu->addAction(QStringLiteral("&About DB-WEAVE..."));
	connect(actTechInfo, &QAction::triggered, this, [this] {
		TechinfoDialog(this).exec();
	});
	connect(actAbout,    &QAction::triggered, this, [this] {
		AboutDialog(this).exec();
	});

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
			if (pattern_canvas) pattern_canvas->update();
		});
		cursorTimer->start();
	}
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
	delete[] xbuf;          xbuf          = nullptr;
	delete[] ybuf;          ybuf          = nullptr;
	delete[] fixeinzug;     fixeinzug     = nullptr;
	delete   blockundo;     blockundo     = nullptr;
	delete   bereichundo;   bereichundo   = nullptr;
	/*  QAction members are owned by `this` via QObject parenting. */
}

void TDBWFRM::refresh()
{
	/*  Reset the cursor blink so it's visible immediately after any
	    mutation (cursor move, paint op, zoom, ...) rather than
	    appearing off mid-flash. */
	cursorVisible = true;
	if (cursorTimer) cursorTimer->start();

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
void __fastcall TDBWFRM::ClearGewebe(int, int)                     {}
void __fastcall TDBWFRM::RedrawGewebe(int, int)                    {}
void __fastcall TDBWFRM::RedrawAufknuepfung(int, int)              {}
