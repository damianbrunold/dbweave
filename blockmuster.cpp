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
#include "dbw3_form.h"
#include "dbw3_strings.h"
#include "felddef.h"
#include "datamodule.h"
#include "cursor.h"
#include "undoredo.h"
#include "blockmuster_form.h"
/*-----------------------------------------------------------------*/
__fastcall Muster::Muster()
{
    Clear();
}
/*-----------------------------------------------------------------*/
void __fastcall Muster::Set (int _i, int _j, char _value)
{
    dbw3_assert(_i<maxx);
    dbw3_assert(_j<maxy);
    if (_i>=maxx || _j>=maxy) return;
    feld[_i+maxx*_j] = _value;
}
/*-----------------------------------------------------------------*/
char __fastcall Muster::Get (int _i, int _j)
{
    dbw3_assert(_i<maxx);
    dbw3_assert(_j<maxy);
    if (_i>=maxx || _j>=maxy) return 0;
    return feld[_i+maxx*_j];
}
/*-----------------------------------------------------------------*/
void __fastcall Muster::Clear()
{
    memset (feld, 0, maxx*maxy);
}
/*-----------------------------------------------------------------*/
bool __fastcall Muster::IsEmpty()
{
    for (int i=0; i<maxx; i++)
        for (int j=0; j<maxy; j++)
            if (feld[i+maxx*j]!=0)
                return false;
    return true;
}
/*-----------------------------------------------------------------*/
Muster& Muster::operator= (const Muster& _m)
{
    if (this==&_m) return *this;
    SetData (_m.Data());
    return *this;
}
/*-----------------------------------------------------------------*/
void __fastcall Muster::SetData (const char* _data)
{
    memcpy (feld, _data, maxx*maxy);
}
/*-----------------------------------------------------------------*/
int __fastcall Muster::SizeX()
{
    bool nonempty = false;
    int i1 = maxx-1, i2 = 0;

    for (int i=0; i<maxx; i++)
        for (int j=0; j<maxy; j++)
            if (feld[i+maxx*j]!=0) {
                if (i1>i) i1 = i;
                if (i2<i) i2 = i;
                nonempty = true;
            }

    return nonempty ? i2 - i1 + 1 : 0;
}
/*-----------------------------------------------------------------*/
int __fastcall Muster::SizeY()
{
    bool nonempty = false;
    int j1 = maxy-1, j2 = 0;

    for (int j=0; j<maxy; j++)
        for (int i=0; i<maxx; i++)
            if (feld[i+maxx*j]!=0) {
                if (j1>j) j1 = j;
                if (j2<j) j2 = j;
                nonempty = true;
            }

    return nonempty ? j2 - j1 + 1 : 0;
}
/*-----------------------------------------------------------------*/
int __fastcall Muster::FirstX()
{
    for (int i=0; i<maxx; i++)
        for (int j=0; j<maxy; j++)
            if (feld[i+maxx*j]!=0) return i;

    return 0;
}
/*-----------------------------------------------------------------*/
int __fastcall Muster::FirstY()
{
    for (int j=0; j<maxy; j++)
        for (int i=0; i<maxx; i++)
            if (feld[i+maxx*j]!=0) return j;

    return 0;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EditBlockmusterClick(TObject *Sender)
{
    dbw3_assert(cursorhandler);
    cursorhandler->DisableCursor();
    try {
        dbw3_assert(blockundo);
        TBlockmusterForm* pFrm = new TBlockmusterForm(this, *blockundo, &blockmuster, currentbm, cursorhandler, SUBST_BLOCK, true);
        if (pFrm->ShowModal()==mrOk) {
            einzugZ = pFrm->EinzugZ->Checked;
            trittfolgeZ = pFrm->TrittfolgeZ->Checked;

            int x = pFrm->mx+1;
            int y = pFrm->my+1;

            // Schlagpatrone ausschalten
            bool vorherschlagpatrone = false;
            if (ViewSchlagpatrone->Checked) {
                vorherschlagpatrone = true;
                ViewSchlagpatrone->Checked = false;
                trittfolge.einzeltritt = true;
                RecalcAll();
            }

            // Blöcke expandieren
            BlockExpandEinzug (y);
            BlockExpandTrittfolge (x);
            BlockExpandAufknuepfung (x, y);

            EzBelassen->Checked = true;
            TfBelassen->Checked = true;

            CalcRange();
            CalcRapport();
            RecalcGewebe();

            if (vorherschlagpatrone) {
                ViewSchlagpatrone->Checked = true;
                trittfolge.einzeltritt = !ViewSchlagpatrone->Checked;
                RecalcSchlagpatrone();
            }

            Invalidate();
            dbw3_assert (undo!=0);
            undo->Snapshot();
        }
        delete pFrm;
    } catch (...) {
    }
    SetModified();
    cursorhandler->EnableCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::BlockExpandEinzug (int _count)
{
    if (kette.a==-1 || kette.b==-1) return;

    try {
        // Temporäre Kopie von Einzug anlegen und Einzug löschen
        short* pData = new short[kette.count()];
        for (int i=kette.a; i<=kette.b; i++) {
            pData[i-kette.a] = einzug.feld.Get(i);
            einzug.feld.Set(i, 0);
        }

        // Kopie expanden in Einzug
        for (int i=kette.a; i<=kette.b; i++) {
            if (pData[i-kette.a]>0) {
                if (einzugZ) {
                    for (int k=0; k<_count; k++) {
                        if (i*_count+k>=Data->MAXX1) break;
                        if ((pData[i-kette.a]-1)*_count+k+1>=Data->MAXY1) ExtendSchaefte();
                        einzug.feld.Set (i*_count+k, short((pData[i-kette.a]-1)*_count+k+1));
                    }
                } else {
                    for (int k=0; k<_count; k++) {
                        if ((i+1)*_count-1-k>=Data->MAXX1) break;
                        if ((pData[i-kette.a]-1)*_count+k+1>=Data->MAXY1) ExtendSchaefte();
                        einzug.feld.Set ((i+1)*_count-1-k, short((pData[i-kette.a]-1)*_count+k+1));
                    }
                }
            }
        }

        // Kopie löschen
        delete[] pData;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::BlockExpandTrittfolge (int _count)
{
    if (schuesse.a==-1 || schuesse.b==-1) return;

    try {
        // Temporäre Kopie von Trittfolge anlegen und Trittfolge löschen
        char* pData = new char[schuesse.count()*Data->MAXX2];
        for (int j=schuesse.a; j<=schuesse.b; j++) {
            for (int i=0; i<Data->MAXX2; i++) {
                pData[j*Data->MAXX2+i] = trittfolge.feld.Get (i, j);
                trittfolge.feld.Set (i, j, 0);
            }
            trittfolge.isempty.Set (j, true);
        }

        // Kopie expanden in Trittfolge
        for (int j=schuesse.a; j<=schuesse.b; j++) {
            for (int i=0; i<Data->MAXX2; i++) {
                if (pData[j*Data->MAXX2+i]!=0) {
                    if (trittfolgeZ) {
                        for (int k=0; k<_count; k++) {
                            if (j*_count+k>=Data->MAXY2) break;
                            if (i*_count+k>=Data->MAXX2) ExtendTritte();
                            trittfolge.feld.Set (i*_count+k, j*_count+k, 1);
                            trittfolge.isempty.Set (j*_count+k, false);
                        }
                    } else {
                        for (int k=0; k<_count; k++) {
                            if (j*_count+k>=Data->MAXY2) break;
                            if ((i+1)*_count-1-k>=Data->MAXX2) ExtendTritte();
                            trittfolge.feld.Set ((i+1)*_count-1-k, j*_count+k, 1);
                            trittfolge.isempty.Set (j*_count+k, false);
                        }
                    }
                }
            }
        }

        // Kopie löschen
        delete[] pData;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::BlockExpandAufknuepfung (int _x, int _y)
{
    try {
        // Temporäre Kopie von Aufknuepfung anlegen und Aufknuepfung löschen
        char* pData = new char[Data->MAXY1*Data->MAXX2];
        for (int j=0; j<Data->MAXY1; j++) {
            for (int i=0; i<Data->MAXX2; i++) {
                pData[j*Data->MAXX2+i] = aufknuepfung.feld.Get (i, j);
                aufknuepfung.feld.Set (i, j, 0);
            }
        }

        // Benutzten Bereich in Aufknuepfung ermitteln
        int i1 = Data->MAXX2-1, i2 = 0;
        int j1 = Data->MAXY1-1, j2 = 0;
        bool nonempty = false;
        for (int i=0; i<Data->MAXX2; i++)
            for (int j=0; j<Data->MAXY1; j++)
                if (pData[j*Data->MAXX2+i]!=0) {
                    if (i1>i) i1 = i;
                    if (i2<i) i2 = i;
                    if (j1>j) j1 = j;
                    if (j2<j) j2 = j;
                    nonempty = true;
                }
        if (!nonempty) { i1 = j1 = 0; i2 = j2 = -1; }

        // Kopie expanden in Aufknuepfung
        for (int i=i1; i<=i2; i++) {
            for (int j=j1; j<=j2; j++) {
                char bindung = pData[j*Data->MAXX2+i];
                dbw3_assert (bindung>=0 && bindung<10);
                for (int ii=_x*i; ii<_x*(i+1); ii++)
                    for (int jj=_y*j; jj<_y*(j+1); jj++)
                        aufknuepfung.feld.Set (ii, jj, blockmuster[bindung].Get(ii-_x*i, jj-_y*j));
            }
        }

        // Kopie löschen
        delete[] pData;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
__fastcall BlockUndo::BlockUndo (PMUSTERARRAY _bindungen, int& _active)
: bindungen(_bindungen), active(_active)
{
    list = current = NULL;
    Allocate();
}
/*-----------------------------------------------------------------*/
__fastcall BlockUndo::~BlockUndo()
{
    BlockUndoItem* p = list->prev;
    while (p!=list) {
        p = p->prev;
        delete p->next;
    }
    delete list;
    bindungen = NULL;
}
/*-----------------------------------------------------------------*/
void __fastcall BlockUndo::Allocate()
{
    try {
        // Ringliste von 100 Elementen anlegen
        list = current = new BlockUndoItem;
        for (int i=1; i<100; i++) {
            current->next = new BlockUndoItem;
            current->next->prev = current;
            current = current->next;
        }
        list->prev = current;
        current->next = list;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall BlockUndo::Snapshot()
{
	dbw3_assert (current!=0);
	dbw3_assert (list!=0);
	if (!current->isempty) {
		if (current==list->prev) {
			list = list->next;
			current = current->next;
		} else {
			current = current->next;
		}
		current->isempty = true;
	}

    if (current && current->isempty) {
        for (int i=0; i<10; i++)
            current->bindungen[i] = (*bindungen)[i];
        current->active = active;
        current->isempty = false;
        if (current->next!=list) {
            current->next->isempty = true;
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall BlockUndo::Undo()
{
	dbw3_assert (current!=0);
	dbw3_assert (list!=0);
	if (current!=list) {
		current = current->prev;
        if (current->isempty) return;
        for (int i=0; i<10; i++)
            (*bindungen)[i] = current->bindungen[i];
        active = current->active;
	}
}
/*-----------------------------------------------------------------*/
void __fastcall BlockUndo::Redo()
{
	dbw3_assert (current!=0);
	dbw3_assert (list!=0);
	if (current!=list->prev) {
        current = current->next;
        if (current->isempty) return;
        for (int i=0; i<10; i++)
            (*bindungen)[i] = current->bindungen[i];
        active = current->active;
    }
}
/*-----------------------------------------------------------------*/
bool __fastcall BlockUndo::CanUndo()
{
    return (current!=list && !current->prev->isempty);
}
/*-----------------------------------------------------------------*/
bool __fastcall BlockUndo::CanRedo()
{
    return (current->next!=list && !current->next->isempty);
}
/*-----------------------------------------------------------------*/
void __fastcall BlockUndo::Clear()
{
    dbw3_assert (current!=0);
    dbw3_assert (list!=0);
    current = list;
    do {
        current->isempty = true;
        current = current->next;
    } while (current!=list);
}
/*-----------------------------------------------------------------*/

