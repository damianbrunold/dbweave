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
#include "datamodule.h"
#include "cursor.h"
/*-----------------------------------------------------------------*/
#define HSKIPLEFT 2
#define HSKIPRIGHT 2
#define VSKIPTOP 2
#define VSKIPBOTTOM 2
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::HandleHomeKey (TShiftState Shift)
{
    cursorhandler->DisableCursor();

    switch (kbd_field) {
        case EINZUG:
            HandleHomeKey (EINZUG, einzug.kbd, true,
                Shift.Contains (ssCtrl),
                scroll_x1, scroll_y1, sb_horz1);
            break;

        case GEWEBE:
            HandleHomeKey (GEWEBE, gewebe.kbd, true,
                Shift.Contains (ssCtrl),
                scroll_x1, scroll_y2, sb_horz1);
            break;

        case BLATTEINZUG:
            HandleHomeKey (BLATTEINZUG, blatteinzug.kbd, true,
                Shift.Contains (ssCtrl),
                scroll_x1, 0, sb_horz1);
            break;

        case KETTFARBEN:
            HandleHomeKey (KETTFARBEN, kettfarben.kbd, true,
                Shift.Contains (ssCtrl),
                scroll_x1, 0, sb_horz1);
            break;

        case TRITTFOLGE:
            HandleHomeKey (TRITTFOLGE, trittfolge.kbd, false,
                Shift.Contains (ssCtrl),
                scroll_x2, scroll_y2, sb_horz2);
            break;

        case AUFKNUEPFUNG:
            HandleHomeKey (AUFKNUEPFUNG, aufknuepfung.kbd, false,
                Shift.Contains (ssCtrl),
                scroll_x2, scroll_y1, sb_horz2);
            break;
    }
    cursorhandler->EnableCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::HandleEndKey (TShiftState Shift)
{
    cursorhandler->DisableCursor();

    switch (kbd_field) {
        case EINZUG:
            HandleEndKey (EINZUG, einzug.kbd,
                einzug.pos.width/einzug.gw, true,
                Shift.Contains (ssCtrl),
                scroll_x1, scroll_y1, sb_horz1);
            break;

        case GEWEBE:
            HandleEndKey (GEWEBE, gewebe.kbd,
                gewebe.pos.width/gewebe.gw, true,
                Shift.Contains (ssCtrl),
                scroll_x1, scroll_y2, sb_horz1);
            break;

        case BLATTEINZUG:
            HandleEndKey (BLATTEINZUG, blatteinzug.kbd,
                blatteinzug.pos.width/blatteinzug.gw, true,
                Shift.Contains (ssCtrl),
                scroll_x1, 0, sb_horz1);
            break;

        case KETTFARBEN:
            HandleEndKey (KETTFARBEN, kettfarben.kbd,
                kettfarben.pos.width/kettfarben.gw, true,
                Shift.Contains (ssCtrl),
                scroll_x1, 0, sb_horz1);
            break;

        case TRITTFOLGE:
            HandleEndKey (TRITTFOLGE, trittfolge.kbd,
                trittfolge.pos.width/trittfolge.gw, false,
                Shift.Contains (ssCtrl),
                scroll_x2, scroll_y2, sb_horz2);
            break;

        case AUFKNUEPFUNG:
            HandleEndKey (AUFKNUEPFUNG, aufknuepfung.kbd,
                aufknuepfung.pos.width/aufknuepfung.gw, false,
                Shift.Contains (ssCtrl),
                scroll_x2, scroll_y1, sb_horz2);
            break;
    }

    cursorhandler->EnableCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::HandlePageUpKey (TShiftState Shift)
{
    cursorhandler->DisableCursor();

    switch (kbd_field) {
        case EINZUG:
            HandlePageUpKey (EINZUG, einzug.kbd,
                einzug.pos.height/einzug.gh, true,
                Shift.Contains (ssCtrl),
                scroll_x1, scroll_y1, sb_vert1);
            break;

        case GEWEBE:
            HandlePageUpKey (GEWEBE, gewebe.kbd,
                gewebe.pos.height/gewebe.gh, false,
                Shift.Contains (ssCtrl),
                scroll_x1, scroll_y2, sb_vert2);
            break;

        case SCHUSSFARBEN:
            HandlePageUpKey (SCHUSSFARBEN, schussfarben.kbd,
                schussfarben.pos.height/schussfarben.gh, false,
                Shift.Contains (ssCtrl),
                0, scroll_y2, sb_vert2);
            break;

        case TRITTFOLGE:
            HandlePageUpKey (TRITTFOLGE, trittfolge.kbd,
                trittfolge.pos.height/trittfolge.gh, false,
                Shift.Contains (ssCtrl),
                scroll_x2, scroll_y2, sb_vert2);
            break;

        case AUFKNUEPFUNG:
            HandlePageUpKey (AUFKNUEPFUNG, aufknuepfung.kbd,
                aufknuepfung.pos.height/aufknuepfung.gh, true,
                Shift.Contains (ssCtrl),
                scroll_x2, scroll_y1, sb_vert1);
            break;
    }
    cursorhandler->EnableCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::HandlePageDownKey (TShiftState Shift)
{
    cursorhandler->DisableCursor();

    switch (kbd_field) {
        case EINZUG:
            HandlePageDownKey (EINZUG, einzug.kbd, true,
                Shift.Contains (ssCtrl),
                scroll_x1, scroll_y1, sb_vert1);
            break;

        case GEWEBE:
            HandlePageDownKey (GEWEBE, gewebe.kbd, false,
                Shift.Contains (ssCtrl),
                scroll_x1, scroll_y2, sb_vert2);
            break;

        case SCHUSSFARBEN:
            HandlePageDownKey (SCHUSSFARBEN, schussfarben.kbd, false,
                Shift.Contains (ssCtrl),
                0, scroll_y2, sb_vert2);
            break;

        case TRITTFOLGE:
            HandlePageDownKey (TRITTFOLGE, trittfolge.kbd, false,
                Shift.Contains (ssCtrl),
                scroll_x2, scroll_y2, sb_vert2);
            break;

        case AUFKNUEPFUNG:
            HandlePageDownKey (AUFKNUEPFUNG, aufknuepfung.kbd, true,
                Shift.Contains (ssCtrl),
                scroll_x2, scroll_y1, sb_vert1);
            break;
    }
    cursorhandler->EnableCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::HandleHomeKey (FELD _feld, INPUTPOS _kbd, bool _left, bool _ctrl, int& _scroll1, int _scroll2, TScrollBar* _sb)
{
    bool redraw = false;
    if (_ctrl) {
        int oldscroll = _scroll1;
        _scroll1 -= _sb->LargeChange / (_left ? HSKIPLEFT : HSKIPRIGHT);
        if (_scroll1<0) _scroll1 = 0;
        redraw = oldscroll!=_scroll1;
        cursorhandler->SetCursor (_feld, _kbd.i+oldscroll, _scroll2+_kbd.j, true);
        cursorhandler->CheckCursorPos();
    } else {
        if (_kbd.i>0) cursorhandler->SetCursor (_feld, _scroll1, _scroll2+_kbd.j, true);
        else {
            int oldscroll = _scroll1;
            _scroll1 -= _sb->LargeChange / (_left ? HSKIPLEFT : HSKIPRIGHT);
            if (_scroll1<0) _scroll1 = 0;
            redraw = oldscroll!=_scroll1;
        }
    }
    if (redraw) {
        if (_left) {
            InvalidateFeld (einzug.pos);
            InvalidateFeld (gewebe.pos);
            InvalidateFeld (blatteinzug.pos);
            InvalidateFeld (kettfarben.pos);
            InvalidateFeld (hlinehorz1);
        } else {
            InvalidateFeld (aufknuepfung.pos);
            InvalidateFeld (trittfolge.pos);
            InvalidateFeld (hlinehorz2);
        }
        _sb->Position = _scroll1;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::HandleEndKey (FELD _feld, INPUTPOS _kbd, int _width, bool _left, bool _ctrl, int& _scroll1, int _scroll2, TScrollBar* _sb)
{
    bool redraw = false;
    if (_ctrl) {
        int oldscroll = _scroll1;
        _scroll1 += _sb->LargeChange / (_left ? HSKIPLEFT : HSKIPRIGHT);
        if (_scroll1>_sb->Max) _scroll1 = _sb->Max;
        redraw = oldscroll!=_scroll1;
        cursorhandler->SetCursor (_feld, _kbd.i+oldscroll, _scroll2+_kbd.j, true);
        cursorhandler->CheckCursorPos();
    } else {
        if (_kbd.i<_width-1) cursorhandler->SetCursor (_feld, _scroll1+_width, _scroll2+_kbd.j, true);
        else {
            int oldscroll = _scroll1;
            _scroll1 += _sb->LargeChange / (_left ? HSKIPLEFT : HSKIPRIGHT);
            if (_scroll1>_sb->Max) _scroll1 = _sb->Max;
            redraw = oldscroll!=_scroll1;
        }
    }
    if (redraw) {
        if (_left) {
            InvalidateFeld (einzug.pos);
            InvalidateFeld (gewebe.pos);
            InvalidateFeld (blatteinzug.pos);
            InvalidateFeld (kettfarben.pos);
            InvalidateFeld (hlinehorz1);
        } else {
            InvalidateFeld (aufknuepfung.pos);
            InvalidateFeld (trittfolge.pos);
            InvalidateFeld (hlinehorz2);
        }
        _sb->Position = _scroll1;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::HandlePageUpKey (FELD _feld, INPUTPOS _kbd, int _height, bool _top, bool _ctrl, int _scroll1, int& _scroll2, TScrollBar* _sb)
{
    bool redraw = false;
    if (_ctrl) {
        int oldscroll = _scroll2;
        _scroll2 += _sb->LargeChange / (_top ? VSKIPTOP : VSKIPBOTTOM);
        if (_scroll2>_sb->Max) _scroll2 = _sb->Max;
        redraw = oldscroll!=_scroll2;
        cursorhandler->SetCursor (_feld, _scroll1+_kbd.i, _kbd.j+oldscroll, true);
        cursorhandler->CheckCursorPos();
    } else {
        if (_kbd.j<_height-1) cursorhandler->SetCursor (_feld, _scroll1+_kbd.i, _scroll2+_height, true);
        else {
            int oldscroll = _scroll2;
            _scroll2 += _sb->LargeChange / (_top ? VSKIPTOP : VSKIPBOTTOM);
            if (_scroll2>_sb->Max) _scroll2 = _sb->Max;
            redraw = oldscroll!=_scroll2;
        }
    }
    if (redraw) {
        if (_top) {
            InvalidateFeld (einzug.pos);
            InvalidateFeld (aufknuepfung.pos);
            InvalidateFeld (hlinevert1);
            _sb->Position = Data->MAXY1-einzug.pos.height/einzug.gh - _scroll2;
        } else {
            InvalidateFeld (gewebe.pos);
            InvalidateFeld (trittfolge.pos);
            InvalidateFeld (schussfarben.pos);
            InvalidateFeld (hlinevert2);
            _sb->Position = Data->MAXY2-gewebe.pos.height/gewebe.gh - _scroll2;
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::HandlePageDownKey (FELD _feld, INPUTPOS _kbd, bool _top, bool _ctrl, int _scroll1, int& _scroll2, TScrollBar* _sb)
{
    bool redraw = false;
    if (_ctrl) {
        int oldscroll = _scroll2;
        _scroll2 -= _sb->LargeChange / (_top ? VSKIPTOP : VSKIPBOTTOM);
        if (_scroll2<0) _scroll2 = 0;
        redraw = oldscroll!=_scroll2;
        cursorhandler->SetCursor (_feld, _scroll1+_kbd.i, _kbd.j+oldscroll, true);
        cursorhandler->CheckCursorPos();
    } else {
        if (_kbd.j>0) cursorhandler->SetCursor (_feld, _scroll1+_kbd.i, _scroll2, true);
        else {
            int oldscroll = _scroll2;
            _scroll2 -= _sb->LargeChange / (_top ? VSKIPTOP : VSKIPBOTTOM);
            if (_scroll2<0) _scroll2 = 0;
            redraw = oldscroll!=_scroll2;
        }
    }
    if (redraw) {
        if (_top) {
            InvalidateFeld (einzug.pos);
            InvalidateFeld (aufknuepfung.pos);
            InvalidateFeld (hlinevert1);
            _sb->Position = Data->MAXY1-einzug.pos.height/einzug.gh - _scroll2;
        } else {
            InvalidateFeld (gewebe.pos);
            InvalidateFeld (trittfolge.pos);
            InvalidateFeld (schussfarben.pos);
            InvalidateFeld (hlinevert2);
            _sb->Position = Data->MAXY2-gewebe.pos.height/gewebe.gh - _scroll2;
        }
    }
}
/*-----------------------------------------------------------------*/

