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
#include <mem.h>
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "cursor.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::InitZoom()
{
    zoom[0] = 5;
    zoom[1] = 7;
    zoom[2] = 9;
    zoom[3] = 11;
    zoom[4] = 13;
    zoom[5] = 15;
    zoom[6] = 17;
    zoom[7] = 19;
    zoom[8] = 21;
    zoom[9] = 23;
    currentzoom = 3;

    ViewZoomOut->Enabled = true;
    ViewZoomOutPopup->Enabled = true;
    SBZoomOut->Enabled = true;
    ViewZoomIn->Enabled  = true;
    ViewZoomInPopup->Enabled = true;
    SBZoomIn->Enabled = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ViewZoomInPopupClick(TObject *Sender)
{
    ViewZoomInClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ViewZoomOutPopupClick(TObject *Sender)
{
    ViewZoomOutClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBZoomInClick(TObject *Sender)
{
    ViewZoomInClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBZoomOutClick(TObject *Sender)
{
    ViewZoomOutClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ViewZoomInClick(TObject *Sender)
{
    ViewZoomOut->Enabled = true;
    ViewZoomOutPopup->Enabled = true;
    SBZoomOut->Enabled = true;
    ViewZoomIn->Enabled  = true;
    ViewZoomInPopup->Enabled = true;
    SBZoomIn->Enabled = true;

    int maxzoom = sizeof(zoom)/sizeof(int)-1;
    if (currentzoom<maxzoom) {
        currentzoom++;
        ClearSelection();
        CalcGrid();
        RecalcDimensions();
        UpdateScrollbars();
        dbw3_assert(cursorhandler);
        cursorhandler->CheckCursorPos();
        Invalidate();
        if (currentzoom==maxzoom) {
            ViewZoomIn->Enabled = false;
            ViewZoomInPopup->Enabled = false;
            SBZoomIn->Enabled = false;
        }
    }
    SetModified();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ViewZoomOutClick(TObject *Sender)
{
    ViewZoomOut->Enabled = true;
    ViewZoomOutPopup->Enabled = true;
    SBZoomOut->Enabled = true;
    ViewZoomIn->Enabled  = true;
    ViewZoomInPopup->Enabled = true;
    SBZoomIn->Enabled = true;

    if (currentzoom>0) {
        currentzoom--;
        ClearSelection();
        CalcGrid();
        RecalcDimensions();
        UpdateScrollbars();
        dbw3_assert(cursorhandler);
        cursorhandler->CheckCursorPos();
        Invalidate();
        if (currentzoom==0) {
            ViewZoomOut->Enabled = false;
            ViewZoomOutPopup->Enabled = false;
            SBZoomOut->Enabled = false;
        }
    }
    SetModified();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ViewZoomNormalClick(TObject *Sender)
{
    if (currentzoom!=3) {
        currentzoom = 3;
        CalcGrid();
        RecalcDimensions();
        UpdateScrollbars();
        Invalidate();
        ViewZoomIn->Enabled = true;
        ViewZoomInPopup->Enabled = true;
        SBZoomIn->Enabled = true;
        ViewZoomOut->Enabled = true;
        ViewZoomOutPopup->Enabled = true;
        SBZoomOut->Enabled = true;
        SetModified();
    }
}
/*-----------------------------------------------------------------*/

