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
/*------------------------------------------------------------------*/
#include <vcl\vcl.h>
#include <mem.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::HandleCommandlineOpen()
{
    // übergebene Datei öffnen
    // falls mehrere Dateinamen übergeben werden,
    // wird nur die erste Datei geöffnet. Grund:
    // kein MDI
    if (ParamCount()>0 && ParamStr(1)!="/p") {
        DateiLaden (ParamStr(1), false, true);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::HandleCommandlinePrint()
{
    // /p-Option behandlen
    if (ParamCount()>1 && ParamStr(1)=="/p") {
        // übergebene Datei öffnen, aber nicht in
        // MRU eintragen.
        DateiLaden (ParamStr(2), false, false);

        // Datei drucken ohne Druckdialog
        SBFilePrintClick (this);

        // Applikation wieder beenden
        PostMessage (Handle, WM_CLOSE, 0, 0L);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::HandleCommandlineArguments()
{
    HandleCommandlinePrint();
    HandleCommandlineOpen();
}
/*------------------------------------------------------------------*/
