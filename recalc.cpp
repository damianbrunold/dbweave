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

#include <vcl\vcl.h>
#include <mem.h>
#pragma hdrstop

#include "assert.h"
#include "dbw3_form.h"
#include "datamodule.h"
#include "felddef.h"
#include "dbw3_strings.h"
#include "recalc.h"

void __fastcall TDBWFRM::RecalcFreieSchaefte()
{
    dbw3_assert(freieschaefte);
    if (!freieschaefte) return;
    for (int i=0; i<Data->MAXY1; i++)
        freieschaefte[i] = true;
    for (int i=0; i<Data->MAXX1; i++)
        if (einzug.feld.Get(i)!=0)
            freieschaefte[einzug.feld.Get(i)-1] = false;
}

void __fastcall TDBWFRM::RecalcFreieTritte()
{
    dbw3_assert(freietritte);
    if (!freietritte) return;
    for (int i=0; i<Data->MAXX2; i++)
        freietritte[i] = true;
    for (int i=0; i<Data->MAXX2; i++)
        for (int j=0; j<Data->MAXY2; j++)
            if (trittfolge.feld.Get(i, j)>0)
                freietritte[i] = false;
}

// RecalcGewebe: Berechnet ausgehend von Einzug, Trittfolge und
// Aufknuepfung das Gewebe komplett neu.
void __fastcall TDBWFRM::RecalcGewebe()
{
    TCursor oldcursor = Cursor;
    Cursor = crHourGlass;
    gewebe.Clear();

    if (!ViewSchlagpatrone->Checked) {
        for (int i=0; i<Data->MAXX1; i++) {
            short n = einzug.feld.Get(i);
            if (n!=0) {
                for (int k=0; k<Data->MAXX2; k++) {
                    char s = aufknuepfung.feld.Get (k, n-1);
                    if (s>0) {
                        for (int m=0; m<Data->MAXY2; m++)
                            if (trittfolge.feld.Get(k, m)>0)
                                gewebe.feld.Set (i, m, s);
                    }
                }
            }
        }
    } else {
        for (int i=0; i<Data->MAXX1; i++) {
            short n = einzug.feld.Get(i);
            if (n!=0) {
                for (int k=0; k<Data->MAXY2; k++) {
                    char s = trittfolge.feld.Get (n-1, k);
                    if (s>0)
                        gewebe.feld.Set (i, k, s);
                }
            }
        }
    }

    RecalcFreieSchaefte();
    RecalcFreieTritte();

    Cursor = oldcursor;
    InvalidateFeld (gewebe.pos);
}

__fastcall RcRecalcAll::RcRecalcAll (TDBWFRM* _frm, TData* _data, bool _schlagpatrone/*=false*/)
{
	dbw3_assert (_frm!=0);
	dbw3_assert (_data!=0);
	frm  = _frm;
	data = _data;
    schlagpatrone = _schlagpatrone;
}

// Achtung: Undo muss vom Aufrufer gehandelt werden!
void __fastcall RcRecalcAll::Recalc()
{
	dbw3_assert(frm!=0);
	if (!frm->EzFixiert->Checked) RecalcEinzug();
    else RecalcEinzugFixiert();
	RecalcTrittfolge();
	RecalcAufknuepfung();

    if (!schlagpatrone) {
    	if (!frm->EzFixiert->Checked)
            frm->RearrangeSchaefte();
        frm->RearrangeTritte();
    }

//    frm->RecalcFreieSchaefte(); // wird schon in RecalcEinzug bzw. RecalcEinzugFixiert gemacht
    frm->RecalcFreieTritte();

	frm->SetModified();
    frm->InvalidateFeld (frm->einzug.pos);
    frm->InvalidateFeld (frm->aufknuepfung.pos);
    frm->InvalidateFeld (frm->trittfolge.pos);
}

void __fastcall RcRecalcAll::CalcK()
{
    k1 = frm->kette.a;
    k2 = frm->kette.b;
}

void __fastcall RcRecalcAll::CalcS()
{
    s1 = frm->schuesse.a;
    s2 = frm->schuesse.b;
}

void __fastcall RcRecalcAll::RecalcSchlagpatrone()
{
	dbw3_assert(frm!=0);
    CalcK();
	RecalcTrittfolge();
	RecalcAufknuepfung();
    frm->RecalcFreieTritte();
	frm->SetModified();
    frm->InvalidateFeld (frm->aufknuepfung.pos);
    frm->InvalidateFeld (frm->trittfolge.pos);
}

void __fastcall RcRecalcAll::RecalcTrittfolgeAufknuepfung()
{
	dbw3_assert(frm!=0);
    CalcK();
	RecalcTrittfolge();
	RecalcAufknuepfung();
    if (!schlagpatrone) frm->RearrangeTritte();
    frm->RecalcFreieTritte();
	frm->SetModified();
    frm->InvalidateFeld (frm->aufknuepfung.pos);
    frm->InvalidateFeld (frm->trittfolge.pos);
}

bool __fastcall RcRecalcAll::KettfadenEqual (int _a, int _b)
{
    if (frm->xbuf[_a]==0 && frm->xbuf[_b]==0) return true;
    for (int j=0; j<data->MAXY2; j++) {
        char s1 = frm->gewebe.feld.Get (_a, j);
        char s2 = frm->gewebe.feld.Get (_b, j);
        if ((s1<=0 && s2>0) || (s1>0 && s2<=0)) return false;
        if (s1>0 && s2>0 && s1!=s2) return false;
    }
    return true;
}

bool __fastcall RcRecalcAll::SchussfadenEqual (int _a, int _b)
{
    if (frm->ybuf[_a]==0 && frm->ybuf[_b]==0) return true;
    for (int i=k1; i<=k2; i++) {
        char s1 = frm->gewebe.feld.Get (i, _a);
        char s2 = frm->gewebe.feld.Get (i, _b);
        if ((s1<=0 && s2>0) || (s1>0 && s2<=0)) return false;
        if (s1>0 && s2>0 && s1!=s2) return false;
    }
    return true;
}

bool __fastcall RcRecalcAll::KettfadenEmpty (int _a)
{
    for (int j=0; j<data->MAXY2; j++)
        if (frm->gewebe.feld.Get (_a, j)>0) return false;
    return true;
}

bool __fastcall RcRecalcAll::SchussfadenEmpty (int _a)
{
    for (int i=k1; i<=k2; i++)
        if (frm->gewebe.feld.Get (i, _a)>0) return false;
    return true;
}

short __fastcall RcRecalcAll::GetSchaft (int _a)
{
    dbw3_assert(frm);
    dbw3_assert(frm->fixeinzug);
    return frm->fixeinzug[_a];
}

void __fastcall RcRecalcAll::RecalcEinzug()
{
	// Einzug löschen und neuberechnen
    frm->einzug.feld.Init(0);
    memset (frm->xbuf, 0, data->MAXX1);
    int first = 0;
	short j = 0;
    k1 = k2 = 0;

    for (int i=0; i<data->MAXY1; i++) frm->freieschaefte[i] = true;

    // Ersten belegten Kettfaden finden
    for (int i=0; i<data->MAXX1; i++)
        if (!KettfadenEmpty(i)) {
            frm->xbuf[i] = 1;
            k1 = k2 = first = i;
            frm->einzug.feld.Set (i, ++j);
            frm->freieschaefte[j-1] = false;
            break;
        }

    // Weitere Schäfte vergeben
    bool rapwrong = false;
	for (int i=first+1; i<data->MAXX1; i++) {
        // Schaft vergeben
        if (!KettfadenEmpty (i)) {
            frm->xbuf[i] = 1;
            if (rapwrong) { k2 = i-1; rapwrong = false; }
            bool ok = false;
            for (int k=k1; k<=k2; k++) {
                if (KettfadenEqual (i, k)) {
                    frm->einzug.feld.Set (i, frm->einzug.feld.Get(k));
                    ok = true;
                    break;
                }
            }
            if (!ok) {
                frm->einzug.feld.Set (i, ++j);
                frm->freieschaefte[j-1] = false;
            }
        }

        if (j+2>data->MAXY1) frm->ExtendSchaefte();

        // Rapport aktualisieren
        if (!KettfadenEqual (i, k1+i%(k2-k1+1))) {
            if (frm->xbuf[i]==0) rapwrong = true;
            else k2 = i;
        }
	}
}

void __fastcall RcRecalcAll::RecalcEinzugFixiert()
{
    // Ist überhaupt ein fixierter Einzug da?!
    if (!frm->fixeinzug || frm->fixeinzug[0]==0) {
        RecalcEinzug();
        return;
    }

	// Einzug löschen und neuberechnen
    frm->einzug.feld.Init(0);
    memset (frm->xbuf, 0, data->MAXX1);
    for (int i=0; i<Data->MAXY1; i++) frm->freieschaefte[i] = true;
    k1 = Data->MAXX1-1;
    k2 = 0;
    short firstfree = frm->firstfree;

    // durch kette gehen
    // Falls faden nicht leer
    //   wenn verlangter schaft leer, dann diesen nehmen
    //   sonst einen suchen, der denselben hat und vergleich
    //     wenn gleich, dann ok
    //     sonst wirklich freien schaft nehmen

    int k = 0;
    for (int i=0; i<data->MAXX1; i++) {
        if (!KettfadenEmpty(i)) {
            if (k1>i) k1 = i;
            if (k2<i) k2 = i;
            frm->xbuf[i] = 1;
            short s = GetSchaft (k++);
            dbw3_assert(s>0);
            if (frm->freieschaefte[s-1]) {
                frm->einzug.feld.Set (i, s);
                frm->freieschaefte[s-1] = false;
            } else {
                bool done = false;
                for (int m=0; m<i; m++)
                    if (frm->einzug.feld.Get(m)==s) {
                        if (KettfadenEqual(i, m)) {
                            frm->einzug.feld.Set (i, s);
                            done = true;
                            break;
                        }
                    } else if (KettfadenEqual(i, m) && frm->einzug.feld.Get(m)>frm->firstfree) {
                        frm->einzug.feld.Set (i, frm->einzug.feld.Get(m));
                        done = true;
                        break;                        
                    }
                if (!done) {
                    frm->einzug.feld.Set (i, short(firstfree+1));
                    frm->freieschaefte[firstfree] = false;
                    firstfree++;
                    if (firstfree>=data->MAXY1) frm->ExtendSchaefte();
                }
            }
        }
    }
}

void __fastcall RcRecalcAll::RecalcTrittfolge()
{
	// Trittfolge löschen und neuberechnen
    frm->trittfolge.feld.Init(0);
    frm->trittfolge.isempty.Init (true);
    if (!schlagpatrone) {
        memset (frm->ybuf, 0, data->MAXY2);
        int first = 0;
        short i = -1;
        s1 = s2 = 0;

        // Ersten belegten Schussfaden finden
        for (int j=0; j<data->MAXY2; j++)
            if (!SchussfadenEmpty(j)) {
                frm->ybuf[j] = 1;
                s1 = s2 = first = j;
                frm->trittfolge.feld.Set (++i, j, 1);
                frm->trittfolge.isempty.Set (j, false);
                break;
            }

        // Weitere Tritte vergeben
        bool rapwrong = false;
        for (int j=first+1; j<data->MAXY2; j++) {
            // Tritt vergeben
            if (!SchussfadenEmpty (j)) {
                frm->ybuf[j] = 1;
                if (rapwrong) { s2 = j-1; rapwrong = false; }
                bool ok = false;
                for (int k=s1; k<=s2; k++) {
                    if (SchussfadenEqual (j, k)) {
                        for (int m=0; m<data->MAXX2; m++)
                            frm->trittfolge.feld.Set (m, j, frm->trittfolge.feld.Get (m, k));
                        frm->trittfolge.isempty.Set (j, frm->trittfolge.isempty.Get(k));
                        ok = true;
                        break;
                    }
                }
                if (!ok) {
                    frm->trittfolge.feld.Set (++i, j, 1);
                    frm->trittfolge.isempty.Set (j, false);
                }
            }

            if (i+2>data->MAXX2) frm->ExtendTritte();

            // Rapport aktualisieren
            if (!SchussfadenEqual (j, s1+j%(s2-s1+1))) {
                if (frm->ybuf[j]==0) rapwrong = true;
                else s2 = j;
            }
        }
    } else {
        for (int j=0; j<data->MAXY2; j++) {
            for (int ii=k1; ii<=k2; ii++) {
                char s = frm->gewebe.feld.Get(ii, j);
                if (s>0) {
                    int jj = frm->einzug.feld.Get(ii);
                    if (jj!=0) {
                        if (jj>=data->MAXX2) frm->ExtendTritte();
                        frm->trittfolge.feld.Set (jj-1, j, s);
                        frm->trittfolge.isempty.Set (j, false);
                    }
                }
            }
        }
    }
}

void __fastcall RcRecalcAll::RecalcAufknuepfung()
{
	int a, b, c;

    frm->aufknuepfung.feld.Init (0);

    if (!schlagpatrone) {
//        for (a=k1; a<=k2; a++)
        for (a=0; a<data->MAXX1; a++)
            if (frm->einzug.feld.Get(a)!=0)
                for (b=0; b<data->MAXX2; b++)
                    for (c=s1; c<=s2; c++)
                        if (frm->trittfolge.feld.Get (b, c)>0) {
                            char s = frm->gewebe.feld.Get (a, c);
                            if (s>0)
                                frm->aufknuepfung.feld.Set (b, frm->einzug.feld.Get(a)-1, s);
                        }
    } else {
        for (a=0; a<min(data->MAXX2, data->MAXY1); a++)
            frm->aufknuepfung.feld.Set (a, a, 1);
    }
}

// RecalcAll: Berechnet einen neuen Einzug, eine neue Trittfolge
// und eine neue Aufknuepfung fuer das Gewebe. Dabei wird gemaess
// den eingestellten Optionen die Schaftvergabe etc. gehandhabt.
void __fastcall TDBWFRM::RecalcAll()
{
	// Diese Funktion sollte nur für die Bereichsmanipulationen
	// eingesetzt werden. Deshalb wird das Undo hier nicht
	// nachgeführt, da es dort schon wird.

    TCursor old = Cursor;
    Cursor = crHourGlass;

	RcRecalcAll reclc (this, Data, ViewSchlagpatrone->Checked);
	reclc.Recalc();

    Cursor = old;
}

// RecalcSchlagpatrone: Berechnet Schlagpatrone
void __fastcall TDBWFRM::RecalcSchlagpatrone()
{
    TCursor old = Cursor;
    Cursor = crHourGlass;

	RcRecalcAll reclc (this, Data, ViewSchlagpatrone->Checked);
	reclc.RecalcSchlagpatrone();

    Cursor = old;
}

// RecalcTrittfolgeAufknuepfung: Berechnet Trittfolge/Aufknüpfung
void __fastcall TDBWFRM::RecalcTrittfolgeAufknuepfung()
{
    TCursor old = Cursor;
    Cursor = crHourGlass;

	RcRecalcAll reclc (this, Data, ViewSchlagpatrone->Checked);
	reclc.RecalcTrittfolgeAufknuepfung();

    Cursor = old;
}

// RecalcFixEinzug: Berechnet einen neuen Einzug gemäss
// dem gespeicherten Fixeinzug.
void __fastcall TDBWFRM::RecalcFixEinzug()
{
	RcRecalcAll reclc (this, Data, ViewSchlagpatrone->Checked);
	reclc.RecalcEinzugFixiert();
//	if (!ViewSchlagpatrone->Checked) reclc.RecalcAufknuepfung();
//    else reclc.RecalcSchlagpatrone();
}


