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
#include "undoredo.h"
#include "datamodule.h"
#include "entervv_form.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EditFillKoeperClick(TObject *Sender)
{
    // Aktuell selektierten Kettfaden zu vollständigen
    // Köper ergänzen falls genügend Platz rechts.
    RANGE savesel = selection;
    selection.Normalize();
    bool valid = selection.Valid();

    dbw3_assert (valid);
    dbw3_assert (selection.feld==GEWEBE);
    if (!valid || selection.feld!=GEWEBE) {
        selection = savesel;
        return;
    }

    if (selection.begin.i==selection.end.i) {
        int dy     = selection.end.j - selection.begin.j;
        int iStart = selection.begin.i;
        int iEnd   = selection.begin.i + dy;

        // Feststellen, wieviel Platz rechts frei ist.
        int i = iStart+1;
        for (/**/; i<=iEnd; i++) {
            bool ok = true;
            for (int j=selection.begin.j; j<=selection.end.j; j++) {
                if (gewebe.feld.Get (i, j)>0) {
                    ok = false;
                    break;
                }
            }
            if (!ok) break;
        }
        i--;

        if (i>=iStart) {
            iEnd = i;
            // So, Köper ausfüllen
            for (int i=iStart+1; i<=iEnd; i++) {
                for (int j=selection.begin.j; j<=selection.begin.j+dy; j++) {
                    int jj = j - selection.begin.j - (i-iStart);
                    while (jj<0) jj += (dy+1);
                    while (jj>dy) jj -= (dy+1);
                    if (gewebe.feld.Get (iStart, selection.begin.j+jj)>0)
                        DoSetGewebe (i-scroll_x1, j-scroll_y2, false, currentrange);
                }
            }
            if (!ViewSchlagpatrone->Checked)
                RearrangeTritte();
            CalcRangeKette();
            CalcRangeSchuesse();
            CalcRapport();
            UpdateStatusBar();
            Invalidate();
            int ii = iEnd;
            if (ii>=scroll_x1+gewebe.pos.width/gewebe.gw)
                ii = scroll_x1+gewebe.pos.width/gewebe.gw-1;
            selection.end.i = ii;
            SetModified();
            dbw3_assert (undo!=0);
            undo->Snapshot();
        }
    } else if (selection.begin.j==selection.end.j) {
        int dx     = selection.end.i - selection.begin.i;
        int jStart = selection.begin.j;
        int jEnd   = selection.begin.j + dx;

        // Feststellen, wieviel Platz oben frei ist.
        int j = jStart+1;
        for (/**/; j<=jEnd; j++) {
            bool ok = true;
            for (int i=selection.begin.i; i<=selection.end.i; i++) {
                if (gewebe.feld.Get (i, j)>0) {
                    ok = false;
                    break;
                }
            }
            if (!ok) break;
        }
        j--;

        if (j>=jStart) {
            jEnd = j;
            // So, Köper ausfüllen
            for (int j=jStart+1; j<=jEnd; j++) {
                for (int i=selection.begin.i; i<=selection.begin.i+dx; i++) {
                    int ii = i - selection.begin.i - (j-jStart);
                    while (ii<0) ii += (dx+1);
                    while (ii>dx) ii -= (dx+1);
                    if (gewebe.feld.Get (selection.begin.i+ii, jStart)>0)
                        DoSetGewebe (i-scroll_x1, j-scroll_y2, false, currentrange);
                }
            }
            if (!ViewSchlagpatrone->Checked)
                RearrangeTritte();
            CalcRangeKette();
            CalcRangeSchuesse();
            CalcRapport();
            int jj = jEnd;
            if (jj>=scroll_y2+gewebe.pos.height/gewebe.gh)
                jj = scroll_y2+gewebe.pos.height/gewebe.gh-1;
            selection.end.j = jj;
            Invalidate();
            SetModified();
            dbw3_assert (undo!=0);
            undo->Snapshot();
        }
    }

    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EditSwapsideClick(TObject *Sender)
{
    // Aufknüpfung/Schlagpatrone invertieren (im belegten Bereich)
    if (!ViewSchlagpatrone->Checked) {
        for (int i=0; i<Data->MAXX2; i++)
            if (!freietritte[i])
                for (int j=0; j<Data->MAXY1; j++)
                    if (!freieschaefte[j]) {
                        char s = aufknuepfung.feld.Get (i, j);
                        aufknuepfung.feld.Set (i, j, char(s==0 ? currentrange : -s));
                    }
    } else {
        for (int i=0; i<min(Data->MAXY1,Data->MAXX2); i++)
            if (!freieschaefte[i])
                for (int j=schuesse.a; j<=schuesse.b; j++)
                    if (!trittfolge.isempty.Get(j)) {
                        char s = trittfolge.feld.Get (i, j);
                        trittfolge.feld.Set (i, j, char(s==0 ? currentrange : -s));
                    }
    }

    // Linksrechts-Spiegeln
    int max = (kette.b-kette.a+1)/2;
    for (int i=kette.a; i<kette.a+max; i++) {
        short s = einzug.feld.Get (i);
        einzug.feld.Set (i, einzug.feld.Get (kette.b-(i-kette.a)));
        einzug.feld.Set (kette.b-(i-kette.a), s);
        char c = kettfarben.feld.Get (i);
        kettfarben.feld.Set (i, kettfarben.feld.Get (kette.b-(i-kette.a)));
        kettfarben.feld.Set (kette.b-(i-kette.a), c);
        bool b = blatteinzug.feld.Get (i);
        blatteinzug.feld.Set (i, blatteinzug.feld.Get (kette.b-(i-kette.a)));
        blatteinzug.feld.Set (kette.b-(i-kette.a), b);
    }

    RecalcGewebe();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    Invalidate();
    SetModified();
    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::KettLancierungClick(TObject *Sender)
{
    try {
        TEnterVVForm* pfrm = new TEnterVVForm(this);
        if (pfrm->ShowModal()==mrOk) {
            int vv[6], maxi = 0;
            vv[0] = atoi(pfrm->VV1->Text.c_str()); if (vv[0]!=0) maxi = 1;
            vv[1] = atoi(pfrm->VV2->Text.c_str()); if (vv[1]!=0) maxi = 2;
            vv[2] = atoi(pfrm->VV3->Text.c_str()); if (vv[2]!=0) maxi = 3;
            vv[3] = atoi(pfrm->VV4->Text.c_str()); if (vv[3]!=0) maxi = 4;
            vv[4] = atoi(pfrm->VV5->Text.c_str()); if (vv[4]!=0) maxi = 5;
            vv[5] = atoi(pfrm->VV6->Text.c_str()); if (vv[5]!=0) maxi = 6;

            if (maxi>1 && (maxi%2)!=0) {
                vv[0] += vv[maxi-1];
                vv[maxi-1] = 0;
                maxi--;

            }
            if (maxi>1) {
                int a, b;
                RANGE savesel = selection;
                selection.Normalize();
                if (selection.Valid()) {
                    a = selection.begin.i;
                    b = selection.end.i;
                } else {
                    a = kette.a;
                    b = kette.b;
                }
                selection = savesel;

                int needed = b-a+1;
                int i = a;
                int idx = 0;
                do {
                    // Zuerst Anzahl Kettfäden abarbeiten
                    int v = vv[idx];
                    while (i<=b && v-->0) i++;
                    idx = (idx+1) % maxi;
                    // Dann kommen eine Anzahl leere hinzu gemäss vv
                    if (i<=b+1) {
                        needed += vv[idx];
                        idx = (idx+1) % maxi;
                    }
                } while (i<=b);
                if (a+needed>=Data->MAXX1) needed = Data->MAXX1-1-a;
                try {
                    int* buff = new int[needed];
                    idx = 0;
                    int ii = 0;
                    for (int i=a; i<=b; i++) {
                        for (int j=0; j<vv[idx]; j++) {
                            buff[ii++] = i++;
                            if (ii>=needed || i-1>b) break;
                        }
                        i--;
                        if (ii>=needed) break;
                        idx = (idx + 1) % maxi;
                        for (int j=0; j<vv[idx]; j++) {
                            buff[ii++] = -1;
                            if (ii>=needed) break;
                        }
                        idx = (idx + 1) % maxi;
                        if (ii>=needed) break;
                    }
                    // Restdaten auf Seite kopieren
                    int maxii = min(Data->MAXX1, kette.b-b+1);
                    for (int i=b+maxii-1; i>b; i--) {
                        einzug.feld.Set (a+needed+i-b-1, einzug.feld.Get(i));
                        for (int j=schuesse.a; j<=schuesse.b; j++)
                            gewebe.feld.Set (a+needed+i-b-1, j, gewebe.feld.Get(i, j));
                    }
                    // Auseinanderziehen
                    for (int i=a+needed-1; i>=a; i--) {
                        if (buff[i-a]==-1) {
                            // Leerer Kettfaden
                            einzug.feld.Set (i, 0);
                            for (int j=schuesse.a; j<=schuesse.b; j++)
                                gewebe.feld.Set (i, j, 0);
                        } else {
                            // Kettfaden kopieren
                            einzug.feld.Set (i, einzug.feld.Get(buff[i-a]));
                            for (int j=schuesse.a; j<=schuesse.b; j++)
                                gewebe.feld.Set (i, j, gewebe.feld.Get (buff[i-a], j));
                        }
                    }
                    delete[] buff;

                    CalcRangeKette();
                    RecalcFreieSchaefte();
                    UpdateRapport();
                    Invalidate();
                    SetModified();
                    undo->Snapshot();
                } catch (...) {
                }
            }
        }
        delete pfrm;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SchussLancierungClick(TObject *Sender)
{
    try {
        TEnterVVForm* pfrm = new TEnterVVForm(this);
        if (pfrm->ShowModal()==mrOk) {
            int vv[6], maxi = 0;
            vv[0] = atoi(pfrm->VV1->Text.c_str()); if (vv[0]!=0) maxi = 1;
            vv[1] = atoi(pfrm->VV2->Text.c_str()); if (vv[1]!=0) maxi = 2;
            vv[2] = atoi(pfrm->VV3->Text.c_str()); if (vv[2]!=0) maxi = 3;
            vv[3] = atoi(pfrm->VV4->Text.c_str()); if (vv[3]!=0) maxi = 4;
            vv[4] = atoi(pfrm->VV5->Text.c_str()); if (vv[4]!=0) maxi = 5;
            vv[5] = atoi(pfrm->VV6->Text.c_str()); if (vv[5]!=0) maxi = 6;

            if (maxi>1 && (maxi%2)!=0) {
                vv[0] += vv[maxi-1];
                vv[maxi-1] = 0;
                maxi--;

            }
            if (maxi>1) {
                int a, b;
                RANGE savesel = selection;
                selection.Normalize();
                if (selection.Valid()) {
                    a = selection.begin.j;
                    b = selection.end.j;
                } else {
                    a = schuesse.a;
                    b = schuesse.b;
                }
                selection = savesel;

                int needed = b-a+1;
                int i = a;
                int idx = 0;
                do {
                    // Zuerst Anzahl Kettfäden abarbeiten
                    int v = vv[idx];
                    while (i<=b && v-->0) i++;
                    idx = (idx+1) % maxi;
                    // Dann kommen eine Anzahl leere hinzu gemäss vv
                    if (i<=b+1) {
                        needed += vv[idx];
                        idx = (idx+1) % maxi;
                    }
                } while (i<=b);
                if (a+needed>=Data->MAXY2) needed = Data->MAXY2-1-a;
                try {
                    int* buff = new int[needed];
                    idx = 0;
                    int ii = 0;
                    for (int i=a; i<=b; i++) {
                        for (int j=0; j<vv[idx]; j++) {
                            buff[ii++] = i++;
                            if (ii>=needed || i-1>b) break;
                        }
                        i--;
                        if (ii>=needed) break;
                        idx = (idx + 1) % maxi;
                        for (int j=0; j<vv[idx]; j++) {
                            buff[ii++] = -1;
                            if (ii>=needed) break;
                        }
                        idx = (idx + 1) % maxi;
                        if (ii>=needed) break;
                    }
                    // Restdaten auf Seite kopieren
                    int maxjj = min(Data->MAXY2, schuesse.b-b+1);
                    for (int j=b+maxjj-1; j>b; j--) {
                        trittfolge.isempty.Set (a+needed+j-b-1, trittfolge.isempty.Get(j));
                        for (int i=0; i<Data->MAXX2; i++)
                            trittfolge.feld.Set (i, a+needed+j-b-1, trittfolge.feld.Get (i, j));
                        for (int i=kette.a; i<=kette.b; i++)
                            gewebe.feld.Set (i, a+needed+j-b-1, gewebe.feld.Get(i, j));
                    }
                    // Auseinanderziehen
                    for (int j=a+needed-1; j>=a; j--) {
                        if (buff[j-a]==-1) {
                            // Leerer Schussfaden
                            trittfolge.isempty.Set (j, true);
                            for (int i=0; i<Data->MAXX2; i++)
                                trittfolge.feld.Set (i, j, 0);
                            for (int i=kette.a; i<=kette.b; i++)
                                gewebe.feld.Set (i, j, 0);
                        } else {
                            // Kettfaden kopieren
                            trittfolge.isempty.Set (j, trittfolge.isempty.Get(buff[j-a]));
                            for (int i=0; i<Data->MAXX2; i++)
                                trittfolge.feld.Set (i, j, trittfolge.feld.Get (i, buff[j-a]));
                            for (int i=kette.a; i<=kette.b; i++)
                                gewebe.feld.Set (i, j, gewebe.feld.Get (i, buff[j-a]));
                        }
                    }
                    delete[] buff;

                    CalcRangeSchuesse();
                    RecalcFreieTritte();
                    UpdateRapport();
                    Invalidate();
                    SetModified();
                    undo->Snapshot();
                } catch (...) {
                }
            }
        }
        delete pfrm;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/

