/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Legacy DoSetBlatteinzug painted directly onto the form's Canvas.
    Qt is event-driven so SetBlatteinzug mutates the field then
    defers the repaint through refresh() -> paintEvent.            */

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "undoredo.h"
/*-----------------------------------------------------------------*/
void TDBWFRM::SetBlatteinzug(int _i)
{
    DoSetBlatteinzug(_i);
    refresh(); /* legacy painted inline; defer to paintEvent */

    dbw3_assert(undo != 0);
    if (undo)
        undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void TDBWFRM::DoSetBlatteinzug(int _i)
{
    // Feld setzen
    blatteinzug.feld.Set(_i + scroll_x1, !blatteinzug.feld.Get(_i + scroll_x1));
    SetModified();
}
/*-----------------------------------------------------------------*/
