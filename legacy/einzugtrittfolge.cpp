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
#include "felddef.h"
#include "datamodule.h"
#include "undoredo.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::CopyEinzugTrittfolgeClick(TObject *Sender)
{
    // Einzug löschen
    for (int i=0; i<Data->MAXX1; i++) {
        einzug.feld.Set (i, 0);
    }

    // Trittfolge in Einzug kopieren
    int maxi = min(Data->MAXY2, Data->MAXX1);
    for (int i=0; i<maxi; i++) {
        int schaft = 0;
        for (int ii=0; ii<Data->MAXX2; ii++)
            if (trittfolge.feld.Get(ii,i)>0) {
                schaft = ii+1;
                break;
            }
        einzug.feld.Set (i, (unsigned short)schaft);
    }

    // Gewebe neu berechnen
    CalcRange();
    CalcRapport();
    RecalcGewebe();

    // Invalidideren (Gewebe wird in RecalcGewebe invalidiert)
    InvalidateFeld (einzug.pos);

    dbw3_assert (undo!=0);
    undo->Snapshot();
    SetModified();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::CopyTrittfolgeEinzugClick(TObject *Sender)
{
    // Trittfolge löschen
    for (int j=0; j<Data->MAXY2; j++) {
        for (int i=0; i<Data->MAXX2; i++) {
            trittfolge.feld.Set (i, j, 0);
        }
        trittfolge.isempty.Set (j, true);
    }

    // Einzug in Trittfolge kopieren
    int maxj = min(Data->MAXY2, Data->MAXX1);
    for (int j=0; j<maxj; j++) {
        int schaft = einzug.feld.Get(j);
        if (schaft!=0) {
            trittfolge.feld.Set(schaft-1, j, 1);
            trittfolge.isempty.Set (j, false);
        }
    }

    // Gewebe neu berechnen
    CalcRange();
    CalcRapport();
    RecalcGewebe();

    // Invalidideren (Gewebe wird in RecalcGewebe invalidiert)
    InvalidateFeld (trittfolge.pos);

    dbw3_assert (undo!=0);
    undo->Snapshot();
    SetModified();
}
/*-----------------------------------------------------------------*/
