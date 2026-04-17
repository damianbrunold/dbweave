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
void __fastcall TSTRGFRM::FormKeyPress(TObject *Sender, char &Key)
{
    if (Key==VK_RETURN) {
        ClearSelection();
        schussselected = !schussselected;
        DrawSelection();
        UpdateStatusbar();
    } else if (!schussselected && Key>='0' && Key<='9') {
        klammern[current_klammer].repetitions = Key-'0';
        DrawKlammer(current_klammer);
        UpdateStatusbar();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::HandleSchussKeyDown (WORD& Key, TShiftState Shift)
{
    if (Key==VK_UP) {
        if (Shift.Contains(ssCtrl)) {
            ClearPositionSelection();
            weave_position += 8;
            if (weave_position>=scrolly+maxj) weave_position = scrolly+maxj-1;
            ValidateWeavePosition();
            DrawPositionSelection();
        } else {
            ClearPositionSelection();
            weave_position++;
            if (weave_position>=scrolly+maxj) weave_position = scrolly+maxj-1;
            ValidateWeavePosition();
            DrawPositionSelection();
        }
        SetModified();
        UpdateStatusbar();
    } else if (Key==VK_DOWN) {
        if (Shift.Contains(ssCtrl)) {
            ClearPositionSelection();
            weave_position -= 8;
            if (weave_position<scrolly) weave_position=scrolly;
            ValidateWeavePosition();
            DrawPositionSelection();
        } else {
            ClearPositionSelection();
            weave_position--;
            if (weave_position<scrolly) weave_position=scrolly;
            ValidateWeavePosition();
            DrawPositionSelection();
        }
        SetModified();
        UpdateStatusbar();
    } else if (Key==VK_PRIOR) {
        int sy = scrolly;
        ClearPositionSelection();
        if (weave_position<scrolly+maxj-1) {
            weave_position = scrolly+maxj-1;
        } else {
            scrolly += maxj;
            if (scrolly>Data->MAXY2-maxj) scrolly = Data->MAXY2-maxj;
            weave_position = scrolly + maxj - 1;
        }
        scrollbar->Position = Data->MAXY2 - maxj - scrolly;
        ValidateWeavePosition();
        if (sy!=scrolly) Invalidate();
        else DrawPositionSelection();
        SetModified();
        UpdateStatusbar();
    } else if (Key==VK_NEXT) {
        int sy = scrolly;
        ClearPositionSelection();
        if (weave_position>scrolly) {
            weave_position = scrolly;
        } else {
            scrolly -= maxj;
            if (scrolly<0) scrolly = 0;
            weave_position = scrolly;
        }
        scrollbar->Position = Data->MAXY2 - maxj - scrolly;
        ValidateWeavePosition();
        if (sy!=scrolly) Invalidate();
        else DrawPositionSelection();
        SetModified();
        UpdateStatusbar();
    } else if (Key==VK_LEFT) {
        if (Shift.Contains(ssCtrl)) {
            WeaveRepetitionDec();
        } else {
            WeaveKlammerLeft();
        }
        UpdateStatusbar();
    } else if (Key==VK_RIGHT) {
        if (Shift.Contains(ssCtrl)) {
            WeaveRepetitionInc();
        } else {
            WeaveKlammerRight();
        }
        UpdateStatusbar();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::HandleKlammerKeyDown (WORD& Key, TShiftState Shift)
{
    if (Key==VK_LEFT) {
        ClearSelection();
        current_klammer = (current_klammer+MAXKLAMMERN-1) % MAXKLAMMERN;
        DrawSelection();
        UpdateStatusbar();
    } else if (Key==VK_RIGHT) {
        ClearSelection();
        current_klammer = (current_klammer+1) % MAXKLAMMERN;
        DrawSelection();
        UpdateStatusbar();
    } else if (Key==VK_UP) {
        if (Shift.Contains(ssShift) && Shift.Contains(ssCtrl)) {
            klammern[current_klammer].first += 8;
            klammern[current_klammer].last += 8;
            if (klammern[current_klammer].last>=Data->MAXY2) {
                klammern[current_klammer].first -= (klammern[current_klammer].last - (Data->MAXY2-1));
                klammern[current_klammer].last = Data->MAXY2-1;
            }
        } else if (Shift.Contains(ssShift)) {
            if (klammern[current_klammer].last<Data->MAXY2-1) {
                klammern[current_klammer].first++;
                klammern[current_klammer].last++;
            }
        } else if (Shift.Contains(ssCtrl)) {
            klammern[current_klammer].first++;
            if (klammern[current_klammer].first>=klammern[current_klammer].last)
                klammern[current_klammer].first = klammern[current_klammer].last-1;
        } else {
            if (klammern[current_klammer].last<Data->MAXY2-1)
                klammern[current_klammer].last++;
        }
        UpdateStatusbar();
        SetModified();
        DrawKlammer (current_klammer);
    } else if (Key==VK_DOWN) {
        if (Shift.Contains(ssShift) && Shift.Contains(ssCtrl)) {
            klammern[current_klammer].first -= 8;
            klammern[current_klammer].last -= 8;
            if (klammern[current_klammer].first<0) {
                klammern[current_klammer].last += (-klammern[current_klammer].first);
                klammern[current_klammer].first = 0;
            }
        } else if (Shift.Contains(ssShift)) {
            if (klammern[current_klammer].first>0) {
                klammern[current_klammer].first--;
                klammern[current_klammer].last--;
            }
        } else if (Shift.Contains(ssCtrl)) {
            if (klammern[current_klammer].first>0)
                klammern[current_klammer].first--;
        } else {
            if (klammern[current_klammer].last>klammern[current_klammer].first+1)
                klammern[current_klammer].last--;
        }
        DrawKlammer (current_klammer);
        SetModified();
        UpdateStatusbar();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
    if (!schussselected) HandleKlammerKeyDown (Key, Shift);
    else HandleSchussKeyDown (Key, Shift);
}
/*-----------------------------------------------------------------*/
