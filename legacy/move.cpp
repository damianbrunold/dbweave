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
#include "datamodule.h"
#include "dbw3_form.h"
#include "einzug.h"
#include "cursor.h"
#include "undoredo.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::UpdateMoveMenu()
{
    // Schaft-Switch
    if (kbd_field==EINZUG || kbd_field==AUFKNUEPFUNG) {
        SchaftMoveUp->Enabled = true;
        SchaftMoveDown->Enabled = true;
        SBMoveUp->Enabled = true;
        SBMoveDown->Enabled = true;
    } else {
        SchaftMoveUp->Enabled = false;
        SchaftMoveDown->Enabled = false;
        SBMoveUp->Enabled = false;
        SBMoveDown->Enabled = false;
    }

    // Tritt-Switch
    if ((kbd_field==TRITTFOLGE || kbd_field==AUFKNUEPFUNG) && !ViewSchlagpatrone->Checked) {
        TrittMoveLeft->Enabled = true;
        TrittMoveRight->Enabled = true;
        SBMoveLeft->Enabled = true;
        SBMoveRight->Enabled = true;
    } else {
        TrittMoveLeft->Enabled = false;
        TrittMoveRight->Enabled = false;
        SBMoveLeft->Enabled = false;
        SBMoveRight->Enabled = false;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SchaftMoveUpClick(TObject *Sender)
{
    dbw3_assert(kbd_field==EINZUG || kbd_field==AUFKNUEPFUNG);
    dbw3_assert(einzughandler);
    dbw3_assert(cursorhandler);

    // Aktuellen Schaft ermitteln
    int currentschaft;
    if (kbd_field==EINZUG) currentschaft = einzug.kbd.j+scroll_y1;
    else currentschaft = aufknuepfung.kbd.j+scroll_y1;
    if (currentschaft==-1) return;

    // Schaft ggf. switchen
    if (currentschaft<Data->MAXY1-1) {
        einzughandler->SwitchSchaefte (currentschaft, currentschaft+1);
        cursorhandler->MoveCursorUp (1, false);
        EzBelassen->Checked = true;

        SetModified();
        dbw3_assert (undo!=0);
        undo->Snapshot();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SchaftMoveDownClick(TObject *Sender)
{
    dbw3_assert(kbd_field==EINZUG || kbd_field==AUFKNUEPFUNG);
    dbw3_assert(einzughandler);
    dbw3_assert(cursorhandler);

    // Aktuellen Schaft ermitteln
    int currentschaft;
    if (kbd_field==EINZUG) currentschaft = einzug.kbd.j+scroll_y1;
    else currentschaft = aufknuepfung.kbd.j+scroll_y1;
    if (currentschaft==-1) return;

    // Schaft ggf. switchen
    if (currentschaft>0) {
        einzughandler->SwitchSchaefte (currentschaft, currentschaft-1);
        cursorhandler->MoveCursorDown (1, false);
        EzBelassen->Checked = true;

        SetModified();
        dbw3_assert (undo!=0);
        undo->Snapshot();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::TrittMoveLeftClick(TObject *Sender)
{
    dbw3_assert(kbd_field==TRITTFOLGE || kbd_field==AUFKNUEPFUNG);
    dbw3_assert(cursorhandler);

    if (ViewSchlagpatrone->Checked) return;

    // Aktuellen Tritt ermitteln
    int currenttritt;
    if (kbd_field==TRITTFOLGE) currenttritt = trittfolge.kbd.i+scroll_x2;
    else currenttritt = aufknuepfung.kbd.i+scroll_x2;
    if (currenttritt==-1) return;

    // Tritt ggf. switchen
    if (currenttritt>0) {
        SwitchTritte (currenttritt, currenttritt-1);
        cursorhandler->MoveCursorLeft (1, false);
        TfBelassen->Checked = true;

        SetModified();
        dbw3_assert (undo!=0);
        undo->Snapshot();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::TrittMoveRightClick(TObject *Sender)
{
    dbw3_assert(kbd_field==TRITTFOLGE || kbd_field==AUFKNUEPFUNG);
    dbw3_assert(!ViewSchlagpatrone->Checked);
    dbw3_assert(cursorhandler);

    if (ViewSchlagpatrone->Checked) return;

    // Aktuellen Tritt ermitteln
    int currenttritt;
    if (kbd_field==TRITTFOLGE) currenttritt = trittfolge.kbd.i+scroll_x2;
    else currenttritt = aufknuepfung.kbd.i+scroll_x2;
    if (currenttritt==-1) return;

    // Tritt ggf. switchen
    if (currenttritt<Data->MAXX2-1) {
        SwitchTritte (currenttritt, currenttritt+1);
        cursorhandler->MoveCursorRight (1, false);
        TfBelassen->Checked = true;

        SetModified();
        dbw3_assert (undo!=0);
        undo->Snapshot();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBMoveUpClick(TObject *Sender)
{
    SchaftMoveUpClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBMoveDownClick(TObject *Sender)
{
    SchaftMoveDownClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBMoveLeftClick(TObject *Sender)
{
    TrittMoveLeftClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBMoveRightClick(TObject *Sender)
{
    TrittMoveRightClick(Sender);
}
/*-----------------------------------------------------------------*/
