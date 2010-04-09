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
#include <stdio.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ViewBlatteinzugPopupClick(TObject *Sender)
{
    ViewBlatteinzugClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ViewFarbePopupClick(TObject *Sender)
{
    ViewFarbeClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ViewEinzugPopupClick(TObject *Sender)
{
    ViewEinzugClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ViewTrittfolgePopupClick(TObject *Sender)
{
    ViewTrittfolgeClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::GewebeNormalPopupClick(TObject *Sender)
{
    GewebeNormalClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::GewebeFarbeffektPopupClick(TObject *Sender)
{
    GewebeFarbeffektClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::GewebeSimulationPopupClick(TObject *Sender)
{
    GewebeSimulationClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::GewebeNonePopupClick(TObject *Sender)
{
    GewebeNoneClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1CancelClick(TObject *Sender)
{
    //
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1CopyClick(TObject *Sender)
{
    EditCopyClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1CutClick(TObject *Sender)
{
    EditCutClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1InvertClick(TObject *Sender)
{
    EditInvertClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1MirrorHClick(TObject *Sender)
{
    EditMirrorHorzClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1MirrorVClick(TObject *Sender)
{
    EditMirrorVertClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1RotateClick(TObject *Sender)
{
    EditRotateClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1CentralsymClick(TObject *Sender)
{
    EditCentralsymClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1DeleteClick(TObject *Sender)
{
    EditDeleteClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1RollUpClick(TObject *Sender)
{
    RollUpClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1RollDownClick(TObject *Sender)
{
    RollDownClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1RollLeftClick(TObject *Sender)
{
    RollLeftClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1RollRightClick(TObject *Sender)
{
    RollRightClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::PastePopupClick(TObject *Sender)
{
    EditPasteClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::PasteTransparentlyPopupClick(TObject *Sender)
{
    EditPasteTransparentClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1PasteClick(TObject *Sender)
{
    EditPasteClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Popup1PasteTransparentlyClick(TObject *Sender)
{
    EditPasteTransparentClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::UpdatePopupMenu1()
{
    RANGE savesel = selection;
    selection.Normalize();
    bool valid = selection.Valid();
    bool square = (selection.feld==GEWEBE ||
                   selection.feld==AUFKNUEPFUNG ||
                   (selection.feld==TRITTFOLGE && ViewSchlagpatrone->Checked)) &&
                  ((selection.end.i-selection.begin.i)==(selection.end.j-selection.begin.j));
    selection = savesel;

    Popup1Rotate->Visible = valid && square;
    Popup1Centralsym->Visible = valid;

    // Paste in idle...
}
/*-----------------------------------------------------------------*/

