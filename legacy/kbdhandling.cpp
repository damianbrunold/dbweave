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
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "datamodule.h"
#include "felddef.h"
#include "cursor.h"
#include "splash_form.h"
/*-----------------------------------------------------------------*/
const int FIELDTOGGLEKEY = ' ';
const int CURSOR_NORMAL_SKIP = 1;
/*-----------------------------------------------------------------*/
int CURSOR_LARGE_SKIP_X;
int CURSOR_LARGE_SKIP_Y;
/*-----------------------------------------------------------------*/
extern TSplashscreen* Splashscreen;
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SetCursor (int _i, int _j, bool _clearselection)
{
    dbw3_assert(cursorhandler);
    cursorhandler->SetCursor (kbd_field, _i, _j, _clearselection);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawCursor()
{
    dbw3_assert(cursorhandler);
    cursorhandler->DrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DeleteCursor()
{
    dbw3_assert(cursorhandler);
    cursorhandler->DeleteCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::GotoNextField()
{
    dbw3_assert(cursorhandler);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::GotoPrevField()
{
    dbw3_assert(cursorhandler);
}
/*-----------------------------------------------------------------*/
static void __fastcall ScrollPage (TScrollBar* _sb, bool _inc)
{
    int pos = _sb->Position;

    if (_inc) pos += _sb->LargeChange;
    else pos -= _sb->LargeChange;

    if (pos>_sb->Max) pos = _sb->Max;
    if (pos<_sb->Min) pos = _sb->Min;

    _sb->Position = pos;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
    dbw3_assert(cursorhandler);

    if (Splashscreen && Splashscreen->Visible) {
        if (!splashdata.demo) Splashscreen->Visible = false;
        return;
    }

    if (dohighlight && Key!=VK_F12) {
       ClearHighlight();
       dohighlight = false;
    }

    if (Key==VK_RETURN || (Key==VK_F6 && !Shift.Contains(ssShift))) {
        cursorhandler->GotoNextField();
        UpdateStatusBar();
    } else if (Key==VK_ESCAPE || (Key==VK_F6 && Shift.Contains(ssShift))) {
        cursorhandler->GotoPrevField();
        UpdateStatusBar();
    } else if (Key==FIELDTOGGLEKEY && !Shift.Contains(ssAlt)) {
        cursorhandler->ToggleField (Shift);
        UpdateStatusBar();
    } else if ((Key=='K' || Key=='k') && !Shift.Contains(ssAlt)) {
        EditCopyClick (this);
    } else if ((Key=='E' || Key=='e') && !Shift.Contains(ssAlt)) {
        EditPasteClick (this);
    } else if ((Key=='T' || Key=='t') && !Shift.Contains(ssAlt)) {
        EditPasteTransparentClick (this);
    } else if (!Shift.Contains(ssAlt) && Key>='0' && Key<='9') {
        if (Shift.Contains(ssShift) && Shift.Contains(ssCtrl)) {
            if (Key>'0') SwitchRange (Key-'0');
        } else if (kbd_field==GEWEBE || kbd_field==AUFKNUEPFUNG || (kbd_field==TRITTFOLGE && ViewSchlagpatrone->Checked)) {
            // Punkt im entsprechenden Bereich setzen!
            if (Key-'0'!=currentrange && Key!='0')
                switch (Key-'0') {
                    case 1: Range1Click(this); break;
                    case 2: Range2Click(this); break;
                    case 3: Range3Click(this); break;
                    case 4: Range4Click(this); break;
                    case 5: Range5Click(this); break;
                    case 6: Range6Click(this); break;
                    case 7: Range7Click(this); break;
                    case 8: Range8Click(this); break;
                    case 9: Range9Click(this); break;
                }
            char save = currentrange;
            if (Key=='0') currentrange = 0;
            cursorhandler->SetField (Key=='0' ? false : true, Shift);
            if (Key=='0') {
                currentrange = save;
                UpdateStatusBar();
            }
        }
    } else if (Key==VK_F12) {
        DrawHighlight();
        dohighlight = true;
    } else if ((Key=='C' || Key=='c') && Shift.Contains(ssShift) && Shift.Contains(ssCtrl) && !Shift.Contains(ssAlt)) {
        UserdefAddSelClick(Sender);
    } else if ((Key=='V' || Key=='v') && Shift.Contains(ssShift) && Shift.Contains(ssCtrl) && !Shift.Contains(ssAlt)) {
        PasteUserdef(false);
    } else if ((Key=='B' || Key=='b') && Shift.Contains(ssShift) && Shift.Contains(ssCtrl) && !Shift.Contains(ssAlt)) {
        PasteUserdef(true);
    } else {
        bool ctrl = Shift.Contains(ssCtrl);
        bool alt = Shift.Contains(ssAlt);
        bool select = Shift.Contains(ssShift);
        int step;
        if (alt) {
            if (Key==VK_LEFT || Key==VK_RIGHT) {
                step = CURSOR_LARGE_SKIP_X;
                int width = selection.Width();
                if (select && width==1) step -= 1;
            } else {
                step = CURSOR_LARGE_SKIP_Y;
                int height = selection.Height();
                if (select && height==1) step -= 1;
            }
        } else if (ctrl) {
            if (Key==VK_LEFT || Key==VK_RIGHT) {
                step = CURSOR_LARGE_SKIP_X / 2;
                int width = selection.Width();
                if (select && width==1) step -= 1;
            } else {
                step = CURSOR_LARGE_SKIP_Y / 2;
                int height = selection.Height();
                if (select && height==1) step -= 1;
            }
        } else
            step = CURSOR_NORMAL_SKIP;
        switch (Key) {
            case VK_LEFT:
                if (ctrl && alt) {
                    CURSORDIRECTION cd = cursorhandler->GetCursorDirection();
                    if (righttoleft && (kbd_field==GEWEBE || kbd_field==EINZUG || kbd_field==KETTFARBEN || kbd_field==BLATTEINZUG)) {
                        if (cd&CD_LEFT) cd = cd & ~CD_LEFT;
                        else cd = cd | CD_RIGHT;
                    } else {
                        if (cd&CD_RIGHT) cd = cd & ~CD_RIGHT;
                        else cd = cd | CD_LEFT;
                    }
                    cursorhandler->SetCursorDirection (cd);
                } else
                    cursorhandler->MoveCursorLeft (step, select);
                UpdateStatusBar();
                break;
            case VK_RIGHT:
                if (ctrl && alt) {
                    CURSORDIRECTION cd = cursorhandler->GetCursorDirection();
                    if (righttoleft && (kbd_field==GEWEBE || kbd_field==EINZUG || kbd_field==KETTFARBEN || kbd_field==BLATTEINZUG)) {
                        if (cd&CD_RIGHT) cd = cd & ~CD_RIGHT;
                        else cd = cd | CD_LEFT;
                    } else {
                        if (cd&CD_LEFT) cd = cd & ~CD_LEFT;
                        else cd = cd | CD_RIGHT;
                    }
                    cursorhandler->SetCursorDirection (cd);
                } else
                    cursorhandler->MoveCursorRight (step, select);
                UpdateStatusBar();
                break;
            case VK_UP:
                if (ctrl && alt) {
                    CURSORDIRECTION cd = cursorhandler->GetCursorDirection();
                    if (toptobottom && (kbd_field==EINZUG || kbd_field==AUFKNUEPFUNG)) {
                        if (cd&CD_UP) cd = cd & ~CD_UP;
                        else cd = cd | CD_DOWN;
                    } else {
                        if (cd&CD_DOWN) cd = cd & ~CD_DOWN;
                        else cd = cd | CD_UP;
                    }
                    cursorhandler->SetCursorDirection (cd);
                } else
                    cursorhandler->MoveCursorUp (step, select);
                UpdateStatusBar();
                break;
            case VK_DOWN:
                if (ctrl && alt) {
                    CURSORDIRECTION cd = cursorhandler->GetCursorDirection();
                    if (toptobottom && (kbd_field==EINZUG || kbd_field==AUFKNUEPFUNG)) {
                        if (cd&CD_DOWN) cd = cd & ~CD_DOWN;
                        else cd = cd | CD_UP;
                    } else {
                        if (cd&CD_UP) cd = cd & ~CD_UP;
                        else cd = cd | CD_DOWN;
                    }
                    cursorhandler->SetCursorDirection (cd);
                } else
                    cursorhandler->MoveCursorDown (step, select);
                UpdateStatusBar();
                break;
            case VK_HOME:
                HandleHomeKey (Shift);
                UpdateStatusBar();
                break;
            case VK_END:
                HandleEndKey (Shift);
                UpdateStatusBar();
                break;
            case VK_PRIOR:
                HandlePageUpKey (Shift);
                UpdateStatusBar();
                break;
            case VK_NEXT:
                HandlePageDownKey (Shift);
                UpdateStatusBar();
                break;
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
    if (Key==VK_TAB && !Shift.Contains(ssAlt) && !Shift.Contains(ssCtrl)) {
        if (Shift.Contains(ssShift)) {
            cursorhandler->GotoPrevField();
            UpdateStatusBar();
        } else {
            cursorhandler->GotoNextField();
            UpdateStatusBar();
        }
    } else if (Key==VK_F12) {
        ClearHighlight();
        dohighlight = false;
    } 
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::CursorTimerTimer(TObject *Sender)
{
    dbw3_assert(cursorhandler);
    cursorhandler->ToggleCursor();
}
/*-----------------------------------------------------------------*/

