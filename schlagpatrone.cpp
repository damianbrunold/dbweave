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
void __fastcall TDBWFRM::ClearSchlagpatroneClick(TObject *Sender)
{
    ClearTrittfolgeClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SpSpiegelnClick(TObject *Sender)
{
    TfSpiegelnClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SpInvertClick(TObject *Sender)
{
    if (!ViewSchlagpatrone->Checked) return;

    int t1 = GetFirstTritt();
    int t2 = GetLastTritt();
    if (t2<t1) return;

    for (int j=schuesse.a; j<=schuesse.b; j++) {
        for (int i=t1; i<=t2; i++) {
            char s = trittfolge.feld.Get (i, j);
            trittfolge.feld.Set (i, j, char(s==0 ? currentrange : -s));
        }
        for (int i=kette.a; i<=kette.b; i++) {
            char s = gewebe.feld.Get (i, j);
            gewebe.feld.Set (i, j, char(s==0 ? currentrange : -s));
        }
    }

    Invalidate();
    SetModified();

    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/

