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
#include "dbw3_form.h"
#include "dbw3_strings.h"
#include "cursor.h"
#include "undoredo.h"
#include "Datamodule.h"
#include "assert.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::IdleHandler (TObject* Sender, bool& Done)
{
    // Bereichsops aktualisieren
    RANGE savesel = selection;
    selection.Normalize();
    bool valid = selection.Valid();
    bool square = (selection.feld==GEWEBE ||
                   selection.feld==AUFKNUEPFUNG ||
                   (selection.feld==TRITTFOLGE && (ViewSchlagpatrone->Checked || !trittfolge.einzeltritt))) &&
                  ((selection.end.i-selection.begin.i)==(selection.end.j-selection.begin.j));
    SBEditCut->Enabled = valid;
    SBEditCopy->Enabled = valid;
    EditCut->Enabled = valid;
    EditCopy->Enabled = valid;
    EditInvert->Enabled = valid;
    EditRotate->Enabled = valid && square;
    EditMirrorVert->Enabled = valid;
    EditMirrorHorz->Enabled = valid;
    EditCentralsym->Enabled = valid;
    EditDelete->Enabled = valid;
    EditFillKoeper->Enabled = valid &&
        selection.feld==GEWEBE &&
        ((selection.begin.i==selection.end.i) ||
         (selection.begin.j==selection.end.j));
    RollUp->Enabled = valid;
    RollDown->Enabled = valid;
    RollLeft->Enabled = valid;
    RollRight->Enabled = valid;
	SteigungInc->Enabled = valid;
	SteigungDec->Enabled = valid;
    selection = savesel;

    PopupMenu = valid ? PopupMenu1 : Popupmenu;
    if (PopupMenu==PopupMenu1) UpdatePopupMenu1();

    dbw3_assert(undo);
    EditUndo->Enabled = undo->CanUndo();
    EditRedo->Enabled = undo->CanRedo();
    SBUndo->Enabled = EditUndo->Enabled;
    SBRedo->Enabled = EditRedo->Enabled;

    SBInvert->Enabled = EditInvert->Enabled;
    SBMirrorH->Enabled = EditMirrorHorz->Enabled;
    SBMirrorV->Enabled = EditMirrorVert->Enabled;
    SBRotate->Enabled = EditRotate->Enabled;
    SBCentralsym->Enabled = EditCentralsym->Enabled;
    SBHighlight->Enabled = kbd_field==GEWEBE || kbd_field==AUFKNUEPFUNG ||
                           kbd_field==EINZUG || kbd_field==TRITTFOLGE;

    bool canTfGerade = (rapport.sr.b-rapport.sr.a+1)<=Data->MAXX2;
    bool canEzGerade = (rapport.kr.b-rapport.kr.a+1)<=Data->MAXY1;
    TfGeradeZ->Enabled = canTfGerade;
    TfGeradeS->Enabled = canTfGerade;
    EzGeradeZ->Enabled = canEzGerade;
    EzGeradeS->Enabled = canEzGerade;

    UpdateMoveMenu();
    UpdateInsertMenu();
    UpdateDeleteMenu();

    // Änderungen verwerfen nur enablen, wenn Datei schon
    // gespeichert und geändert!
    FileRevert->Enabled = filename!="" && modified;

    // Paste nur enablen, wenn Zwischenablage gefüllt!
    bool can_paste = false;
    if (::OpenClipboard(NULL)) {
        HGLOBAL hMem = ::GetClipboardData (CF_TEXT);
        if (hMem) {
            char* ptr = (char*)GlobalLock(hMem);
            if (strstr(ptr, "dbw")==ptr) {
                can_paste = true;
            }
            GlobalUnlock (hMem);
        }
        ::CloseClipboard();
    }
    EditPaste->Enabled = can_paste;
    EditPasteTransparent->Enabled = can_paste;
    SBEditPaste->Enabled = can_paste;
    PastePopup->Visible = can_paste;
    PasteTransparentlyPopup->Visible = can_paste;
    PasteDividerPopup->Visible = can_paste;
    Paste1Popup->Visible = can_paste;
    Paste1TransparentlyPopup->Visible = can_paste; 

    MenuEinfuegen->Visible = (kbd_field==GEWEBE);
    ReplaceColor->Visible = (kbd_field==KETTFARBEN || kbd_field==SCHUSSFARBEN);

    MenuTrittfolge->Visible = !ViewSchlagpatrone->Checked;
    MenuSchlagpatrone->Visible = ViewSchlagpatrone->Checked;
    MenuAufknuepfung->Visible = !ViewSchlagpatrone->Checked;
    if (ViewSchlagpatrone->Checked) {
        if (TfMinimalZ->Checked) savedtrittfolgenstyle = TfMinimalZ;
        else if (TfMinimalS->Checked) savedtrittfolgenstyle = TfMinimalS;
        else if (TfGeradeZ->Checked) savedtrittfolgenstyle = TfGeradeZ;
        else if (TfGeradeS->Checked) savedtrittfolgenstyle = TfGeradeS;
        else if (TfGesprungen->Checked) savedtrittfolgenstyle = TfGesprungen;
        TfBelassen->Checked = true;
    }

    if ((GetKeyState(VK_CONTROL)&0x8000) &&
        (GetKeyState(VK_MENU)&0x8000))
    {
        String line = CURSORMOVE;
        CURSORDIRECTION cd = cursorhandler->GetCursorDirection();
        if (cd&CD_LEFT) {
            if (righttoleft && (kbd_field==GEWEBE || kbd_field==EINZUG || kbd_field==KETTFARBEN || kbd_field==BLATTEINZUG))
                line += String(CURSORRIGHT);
            else
                line += String(CURSORLEFT);
        }
        if (cd&CD_RIGHT) {
            if (righttoleft && (kbd_field==GEWEBE || kbd_field==EINZUG || kbd_field==KETTFARBEN || kbd_field==BLATTEINZUG))
                line += String(CURSORLEFT);
            else
                line += String(CURSORRIGHT);
        }
        if (cd&CD_UP) {
            if (toptobottom && (kbd_field==EINZUG || kbd_field==AUFKNUEPFUNG))
                line += String(CURSORDOWN);
            else
                line += String(CURSORUP);
        }
        if (cd&CD_DOWN) {
            if (toptobottom && (kbd_field==EINZUG || kbd_field==AUFKNUEPFUNG))
                line += String(CURSORUP);
            else
                line += String(CURSORDOWN);
        }
        if (cd==CD_NONE) line = CURSORNOMOVE;
        Statusbar->Panels->Items[0]->Text = line;
    }

    if (hlinedrag && hline) {
        String line = hline->typ==HL_VERT ? HLINEVERT : HLINEHORZ;
        line += String(HLINEPOS);
        line += IntToStr(hline->pos);
        line += String("/");
        line += IntToStr(hline->pos+1);
        Statusbar->Panels->Items[0]->Text = line;
    }

    if (showweaveposition) {
        showweaveposition = false;
        char buff[40];
        wsprintf (buff, STRG_WEBPOS, weave_position+1);
        Statusbar->Panels->Items[0]->Text = buff;
    }

    if (Statusbar->Panels->Items[0]->Text=="") {
        if (!bSelectionCleared) {
            RANGE sel = selection;
            sel.Normalize();
            if (sel.Valid()) {
                int dx = sel.end.i-sel.begin.i+1;
                int dy = sel.end.j-sel.begin.j+1;
                if (dx>1 || dy>1) {
                    UpdateStatusBar();
                }
            }
        }
    }

    bool swapside = ViewSchlagpatrone->Checked || trittfolge.einzeltritt;
    EditSwapside->Enabled = swapside;
    EditSwapside->Visible = swapside;

    UpdateToolButtons();

    UserdefAddSel->Enabled = valid && selection.feld==GEWEBE;

    DebugChecks();

    if (handlecommandline) {
        handlecommandline = false;
        HandleCommandlineArguments();
    }
}
/*-----------------------------------------------------------------*/

