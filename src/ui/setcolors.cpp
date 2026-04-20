/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Palette-index selection helpers and bulk colour operations --
    port of legacy/setcolors.cpp. The SelectColor[Index] helpers
    reuse the ChoosePaletteDialog from batch 5. The click handlers
    apply the chosen colour to the full kettfarben / schussfarben
    strip (or replace-by-match under the keyboard cursor).         */

#include "mainwindow.h"
#include "datamodule.h"
#include "palette.h"
#include "undoredo.h"
#include "choosecolordialog.h"

#include <algorithm>

/*-----------------------------------------------------------------*/
int TDBWFRM::SelectColorIndex(int _index)
{
    ChoosePaletteDialog dlg(this);
    dlg.SelectColor(Data->palette->GetColor(_index));
    if (dlg.exec() != QDialog::Accepted)
        return -1;
    return dlg.GetSelectedIndex();
}

bool TDBWFRM::SelectColor(COLORREF& _col)
{
    ChoosePaletteDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return false;
    _col = dlg.GetSelectedColor();
    return true;
}

/*-----------------------------------------------------------------*/
void TDBWFRM::SetKettfarbeClick()
{
    const int col = SelectColorIndex(Data->color);
    if (col == -1)
        return;
    for (int i = 0; i < Data->MAXX1; i++)
        kettfarben.feld.Set(i, char(col));
    Data->color = (unsigned char)col;
    refresh();
    UpdateStatusBar();
    SetModified();
    if (undo)
        undo->Snapshot();
}

void TDBWFRM::SetSchussfarbeClick()
{
    const int col = SelectColorIndex(Data->color);
    if (col == -1)
        return;
    for (int j = 0; j < Data->MAXY2; j++)
        schussfarben.feld.Set(j, char(col));
    Data->color = (unsigned char)col;
    refresh();
    UpdateStatusBar();
    SetModified();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::ReplaceColorClick()
{
    int col = -1;
    if (kbd_field == KETTFARBEN) {
        col = kettfarben.feld.Get(scroll_x1 + kettfarben.kbd.i);
        const int newcol = SelectColorIndex(col);
        if (newcol == -1)
            return;
        for (int i = 0; i < Data->MAXX1; i++)
            if (col == kettfarben.feld.Get(i))
                kettfarben.feld.Set(i, char(newcol));
        col = newcol;
    } else if (kbd_field == SCHUSSFARBEN) {
        col = schussfarben.feld.Get(scroll_y2 + schussfarben.kbd.j);
        const int newcol = SelectColorIndex(col);
        if (newcol == -1)
            return;
        for (int j = 0; j < Data->MAXY2; j++)
            if (col == schussfarben.feld.Get(j))
                schussfarben.feld.Set(j, char(newcol));
        col = newcol;
    } else {
        return;
    }
    Data->color = (unsigned char)col;
    refresh();
    UpdateStatusBar();
    SetModified();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::SwitchColorsClick()
{
    const int maxi = Data->MAXX1 > Data->MAXY2 ? int(Data->MAXY2) : int(Data->MAXX1);
    for (int i = 0; i < maxi; i++) {
        const char col = kettfarben.feld.Get(i);
        kettfarben.feld.Set(i, schussfarben.feld.Get(i));
        schussfarben.feld.Set(i, col);
    }
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
/*  Overwrite every warp colour with the matching weft colour up to
    the min of the two axis lengths; pad any remaining warp indices
    with the default weft colour. Verbatim port of legacy
    KettfarbenWieSchussfarbenClick. */
void TDBWFRM::KettfarbenWieSchussfarbenClick()
{
    const int maxi = std::min(int(Data->MAXY2), int(Data->MAXX1));
    for (int j = 0; j < maxi; j++)
        kettfarben.feld.Set(j, schussfarben.feld.Get(j));
    if (Data->MAXY2 > Data->MAXX1)
        for (int j = Data->MAXX1; j < Data->MAXY2; j++)
            kettfarben.feld.Set(j, char(DEFAULT_COLORV));
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::SchussfarbenWieKettfarbenClick()
{
    const int maxi = std::min(int(Data->MAXX1), int(Data->MAXY2));
    for (int i = 0; i < maxi; i++)
        schussfarben.feld.Set(i, kettfarben.feld.Get(i));
    /*  Legacy pads with DEFAULT_COLORH up to MAXY1 when MAXX1>MAXY2;
        the target is schussfarben which is bounded by MAXY2, so the
        loop is capped at MAXY2 here (the legacy loop beyond MAXY2
        would have written out of bounds). */
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}
