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
#include "rapport_form.h"
#include "undoredo.h"
#include "rapport.h"
#include "dbw3_strings.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RapportSchuss (int _ry, bool _withcolors)
{
    int j, j1, j2, sj1, sj2;

    // Schussbereich bestimmen
    CalcRangeSchuesse();
    j1 = schuesse.a;
    j2 = schuesse.b;
    if (j1==-1) return;
    if (j2==-1) return;

    // Schussrapport bestimmen
    CalcRapport();
    sj1 = rapport.sr.a;
    sj2 = rapport.sr.b;
    if (sj1==0 && sj2==-1) {
        sj1 = j1;
        sj2 = j2;
    }

    // Alles ausserhalb Rapport löschen
    for (j=j1; j<sj1; j++) ClearSchussfaden (j);
    for (j=sj2+1; j<=j2; j++) ClearSchussfaden (j);

    // Rapportieren
    //xxxx momentan rapportiere ich nur nach oben...
    int maxj;
    if (_ry!=-1) maxj = sj1+(sj2-sj1+1)*_ry;
    else maxj = Data->MAXY2;
    if (maxj>=Data->MAXY2) maxj = Data->MAXY2;
    for (j=sj2+1; j<maxj; j++)
        CopySchussfaden (sj1+(j-sj2-1)%(sj2-sj1+1), j, _withcolors);

    // Schussbereich nachführen
    //xxx könnte optimaler sein...
    CalcRangeSchuesse();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RapportKette (int _rx, bool _withcolors)
{
    int i, i1, i2, ki1, ki2;

    // Kettbereich bestimmen
    CalcRangeKette();
    i1 = kette.a;
    i2 = kette.b;
    if (i1==-1) return;
    if (i2==-1) return;

    // Kettrapport bestimmen
    CalcRapport();
    ki1 = rapport.kr.a;
    ki2 = rapport.kr.b;
    if (ki1==0 && ki2==-1) {
        ki1 = i1;
        ki2 = i2;
    }

    // Alles ausserhalb Rapport löschen
    for (i=i1; i<ki1; i++) ClearKettfaden (i);
    for (i=ki2+1; i<=i2; i++) ClearKettfaden (i);

    // Rapportieren
    //xxxx momentan rapportiere ich nur nach rechts...
    int maxi;
    if (_rx!=-1) maxi = ki1+(ki2-ki1+1)*_rx;
    else maxi = Data->MAXX1;
    if (maxi>=Data->MAXX1) maxi = Data->MAXX1;
    for (i=ki2+1; i<maxi; i++)
        CopyKettfaden (ki1+(i-ki2-1)%(ki2-ki1+1), i, _withcolors);

    // Kettbereich nachführen
    //xxx könnte optimaler sein...
    CalcRangeKette();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::CopyKettfaden (int _von, int _nach, bool _withcolors)
{
    // Blatteinzug kopieren
    //xxxx ?? muss ich das?!
//    blatteinzug.Set (_nach, blatteinzug.Get (_von));

    // Farbe
    if (_withcolors) kettfarben.feld.Set (_nach, kettfarben.feld.Get (_von));

    // Einzug kopieren
    einzug.feld.Set (_nach, einzug.feld.Get (_von));

    // Gewebe kopieren
    if (schuesse.a!=-1 && schuesse.b!=-1) {
        for (int j=schuesse.a; j<=schuesse.b; j++)
            gewebe.feld.Set (_nach, j, gewebe.feld.Get (_von, j));
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::CopySchussfaden (int _von, int _nach, bool _withcolors)
{
    // IsTrittfolgeEmpty kopieren
    trittfolge.isempty.Set (_nach, trittfolge.isempty.Get(_von));

    // Tritt kopieren
    for (int i=0; i<Data->MAXX2; i++)
        trittfolge.feld.Set (i, _nach, trittfolge.feld.Get (i, _von));

    // Farbe kopieren
    if (_withcolors) schussfarben.feld.Set (_nach, schussfarben.feld.Get (_von));

    // Gewebe kopieren
    if (kette.a!=-1 && kette.b!=-1) {
        for (int i=kette.a; i<=kette.b; i++)
            gewebe.feld.Set (i, _nach, gewebe.feld.Get (i, _von));
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ClearKettfaden (int _i)
{
    einzug.feld.Set (_i, 0);
    if (schuesse.b!=-1) for (int j=schuesse.a; j<=schuesse.b; j++) gewebe.feld.Set (_i, j, 0);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ClearSchussfaden (int _j)
{
    for (int i=0; i<Data->MAXX2; i++) trittfolge.feld.Set (i, _j, 0);
    trittfolge.isempty.Set (_j, true);
    if (kette.b!=-1) for (int i=kette.a; i<=kette.b; i++) gewebe.feld.Set (i, _j, 0);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RappRapportierenClick(TObject *Sender)
{
    try {
        TRapportForm* frm = new TRapportForm(this);
        frm->RappAll->Checked = false;
        int kx = kette.b-kette.a+1;
        int rx = rapport.kr.b-rapport.kr.a+1;
        if (kx!=0 && rx!=0) {
            int xx = kx/rx;
            frm->rapp_horz->Text = IntToStr(xx);
        }
        int ky = schuesse.b-schuesse.a+1;
        int ry = rapport.sr.b-rapport.sr.a+1;
        if (ky!=0 && ry!=0) {
            int yy = ky/ry;
            frm->rapp_vert->Text = IntToStr(yy);
        }
        if (frm->ShowModal()==mrOk) {
            int rx = atoi(frm->rapp_horz->Text.c_str());
            int ry = atoi(frm->rapp_vert->Text.c_str());
            if (frm->RappAll->Checked) rx = ry = -1;
            bool rappcolors = frm->RappColors->Checked;
            RapportKette (rx==0 ? 1 : rx, rappcolors);
            RapportSchuss (ry==0 ? 1 : ry, rappcolors);

//            if (EzFixiert->Checked) //xxxx ??
//                RecalcAll();

            CalcRangeSchuesse();
            CalcRangeKette();
            CalcRapport();

            UpdateStatusBar();
            SetModified();
            Invalidate();

            dbw3_assert (undo!=0);
            undo->Snapshot();
        }
        delete frm;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RappReduzierenClick(TObject *Sender)
{
    RapportKette (1, false);
    RapportSchuss (1, false);
    if (EzFixiert->Checked) //xxxx ??
        RecalcAll();

    CalcRangeSchuesse();
    CalcRangeKette();
    CalcRapport();

    UpdateStatusBar();
    SetModified();
    Invalidate();

    dbw3_assert (undo!=0);
    undo->Snapshot();
#if(0)
	// Rapport ausrechnen
	UpdateRapport();

	// Alles ausserhalb löschen
	int i, j;
    // Einzug
	for (i=0; i<rapport.kr.a; i++) {
		einzug.feld.Set(i, 0);
	}
	for (i=rapport.kr.b+1; i<Data->MAXX1; i++) {
		einzug.feld.Set (i, 0);
	}
	// Trittfolge
	for (j=0; j<rapport.sr.a; j++) {
		for (i=0; i<Data->MAXX2; i++) trittfolge.feld.Set (i, j, 0);
		trittfolge.isempty.Set (j, true);
	}
	for (j=rapport.sr.b+1; j<Data->MAXY2; j++) {
		for (i=0; i<Data->MAXX2; i++) trittfolge.feld.Set (i, j, 0);
		trittfolge.isempty.Set (j, true);
	}
    MinimizeAufknuepfung();

    CalcRangeSchuesse();
    CalcRangeKette();

	// Gewebe neu berechnen
	RecalcGewebe();

	// Neu zeichnen
	SetModified();
    UpdateStatusBar();
	Invalidate();

    dbw3_assert (undo!=0);
    undo->Snapshot();
#endif
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RappOverrideClick(TObject *Sender)
{
    if (!rapport.overridden) {
        if (selection.Valid() && selection.feld==GEWEBE) {
            RANGE save = selection;
            selection.Normalize();
            if (RappViewRapport->Checked) ClearRapport();
            rapport.overridden = true;
            RAPPORT old = rapport;
            rapport.kr.a = selection.begin.i;
            rapport.kr.b = selection.end.i;
            rapport.sr.a = selection.begin.j;
            rapport.sr.b = selection.end.j;
            RappOverride->Caption = RECALCRAPPORT;
            if (RappViewRapport->Checked) {
                DrawRapport();
                dbw3_assert(rapporthandler);
                rapporthandler->DrawDifferences (old, rapport);
            }
            ClearSelection();
        }
    } else {
        rapport.overridden = false;
        UpdateRapport();
        RappOverride->Caption = SETRAPPORTTOSELECTION;
    }
    UpdateStatusBar();
}
/*-----------------------------------------------------------------*/

