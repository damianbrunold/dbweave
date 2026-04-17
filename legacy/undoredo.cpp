/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "undoredo.h"
#include "dbw3_form.h"
#include "datamodule.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EditUndoClick(TObject *Sender)
{
    dbw3_assert (undo!=0);
    if (undo->Undo()) {
        Invalidate();
        SetModified();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EditRedoClick(TObject *Sender)
{
    dbw3_assert (undo!=0);
    if (undo->Redo()) {
        Invalidate();
        SetModified();
    }
}
/*-----------------------------------------------------------------*/
__fastcall UrUndoItem::UrUndoItem (TDBWFRM* _mainfrm, UrUndoItem* _prev/*=0*/, UrUndoItem* _next/*=0*/)
{
	prev = _prev;
	next = _next;
	active = false;

    einzug = NULL;
    aufknuepfung = NULL;
    trittfolge = NULL;
    isempty = NULL;
    schussfarben = NULL;
    kettfarben = NULL;
    blatteinzug = NULL;

    Allocate (_mainfrm);
}
/*-----------------------------------------------------------------*/
__fastcall UrUndoItem::~UrUndoItem()
{
    Clean();
    delete einzug;
    delete aufknuepfung;
    delete trittfolge;
    delete isempty;
    delete schussfarben;
    delete kettfarben;
    delete blatteinzug;
	// prev und next werden von UrUndo gelöscht, da zirkuläre Struktur
}
/*-----------------------------------------------------------------*/
void __fastcall UrUndoItem::Allocate (TDBWFRM* _mainfrm)
{
    dbw3_assert(_mainfrm);

    try {
        einzug = new FeldVectorShort(_mainfrm->einzug.feld.Size(), 0);
        aufknuepfung = new FeldGridChar(_mainfrm->aufknuepfung.feld.SizeX(),
                                        _mainfrm->aufknuepfung.feld.SizeY(),
                                        0);
        trittfolge = new FeldGridChar(_mainfrm->trittfolge.feld.SizeX(),
                                      _mainfrm->trittfolge.feld.SizeY(),
                                      0);
        isempty = new FeldVectorBool(_mainfrm->trittfolge.isempty.Size(), true);
        schussfarben = new FeldVectorChar(_mainfrm->schussfarben.feld.Size(), 0);
        kettfarben = new FeldVectorChar(_mainfrm->kettfarben.feld.Size(), 0);
        blatteinzug = new FeldVectorChar(_mainfrm->blatteinzug.feld.Size(), 0);
    } catch (...) {
        dbw3_assert(false);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall UrUndoItem::SetData (TDBWFRM* _mainfrm)
{
    dbw3_assert(einzug);
    dbw3_assert(aufknuepfung);
    dbw3_assert(trittfolge);
    dbw3_assert(isempty);
    dbw3_assert(schussfarben);
    dbw3_assert(kettfarben);
    dbw3_assert(blatteinzug);

    // Daten kopieren
    *einzug = _mainfrm->einzug.feld;
    *aufknuepfung = _mainfrm->aufknuepfung.feld;
    *trittfolge = _mainfrm->trittfolge.feld;
    *isempty = _mainfrm->trittfolge.isempty;
    *schussfarben = _mainfrm->schussfarben.feld;
    *kettfarben = _mainfrm->kettfarben.feld;
    *blatteinzug = _mainfrm->blatteinzug.feld;

    kbd_feld = _mainfrm->kbd_field;
    switch (kbd_feld) {
        case EINZUG: kbd_pos = _mainfrm->einzug.kbd; break;
        case AUFKNUEPFUNG: kbd_pos = _mainfrm->aufknuepfung.kbd; break;
        case TRITTFOLGE: kbd_pos = _mainfrm->trittfolge.kbd; break;
        case GEWEBE: kbd_pos = _mainfrm->gewebe.kbd; break;
        case SCHUSSFARBEN: kbd_pos = _mainfrm->schussfarben.kbd; break;
        case KETTFARBEN: kbd_pos = _mainfrm->kettfarben.kbd; break;
        case BLATTEINZUG: kbd_pos = _mainfrm->blatteinzug.kbd; break;
    }
    // Einzugstil speichern
    if (_mainfrm->EzMinimalZ->Checked) einzugstil = _mainfrm->EzMinimalZ;
    else if (_mainfrm->EzMinimalS->Checked) einzugstil = _mainfrm->EzMinimalS;
    else if (_mainfrm->EzGeradeZ->Checked) einzugstil = _mainfrm->EzGeradeZ;
    else if (_mainfrm->EzGeradeS->Checked) einzugstil = _mainfrm->EzGeradeS;
    else if (_mainfrm->EzChorig2->Checked) einzugstil = _mainfrm->EzChorig2;
    else if (_mainfrm->EzChorig3->Checked) einzugstil = _mainfrm->EzChorig3;
    else if (_mainfrm->EzBelassen->Checked) einzugstil = _mainfrm->EzBelassen;
    else if (_mainfrm->EzFixiert->Checked) einzugstil = _mainfrm->EzFixiert;
    // Trittfolgenstil speichern
    if (_mainfrm->TfMinimalZ->Checked) trittfolgenstil = _mainfrm->TfMinimalZ;
    else if (_mainfrm->TfMinimalS->Checked) trittfolgenstil = _mainfrm->TfMinimalS;
    else if (_mainfrm->TfGeradeZ->Checked) trittfolgenstil = _mainfrm->TfGeradeZ;
    else if (_mainfrm->TfGeradeS->Checked) trittfolgenstil = _mainfrm->TfGeradeS;
    else if (_mainfrm->TfGesprungen->Checked) trittfolgenstil = _mainfrm->TfGesprungen;
    else if (_mainfrm->TfBelassen->Checked) trittfolgenstil = _mainfrm->TfBelassen;
    pegplan = _mainfrm->ViewSchlagpatrone->Checked;

    active = true;
}
/*-----------------------------------------------------------------*/
void __fastcall UrUndoItem::UpdateSize()
{
    dbw3_assert(einzug);
    dbw3_assert(aufknuepfung);
    dbw3_assert(trittfolge);
    dbw3_assert(isempty);
    dbw3_assert(schussfarben);
    dbw3_assert(kettfarben);
    dbw3_assert(blatteinzug);

    einzug->Resize (Data->MAXX1, 0);
    aufknuepfung->Resize (Data->MAXX2, Data->MAXY1, 0);
    trittfolge->Resize (Data->MAXX2, Data->MAXY2, 0);
    isempty->Resize (Data->MAXY2, true);
    schussfarben->Resize (Data->MAXY2, Data->defcolorv);
    kettfarben->Resize (Data->MAXX1, Data->defcolorh);
    blatteinzug->Resize (Data->MAXX1, 0);
}
/*-----------------------------------------------------------------*/
void __fastcall UrUndoItem::Undo (TDBWFRM* _mainfrm)
{
    if (IsEmpty()) return;
    
    dbw3_assert(einzug);
    dbw3_assert(aufknuepfung);
    dbw3_assert(trittfolge);
    dbw3_assert(isempty);
    dbw3_assert(schussfarben);
    dbw3_assert(kettfarben);
    dbw3_assert(blatteinzug);

    TCursor old = _mainfrm->Cursor;
    _mainfrm->Cursor = crHourGlass;

    _mainfrm->einzug.feld = *einzug;
    _mainfrm->aufknuepfung.feld = *aufknuepfung;
    _mainfrm->trittfolge.feld = *trittfolge;
    _mainfrm->trittfolge.isempty = *isempty;
    _mainfrm->schussfarben.feld = *schussfarben;
    _mainfrm->kettfarben.feld = *kettfarben;
    _mainfrm->blatteinzug.feld = *blatteinzug;

    einzugstil->Checked = true;
    trittfolgenstil->Checked = true;
    _mainfrm->ViewSchlagpatrone->Checked = pegplan;

    _mainfrm->RecalcGewebe();

    _mainfrm->CalcRangeKette();
    _mainfrm->CalcRangeSchuesse();
    _mainfrm->CalcRapport();

    _mainfrm->kbd_field = kbd_feld;
    _mainfrm->SetCursor (kbd_pos.i, kbd_pos.j);

    _mainfrm->Cursor = old;
    _mainfrm->Invalidate();
    _mainfrm->SetModified();
}
/*-----------------------------------------------------------------*/
void __fastcall UrUndoItem::Redo (TDBWFRM* _mainfrm)
{
    Undo(_mainfrm);
}
/*-----------------------------------------------------------------*/
void __fastcall UrUndoItem::Clean()
{
    active = false;
}
/*-----------------------------------------------------------------*/
__fastcall UrUndo::UrUndo (TDBWFRM* _mainfrm, int _maxundo/*=100*/)
{
    maxundo = _maxundo;
	mainfrm = _mainfrm;
	Init (_maxundo);
    locked = false;
}
/*-----------------------------------------------------------------*/
__fastcall UrUndo::~UrUndo()
{
	mainfrm = 0;
	Cleanup();
}
/*-----------------------------------------------------------------*/
bool __fastcall UrUndo::Undo()
{
    if (locked) return false;
	dbw3_assert (current!=0);
	dbw3_assert (first!=0);
    locked = true;
	if (current!=first && !current->Prev()->IsEmpty()) {
		current = current->Prev();
		current->Undo (mainfrm);
        locked = false;
        return true;
	}
    locked = false;
    return false;
}
/*-----------------------------------------------------------------*/
bool __fastcall UrUndo::Redo()
{
    if (locked) return false;
	dbw3_assert (current!=0);
	dbw3_assert (first!=0);
    locked = true;
	if (current!=first->Prev() && !current->Next()->IsEmpty()) {
        current = current->Next();
		current->Redo (mainfrm);
        locked = false;
        return true;
	}
    locked = false;
    return false;
}
/*-----------------------------------------------------------------*/
void __fastcall UrUndo::Snapshot()
{
	dbw3_assert (current!=0);
	dbw3_assert (first!=0);
	if (!current->IsEmpty()) {
		if (current==first->Prev()) {
			first = first->Next();
			current = current->Next();
		} else {
			current = current->Next();
		}
		current->Clean();
	}

    if (current && current->IsEmpty()) {
        current->SetData (mainfrm);
        if (current->Next()!=first) {
            current->Next()->Clean();
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall UrUndo::Init (int _maxundo)
{
	try {
		first = current = new UrUndoItem (mainfrm, 0, 0);
		for (int i=0; i<_maxundo; i++) {
			UrUndoItem* pnew = new UrUndoItem (mainfrm, current, 0);
			current->SetNext (pnew);
			current = pnew;
		}
		first->SetPrev (current);
		current->SetNext (first);
		current = first;
	} catch (...) {
        dbw3_assert(false);
	}
}
/*-----------------------------------------------------------------*/
void __fastcall UrUndo::Clear()
{
    dbw3_assert (current!=0);
    dbw3_assert (first!=0);
    current = first;
    do {
        current->Clean();
        current = current->Next();
    } while(current && current!=first);
    if (current!=first) current = first;
}
/*-----------------------------------------------------------------*/
void __fastcall UrUndo::UpdateSize()
{
    dbw3_assert (mainfrm!=0);
    dbw3_assert (first!=0);
    UrUndoItem* p = first;
    do {
        p->UpdateSize();
        p = p->Next();
    } while(p!=first);
}
/*-----------------------------------------------------------------*/
void __fastcall UrUndo::Cleanup()
{
	first->Prev()->SetNext(0); // ausklinken
	while (first) {
		UrUndoItem* n = first->Next();
		delete first;
		first = n;
	}
	first = current = 0;
}
/*-----------------------------------------------------------------*/
bool __fastcall UrUndo::CanUndo()
{
    return (current!=first && !current->Prev()->IsEmpty());
}
/*-----------------------------------------------------------------*/
bool __fastcall UrUndo::CanRedo()
{
    return (current->Next()!=first && !current->Next()->IsEmpty());
}
/*-----------------------------------------------------------------*/

