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
void __fastcall TSTRGFRM::WeaveKlammerRight()
{
    ClearPositionSelection();
    weave_klammer++;
    while (weave_klammer<MAXKLAMMERN && klammern[weave_klammer].repetitions==0)
        weave_klammer++;
    if (weave_klammer>=MAXKLAMMERN || weave_klammer<0) weave_klammer=0;
    if (weave_repetition>klammern[weave_klammer].repetitions)
        weave_repetition = klammern[weave_klammer].repetitions;
    ValidateWeavePosition();
    DrawPositionSelection();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::WeaveKlammerLeft()
{
    ClearPositionSelection();
    weave_klammer--;
    while (weave_klammer>0 && klammern[weave_klammer].repetitions==0)
        weave_klammer--;
    if (weave_klammer<0 || weave_klammer>=MAXKLAMMERN) weave_klammer=0;
    if (weave_repetition>klammern[weave_klammer].repetitions)
        weave_repetition = klammern[weave_klammer].repetitions;
    ValidateWeavePosition();
    DrawPositionSelection();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::WeaveRepetitionInc()
{
    weave_repetition++;
    if (weave_repetition>klammern[weave_klammer].repetitions)
        weave_repetition = klammern[weave_klammer].repetitions;
    ValidateWeavePosition();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::WeaveRepetitionDec()
{
    weave_repetition--;
    if (weave_repetition<1) weave_repetition = 1;
    ValidateWeavePosition();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::ValidateWeavePosition()
{
    if (!IsValidWeavePosition()) {
        // Versuchen ne passende Klammer zu finden
        for (int i=weave_klammer+1; i<=weave_klammer+MAXKLAMMERN; i++) {
            int wk = i % MAXKLAMMERN;
            if (klammern[wk].repetitions>0) {
                if (weave_position>=klammern[wk].first && weave_position<=klammern[wk].last) {
                    weave_klammer = wk;
                    weave_repetition = 1;
                    return;
                }
            }
        }
        weave_klammer = 0;
        weave_repetition = 0;
    }
}
/*-----------------------------------------------------------------*/
bool __fastcall TSTRGFRM::IsValidWeavePosition()
{
    if (weave_klammer<0) return false;
    if (weave_klammer>=MAXKLAMMERN) return false;
    if (klammern[weave_klammer].repetitions==0) return false;
    if (klammern[weave_klammer].repetitions<weave_repetition) return false;
    if (klammern[weave_klammer].first>weave_position) return false;
    if (klammern[weave_klammer].last<weave_position) return false;
    return true;
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoLastPosition()
{
    ClearPositionSelection();
    weave_position = last_position;
    weave_klammer = last_klammer;
    weave_repetition = last_repetition;
    DrawPositionSelection();
    AutoScroll();
    UpdateStatusbar();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::UpdateLastPosition()
{
    last_position = weave_position;
    last_klammer = weave_klammer;
    last_repetition = weave_repetition;
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::SBSetCurrentPosClick(TObject *Sender)
{
    SetCurrentPosClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoLastPosClick(TObject *Sender)
{
    GotoLastPosition();
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer (int _klammer)
{
    dbw3_assert(_klammer>=0);
    dbw3_assert(_klammer<MAXKLAMMERN);
    if (klammern[_klammer].repetitions!=0) {
        ClearPositionSelection();
        weave_position = klammern[_klammer].first;
        weave_repetition = 1;
        weave_klammer = _klammer;
        DrawPositionSelection();
        AutoScroll();
        UpdateStatusbar();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer1Click(TObject *Sender)
{
    GotoKlammer(0);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer2Click(TObject *Sender)
{
    GotoKlammer(1);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer3Click(TObject *Sender)
{
    GotoKlammer(2);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer4Click(TObject *Sender)
{
    GotoKlammer(3);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer5Click(TObject *Sender)
{
    GotoKlammer(4);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer6Click(TObject *Sender)
{
    GotoKlammer(5);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer7Click(TObject *Sender)
{
    GotoKlammer(6);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer8Click(TObject *Sender)
{
    GotoKlammer(7);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer9Click(TObject *Sender)
{
    GotoKlammer(8);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::sbGoto1Click(TObject *Sender)
{
    GotoKlammer(0);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::sbGoto2Click(TObject *Sender)
{
    GotoKlammer(1);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::sbGoto3Click(TObject *Sender)
{
    GotoKlammer(2);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::sbGoto4Click(TObject *Sender)
{
    GotoKlammer(3);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::sbGoto5Click(TObject *Sender)
{
    GotoKlammer(4);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::sbGoto6Click(TObject *Sender)
{
    GotoKlammer(5);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::sbGoto7Click(TObject *Sender)
{
    GotoKlammer(6);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::sbGoto8Click(TObject *Sender)
{
    GotoKlammer(7);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::sbGoto9Click(TObject *Sender)
{
    GotoKlammer(8);
}
/*-----------------------------------------------------------------*/

