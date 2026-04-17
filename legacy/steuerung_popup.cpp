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
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "steuerung_form.h"
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::CancelClick(TObject *Sender)
{
    //
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::StartWebenClick(TObject *Sender)
{
    WeaveStartClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::StopWebenClick(TObject *Sender)
{
    WeaveStopClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::ReverseWebenClick(TObject *Sender)
{
    WeaveBackwardsClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoLastClick(TObject *Sender)
{
    GotoLastPosClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer1PopupClick(TObject *Sender)
{
    GotoKlammer1Click (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer2PopupClick(TObject *Sender)
{
    GotoKlammer2Click (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer3PopupClick(TObject *Sender)
{
    GotoKlammer3Click (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer4PopupClick(TObject *Sender)
{
    GotoKlammer4Click (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer5PopupClick(TObject *Sender)
{
    GotoKlammer5Click (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer6PopupClick(TObject *Sender)
{
    GotoKlammer6Click (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer7PopupClick(TObject *Sender)
{
    GotoKlammer7Click (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer8PopupClick(TObject *Sender)
{
    GotoKlammer8Click (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::GotoKlammer9PopupClick(TObject *Sender)
{
    GotoKlammer9Click (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::ViewPatronePopupClick(TObject *Sender)
{
    ViewPatroneClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::ViewFarbeffektPopupClick(TObject *Sender)
{
    ViewFarbeffektClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::ViewGewebesimulationPopupClick(TObject *Sender)
{
    ViewGewebesimulationClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::ZoomInPopupClick(TObject *Sender)
{
    ZoomInClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::NormalPopupClick(TObject *Sender)
{
    ZoomNormalClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::ZoomOutPopupClick(TObject *Sender)
{
    ZoomOutClick(Sender);
}
/*-----------------------------------------------------------------*/

