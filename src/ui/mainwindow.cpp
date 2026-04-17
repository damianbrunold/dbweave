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
#include "undoredo.h"

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

	/*  Undo stack is constructed after the fields so UrUndoItem's
	    Allocate() can read field dimensions via `this`.            */
	undo = new UrUndo(this);
}

TDBWFRM::~TDBWFRM()
{
	delete undo;
	/*  QAction members are owned by `this` via QObject parenting. */
}

void __fastcall TDBWFRM::ReloadLanguage()
{
	/*  Placeholder until lang_main.cpp is ported. */
}

void __fastcall TDBWFRM::RecalcGewebe()       {}
void __fastcall TDBWFRM::CalcRangeKette()     {}
void __fastcall TDBWFRM::CalcRangeSchuesse()  {}
void __fastcall TDBWFRM::CalcRapport()        {}
void __fastcall TDBWFRM::SetModified(bool)    {}
void __fastcall TDBWFRM::SetCursor(int, int)  {}
void __fastcall TDBWFRM::SetAppTitle()        {}
