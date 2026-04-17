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
#include "dbw3_form.h"
#include "Datamodule.h"
#include "assert.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DebugChecks()
{
#ifdef _DEBUG
#ifndef NODEBUGCHECKS
    // Schussbereich
    String line;
    SZ oldschuesse = schuesse;
    CalcRangeSchuesse();
    if (oldschuesse.a!=schuesse.a || oldschuesse.b!=schuesse.b) {
        ::MessageBeep (MB_ICONEXCLAMATION);
        line = "schuesse falsch  ";
    }

    // Kettbereich
    SZ oldkette = kette;
    CalcRangeKette();
    if (oldkette.a!=kette.a || oldkette.b!=kette.b) {
        ::MessageBeep (MB_ICONEXCLAMATION);
        line += "kette falsch  ";
    }

    // Freie Schäfte
    bool empty[400];
    dbw3_assert(Data->MAXY1<400);
    for (int i=0; i<Data->MAXY1; i++)
        empty[i] = freieschaefte[i];
    RecalcFreieSchaefte();
    for (int i=0; i<Data->MAXY1; i++)
        if(empty[i]!=freieschaefte[i]) {
             ::MessageBeep (MB_ICONEXCLAMATION);
            line += "freie schäfte falsch  ";
            break;
        }

    // Freie Schüsse
    dbw3_assert(Data->MAXX2<400);
    for (int i=0; i<Data->MAXX2; i++)
        empty[i] = freietritte[i];
    RecalcFreieTritte();
    for (int i=0; i<Data->MAXX2; i++)
        if(empty[i]!=freietritte[i]) {
            ::MessageBeep (MB_ICONEXCLAMATION);
            line += "freie tritte falsch  ";
            break;
        }

    // Rapport
    //xxx

    // Fixierter Einzug
    if (EzFixiert->Checked && fixeinzug) {
        bool empty = false;
        for (int i=kette.a; i<Data->MAXX1; i++)
            if (einzug.feld.Get(i)==0) empty = true;
            else {
                if (empty) {
                    ::MessageBeep (MB_ICONEXCLAMATION);
                    line += "fixeinzug löchrig  ";
                    UpdateEinzugFixiert();
                }
                break;
            }
    }

    // Anz Schaefte == Anz Tritte
    if (Data->MAXY1!=Data->MAXX2)
        line += "#Schäfte!=#Tritte ";

    // Xbuf size
    if (Data->MAXX1!=xbufsize) {
        ::MessageBeep (MB_ICONEXCLAMATION);
        line += "xbufsize ";
    }

    // Ybuf size
    if (Data->MAXY2!=ybufsize) {
        ::MessageBeep (MB_ICONEXCLAMATION);
        line += "ybufsize ";
    }

    // fixeinzug size
    if (Data->MAXX1!=fixeinzugsize) {
        ::MessageBeep (MB_ICONEXCLAMATION);
        line += "fixeinzugsize ";
    }

    // freieschaefte size
    if (Data->MAXY1!=freieschaeftesize) {
        ::MessageBeep (MB_ICONEXCLAMATION);
        line += "freieschaeftesize ";
    }

    // freietritte size
    if (Data->MAXX2!=freietrittesize) {
        ::MessageBeep (MB_ICONEXCLAMATION);
        line += "freietrittesize ";
    }

    // Nachprüfen, ob Bindung mit Aufknüpfung/Trittfolge/Einzug übereinstimmt
    for (int i=0; i<Data->MAXX1; i++)
        for (int j=0; j<Data->MAXY2; j++)
            if (gewebe.feld.Get(i, j)>0) {
                bool set = false;
                int k = einzug.feld.Get(i);
                if (k-->0) {
                    for (int m=0; m<Data->MAXX2; m++)
                        if (trittfolge.feld.Get(m, j)>0)
                            if (aufknuepfung.feld.Get(m, k)>0) {
                                set = true;
                                break;
                            }
                }
                if (!set) {
                    line += "Recalc falsch ";
                    goto g_weiter;
                }
            }

g_weiter:
    if (line!="")
        Statusbar->Panels->Items[0]->Text = line;
#endif // !NODEBUGCHECKS
#endif // _DEBUG
}
/*-----------------------------------------------------------------*/
