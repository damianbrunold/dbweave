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
#include "dbw3_form.h"
#include "felddef.h"
#include "datamodule.h"
#include "undoredo.h"
#include "importbmp_form.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ImportBitmapClick(TObject *Sender)
{
    // Bilddatei auswählen
    if (Data->ImportBmpDialog->Execute()) {
        String filename = Data->ImportBmpDialog->FileName;
        // Importform anzeigen
        try {
            TImportBmpForm* pFrm = new TImportBmpForm (this, filename);
            if (pFrm->ShowModal()==mrOk) {
                // Auf Schlagpatrone umschalten
                ViewSchlagpatrone->Checked = true;
                trittfolge.einzeltritt = false;
                // Gewebe löschen
                for (int i=0; i<Data->MAXX1; i++)
                    for (int j=0; j<Data->MAXY2; j++)
                        gewebe.feld.Set (i, j, 0);
                // Bild in Gewebe setzen
                for (int i=0; i<min(Data->MAXX1, pFrm->x); i++)
                    for (int j=0; j<min(Data->MAXY2, pFrm->y); j++)
                        gewebe.feld.Set (i, j, pFrm->data[i+j*pFrm->x]);
                TCursor old = Cursor;
                Cursor = crHourGlass;
                CalcRange();
                CalcRapport();
                RecalcAll();
                Cursor = old;
                Invalidate();
                undo->Snapshot();
                SetModified();
            }
            delete pFrm;
        } catch (...) {
        }
    }
}
/*-----------------------------------------------------------------*/
