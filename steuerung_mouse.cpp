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
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "steuerung_form.h"
#include "dbw3_form.h"
#include "assert.h"
#include "datamodule.h"
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    bool updatestatusbar = false;

    // Ist es im Schlagpatronenbereich?
    if (X>left+(x1+1)*gridsize && X<left+maxi*gridsize && Y<bottom && Y>bottom-maxj*gridsize) {
        ClearSelection();
        int schuss = scrolly + (bottom - Y) / gridsize;
        ClearPositionSelection();
        weave_position = schuss;
        if (weave_position>=scrolly+maxj) weave_position = scrolly+maxj-1;
        ValidateWeavePosition();
        DrawPositionSelection();
        schussselected = true;
        DrawSelection();
        updatestatusbar = true;
    }

    // Ist es im Klammernbereich?
    if (X>left+maxi*gridsize+dx && X<left+maxi*gridsize+dx+MAXKLAMMERN*11 && Y<bottom && Y>bottom-maxj*gridsize) {
        // Klammer bestimmen
        int klammer = (X-left-maxi*gridsize-dx) / 11;
        dbw3_assert (klammer>=0);
        dbw3_assert (klammer<MAXKLAMMERN);

        // Wenn Klammer nicht getroffen, diese nur selektieren...
        int start, stop;
        if (klammern[klammer].first<scrolly) start = 0;
        else start = klammern[klammer].first-scrolly;
        if (klammern[klammer].last>scrolly+maxj) stop = maxj;
        else stop = klammern[klammer].last-scrolly;
        int j = (bottom - Y) / gridsize;

        ClearSelection();
        schussselected = false;
        current_klammer = klammer;
        DrawSelection();
        updatestatusbar = true;

        if (j>=start && j<=stop) {
            dragging = true;
            drag_klammer = klammer;
            drag_j = j;
            if (j==start) drag_style = DRAG_BOTTOM;
            else if (j==stop) drag_style = DRAG_TOP;
            else drag_style = DRAG_MIDDLE;
        }

    }

    if (updatestatusbar) UpdateStatusbar();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::FormMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
    // Wird gedraggt? Dann updaten
    if (dragging) {
        int j = (bottom - Y) / gridsize;
        if (j!=drag_j) {
            ClearKlammerSelection();
            switch (drag_style) {
                case DRAG_TOP:
                    klammern[drag_klammer].last = scrolly+j;
                    if (klammern[drag_klammer].last<klammern[drag_klammer].first+1)
                        klammern[drag_klammer].last = klammern[drag_klammer].first + 1;
                    break;
                case DRAG_BOTTOM:
                    klammern[drag_klammer].first = scrolly+j;
                    if (klammern[drag_klammer].first>klammern[drag_klammer].last-1)
                        klammern[drag_klammer].first = klammern[drag_klammer].last-1;
                    break;
                case DRAG_MIDDLE: {
                    int diff = j - drag_j;
                    klammern[drag_klammer].first += diff;
                    klammern[drag_klammer].last += diff;
                    if (klammern[drag_klammer].last>=Data->MAXY2) klammern[drag_klammer].last = Data->MAXY2-1;
                    if (klammern[drag_klammer].first>=Data->MAXY2) klammern[drag_klammer].first = Data->MAXY2-1;
                    if (klammern[drag_klammer].last<0) klammern[drag_klammer].last = 0;
                    if (klammern[drag_klammer].first<0) klammern[drag_klammer].first = 0;
                    break;
                }
            }
            drag_j = j;
            DrawKlammerSelection();
            SetModified();
            UpdateStatusbar();
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    // Wird Klammer gedraggt? Dann dragging beenden
    if (dragging) {
        dragging = false;
    }
}
/*-----------------------------------------------------------------*/
