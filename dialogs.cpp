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
#include <stdio.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "felddef.h"
#include "einstellverh_form.h"
#include "datamodule.h"
#include "undoredo.h"
#include "cursor.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EinstellVerhaeltnisClick(TObject *Sender)
{
    TEinstellVerhaeltnisForm* frm = new TEinstellVerhaeltnisForm (this);
    cursorhandler->DisableCursor();
    char buff[10];
    sprintf (buff, "%0.2f", faktor_schuss);
    frm->FaktorSchuss->Text = buff;
    sprintf (buff, "%0.2f", faktor_kette);
    frm->FaktorKette->Text = buff;
    if (frm->ShowModal()==IDOK) {
        double fs = atof (frm->FaktorSchuss->Text.c_str());
        double fk = atof (frm->FaktorKette->Text.c_str());
        if (fs>0.0 && fs<=10.0 && fk>0.0 && fk<=10.0) {
            faktor_schuss = fs;
            faktor_kette = fk;
            CalcGrid();
            RecalcDimensions();
            Invalidate();
        }
    }
    delete frm;
    cursorhandler->EnableCursor();
}
/*-----------------------------------------------------------------*/

