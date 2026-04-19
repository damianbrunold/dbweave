/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope for this slice:
      * full state management -- position tracking, left/right/up/down
        movement, shared-coordinate propagation across fields, field-list
        traversal, locked-mode rapport snap, visibility checks;
      * STUB rendering (DrawCursor / DeleteCursor / ToggleCursor /
        DisableCursor / EnableCursor) -- the real bodies need a blinking
        QTimer, a paint target and access to strongline / hilfslinien /
        rapport-line drawing that has not landed yet;
      * STUB editor-op dispatch (Toggle / Set) -- these call into
        SetGewebe / SetEinzug / SetAufknuepfung etc. that have not
        landed yet. Guards prevent dereferencing null helpers so the
        test harness can exercise movement without firing dispatch.
*/

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "cursor.h"
#include "cursorimpl.h"
#include "mainwindow.h"
#include "datamodule.h"
#include <QPainter>
#include <QPen>
/*-----------------------------------------------------------------*/
#define MAXX (fb.pos.width / fb.gw)
#define MAXY (fb.pos.height / fb.gh)
/*-----------------------------------------------------------------*/
CrCursorHandler* CrCursorHandler::CreateInstance(TDBWFRM* _frm, TData* _data)
{
    CrCursorHandler* p = 0;
    try {
        p = new CrCursorHandlerImpl(_frm, _data);
    } catch (...) {
    }
    return p;
}
/*-----------------------------------------------------------------*/
void CrCursorHandler::Release(CrCursorHandler* _cursorhandler)
{
    delete _cursorhandler;
}
/*-----------------------------------------------------------------*/
CrCursorHandlerImpl::CrCursorHandlerImpl(TDBWFRM* _frm, TData* _data)
    : frm(_frm)
    , data(_data)
{
    feld = nullptr;
    locked = false;
    try {
        CrFeld* first
            = new CrFeld(GEWEBE, frm, data, frm->gewebe, sharedcoord, CrShareX1 | CrShareY2, 0);

        CrFeld* p
            = new CrFeld(EINZUG, frm, data, frm->einzug, sharedcoord, CrShareX1 | CrShareY1, first);
        p->prev->next = p;

        p = new CrFeld(TRITTFOLGE, frm, data, frm->trittfolge, sharedcoord, CrShareX2 | CrShareY2,
                       p);
        p->prev->next = p;

        p = new CrFeld(AUFKNUEPFUNG, frm, data, frm->aufknuepfung, sharedcoord,
                       CrShareX2 | CrShareY1, p);
        p->prev->next = p;

        p = new CrFeld(SCHUSSFARBEN, frm, data, frm->schussfarben, sharedcoord, CrShareY2, p);
        p->prev->next = p;

        p = new CrFeld(KETTFARBEN, frm, data, frm->kettfarben, sharedcoord, CrShareX1, p);
        p->prev->next = p;

        p = new CrFeld(BLATTEINZUG, frm, data, frm->blatteinzug, sharedcoord, CrShareX1, p);
        p->prev->next = p;

        first->prev = p;
        p->next = first;

        feld = first;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
CrCursorHandlerImpl::~CrCursorHandlerImpl()
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
void CrCursorHandlerImpl::Init()
{
    sharedcoord.Init();
}
/*-----------------------------------------------------------------*/
CrFeld* CrCursorHandlerImpl::GetFeld(FELD _feld)
{
    CrFeld* p = feld;
    int count = 0;
    while (p) {
        if (p->feld == _feld) {
            return p;
        }
        p = p->next;
        ++count;
        if (count > 100)
            break; // Notbremse!
    }
    // Ungueltiges Feld?!
    dbw3_assert(false);
    return NULL;
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::DisableCursor()
{
    /*  Real implementation disables the blink QTimer and erases the
        cursor rectangle. Neither exists yet. */
    DeleteCursor();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::EnableCursor()
{
    /*  Real implementation draws the cursor rectangle and enables the
        blink QTimer. Neither exists yet. */
    DrawCursor();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::ToggleField(TShiftState _shift)
{
    dbw3_assert(feld);
    if (feld)
        feld->Toggle(_shift);
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::SetField(bool _set, TShiftState _shift)
{
    dbw3_assert(feld);
    if (feld)
        feld->Set(_set, _shift);
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::SetCursor(FELD _feld, int _i, int _j, bool _clearselection /*=true*/)
{
    DisableCursor();
    CrFeld* p = GetFeld(_feld);
    if (p) {
        p->SetCursor(_i, _j);
        feld = p;
    }
    if (feld)
        frm->kbd_field = feld->feld;
    if (_clearselection)
        frm->ClearSelection();
    EnableCursor();
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::CheckCursorPos()
{
    dbw3_assert(feld);
    if (feld)
        feld->CheckCursorPos();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::MoveCursorLeft(int _step, bool _select)
{
    dbw3_assert(feld);
    if (feld) {
        if (frm->righttoleft
            && (feld->feld == GEWEBE || feld->feld == EINZUG || feld->feld == KETTFARBEN
                || feld->feld == BLATTEINZUG))
            feld->MoveCursorRight(_step, _select);
        else
            feld->MoveCursorLeft(_step, _select);
    }
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::MoveCursorRight(int _step, bool _select)
{
    dbw3_assert(feld);
    if (feld) {
        if (frm->righttoleft
            && (feld->feld == GEWEBE || feld->feld == EINZUG || feld->feld == KETTFARBEN
                || feld->feld == BLATTEINZUG))
            feld->MoveCursorLeft(_step, _select);
        else
            feld->MoveCursorRight(_step, _select);
    }
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::MoveCursorUp(int _step, bool _select)
{
    dbw3_assert(feld);
    if (feld) {
        if (frm->toptobottom && (feld->feld == EINZUG || feld->feld == AUFKNUEPFUNG))
            feld->MoveCursorDown(_step, _select);
        else
            feld->MoveCursorUp(_step, _select);
    }
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::MoveCursorDown(int _step, bool _select)
{
    dbw3_assert(feld);
    if (feld) {
        if (frm->toptobottom && (feld->feld == EINZUG || feld->feld == AUFKNUEPFUNG))
            feld->MoveCursorUp(_step, _select);
        else
            feld->MoveCursorDown(_step, _select);
    }
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::CheckLocked()
{
    if (locked && feld) {
        // Position modulo Rapport
        dbw3_assert(frm);
        DisableCursor();

        if (feld->feld == GEWEBE || feld->feld == EINZUG) {
            int rx = frm->rapport.kr.count();
            if (rx > 0) {
                int x0 = frm->rapport.kr.a;
                int i = feld->fb.kbd.i + frm->scroll_x1;
                int ii = i - x0;
                while (ii < 0)
                    ii += rx;
                int newi = ii % rx + x0;
                if (newi < frm->scroll_x1)
                    newi = frm->scroll_x1;
                feld->fb.kbd.i = newi - frm->scroll_x1;
            }
        }

        if (feld->feld == GEWEBE || feld->feld == TRITTFOLGE) {
            int ry = frm->rapport.sr.count();
            if (ry > 0) {
                int y0 = frm->rapport.sr.a;
                int j = feld->fb.kbd.j + frm->scroll_y2;
                int jj = j - y0;
                while (jj < 0)
                    jj += ry;
                int newj = jj % ry + y0;
                if (newj < frm->scroll_y2)
                    newj = frm->scroll_y2;
                feld->fb.kbd.j = newj - frm->scroll_y2;
            }
        }

        EnableCursor();
    }
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::GotoNextField()
{
    dbw3_assert(feld);
    dbw3_assert(frm);
    if (feld) {
        DisableCursor();
        // Funktioniert nur wenn mindestens ein Feld sichtbar ist.
        int safety = 0;
        do {
            feld = feld->next;
            if (frm->ViewSchlagpatrone && frm->ViewSchlagpatrone->isChecked()
                && feld->feld == AUFKNUEPFUNG)
                feld = feld->next;
            if (++safety > 20)
                break; // avoid infinite loop if no field visible
        } while (!feld->IsVisible());
        frm->kbd_field = feld->feld;
        feld->SyncSharedCoord();
        EnableCursor();
        frm->ClearSelection();
        CheckLocked();
    }
    dbw3_assert(feld);
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::GotoPrevField()
{
    dbw3_assert(feld);
    dbw3_assert(frm);
    if (feld) {
        DisableCursor();
        int safety = 0;
        do {
            feld = feld->prev;
            if (frm->ViewSchlagpatrone && frm->ViewSchlagpatrone->isChecked()
                && feld->feld == AUFKNUEPFUNG)
                feld = feld->prev;
            if (++safety > 20)
                break;
        } while (!feld->IsVisible());
        frm->kbd_field = feld->feld;
        feld->SyncSharedCoord();
        EnableCursor();
        frm->ClearSelection();
        CheckLocked();
    }
    dbw3_assert(feld);
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::GotoField(FELD _feld)
{
    dbw3_assert(feld);
    dbw3_assert(frm);
    CrFeld* p = GetFeld(_feld);
    if (!p || !p->IsVisible())
        return;
    DisableCursor();
    feld = p;
    frm->kbd_field = _feld;
    feld->SyncSharedCoord();
    EnableCursor();
    frm->ClearSelection();
    CheckLocked();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::SetInvisible(FELD _feld)
{
    dbw3_assert(feld);
    if (_feld == feld->feld)
        GotoNextField();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::DrawCursor()
{
    if (feld)
        feld->DrawCursor();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::DeleteCursor()
{
    if (feld)
        feld->DeleteCursor();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::ToggleCursor()
{
    if (feld)
        feld->ToggleCursor();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::SetCursorLocked(bool _locked /*=true*/)
{
    locked = _locked;
    if (locked)
        CheckLocked();
}
/*-----------------------------------------------------------------*/
void CrCursorHandlerImpl::SetCursorDirection(CURSORDIRECTION _cd)
{
    CrFeld* current = feld;
    while (current != NULL) {
        current->cursordirection = _cd;
        current = current->next;
        if (current == feld)
            break;
    }
}
/*-----------------------------------------------------------------*/
CURSORDIRECTION CrCursorHandlerImpl::GetCursorDirection()
{
    if (feld != NULL)
        return feld->cursordirection;
    else
        return CD_DEFAULT;
}
/*-----------------------------------------------------------------*/
CrFeld::CrFeld(FELD _feld, TDBWFRM* _frm, TData* _data, FeldBase& _fb, CrSharedCoord& _sharedcoord,
               CrShareFlags _shareflags, CrFeld* _prev)
    : frm(_frm)
    , data(_data)
    , fb(_fb)
    , sharedcoord(_sharedcoord)
    , shareflags(_shareflags)
    , prev(_prev)
{
    feld = _feld;
    cursordirection = CD_DEFAULT;
    next = 0;
}
/*-----------------------------------------------------------------*/
CrFeld::~CrFeld() { }
/*-----------------------------------------------------------------*/
bool CrFeld::IsVisible() const
{
    return fb.pos.width != 0 && fb.pos.height != 0;
}
/*-----------------------------------------------------------------*/
void CrFeld::DisableCursor()
{
    /*  STUB: legacy cancels the blink timer here. */
    DeleteCursor();
}
/*-----------------------------------------------------------------*/
void CrFeld::EnableCursor()
{
    /*  STUB: legacy restarts the blink timer here. */
    DrawCursor();
}
/*-----------------------------------------------------------------*/
void CrFeld::Toggle(TShiftState _shift)
{
    /*  Verbatim port of legacy cursor.cpp:CrFeld::Toggle. Space
        routes here via CrCursorHandler::ToggleField. Skipped:
          * OptionsLockGewebe (not ported yet) -- gewebe is always
            editable in the port.
          * FarbPalette / Statusbar invalidation after colour-pick
            -- refresh() in the caller redraws the whole canvas. */
    switch (feld) {
    case BLATTEINZUG:
        frm->SetBlatteinzug(fb.kbd.i);
        MoveCursorRight(1, false);
        return;
    case KETTFARBEN:
        if (_shift.Contains(ssShift) || _shift.Contains(ssCtrl)) {
            Data->color = (unsigned char)frm->kettfarben.feld.Get(fb.kbd.i + frm->scroll_x1);
        } else {
            frm->SetKettfarben(fb.kbd.i);
            MoveCursorRight(1, false);
        }
        return;
    case EINZUG:
        frm->SetEinzug(fb.kbd.i, fb.kbd.j);
        break;
    case GEWEBE:
        frm->SetGewebe(fb.kbd.i, fb.kbd.j, false, 1);
        break;
    case AUFKNUEPFUNG:
        frm->SetAufknuepfung(fb.kbd.i, fb.kbd.j, false, 1);
        break;
    case TRITTFOLGE:
        frm->SetTrittfolge(fb.kbd.i, fb.kbd.j, false, 1);
        break;
    case SCHUSSFARBEN:
        if (_shift.Contains(ssShift) || _shift.Contains(ssCtrl)) {
            Data->color = (unsigned char)frm->schussfarben.feld.Get(frm->scroll_y2 + fb.kbd.j);
        } else {
            frm->SetSchussfarben(fb.kbd.j);
        }
        break;
    default:
        break;
    }

    /*  Cursor auto-advance after Toggle. Both branches are
        identical in the legacy source; kept as-is. */
    if (frm->toptobottom && (feld == EINZUG || feld == AUFKNUEPFUNG)) {
        if (cursordirection & CD_UP)
            MoveCursorUp(1, false);
        else if (cursordirection & CD_DOWN)
            MoveCursorDown(1, false);
    } else {
        if (cursordirection & CD_UP)
            MoveCursorUp(1, false);
        else if (cursordirection & CD_DOWN)
            MoveCursorDown(1, false);
    }
    if (frm->righttoleft
        && (feld == EINZUG || feld == GEWEBE || feld == BLATTEINZUG || feld == KETTFARBEN)) {
        if (cursordirection & CD_LEFT)
            MoveCursorRight(1, false);
        else if (cursordirection & CD_RIGHT)
            MoveCursorLeft(1, false);
    } else {
        if (cursordirection & CD_LEFT)
            MoveCursorLeft(1, false);
        else if (cursordirection & CD_RIGHT)
            MoveCursorRight(1, false);
    }
}
/*-----------------------------------------------------------------*/
void CrFeld::Set(bool /*_set*/, TShiftState /*_shift*/)
{
    /*  STUB: same dispatch pattern as Toggle. */
}
/*-----------------------------------------------------------------*/
void CrFeld::SetCursor(int _i, int _j)
{
    if (fb.gw > 0 && _i >= fb.ScrollX() + fb.pos.width / fb.gw)
        _i = fb.ScrollX() + fb.pos.width / fb.gw - 1;
    if (fb.gh > 0 && _j >= fb.ScrollY() + fb.pos.height / fb.gh)
        _j = fb.ScrollY() + fb.pos.height / fb.gh - 1;

    fb.kbd.i = _i - fb.ScrollX();
    fb.kbd.j = _j - fb.ScrollY();

    if (fb.kbd.i < 0)
        fb.kbd.i = 0;
    if (fb.kbd.j < 0)
        fb.kbd.j = 0;

    UpdateSharedCoord(fb.kbd.i, fb.kbd.j);
}
/*-----------------------------------------------------------------*/
void CrFeld::CheckCursorPos()
{
    int i = fb.kbd.i;
    int j = fb.kbd.j;

    if (fb.gw > 0 && i >= fb.pos.width / fb.gw)
        i = fb.pos.width / fb.gw - 1;
    if (fb.gh > 0 && j >= fb.pos.height / fb.gh)
        j = fb.pos.height / fb.gh - 1;

    if (i < 0)
        i = 0;
    if (j < 0)
        j = 0;

    fb.kbd.i = i;
    fb.kbd.j = j;

    UpdateSharedCoord(fb.kbd.i, fb.kbd.j);
}
/*-----------------------------------------------------------------*/
void CrFeld::MoveCursorLeft(int _step, bool _select)
{
    DisableCursor();

    if (_select)
        frm->ResizeSelection(fb.kbd.i + fb.ScrollX(), fb.kbd.j + fb.ScrollY(), feld, false);

    fb.kbd.i -= _step;
    if (fb.kbd.i < 0)
        fb.kbd.i = 0;

    UpdateSharedCoord(fb.kbd.i, fb.kbd.j);

    EnableCursor();
    if (_select)
        frm->ResizeSelection(fb.kbd.i + fb.ScrollX(), fb.kbd.j + fb.ScrollY(), feld, false);
    else
        frm->ClearSelection();
}
/*-----------------------------------------------------------------*/
void CrFeld::MoveCursorRight(int _step, bool _select)
{
    DisableCursor();

    if (_select)
        frm->ResizeSelection(fb.kbd.i + fb.ScrollX(), fb.kbd.j + fb.ScrollY(), feld, false);

    fb.kbd.i += _step;
    if (fb.gw > 0 && MAXX > 0 && fb.kbd.i >= MAXX)
        fb.kbd.i = MAXX - 1;

    UpdateSharedCoord(fb.kbd.i, fb.kbd.j);

    EnableCursor();
    if (_select)
        frm->ResizeSelection(fb.kbd.i + fb.ScrollX(), fb.kbd.j + fb.ScrollY(), feld, false);
    else
        frm->ClearSelection();
}
/*-----------------------------------------------------------------*/
void CrFeld::MoveCursorUp(int _step, bool _select)
{
    DisableCursor();

    if (_select)
        frm->ResizeSelection(fb.kbd.i + fb.ScrollX(), fb.kbd.j + fb.ScrollY(), feld, false);

    fb.kbd.j += _step;
    if (fb.gh > 0 && MAXY > 0 && fb.kbd.j >= MAXY)
        fb.kbd.j = MAXY - 1;

    UpdateSharedCoord(fb.kbd.i, fb.kbd.j);

    EnableCursor();
    if (_select)
        frm->ResizeSelection(fb.kbd.i + fb.ScrollX(), fb.kbd.j + fb.ScrollY(), feld, false);
    else
        frm->ClearSelection();
}
/*-----------------------------------------------------------------*/
void CrFeld::MoveCursorDown(int _step, bool _select)
{
    DisableCursor();

    if (_select)
        frm->ResizeSelection(fb.kbd.i + fb.ScrollX(), fb.kbd.j + fb.ScrollY(), feld, false);

    fb.kbd.j -= _step;
    if (fb.kbd.j < 0)
        fb.kbd.j = 0;

    UpdateSharedCoord(fb.kbd.i, fb.kbd.j);

    EnableCursor();
    if (_select)
        frm->ResizeSelection(fb.kbd.i + fb.ScrollX(), fb.kbd.j + fb.ScrollY(), feld, false);
    else
        frm->ClearSelection();
}
/*-----------------------------------------------------------------*/
void CrFeld::DrawCursor()
{
    QPainter* p = frm->currentPainter;
    if (!p)
        return;
    if (fb.gw <= 0 || fb.gh <= 0)
        return;
    if (fb.pos.width <= 0 || fb.pos.height <= 0)
        return;

    /*  Compute the cell rectangle, honouring orientation flips for
        the fields where they apply. BLATTEINZUG is the exception --
        the whole strip height lights up regardless of kbd.j. */
    int i = fb.kbd.i;
    if (frm->righttoleft
        && (feld == EINZUG || feld == GEWEBE || feld == BLATTEINZUG || feld == KETTFARBEN)) {
        i = fb.pos.width / fb.gw - fb.kbd.i - 1;
    }
    int j = fb.kbd.j;
    if (frm->toptobottom && (feld == EINZUG || feld == AUFKNUEPFUNG)) {
        j = fb.pos.height / fb.gh - fb.kbd.j - 1;
    }

    const int x0 = fb.pos.x0 + i * fb.gw;
    int y0 = fb.pos.y0 + fb.pos.height - (j + 1) * fb.gh;
    const int x1 = x0 + fb.gw;
    int y1 = y0 + fb.gh;
    if (feld == BLATTEINZUG) {
        y0 = fb.pos.y0;
        y1 = y0 + fb.pos.height;
    }

    /*  Legacy default cursor colour is clWhite. A farbeffekt-bright-
        gewebe override that flips to black lives in the legacy; it
        is deferred here -- simple white outline is adequate for the
        basic editing UX and platform-independent in tests.        */
    p->setPen(QPen(QColor(Qt::white)));
    p->drawLine(x0, y0, x1, y0);
    p->drawLine(x1, y0, x1, y1);
    p->drawLine(x1, y1, x0, y1);
    p->drawLine(x0, y1, x0, y0);
}
/*-----------------------------------------------------------------*/
void CrFeld::DeleteCursor()
{
    /*  Qt no-op. Legacy's VCL retained-mode canvas needed to erase
        the cursor rectangle plus restore strongline / hilfslinien /
        rapport-line pixels underneath. Under Qt the widget repaints
        from scratch on update(), so erasure is implicit.          */
}
/*-----------------------------------------------------------------*/
void CrFeld::ToggleCursor()
{
    /*  Used by the blink timer (pending). For now, no-op. When the
        blink QTimer lands it will flip a TDBWFRM state flag and
        issue update(cursor-rect). */
}
/*-----------------------------------------------------------------*/
void CrFeld::UpdateSharedCoord(int _i, int _j)
{
    if (shareflags & CrShareX1)
        sharedcoord.x1 = _i;
    if (shareflags & CrShareX2)
        sharedcoord.x2 = _i;
    if (shareflags & CrShareY1)
        sharedcoord.y1 = _j;
    if (shareflags & CrShareY2)
        sharedcoord.y2 = _j;
}
/*-----------------------------------------------------------------*/
void CrFeld::SyncSharedCoord()
{
    if (shareflags & CrShareX1)
        fb.kbd.i = sharedcoord.x1;
    if (shareflags & CrShareX2)
        fb.kbd.i = sharedcoord.x2;
    if (shareflags & CrShareY1)
        fb.kbd.j = sharedcoord.y1;
    if (shareflags & CrShareY2)
        fb.kbd.j = sharedcoord.y2;
}
/*-----------------------------------------------------------------*/
