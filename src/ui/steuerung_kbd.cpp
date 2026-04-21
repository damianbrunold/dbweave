/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port of legacy steuerung_kbd.cpp. Qt collapses FormKeyPress and
    FormKeyDown into a single keyPressEvent; this file handles both
    the navigation keys (arrows / PgUp / PgDown with Ctrl / Shift
    variants) and the textual keys (Enter, 0..9) in one method.  */

#include "steuerung.h"

#include "datamodule.h"
#include "steuerungcanvas.h"

#include <QKeyEvent>

/*-----------------------------------------------------------------*/
static void HandleSchussKeyDown(TSTRGFRM* _f, QKeyEvent* _e)
{
    const int key = _e->key();
    const auto mods = _e->modifiers();
    const bool ctrl = (mods & Qt::ControlModifier) != 0;
    bool handled = false;

    if (key == Qt::Key_Up) {
        const int delta = ctrl ? 8 : 1;
        _f->weave_position += delta;
        if (_f->weave_position >= _f->scrolly + _f->maxj)
            _f->weave_position = _f->scrolly + _f->maxj - 1;
        _f->ValidateWeavePosition();
        _f->SetModified();
        handled = true;
    } else if (key == Qt::Key_Down) {
        const int delta = ctrl ? 8 : 1;
        _f->weave_position -= delta;
        if (_f->weave_position < _f->scrolly)
            _f->weave_position = _f->scrolly;
        _f->ValidateWeavePosition();
        _f->SetModified();
        handled = true;
    } else if (key == Qt::Key_PageUp) {
        /*  Legacy: PageUp either jumps weave_position to top of
            the visible area, or scrolls one page up and puts the
            cursor at the top of the new view.                    */
        const int sy = _f->scrolly;
        if (_f->weave_position < _f->scrolly + _f->maxj - 1) {
            _f->weave_position = _f->scrolly + _f->maxj - 1;
        } else {
            _f->scrolly += _f->maxj;
            if (_f->data && _f->scrolly > _f->data->MAXY2 - _f->maxj)
                _f->scrolly = _f->data->MAXY2 - _f->maxj;
            if (_f->scrolly < 0)
                _f->scrolly = 0;
            _f->weave_position = _f->scrolly + _f->maxj - 1;
        }
        _f->ValidateWeavePosition();
        _f->SetModified();
        if (sy != _f->scrolly)
            _f->UpdateScrollbar();
        handled = true;
    } else if (key == Qt::Key_PageDown) {
        const int sy = _f->scrolly;
        if (_f->weave_position > _f->scrolly) {
            _f->weave_position = _f->scrolly;
        } else {
            _f->scrolly -= _f->maxj;
            if (_f->scrolly < 0)
                _f->scrolly = 0;
            _f->weave_position = _f->scrolly;
        }
        _f->ValidateWeavePosition();
        _f->SetModified();
        if (sy != _f->scrolly)
            _f->UpdateScrollbar();
        handled = true;
    } else if (key == Qt::Key_Left) {
        if (ctrl)
            _f->WeaveRepetitionDec();
        else
            _f->WeaveKlammerLeft();
        handled = true;
    } else if (key == Qt::Key_Right) {
        if (ctrl)
            _f->WeaveRepetitionInc();
        else
            _f->WeaveKlammerRight();
        handled = true;
    }

    if (handled) {
        _f->UpdateStatusbar();
        if (_f->canvas)
            _f->canvas->update();
        _e->accept();
    }
}

/*-----------------------------------------------------------------*/
static void HandleKlammerKeyDown(TSTRGFRM* _f, QKeyEvent* _e)
{
    const int key = _e->key();
    const auto mods = _e->modifiers();
    const bool ctrl = (mods & Qt::ControlModifier) != 0;
    const bool shift = (mods & Qt::ShiftModifier) != 0;
    bool handled = false;

    if (key == Qt::Key_Left) {
        _f->current_klammer
            = (_f->current_klammer + TSTRGFRM::MAXKLAMMERN - 1) % TSTRGFRM::MAXKLAMMERN;
        handled = true;
    } else if (key == Qt::Key_Right) {
        _f->current_klammer = (_f->current_klammer + 1) % TSTRGFRM::MAXKLAMMERN;
        handled = true;
    } else if (key == Qt::Key_Up) {
        Klammer& k = _f->klammern[_f->current_klammer];
        if (shift && ctrl) {
            k.first += 8;
            k.last += 8;
            if (_f->data && k.last >= _f->data->MAXY2) {
                k.first -= (k.last - (_f->data->MAXY2 - 1));
                k.last = _f->data->MAXY2 - 1;
            }
        } else if (shift) {
            if (_f->data && k.last < _f->data->MAXY2 - 1) {
                k.first++;
                k.last++;
            }
        } else if (ctrl) {
            k.first++;
            if (k.first >= k.last)
                k.first = k.last - 1;
        } else {
            if (_f->data && k.last < _f->data->MAXY2 - 1)
                k.last++;
        }
        _f->SetModified();
        handled = true;
    } else if (key == Qt::Key_Down) {
        Klammer& k = _f->klammern[_f->current_klammer];
        if (shift && ctrl) {
            k.first -= 8;
            k.last -= 8;
            if (k.first < 0) {
                k.last += (-k.first);
                k.first = 0;
            }
        } else if (shift) {
            if (k.first > 0) {
                k.first--;
                k.last--;
            }
        } else if (ctrl) {
            if (k.first > 0)
                k.first--;
        } else {
            if (k.last > k.first + 1)
                k.last--;
        }
        _f->SetModified();
        handled = true;
    }

    if (handled) {
        _f->UpdateStatusbar();
        _f->refreshGotoActions();
        if (_f->canvas)
            _f->canvas->update();
        _e->accept();
    }
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::FormKeyDown(QKeyEvent* _e)
{
    if (!_e)
        return;

    const int key = _e->key();

    /*  Enter toggles the schuss / klammer focus mode. Works in
        both modes.                                                */
    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        schussselected = !schussselected;
        UpdateStatusbar();
        if (canvas)
            canvas->update();
        _e->accept();
        return;
    }

    /*  Digit keys (0..9) set the current klammer's repetitions
        when we are in klammer-focus mode. Legacy FormKeyPress
        handled this as a textual-input event.                    */
    if (!schussselected && key >= Qt::Key_0 && key <= Qt::Key_9) {
        klammern[current_klammer].repetitions = key - Qt::Key_0;
        SetModified();
        UpdateStatusbar();
        refreshGotoActions();
        if (canvas)
            canvas->update();
        _e->accept();
        return;
    }

    if (schussselected)
        HandleSchussKeyDown(this, _e);
    else
        HandleKlammerKeyDown(this, _e);
}
