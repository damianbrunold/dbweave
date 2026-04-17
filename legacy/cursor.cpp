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
#include "cursor.h"
#include "cursorimpl.h"
#include "dbw3_form.h"
#include "datamodule.h"
#include "farbpalette_form.h"
#include "palette.h"
/*-----------------------------------------------------------------*/
static TColor inputposcol = clWhite;
static TColor inputposcol2 = clBlack;
static bool visible = false;
/*-----------------------------------------------------------------*/
#define GETPALCOL(col) \
    ((TColor)Data->palette->GetColor(col))
/*-----------------------------------------------------------------*/
#define MAXX (fb.pos.width/fb.gw)
#define MAXY (fb.pos.height/fb.gh)
/*-----------------------------------------------------------------*/
CrCursorHandler* __fastcall CrCursorHandler::CreateInstance (TDBWFRM* _frm, TData* _data)
{
    CrCursorHandler* p = 0;
    try {
        p = new CrCursorHandlerImpl (_frm, _data);
    } catch (...) {
    }
    return p;
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandler::Release (CrCursorHandler* _cursorhandler)
{
    delete _cursorhandler;
}
/*-----------------------------------------------------------------*/
__fastcall CrCursorHandlerImpl::CrCursorHandlerImpl (TDBWFRM* _frm, TData* _data)
: frm(_frm), data(_data)
{
    locked = false;
    try {
        CrFeld* first = new CrFeld (GEWEBE, frm, data, frm->gewebe, sharedcoord, CrShareX1|CrShareY2, 0);

        CrFeld* p = new CrFeld (EINZUG, frm, data, frm->einzug, sharedcoord, CrShareX1|CrShareY1, first);
        p->prev->next = p;

        p = new CrFeld (TRITTFOLGE, frm, data, frm->trittfolge, sharedcoord, CrShareX2|CrShareY2, p);
        p->prev->next = p;

        p = new CrFeld (AUFKNUEPFUNG, frm, data, frm->aufknuepfung, sharedcoord, CrShareX2|CrShareY1, p);
        p->prev->next = p;

        p = new CrFeld (SCHUSSFARBEN, frm, data, frm->schussfarben, sharedcoord, CrShareY2, p);
        p->prev->next = p;

        p = new CrFeld (KETTFARBEN, frm, data, frm->kettfarben, sharedcoord, CrShareX1, p);
        p->prev->next = p;

        p = new CrFeld (BLATTEINZUG, frm, data, frm->blatteinzug, sharedcoord, CrShareX1, p);
        p->prev->next = p;

        first->prev = p;
        p->next = first;

        feld = first;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
__fastcall CrCursorHandlerImpl::~CrCursorHandlerImpl()
{
    if (feld) {
        feld->prev->next = 0;
        while (feld) {
            CrFeld* next = feld->next;
            delete feld;
            feld = next;
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::Init()
{
    sharedcoord.Init();
}
/*-----------------------------------------------------------------*/
CrFeld* CrCursorHandlerImpl::GetFeld (FELD _feld)
{
    CrFeld* p = feld;
    int count = 0;
    while (p) {
        if (p->feld==_feld) {
            return p;
        }
        p = p->next;
        ++ count;
        if (count > 100) break; // Notbremse!
    }
    // Ungültiges Feld?!
    dbw3_assert(false);
    return NULL;
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::DisableCursor()
{
    dbw3_assert(frm);
    dbw3_assert(frm->CursorTimer);
    frm->CursorTimer->Enabled = false;
    DeleteCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::EnableCursor()
{
    dbw3_assert(frm);
    dbw3_assert(frm->CursorTimer);
    DrawCursor();
    frm->CursorTimer->Enabled = true;
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::ToggleField (TShiftState _shift)
{
    dbw3_assert (feld);
    if (feld) feld->Toggle (_shift);
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::SetField (bool _set, TShiftState _shift)
{
    dbw3_assert (feld);
    if (feld) feld->Set (_set, _shift);
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::SetCursor (FELD _feld, int _i, int _j, bool _clearselection/*=true*/)
{
    DisableCursor();
    CrFeld* p = GetFeld (_feld);
    if (p) {
        p->SetCursor (_i, _j);
        feld = p;
    }
    frm->kbd_field = feld->feld; //xxx Obsolet?!
    if (_clearselection) frm->ClearSelection();
    EnableCursor();
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::CheckCursorPos()
{
    dbw3_assert(feld);
    if (feld) feld->CheckCursorPos();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::MoveCursorLeft (int _step, bool _select)
{
    dbw3_assert(feld);
    if (feld) {
        if (frm->righttoleft && (feld->feld==GEWEBE || feld->feld==EINZUG || feld->feld==KETTFARBEN || feld->feld==BLATTEINZUG))
            feld->MoveCursorRight (_step, _select);
        else
            feld->MoveCursorLeft (_step, _select);
    }
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::MoveCursorRight (int _step, bool _select)
{
    dbw3_assert(feld);
    if (feld) {
        if (frm->righttoleft && (feld->feld==GEWEBE || feld->feld==EINZUG || feld->feld==KETTFARBEN || feld->feld==BLATTEINZUG))
            feld->MoveCursorLeft (_step, _select);
        else
            feld->MoveCursorRight (_step, _select);
    }
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::MoveCursorUp (int _step, bool _select)
{
    dbw3_assert(feld);
    if (feld) {
        if (frm->toptobottom && (feld->feld==EINZUG || feld->feld==AUFKNUEPFUNG))
            feld->MoveCursorDown (_step, _select);
        else
            feld->MoveCursorUp (_step, _select);
    }
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::MoveCursorDown (int _step, bool _select)
{
    dbw3_assert(feld);
    if (feld) {
        if (frm->toptobottom && (feld->feld==EINZUG || feld->feld==AUFKNUEPFUNG))
            feld->MoveCursorUp (_step, _select);
        else
            feld->MoveCursorDown (_step, _select);
    }
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::CheckLocked()
{
    if (locked && feld) {
        // Position modulo Rapport
        dbw3_assert(frm);
        DisableCursor();

        if (feld->feld==GEWEBE || feld->feld==EINZUG) {
            int rx = frm->rapport.kr.count();
            if (rx>0) {
                int x0 = frm->rapport.kr.a;
                int i = feld->fb.kbd.i+frm->scroll_x1;
                int ii = i-x0;
                while (ii<0) ii += rx;
                int newi = ii%rx + x0;
                if (newi<frm->scroll_x1) newi = frm->scroll_x1;
                feld->fb.kbd.i = newi-frm->scroll_x1;
            }
        }

        if (feld->feld==GEWEBE || feld->feld==TRITTFOLGE) {
            int ry = frm->rapport.sr.count();
            if (ry>0) {
                int y0 = frm->rapport.sr.a;
                int j = feld->fb.kbd.j+frm->scroll_y2;
                int jj = j-y0;
                while (jj<0) jj += ry;
                int newj = jj%ry + y0;
                if (newj<frm->scroll_y2) newj = frm->scroll_y2;
                feld->fb.kbd.j = newj-frm->scroll_y2;
            }
        }

        EnableCursor();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::GotoNextField()
{
    dbw3_assert(feld);
    dbw3_assert(frm);
    if (feld) {
        DisableCursor();
        // Funktioniert nur wenn mindestens ein
        // Feld sichtbar ist.
        do {
            feld = feld->next;
            if (frm->ViewSchlagpatrone->Checked && feld->feld==AUFKNUEPFUNG)
                feld = feld->next;
        } while (!feld->IsVisible());
        frm->kbd_field = feld->feld; //xxx Obsolet?!
        feld->SyncSharedCoord();
        EnableCursor();
        frm->ClearSelection();
        CheckLocked();
    }
    dbw3_assert(feld);
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::GotoPrevField()
{
    dbw3_assert(feld);
    dbw3_assert(frm);
    if (feld) {
        DisableCursor();
        // Funktioniert nur wenn mindestens ein
        // Feld sichtbar ist.
        do {
            feld = feld->prev;
            if (frm->ViewSchlagpatrone->Checked && feld->feld==AUFKNUEPFUNG)
                feld = feld->prev;
        } while (!feld->IsVisible());
        frm->kbd_field = feld->feld; //xxx Obsolet?!
        feld->SyncSharedCoord();
        EnableCursor();
        frm->ClearSelection();
        CheckLocked();
    }
    dbw3_assert(feld);
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::GotoField (FELD _feld)
{
    dbw3_assert(feld);
    dbw3_assert(frm);
    CrFeld* p = GetFeld (_feld);
    if (!p || !p->IsVisible()) return;
    DisableCursor();
    feld = p;
    frm->kbd_field = _feld;
    feld->SyncSharedCoord();
    EnableCursor();
    frm->ClearSelection();
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::SetInvisible (FELD _feld)
{
    dbw3_assert(feld);
    if (_feld==feld->feld) GotoNextField();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::DrawCursor()
{
    dbw3_assert(feld);
    if (feld) feld->DrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::DeleteCursor()
{
    dbw3_assert(feld);
    if (feld) feld->DeleteCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::ToggleCursor()
{
    dbw3_assert(feld);
    if (feld) feld->ToggleCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::SetCursorLocked (bool _locked/*=true*/)
{
    locked = _locked;
    if (locked) CheckLocked();
}
/*-----------------------------------------------------------------*/
void __fastcall CrCursorHandlerImpl::SetCursorDirection (CURSORDIRECTION _cd)
{
    CrFeld* current = feld;
    while (current!=NULL) {
        current->cursordirection = _cd;
        current = current->next;
        if (current==feld) break;
    }
}
/*-----------------------------------------------------------------*/
CURSORDIRECTION __fastcall CrCursorHandlerImpl::GetCursorDirection()
{
    if (feld!=NULL) return feld->cursordirection;
    else return CD_DEFAULT;
}
/*-----------------------------------------------------------------*/
__fastcall CrFeld::CrFeld (FELD _feld, TDBWFRM* _frm, TData* _data, FeldBase& _fb, CrSharedCoord& _sharedcoord, CrShareFlags _shareflags, CrFeld* _prev)
: frm(_frm), data(_data), prev(_prev), fb(_fb), sharedcoord(_sharedcoord), shareflags(_shareflags)
{
    feld = _feld;
    cursordirection = CD_DEFAULT;
    next = 0;
}
/*-----------------------------------------------------------------*/
__fastcall CrFeld::~CrFeld()
{
}
/*-----------------------------------------------------------------*/
bool __fastcall CrFeld::IsVisible() const
{
    return fb.pos.width!=0 && fb.pos.height!=0;
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::DisableCursor()
{
    dbw3_assert(frm);
    dbw3_assert(frm->CursorTimer);
    frm->CursorTimer->Enabled = false;
    DeleteCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::EnableCursor()
{
    dbw3_assert(frm);
    dbw3_assert(frm->CursorTimer);
    DrawCursor();
    frm->CursorTimer->Enabled = true;
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::Toggle (TShiftState _shift)
{
    //xxx Kann ich in FieldBase eine virtuelle
    //xxx SetField-Funktion einbauen, sodass ich
    //xxx hier diese aufrufen könnte, statt jeweils
    //xxx den switch zu machen?!
    switch (feld) {
        case BLATTEINZUG: {
            frm->SetBlatteinzug (fb.kbd.i);
            MoveCursorRight (1, false); //xxx siehe unten
            return;
        }
        case KETTFARBEN:
            // Mit Shift oder Control wird die Farbe selektiert!
            if (_shift.Contains (ssShift) || _shift.Contains (ssCtrl)) {
                Data->color = frm->kettfarben.feld.Get (fb.kbd.i+frm->scroll_x2);
                if (FarbPalette->Visible) FarbPalette->Invalidate();
                frm->Statusbar->Invalidate();
            } else {
                frm->SetKettfarben (fb.kbd.i);
                MoveCursorRight (1, false); //xxx siehe unten
            }
            return;
        case EINZUG: frm->SetEinzug (fb.kbd.i, fb.kbd.j); break;
        case GEWEBE:
            if (!frm->OptionsLockGewebe->Checked)
                frm->SetGewebe (fb.kbd.i, fb.kbd.j, false, 1);
            else
                MessageBeep (MB_OK);
            break;
        case AUFKNUEPFUNG: frm->SetAufknuepfung (fb.kbd.i, fb.kbd.j, false, 1); break;
        case TRITTFOLGE: frm->SetTrittfolge (fb.kbd.i, fb.kbd.j, false, 1); break;
        case SCHUSSFARBEN:
            // Mit Shift oder Control wird die Farbe selektiert!
            if (_shift.Contains (ssShift) || _shift.Contains (ssCtrl)) {
                Data->color = frm->schussfarben.feld.Get (frm->scroll_y2+fb.kbd.j);
                if (FarbPalette->Visible) FarbPalette->Invalidate();
                frm->Statusbar->Invalidate();
            } else {
                frm->SetSchussfarben (fb.kbd.j);
            }
            break;
    }

    if (frm->toptobottom && (feld==EINZUG || feld==AUFKNUEPFUNG)) {
        if (cursordirection&CD_UP) MoveCursorUp (1, false);
        else if (cursordirection&CD_DOWN) MoveCursorDown (1, false);
    } else {
        if (cursordirection&CD_UP) MoveCursorUp (1, false);
        else if (cursordirection&CD_DOWN) MoveCursorDown (1, false);
    }
    if (frm->righttoleft && (feld==EINZUG || feld==GEWEBE || feld==BLATTEINZUG || feld==KETTFARBEN)) {
        if (cursordirection&CD_LEFT) MoveCursorLeft (1, false);
        else if (cursordirection&CD_RIGHT) MoveCursorRight (1, false);
    } else {
        if (cursordirection&CD_LEFT) MoveCursorLeft (1, false);
        else if (cursordirection&CD_RIGHT) MoveCursorRight (1, false);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::Set (bool _set, TShiftState _shift)
{
    switch (feld) {
        case GEWEBE:
            if (!frm->OptionsLockGewebe->Checked)
                frm->SetGewebe (fb.kbd.i, fb.kbd.j, true, frm->currentrange);
            else
                MessageBeep (MB_OK);
            break;

        case AUFKNUEPFUNG:
            frm->SetAufknuepfung (fb.kbd.i, fb.kbd.j, true, frm->currentrange);
            break;

        case TRITTFOLGE:
            frm->SetTrittfolge (fb.kbd.i, fb.kbd.j, true, frm->currentrange);
            break;
    }

    if (frm->toptobottom && feld==AUFKNUEPFUNG) {
        if (cursordirection&CD_UP) MoveCursorUp (1, false);
        else if (cursordirection&CD_DOWN) MoveCursorDown (1, false);
    } else {
        if (cursordirection&CD_UP) MoveCursorUp (1, false);
        else if (cursordirection&CD_DOWN) MoveCursorDown (1, false);
    }
    if (frm->righttoleft && feld==GEWEBE) {
        if (cursordirection&CD_LEFT) MoveCursorRight (1, false);
        else if (cursordirection&CD_RIGHT) MoveCursorLeft (1, false);
    } else {
        if (cursordirection&CD_LEFT) MoveCursorLeft (1, false);
        else if (cursordirection&CD_RIGHT) MoveCursorRight (1, false);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::SetCursor (int _i, int _j)
{
    if (_i>=fb.ScrollX()+fb.pos.width/fb.gw) _i = fb.ScrollX()+fb.pos.width/fb.gw-1;
    if (_j>=fb.ScrollY()+fb.pos.height/fb.gh) _j = fb.ScrollY()+fb.pos.height/fb.gh-1;

    fb.kbd.i = _i - fb.ScrollX();
    fb.kbd.j = _j - fb.ScrollY();

    if (fb.kbd.i<0) fb.kbd.i = 0;
    if (fb.kbd.j<0) fb.kbd.j = 0;

    UpdateSharedCoord (fb.kbd.i, fb.kbd.j);
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::CheckCursorPos()
{
    int i = fb.kbd.i;
    int j = fb.kbd.j;

    if (i>=fb.pos.width/fb.gw) i = fb.pos.width/fb.gw-1;
    if (j>=fb.pos.height/fb.gh) j = fb.pos.height/fb.gh-1;

    if (i<0) i=0;
    if (j<0) j=0;

    fb.kbd.i = i;
    fb.kbd.j = j;

    UpdateSharedCoord (fb.kbd.i, fb.kbd.j);
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::MoveCursorLeft (int _step, bool _select)
{
    DisableCursor();

    if (_select) frm->ResizeSelection (fb.kbd.i+fb.ScrollX(), fb.kbd.j+fb.ScrollY(), feld, false);

    fb.kbd.i -= _step;
    if (fb.kbd.i<0) fb.kbd.i = 0;

    UpdateSharedCoord (fb.kbd.i, fb.kbd.j);

    EnableCursor();
    if (_select) frm->ResizeSelection (fb.kbd.i+fb.ScrollX(), fb.kbd.j+fb.ScrollY(), feld, false);
    else frm->ClearSelection();
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::MoveCursorRight (int _step, bool _select)
{
    DisableCursor();

    if (_select) frm->ResizeSelection (fb.kbd.i+fb.ScrollX(), fb.kbd.j+fb.ScrollY(), feld, false);

    fb.kbd.i += _step;
    if (fb.kbd.i>=MAXX) fb.kbd.i = MAXX-1;

    UpdateSharedCoord (fb.kbd.i, fb.kbd.j);

    EnableCursor();
    if (_select) frm->ResizeSelection (fb.kbd.i+fb.ScrollX(), fb.kbd.j+fb.ScrollY(), feld, false);
    else frm->ClearSelection();
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::MoveCursorUp (int _step, bool _select)
{
    DisableCursor();

    if (_select) frm->ResizeSelection (fb.kbd.i+fb.ScrollX(), fb.kbd.j+fb.ScrollY(), feld, false);

    fb.kbd.j += _step;
    if (fb.kbd.j>=MAXY) fb.kbd.j = MAXY-1;

    UpdateSharedCoord (fb.kbd.i, fb.kbd.j);

    EnableCursor();
    if (_select) frm->ResizeSelection (fb.kbd.i+fb.ScrollX(), fb.kbd.j+fb.ScrollY(), feld, false);
    else frm->ClearSelection();
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::MoveCursorDown (int _step, bool _select)
{
    DisableCursor();

    if (_select) frm->ResizeSelection (fb.kbd.i+fb.ScrollX(), fb.kbd.j+fb.ScrollY(), feld, false);

    fb.kbd.j -= _step;
    if (fb.kbd.j<0) fb.kbd.j = 0;

    UpdateSharedCoord (fb.kbd.i, fb.kbd.j);

    EnableCursor();
    if (_select) frm->ResizeSelection (fb.kbd.i+fb.ScrollX(), fb.kbd.j+fb.ScrollY(), feld, false);
    else frm->ClearSelection();
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::DrawCursor()
{
    if (visible) return;

    int i = fb.kbd.i;
    if (frm->righttoleft && (feld==EINZUG || feld==GEWEBE || feld==BLATTEINZUG || feld==KETTFARBEN))
        i = fb.pos.width/fb.gw - fb.kbd.i - 1;

    int j = fb.kbd.j;
    if (frm->toptobottom && (feld==EINZUG || feld==AUFKNUEPFUNG))
        j = fb.pos.height/fb.gh - fb.kbd.j - 1;

    int x0 = fb.pos.x0 + i*fb.gw;
    int y0 = fb.pos.y0 + fb.pos.height - (j+1)*fb.gh;
    int x1 = x0 + fb.gw;
    int y1 = y0 + fb.gh;

    // Extrawurst für Blatteinzug, weil der kein
    // Vielfaches der Gridheight hoch ist...
    if (feld==BLATTEINZUG) {
        y0 = fb.pos.y0;
        y1 = y0 + fb.pos.height;
    }

    frm->Canvas->Pen->Color = inputposcol;

    // Falls Gewebe und Farbeffekt und aktuelles Feld
    // hat weisse Farbe, dann muss Cursor rot
    // gemalt werden!
    if (feld==GEWEBE && frm->GewebeFarbeffekt->Checked) {
        int ii = frm->scroll_x1+fb.kbd.i;
        int jj = frm->scroll_y2+fb.kbd.j;
        if (ii>=frm->kette.a && ii<=frm->kette.b && jj>=frm->schuesse.a && jj<=frm->schuesse.b) {
            if (frm->gewebe.feld.Get(ii, jj)>0) {
                DWORD color = ColorToRGB(GETPALCOL(frm->kettfarben.feld.Get (ii)));
                WORD value = WORD((GetRValue(color)+GetGValue(color)+GetBValue(color))/3);
                if (value>=128)
                    frm->Canvas->Pen->Color = inputposcol2;
            } else {
                DWORD color = ColorToRGB(GETPALCOL(frm->schussfarben.feld.Get (jj)));
                WORD value = WORD((GetRValue(color)+GetGValue(color)+GetBValue(color))/3);
                if (value>=128)
                    frm->Canvas->Pen->Color = inputposcol2;
            }
        }
    }

    frm->Canvas->MoveTo (x0, y0);
    frm->Canvas->LineTo (x1, y0);
    frm->Canvas->LineTo (x1, y1);
    frm->Canvas->LineTo (x0, y1);
    frm->Canvas->LineTo (x0, y0);

    visible = true;
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::DeleteCursor()
{
    if (!visible) return;

    int icor = fb.kbd.i;
    if (frm->righttoleft && (feld==EINZUG || feld==GEWEBE || feld==BLATTEINZUG || feld==KETTFARBEN))
        icor = fb.pos.width/fb.gw - fb.kbd.i - 1;

    int jcor = fb.kbd.j;
    if (frm->toptobottom && (feld==EINZUG || feld==AUFKNUEPFUNG))
        jcor = fb.pos.height/fb.gh - fb.kbd.j - 1;

    int x0 = fb.pos.x0 + icor*fb.gw;
    int y0 = fb.pos.y0 + fb.pos.height - (jcor+1)*fb.gh;
    int x1 = x0 + fb.gw;
    int y1 = y0 + fb.gh;

    // Extrawurst für Blatteinzug, weil der kein
    // Vielfaches der Gridheight hoch ist...
    if (feld==BLATTEINZUG) {
        y0 = fb.pos.y0;
        y1 = y0 + fb.pos.height;
    }

    int i = fb.ScrollX() + fb.kbd.i;
    int j = fb.ScrollY() + fb.kbd.j;

    frm->Canvas->Pen->Color = clBtnShadow;
    frm->Canvas->MoveTo (x0, y0);
    frm->Canvas->LineTo (x1, y0);
    frm->Canvas->LineTo (x1, y1);
    frm->Canvas->LineTo (x0, y1);
    frm->Canvas->LineTo (x0, y0);

    // Strongline aktualisieren
    if (feld!=BLATTEINZUG && feld!=KETTFARBEN && feld!=SCHUSSFARBEN) {
        int sx = fb.pos.strongline_x;
        int sy = fb.pos.strongline_y;

        frm->Canvas->Pen->Color = frm->strongclr;
        if (sx!=0) {
            if ((i % sx)==0 && fb.kbd.i>0) {
                if (frm->righttoleft && (feld==EINZUG || feld==GEWEBE)) {
                    frm->Canvas->MoveTo (x1, y0);
                    frm->Canvas->LineTo (x1, y1+1);
                } else {
                    frm->Canvas->MoveTo (x0, y0);
                    frm->Canvas->LineTo (x0, y1+1);
                }
            }
            if (((i+1) % sx)==0 && fb.kbd.i+1<fb.pos.width/fb.gw) {
                if (frm->righttoleft && (feld==EINZUG || feld==GEWEBE)) {
                    frm->Canvas->MoveTo (x0, y0);
                    frm->Canvas->LineTo (x0, y1+1);
                } else {
                    frm->Canvas->MoveTo (x1, y0);
                    frm->Canvas->LineTo (x1, y1+1);
                }
            }
        }
        if (sy!=0) {
            if ((j % sy)==0 && fb.kbd.j>0) {
                if (frm->toptobottom && (feld==EINZUG || feld==AUFKNUEPFUNG)) {
                    frm->Canvas->MoveTo (x0, y0);
                    frm->Canvas->LineTo (x1+1, y0);
                } else {
                    frm->Canvas->MoveTo (x0, y1);
                    frm->Canvas->LineTo (x1+1, y1);
                }
            }
            if (((j+1) % sy)==0) {
                if (frm->toptobottom && (feld==EINZUG || feld==AUFKNUEPFUNG)) {
                    frm->Canvas->MoveTo (x0, y1);
                    frm->Canvas->LineTo (x1+1, y1);
                } else {
                    frm->Canvas->MoveTo (x0, y0);
                    frm->Canvas->LineTo (x1+1, y0);
                }
            }
        }
    }

    // Hilfslinien aktualisieren
    if (frm->ViewHlines->Checked) {
        frm->DrawHilfslinien();
    }

    // Rapportlinien aktualisieren
    if (frm->RappViewRapport->Checked && (feld==EINZUG || feld==TRITTFOLGE)) {
        frm->DrawRapport();
    }

    if (feld==GEWEBE && !frm->GewebeNormal->Checked && !frm->GewebeNone->Checked) {
        frm->DrawGewebeRahmen (fb.kbd.i, fb.kbd.j);
        frm->DrawGewebe (fb.kbd.i, fb.kbd.j);
        bool maxx = fb.kbd.i >= fb.pos.width/fb.gw-1;
        bool maxy = fb.kbd.j == 0;
        if (frm->righttoleft) {
            if (!maxx && fb.kbd.i>0) frm->DrawGewebe (fb.kbd.i-1, fb.kbd.j);
            if (!maxy && fb.kbd.j>0) frm->DrawGewebe (fb.kbd.i, fb.kbd.j-1);
            if (!maxx && !maxy && fb.kbd.i>0 && fb.kbd.j>0) frm->DrawGewebe (fb.kbd.i-1, fb.kbd.j-1);
        } else {
            if (!maxx) frm->DrawGewebe (fb.kbd.i+1, fb.kbd.j);
            if (!maxy && fb.kbd.j>0) frm->DrawGewebe (fb.kbd.i, fb.kbd.j-1);
            if (!maxx && !maxy && fb.kbd.j>0) frm->DrawGewebe (fb.kbd.i+1, fb.kbd.j-1);
        }
    }

    visible = false;
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::ToggleCursor()
{
    if (visible) DeleteCursor();
    else DrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::UpdateSharedCoord (int _i, int _j)
{
    if (shareflags&CrShareX1) sharedcoord.x1 = _i;
    if (shareflags&CrShareX2) sharedcoord.x2 = _i;
    if (shareflags&CrShareY1) sharedcoord.y1 = _j;
    if (shareflags&CrShareY2) sharedcoord.y2 = _j;
}
/*-----------------------------------------------------------------*/
void __fastcall CrFeld::SyncSharedCoord()
{
    if (shareflags&CrShareX1) fb.kbd.i = sharedcoord.x1;
    if (shareflags&CrShareX2) fb.kbd.i = sharedcoord.x2;
    if (shareflags&CrShareY1) fb.kbd.j = sharedcoord.y1;
    if (shareflags&CrShareY2) fb.kbd.j = sharedcoord.y2;
}
/*-----------------------------------------------------------------*/

