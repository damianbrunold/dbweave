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
#include "datamodule.h"
#include "assert_compat.h"

TDBWFRM* DBWFRM = nullptr;

TDBWFRM::TDBWFRM(QWidget* parent)
	: QMainWindow(parent)
{
	setWindowTitle(QStringLiteral("DB-WEAVE"));
	resize(1024, 768);

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

	/*  The pattern canvas is the central widget. Ownership is via Qt
	    parent-child; Qt will delete it with the window. */
	pattern_canvas = new PatternCanvas(this, this);
	setCentralWidget(pattern_canvas);
}

TDBWFRM::~TDBWFRM()
{
	EinzugRearrange::ReleaseInstance(einzughandler); einzughandler = nullptr;
	RpRapport::ReleaseInstance(rapporthandler);      rapporthandler = nullptr;
	delete undo;
	delete[] freieschaefte; freieschaefte = nullptr;
	delete[] freietritte;   freietritte   = nullptr;
	/*  QAction members are owned by `this` via QObject parenting. */
}

void __fastcall TDBWFRM::ReloadLanguage()
{
	/*  Placeholder until lang_main.cpp is ported. */
}

void __fastcall TDBWFRM::RecalcGewebe()       {}
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

void __fastcall TDBWFRM::DrawHilfslinien()                         {}
/*  DrawGewebe body lives in draw.cpp. */
void __fastcall TDBWFRM::DrawGewebeRahmen(int, int)                {}
void __fastcall TDBWFRM::DrawEinzug(int, int)                      {}
void __fastcall TDBWFRM::DrawAufknuepfung(int, int)                {}
void __fastcall TDBWFRM::DrawTrittfolge(int, int)                  {}
void __fastcall TDBWFRM::DrawGewebeKette(int)                      {}
void __fastcall TDBWFRM::DeleteGewebeKette(int)                    {}
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
void __fastcall TDBWFRM::DrawGewebeSchuss(int)                     {}
void __fastcall TDBWFRM::DeleteGewebeSchuss(int)                   {}

void __fastcall TDBWFRM::ClearSelection()                        {}
void __fastcall TDBWFRM::ResizeSelection(int, int, FELD, bool)   {}
