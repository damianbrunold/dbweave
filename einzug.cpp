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
#include "einzugimpl.h"
#include "undoredo.h"
#include "recalc.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ClearEinzugClick(TObject *Sender)
{
    for (int i=0; i<Data->MAXX1; i++) {
        einzug.feld.Set (i, 0);
        for (int j=0; j<Data->MAXY2; j++)
            gewebe.feld.Set (i, j, 0);
    }

    CalcRangeSchuesse();
    CalcRangeKette();
    RecalcFreieSchaefte();
    UpdateRapport();
    Invalidate();
    SetModified();

    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EzSpiegelnClick(TObject *Sender)
{
    EzBelassen->Checked = true;
    for (int i=kette.a; i<=(kette.b-kette.a)/2; i++) {
        short s = einzug.feld.Get (i);
        einzug.feld.Set (i, einzug.feld.Get(kette.b-(i-kette.a)));
        einzug.feld.Set (kette.b-(i-kette.a), s);
        for (int j=schuesse.a; j<=schuesse.b; j++) {
            char s = gewebe.feld.Get (i, j);
            gewebe.feld.Set (i, j, gewebe.feld.Get (kette.b-(i-kette.a), j));
            gewebe.feld.Set (kette.b-(i-kette.a), j, s);
        }
    }

    UpdateRapport();
    Invalidate();
    SetModified();

    dbw3_assert(undo);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RearrangeSchaefte()
{
    dbw3_assert(einzughandler);
    einzughandler->Rearrange();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::UpdateEinzugFixiert()
{
    // Aktuellen Zustand des Einzugs als Referenz
    // in fixeinzug speichern. Beim Recalc etc.
    // werden dann (nach Möglichkeit) die Schäfte
    // gemäss diesem Muster vergeben

    dbw3_assert (fixeinzug);
    if (fixeinzug) {
        // Leeren
        memset (fixeinzug, 0, Data->MAXX1*sizeof(short));
        firstfree = 1;
        // Kopieren
        int ii = 0;
        for (int i=0; i<Data->MAXX1; i++) {
            short s = einzug.feld.Get(i);
            if (s!=0) fixeinzug[ii++] = s;
        }
        // Und schliesslich rapportieren
        for (ii=Data->MAXX1-1; ii>=0; ii--)
            if (fixeinzug[ii]!=0) break;
        int k = 0;
        fixsize = ii;
        for (int i=ii+1; i<Data->MAXX1; i++) {
            fixeinzug[i] = fixeinzug[k++];
            if (k>ii) k = 0;
        }
        // Nun noch herausfinden, welcher Schaft der erste freie ist!
        // Da eine Liste der freien Schäfte existiert, ist das einfach...
        firstfree = 0; // kein freier...
        for (int i=Data->MAXY1-1; i>=0; i--)
            if (!freieschaefte[i]) {
                firstfree = short(i+1);
                break;
            }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EzFixiertClick(TObject *Sender)
{
    EditFixeinzug(this);

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EzBelassenClick(TObject *Sender)
{
    EzBelassen->Checked = true;
    dbw3_assert(einzughandler);
    einzughandler->Belassen();
    CalcRapport();
    UpdateStatusBar();

    dbw3_assert(undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EzGeradeZClick(TObject *Sender)
{
    EzGeradeZ->Checked = true;
    dbw3_assert(einzughandler);
    einzughandler->GeradeZ();
    CalcRange();
    CalcRapport();
    UpdateStatusBar();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EzGeradeSClick(TObject *Sender)
{
    EzGeradeS->Checked = true;
    dbw3_assert(einzughandler);
    einzughandler->GeradeS();
    CalcRange();
    CalcRapport();
    UpdateStatusBar();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EzMinimalZClick(TObject *Sender)
{
    EzMinimalZ->Checked = true;
    dbw3_assert(einzughandler);
    einzughandler->NormalZ();
    CalcRange();
    CalcRapport();
    UpdateStatusBar();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EzMinimalSClick(TObject *Sender)
{
    EzMinimalS->Checked = true;
    dbw3_assert(einzughandler);
    einzughandler->NormalS();
    CalcRange();
    CalcRapport();
    UpdateStatusBar();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EzChorig2Click(TObject *Sender)
{
    EzChorig2->Checked = true;
    dbw3_assert(einzughandler);
    einzughandler->Chorig2();
    CalcRange();
    CalcRapport();
    UpdateStatusBar();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EzChorig3Click(TObject *Sender)
{
    EzChorig3->Checked = true;
    dbw3_assert(einzughandler);
    einzughandler->Chorig3();
    CalcRange();
    CalcRapport();
    UpdateStatusBar();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}
/*-----------------------------------------------------------------*/
__fastcall EinzugRearrangeImpl::EinzugRearrangeImpl (TDBWFRM* _frm, TData* _data)
: frm(_frm), data(_data)
{
}
/*-----------------------------------------------------------------*/
__fastcall EinzugRearrangeImpl::~EinzugRearrangeImpl()
{
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::NormalZ()
{
    CalcRange();
    if (j1>=j2) return;
    MergeSchaefte();
    for (int i=frm->kette.a; i<=frm->kette.b; i++) {
        int jj = frm->einzug.feld.Get(i);
        if (jj==0) continue;
        if (jj-1>j1) SwitchSchaefte (j1, frm->einzug.feld.Get(i)-1);
        j1++;
        if (j1>=j2) break;
    }
    EliminateEmptySchaft();
    frm->CalcRangeKette();
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::NormalS()
{
    CalcRange();
    if (j1>=j2) return;
    MergeSchaefte();
    for (int i=frm->kette.a; i<=frm->kette.b; i++) {
        int jj = frm->einzug.feld.Get(i);
        if (jj==0) continue;
        if (jj-1<j2) SwitchSchaefte (j2, frm->einzug.feld.Get(i)-1);
        j2--;
        if (j2<=j1) break;
    }
    EliminateEmptySchaft();
    frm->CalcRangeKette();
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::GeradeZ()
{
    CalcRange();
    if (j1>=j2) return;

    r = CalcRapportRange();
    if (data->MAXY1<(r.b-r.a+1)) return;
    int save_j1 = j1;

    MergeSchaefte();

    // Splitten
    for (int i=r.a; i<=r.b; i++) {
        int jj = frm->einzug.feld.Get(i);
        if (jj==0) continue;
        if (jj-1<=j1) {
            int newj = SplitSchaft (j1, jj-1);
            if (newj>j2) j2=newj;
        }
        j1++;
        if (j1>=j2) break;
    }

    // Sortieren
    j1 = save_j1;
    for (int i=r.a; i<=r.b; i++) {
        int jj = frm->einzug.feld.Get(i);
        if (jj==0) continue;
        if (jj-1>j1) SwitchSchaefte (j1, frm->einzug.feld.Get(i)-1);
        j1++;
        if (j1>=j2) break;
    }

    EliminateEmptySchaft();
    frm->CalcRangeKette();
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::GeradeS()
{
    CalcRange();
    if (j1>=j2) return;

    r = CalcRapportRange();
    if (data->MAXY1<(r.b-r.a+1)) return;
    int save_j1 = j1;

    MergeSchaefte();

    // Splitten
    for (int i=r.a; i<=r.b; i++) {
        int jj = frm->einzug.feld.Get(i);
        if (jj==0) continue;
        if (jj-1<=j1) {
            int newj = SplitSchaft (j1, jj-1);
            if (newj>j2) j2=newj;
        }
        j1++;
        if (j1>=j2) break;
    }

    // Sortieren
    j1 = save_j1;
    for (int i=r.a; i<=r.b; i++) {
        int jj = frm->einzug.feld.Get(i);
        if (jj==0) continue;
        if (jj-1<j2) SwitchSchaefte (j2, frm->einzug.feld.Get(i)-1);
        j2--;
        if (j2<=j1) break;
    }

    EliminateEmptySchaft();
    frm->CalcRangeKette();
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::Chorig2()
{
    if (frm->kette.b-frm->kette.a < 2) return;
    CalcRange();
    MergeSchaefte();
    r = CalcRapportRange();
    // Ggf. Schäfte splitten
    for (int i=frm->kette.a+1; i<=frm->kette.b; i+=2) {
        short s = frm->einzug.feld.Get(i);
        if (s==0) continue;
        for (int ii=frm->kette.a; ii<=frm->kette.b; ii+=2) {
            short ss = frm->einzug.feld.Get(ii);
            if (ss==0) continue;
            if (s==ss) SplitSchaft (0, s-1);
        }
    }
    // Chore auseinanderbeineln
    short endchor1 = 0;
    for (int i=frm->kette.a; i<=frm->kette.b; i+=2) {
        short s = frm->einzug.feld.Get(i);
        if (s==0) continue;
        if (s>endchor1) {
            endchor1++;
            if (s!=endchor1) SwitchSchaefte (s-1, endchor1-1);
        }
    }
    // Chor2 sortieren
    int max = endchor1;
    for (int i=frm->kette.a+1; i<=frm->kette.b; i+=2) {
        short s = frm->einzug.feld.Get(i);
        if (s==0) continue;
        if (s>max) {
            max++;
            if (s!=max) SwitchSchaefte (s-1, max-1);
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::Chorig3()
{
    if (frm->kette.b-frm->kette.a < 3) return;
    CalcRange();
    MergeSchaefte();
    r = CalcRapportRange();
    // Ggf. Schäfte splitten
    for (int i=frm->kette.a+1; i<=frm->kette.b; i+=3) {
        short s = frm->einzug.feld.Get(i);
        if (s==0) continue;
        for (int ii=frm->kette.a; ii<=frm->kette.b; ii+=3) {
            short ss = frm->einzug.feld.Get(ii);
            if (ss==0) continue;
            if (s==ss) SplitSchaft (0, s-1);
        }
    }
    for (int i=frm->kette.a+2; i<=frm->kette.b; i+=3) {
        short s = frm->einzug.feld.Get(i);
        if (s==0) continue;
        for (int ii=frm->kette.a+1; ii<=frm->kette.b; ii+=3) {
            short ss = frm->einzug.feld.Get(ii);
            if (ss==0) continue;
            if (s==ss) SplitSchaft (0, s-1);
        }
        for (int ii=frm->kette.a; ii<=frm->kette.b; ii+=3) {
            short ss = frm->einzug.feld.Get(ii);
            if (ss==0) continue;
            if (s==ss) SplitSchaft (0, s-1);
        }
    }
    // Chore auseinanderbeineln
    short endchor1 = 0;
    for (int i=frm->kette.a; i<=frm->kette.b; i+=3) {
        short s = frm->einzug.feld.Get(i);
        if (s==0) continue;
        if (s>endchor1) {
            endchor1++;
            if (s!=endchor1) SwitchSchaefte (s-1, endchor1-1);
        }
    }
    short endchor2 = endchor1;
    for (int i=frm->kette.a+1; i<=frm->kette.b; i+=3) {
        short s = frm->einzug.feld.Get(i);
        if (s==0) continue;
        if (s>endchor2) {
            endchor2++;
            if (s!=endchor2) SwitchSchaefte (s-1, endchor2-1);
        }
    }
    // Chor3 sortieren
    int max = endchor2;
    for (int i=frm->kette.a+2; i<=frm->kette.b; i+=3) {
        short s = frm->einzug.feld.Get(i);
        if (s==0) continue;
        if (s>max) {
            max++;
            if (s!=max) SwitchSchaefte (s-1, max-1);
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::Fixiert()
{
    try {
        // Rapport bestimmen
//        r = CalcRapportRange();

        // Bildschirm löschen
        frm->_ClearEinzug();
        if (!frm->ViewSchlagpatrone->Checked) frm->_ClearAufknuepfung();
        else frm->_ClearSchlagpatrone();

        // Einzug neuberechnen
        RcRecalcAll reclc (frm, Data, frm->ViewSchlagpatrone->Checked);
        reclc.RecalcEinzugFixiert();

        // Range neu
        frm->CalcRangeSchuesse();
        frm->CalcRangeKette();

        // Aufknüpfung oder Schlagpatrone neu
        reclc.CalcK();
        reclc.CalcS();
        if (!frm->ViewSchlagpatrone->Checked) reclc.RecalcAufknuepfung();
        else reclc.RecalcSchlagpatrone();
        frm->RecalcFreieSchaefte();
        frm->RecalcFreieTritte();

        // Alles neuzeichnen
        frm->_DrawEinzug();
        if (!frm->ViewSchlagpatrone->Checked) frm->_DrawAufknuepfung();
        else frm->_DrawSchlagpatrone();
    } catch(...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::Belassen()
{
    // Nichts tun...
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::Rearrange()
{
    if (frm->EzFixiert->Checked) Fixiert();
    else if (frm->EzMinimalZ->Checked) NormalZ();
    else if (frm->EzMinimalS->Checked) NormalS();
    else if (frm->EzGeradeZ->Checked) GeradeZ();
    else if (frm->EzGeradeS->Checked) GeradeS();
    else if (frm->EzBelassen->Checked) Belassen();
    else if (frm->EzChorig2->Checked) Chorig2();
    else if (frm->EzChorig3->Checked) Chorig3();
    else dbw3_assert(false);
}
/*-----------------------------------------------------------------*/
bool __fastcall EinzugRearrangeImpl::EinzugEqual (int _i1, int _i2)
{
    for (int j=frm->schuesse.a; j<=frm->schuesse.b; j++)
        if (frm->gewebe.feld.Get(_i1, j)!=frm->gewebe.feld.Get(_i2, j)) return false;
    return true;
}
/*-----------------------------------------------------------------*/
SZ __fastcall EinzugRearrangeImpl::CalcRapportRange()
{
    SZ r(0, -1);

    int i;
    int i1 = frm->kette.a;
    int i2 = frm->kette.b;
    if (i1==-1 || i2==-1) return r;

    // Kettrapport bestimmen
    r.a = i1;
    r.b = i1;
g_again:
    // potentiellen Rapportanfang finden
    for (i=r.b+1; i<=i2; i++) {
        if (EinzugEqual (r.a, i))
            break;
    }
    r.b = i-1;
    // Prüfen ob es tatsächlich Rapport ist
    for (i=r.b+1; i<=i2; i++) {
        if (!EinzugEqual (r.a+(i-r.b-1)%(r.b-r.a+1), i)) {
            r.b++;
            goto g_again;
        }
    }

    return r;
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::CalcRange()
{
    j1 = j2 = 0;

    for (int i=0; i<Data->MAXX1; i++) {
        int e = frm->einzug.feld.Get(i);
        if (e>0) {
            if (e-1<j1) j1 = e-1;
            if (e-1>j2) j2 = e-1;
        }
    }
}
/*-----------------------------------------------------------------*/
bool __fastcall EinzugRearrangeImpl::IsEmptySchaft (int _j)
{
    return frm->freieschaefte[_j];
}
/*-----------------------------------------------------------------*/
int __fastcall EinzugRearrangeImpl::GetFirstNonemptySchaft (int _j)
{
    for (int j=_j+1; j<data->MAXY1; j++)
        if (!IsEmptySchaft (j)) return j;
    return -1;
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::RedrawSchaft (int _j)
{
    if (!frm->ViewEinzug->Checked) return;
    if (_j<frm->scroll_y1 || _j>=frm->scroll_y1+frm->einzug.pos.height/frm->einzug.gh) return; // nicht sichtbar
    for (int i=0; i<frm->einzug.pos.width/frm->einzug.gw; i++)
        frm->DrawEinzug (i, _j-frm->scroll_y1);
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::RedrawAufknuepfungSchaft (int _j)
{
    if (!frm->ViewEinzug->Checked || !frm->ViewTrittfolge->Checked) return;
    if (_j<frm->scroll_y1 || _j>=frm->scroll_y1+frm->einzug.pos.height/frm->einzug.gh) return; // nicht sichtbar
    for (int i=frm->scroll_x2; i<frm->scroll_x2+frm->aufknuepfung.pos.width/frm->einzug.gw; i++)
        frm->DrawAufknuepfung (i-frm->scroll_x2, _j-frm->scroll_y1);
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::RedrawSchlagpatrone (int _i)
{
    if (!frm->ViewTrittfolge->Checked) return;
    if (_i<frm->scroll_x2 || _i>=frm->scroll_x2+frm->trittfolge.pos.width/frm->trittfolge.gw) return; // nicht sichtbar
    for (int j=frm->scroll_y2; j<frm->scroll_y2+frm->trittfolge.pos.height/frm->trittfolge.gh; j++)
          frm->DrawTrittfolge (_i-frm->scroll_x2, j-frm->scroll_y2);
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::MoveSchaft (int _von, int _nach)
{
    int i;
    for (i=frm->kette.a; i<=frm->kette.b; i++)
        if (frm->einzug.feld.Get(i)==_von+1) frm->einzug.feld.Set(i, (short)(_nach+1));

    frm->freieschaefte[_nach] = frm->freieschaefte[_von];
    frm->freieschaefte[_von] = true;

    if (!frm->ViewSchlagpatrone->Checked) {
        for (i=0; i<data->MAXX2; i++) {
            frm->aufknuepfung.feld.Set (i, _nach, frm->aufknuepfung.feld.Get (i, _von));
            frm->aufknuepfung.feld.Set (i, _von, 0);
        }
        RedrawAufknuepfungSchaft (_von);
        RedrawAufknuepfungSchaft (_nach);
    } else {
        for (int j=0; j<data->MAXY2; j++) {
            frm->trittfolge.feld.Set (_nach, j, frm->trittfolge.feld.Get (_von, j));
            frm->trittfolge.feld.Set (_von,  j, 0);
        }
        RedrawSchlagpatrone (_von);
        RedrawSchlagpatrone (_nach);
        dbw3_assert(_nach<data->MAXX2);
        dbw3_assert(_von<data->MAXX2);
        frm->freietritte[_nach] = frm->freietritte[_von];
        frm->freietritte[_von] = true;
    }

    RedrawSchaft (_von);
    RedrawSchaft (_nach);
}
/*-----------------------------------------------------------------*/
bool __fastcall EinzugRearrangeImpl::IsTotalEmptySchaft (int _j)
{
    if (!frm->ViewSchlagpatrone->Checked) {
        for (int i=0; i<Data->MAXX2; i++)
            if (frm->aufknuepfung.feld.Get(i, _j)>0)
//                return false;
                for (int j=0; j<Data->MAXY2; j++)
                    if (frm->trittfolge.feld.Get(i, j)>0)
                        return false;
        return true;
    } else {
        for (int i=0; i<Data->MAXY2; i++) {
            char s = frm->trittfolge.feld.Get (_j, i);
            if (s>0) return false;
        }
        return true;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::EliminateEmptySchaft()
{
    for (int j=0; j<data->MAXY1; j++)
        if (IsTotalEmptySchaft(j)) {
            for (int i=0; i<Data->MAXX1; i++)
                if (frm->einzug.feld.Get(i)==j+1) {
                    frm->einzug.feld.Set (i, 0);
                    if (i>=frm->scroll_x1 && i<frm->scroll_x1+frm->einzug.pos.width/frm->einzug.gw)
                        frm->DrawEinzug (i-frm->scroll_x1, j-frm->scroll_y1);
                }
            if (!frm->ViewSchlagpatrone->Checked) {
                for (int i=0; i<Data->MAXX2; i++)
                    if (frm->aufknuepfung.feld.Get(i, j)>0) {
                        frm->aufknuepfung.feld.Set (i, j, 0);
                        if (i>=frm->scroll_x2 && i<frm->scroll_x2+frm->aufknuepfung.pos.width/frm->aufknuepfung.gw)
                            frm->DrawAufknuepfung (i-frm->scroll_x2, j-frm->scroll_y1);
                    }
            }
            frm->freieschaefte[j] = true;
            int firstnonempty = GetFirstNonemptySchaft(j);
            if (firstnonempty==-1 || firstnonempty<j)
                return;
            MoveSchaft (firstnonempty, j);
        } else if (IsEmptySchaft (j)) {
            int firstnonempty = GetFirstNonemptySchaft(j);
            if (firstnonempty==-1 || firstnonempty<j) {
                if (!frm->ViewSchlagpatrone->Checked) {
                    for (int i=0; i<Data->MAXX2; i++) {
                        frm->aufknuepfung.feld.Set (i, j, 0);
                        if (i>=frm->scroll_x2 && i<frm->scroll_x2+frm->aufknuepfung.pos.width/frm->aufknuepfung.gw)
                           frm->DrawAufknuepfung (i-frm->scroll_x2, j-frm->scroll_y1);
                    }
                } else {
                    for (int i=0; i<Data->MAXY2; i++) {
                        if (j<Data->MAXX2) frm->trittfolge.feld.Set (j, i, 0);
                        frm->trittfolge.isempty.Set (i, true);
                        for (int ii=0; ii<Data->MAXX2; ii++)
                            if (frm->trittfolge.feld.Get(ii, i)>0) {
                                frm->trittfolge.isempty.Set (i, false);
                                break;
                            }
                        if (j<Data->MAXX2) frm->freietritte[j] = true;
                        if (i>=frm->scroll_y2 && i<frm->scroll_y2+frm->trittfolge.pos.height/frm->trittfolge.gh)
                            frm->DrawTrittfolge (j-frm->scroll_x2, i-frm->scroll_y2);
                    }
                }
                return;
            }
            MoveSchaft (firstnonempty, j);
        }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EliminateEmptySchaft()
{
    dbw3_assert(einzughandler);
    einzughandler->EliminateEmptySchaft();
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::SwitchSchaefte (int _a, int _b)
{
    int i;
    for (i=frm->kette.a; i<=frm->kette.b; i++) {
        if (frm->einzug.feld.Get(i)==_a+1) frm->einzug.feld.Set(i, (short)(_b+1));
        else if (frm->einzug.feld.Get(i)==_b+1) frm->einzug.feld.Set(i, (short)(_a+1));
    }

    bool bEmpty = frm->freieschaefte[_a];
    frm->freieschaefte[_a] = frm->freieschaefte[_b];
    frm->freieschaefte[_b] = bEmpty;

    if (!frm->ViewSchlagpatrone->Checked) {
        for (i=0; i<data->MAXX2; i++) {
            short temp = frm->aufknuepfung.feld.Get (i, _a);
            frm->aufknuepfung.feld.Set (i, _a, frm->aufknuepfung.feld.Get (i, _b));
            frm->aufknuepfung.feld.Set (i, _b, temp);
        }
        RedrawAufknuepfungSchaft (_a);
        RedrawAufknuepfungSchaft (_b);
    } else {
        for (int j=0; j<data->MAXY2; j++) {
            short temp = frm->trittfolge.feld.Get (_a, j);
            frm->trittfolge.feld.Set (_a, j, frm->trittfolge.feld.Get (_b, j));
            frm->trittfolge.feld.Set (_b, j, temp);
        }
        RedrawSchlagpatrone (_a);
        RedrawSchlagpatrone (_b);
        dbw3_assert(_a<data->MAXX2);
        dbw3_assert(_b<data->MAXX2);
        bool bEmpty = frm->freietritte[_a];
        frm->freietritte[_a] = frm->freietritte[_b];
        frm->freietritte[_b] = bEmpty;
    }

    RedrawSchaft (_a);
    RedrawSchaft (_b);
}
/*-----------------------------------------------------------------*/
// SplitSchaft: Um einen Einzug genau aufsteigend zu machen, kann
// es noetig sein, dass ein Einzug, der eigentlich schon vorhanden
// waere, nicht genommen wird, sondern identisch auf einen anderen
// Schaft gelegt wird.
int __fastcall EinzugRearrangeImpl::SplitSchaft (int _searchj, int _sourcej)
{
	int lastnewj = 0;
	// Rapport durchgehen
    int counter = 0;
	for (int i=r.a; i<=r.b; i++) {
		if (frm->einzug.feld.Get(i)-1==_sourcej) {
			// Ab zweitem Punkt
			if (counter>0) {
				// einen leeren Einzug suchen
				int j;
				for (j=_searchj; j<data->MAXY1; j++) {
					if (IsEmptySchaft(j)) {
						lastnewj = j;
                        // alle entsprechenden Punkte im Einzug transferieren
                        int x = i;
						while (x<data->MAXX1) {
							if (frm->einzug.feld.Get(x)-1==_sourcej) {
								frm->einzug.feld.Set (x, (short)(j+1));
                                frm->freieschaefte[j] = false;
                            }
							x += (r.b - r.a + 1);
						}
                        if (!frm->ViewSchlagpatrone->Checked) {
                            // Aufknuepfung kopieren
                            for (int i=0; i<data->MAXX2; i++)
                                frm->aufknuepfung.feld.Set (i, j, frm->aufknuepfung.feld.Get(i,_sourcej));
                            // Neuen Schaft neuzeichnen
                            RedrawAufknuepfungSchaft (j);
                        } else {
                            // Trittfolge kopieren
                            for (int k=0; k<data->MAXY2; k++) {
                                char s = frm->trittfolge.feld.Get(_sourcej, k);
                                frm->trittfolge.feld.Set (j, k, s);
                                if (s>0) {
                                    dbw3_assert(j<Data->MAXX2);
                                    frm->freietritte[j] = false;
                                }
                            }
                            // Neuen 'Schaft' neuzeichnen
                            RedrawSchlagpatrone (j);
                        }
                        RedrawSchaft (j);
						break;
					}
				}
			}
			++counter;
		}
	}

	// Neuzeichnen
    RedrawSchaft (_sourcej);
    if (!frm->ViewSchlagpatrone->Checked) RedrawAufknuepfungSchaft (_sourcej);
    else RedrawSchlagpatrone (_sourcej);
	return lastnewj;
}
/*-----------------------------------------------------------------*/
bool __fastcall EinzugRearrangeImpl::SchaefteEqual (int _j1, int _j2)
{
    if (!frm->ViewSchlagpatrone->Checked) {
        dbw3_assert(_j1>=0 && _j1<data->MAXY1);
        dbw3_assert(_j2>=0 && _j2<data->MAXY1);
        for (int i=0; i<data->MAXX2; i++) {
            int a1 = frm->aufknuepfung.feld.Get(i, _j1);
            int a2 = frm->aufknuepfung.feld.Get(i, _j2);
            if ((a1>0 && a2<=0) || (a1<=0 && a2>0))
                return false;
            if (a1>0 && a2>0 && a1!=a2)
                return false;
        }
        return true;
    } else {
        dbw3_assert(_j1>=0 && _j1<data->MAXX2);
        dbw3_assert(_j2>=0 && _j2<data->MAXX2);
        for (int j=0; j<data->MAXY2; j++) {
            int t1 = frm->trittfolge.feld.Get (_j1, j);
            int t2 = frm->trittfolge.feld.Get(_j2, j);
            if ((t1>0 && t2<=0) || (t1<=0 && t2>0))
                return false;
            if (t1>0 && t2>0 && t1!=t2)
                return false;
        }
        return true;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrangeImpl::MergeSchaefte()
{
    // Identische Schaefte suchen
    for (int i=frm->kette.a; i<=frm->kette.b; i++) {
        for (int ii=frm->kette.a; ii<i; ii++) {
            int jj1 = frm->einzug.feld.Get(i);
            int jj2 = frm->einzug.feld.Get(ii);
            if (jj1!=jj2 && jj1!=0 && jj2!=0) {
                if (SchaefteEqual (jj1-1, jj2-1)) {
                    int j1 = frm->einzug.feld.Get(i)-1;
                    int j2 = frm->einzug.feld.Get(ii)-1;
                    if (j1>j2) { int temp=j1; j1=j2; j2=temp; }
                    for (int i=frm->kette.a; i<=frm->kette.b; i++)
                        if (frm->einzug.feld.Get(i)-1==j2)
                            frm->einzug.feld.Set(i, (short)(j1+1));
                    if (!frm->ViewSchlagpatrone->Checked) {
                        for (int i=0; i<data->MAXX2; i++)
                            frm->aufknuepfung.feld.Set (i, j2, 0);
                        RedrawAufknuepfungSchaft (j2);
                    } else {
                        for (int j=0; j<data->MAXY2; j++)
                            frm->trittfolge.feld.Set (j2, j, 0);
                        RedrawSchlagpatrone (j2);
                        dbw3_assert(j2<Data->MAXX2);
                        frm->freietritte[j2] = true;
                    }
                    RedrawSchaft (j1);
                    RedrawSchaft (j2);
                    frm->freieschaefte[j2] = true;
                }
            }
        }
    }
}
/*-----------------------------------------------------------------*/
EinzugRearrange* __fastcall EinzugRearrange::CreateInstance (TDBWFRM* _frm, TData* _data)
{
    EinzugRearrange* p = NULL;
    try {
        p = new EinzugRearrangeImpl (_frm, _data);
    } catch(...) {
    }
    return p;
}
/*-----------------------------------------------------------------*/
void __fastcall EinzugRearrange::ReleaseInstance (EinzugRearrange* _einzug)
{
    delete _einzug;
}
/*-----------------------------------------------------------------*/

