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
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "rapportimpl.h"
#include "dbw3_form.h"
#include "felddef.h"
#include "datamodule.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawRapport()
{
    dbw3_assert (rapporthandler);
    if (rapporthandler) rapporthandler->DrawRapport();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::CalcRapport()
{
    dbw3_assert (rapporthandler);
    if (rapporthandler) rapporthandler->CalcRapport();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::UpdateRapport()
{
    dbw3_assert (rapporthandler);
    if (rapporthandler) rapporthandler->UpdateRapport();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ClearRapport()
{
    dbw3_assert (rapporthandler);
    if (rapporthandler) rapporthandler->ClearRapport();
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::IsInRapport (int _i, int _j)
{
    dbw3_assert (rapporthandler);
    if (rapporthandler) return rapporthandler->IsInRapport(_i, _j);
    else return false;
}
/*-----------------------------------------------------------------*/
__fastcall RpRapportImpl::RpRapportImpl (TDBWFRM* _frm, TData* _data)
: frm(_frm), data(_data)
{
}
/*-----------------------------------------------------------------*/
__fastcall RpRapportImpl::~RpRapportImpl()
{
}
/*-----------------------------------------------------------------*/
bool __fastcall RpRapportImpl::IsInRapport (int _i, int _j)
{
    return _i>=frm->rapport.kr.a && _i<=frm->rapport.kr.b &&
           _j>=frm->rapport.sr.a && _j<=frm->rapport.sr.b;
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::UpdateRapport()
{
    if (frm->RappViewRapport->Checked) ClearRapport();
    RAPPORT oldrapport = frm->rapport;
    CalcRapport();
    if (frm->RappViewRapport->Checked) {
        DrawRapport();
        DrawDifferences (oldrapport, frm->rapport);
    }
    frm->UpdateStatusBar();
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::ClearRapport()
{
    int w = frm->gewebe.pos.width/frm->gewebe.gw;
    int h = frm->gewebe.pos.height/frm->gewebe.gh;

    bool changed = false;

    // Rapport in Einzug löschen
    int i1 = frm->rapport.kr.a;
    int i2 = frm->rapport.kr.b;
    if (i2>=frm->scroll_x1 && i1<=frm->scroll_x1+w) {
        if (i1<frm->scroll_x1 && i2>=frm->scroll_x1) i1 = frm->scroll_x1;
        if (i2>frm->scroll_x1+w-1 && i1<=frm->scroll_x1+w-1) i2 = frm->scroll_x1+w-1;
        int x  = frm->gewebe.pos.x0 + (i1-frm->scroll_x1)*frm->gewebe.gw;
        int xx = frm->gewebe.pos.x0 + (i2-frm->scroll_x1+1)*frm->gewebe.gw;
        if (frm->righttoleft) {
            x = frm->gewebe.pos.width - x + 2*frm->gewebe.pos.x0;
            xx = frm->gewebe.pos.width - xx + 2*frm->gewebe.pos.x0;
        }
        frm->Canvas->Pen->Color = clBtnShadow;
        frm->Canvas->MoveTo (x, frm->einzug.pos.y0);
        frm->Canvas->LineTo (x, frm->einzug.pos.y0+frm->einzug.pos.height);
        frm->Canvas->MoveTo (xx, frm->einzug.pos.y0);
        frm->Canvas->LineTo (xx, frm->einzug.pos.y0+frm->einzug.pos.height);
        changed = true;
    }

    // Rapport in Trittfolge löschen
    int j1 = frm->rapport.sr.a;
    int j2 = frm->rapport.sr.b;
    if (j2>=frm->scroll_y2 && j1<=frm->scroll_y2+h) {
        if (j1<frm->scroll_y2 && j2>=frm->scroll_y2) j1 = frm->scroll_y2;
        if (j2>frm->scroll_y2+h-1 && j1<=frm->scroll_y2+h-1) j2 = frm->scroll_y2+h-1;
        int y  = frm->gewebe.pos.y0+frm->gewebe.pos.height - (j1-frm->scroll_y2)*frm->gewebe.gh;
        int yy = frm->gewebe.pos.y0+frm->gewebe.pos.height - (j2-frm->scroll_y2+1)*frm->gewebe.gh;
        frm->Canvas->Pen->Color = clBtnShadow;
        frm->Canvas->MoveTo (frm->trittfolge.pos.x0, y);
        frm->Canvas->LineTo (frm->trittfolge.pos.x0+frm->trittfolge.pos.width, y);
        frm->Canvas->MoveTo (frm->trittfolge.pos.x0, yy);
        frm->Canvas->LineTo (frm->trittfolge.pos.x0+frm->trittfolge.pos.width, yy);
        changed = true;
    }

    // Hilfslinien neuzeichnen
    if (changed) frm->DrawHilfslinien();
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::DrawRapport()
{
    int w = frm->gewebe.pos.width/frm->gewebe.gw;
    int h = frm->gewebe.pos.height/frm->gewebe.gh;

    int i1 = frm->rapport.kr.a;
    int i2 = frm->rapport.kr.b;
    if (i2>=frm->scroll_x1 && i1<=frm->scroll_x1+w) {
        if (i1<frm->scroll_x1 && i2>=frm->scroll_x1) i1 = frm->scroll_x1;
        if (i2>frm->scroll_x1+w-1 && i1<=frm->scroll_x1+w-1) i2 = frm->scroll_x1+w-1;
        int x  = frm->gewebe.pos.x0 + (i1-frm->scroll_x1)*frm->gewebe.gw;
        int xx = frm->gewebe.pos.x0 + (i2-frm->scroll_x1+1)*frm->gewebe.gw;
        if (frm->righttoleft) {
            x = frm->gewebe.pos.width - x + 2*frm->gewebe.pos.x0;
            xx = frm->gewebe.pos.width - xx + 2*frm->gewebe.pos.x0;
        }
        frm->Canvas->Pen->Color = clRed;
        frm->Canvas->MoveTo (x, frm->einzug.pos.y0);
        frm->Canvas->LineTo (x, frm->einzug.pos.y0+frm->einzug.pos.height);
        frm->Canvas->MoveTo (xx, frm->einzug.pos.y0);
        frm->Canvas->LineTo (xx, frm->einzug.pos.y0+frm->einzug.pos.height);
    }

    int j1 = frm->rapport.sr.a;
    int j2 = frm->rapport.sr.b;
    if (j2>=frm->scroll_y2 && j1<=frm->scroll_y2+h) {
        if (j1<frm->scroll_y2 && j2>=frm->scroll_y2) j1 = frm->scroll_y2;
        if (j2>frm->scroll_y2+h-1 && j1<=frm->scroll_y2+h-1) j2 = frm->scroll_y2+h-1;
        int y  = frm->gewebe.pos.y0+frm->gewebe.pos.height - (j1-frm->scroll_y2)*frm->gewebe.gh;
        int yy = frm->gewebe.pos.y0+frm->gewebe.pos.height - (j2-frm->scroll_y2+1)*frm->gewebe.gh;
        frm->Canvas->Pen->Color = clRed;
        frm->Canvas->MoveTo (frm->trittfolge.pos.x0, y);
        frm->Canvas->LineTo (frm->trittfolge.pos.x0+frm->trittfolge.pos.width, y);
        frm->Canvas->MoveTo (frm->trittfolge.pos.x0, yy);
        frm->Canvas->LineTo (frm->trittfolge.pos.x0+frm->trittfolge.pos.width, yy);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::DrawDifferences (const RAPPORT& _old, const RAPPORT& _new)
{
    //xxx Vorerst wird die Union von altem und neuem Rapport
    //xxx neugezeichnet
    //xxx Theoretisch könnte man da ein viel ausgelklügelteres
    //xxx System aufbauen...
    RAPPORT rapp = _old;

    if (_new.kr.a < rapp.kr.a) rapp.kr.a = _new.kr.a;
    if (_new.kr.b > rapp.kr.b) rapp.kr.b = _new.kr.b;

    if (_new.sr.a < rapp.sr.a) rapp.sr.a = _new.sr.a;
    if (_new.sr.b > rapp.sr.b) rapp.sr.b = _new.sr.b;

    // Beschränken auf sichtbaren Teil des Rapportes
    int w = frm->gewebe.pos.width/frm->gewebe.gw;
    int h = frm->gewebe.pos.height/frm->gewebe.gh;
    if (rapp.kr.a<frm->scroll_x1 && rapp.kr.b>=frm->scroll_x1) rapp.kr.a = frm->scroll_x1;
    if (rapp.kr.b>frm->scroll_x1+w-1 && rapp.kr.a<=frm->scroll_x1+w-1) rapp.kr.b = frm->scroll_x1+w-1;
    if (rapp.sr.a<frm->scroll_y2 && rapp.sr.b>=frm->scroll_y2) rapp.sr.a = frm->scroll_y2;
    if (rapp.sr.b>frm->scroll_y2+h-1 && rapp.sr.a<=frm->scroll_y2+h-1) rapp.sr.b = frm->scroll_y2+h-1;

    // Aktualisieren
    for (int i=rapp.kr.a; i<=rapp.kr.b; i++)
        for (int j=rapp.sr.a; j<=rapp.sr.b; j++) {
            if (frm->GewebeFarbeffekt->Checked || frm->GewebeSimulation->Checked)
                frm->DrawGewebeRahmen (i, j);
            frm->DrawGewebe (i, j);
        }
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::CalcKettrapport()
{
    // Rapport löschen
    frm->rapport.kr = SZ(0, -1);

    int i;
    int i1 = frm->kette.a;
    int i2 = frm->kette.b;
    if (i1==-1 || i2==-1) return;

    // Kettrapport bestimmen
    frm->rapport.kr.a = i1;
    frm->rapport.kr.b = i1;
g_again:
    // potentiellen Rapportanfang finden
    for (i=frm->rapport.kr.b+1; i<=i2; i++) {
        if (EinzugEqual (frm->rapport.kr.a, i))
            break;
    }
    frm->rapport.kr.b = i-1;
    // Prüfen ob es tatsächlich Rapport ist
    for (i=frm->rapport.kr.b+1; i<=i2; i++) {
        if (!EinzugEqual (frm->rapport.kr.a+(i-frm->rapport.kr.b-1)%(frm->rapport.kr.b-frm->rapport.kr.a+1), i)) {
            frm->rapport.kr.b++;
            goto g_again;
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::CalcSchussrapport()
{
    // Rapport löschen
    frm->rapport.sr = SZ(0, -1);

    int j;
    int j1 = frm->schuesse.a;
    int j2 = frm->schuesse.b;
    if (j1==-1 || j2==-1) return;

    // Schussrapport bestimmen
    frm->rapport.sr.a = j1;
    frm->rapport.sr.b = j1;
g_again:
    // potentiellen Rapportanfang finden
    for (j=frm->rapport.sr.b+1; j<=j2; j++) {
        if (TrittfolgeEqual (frm->rapport.sr.a, j))
            break;
    }
    frm->rapport.sr.b = j-1;
    // Prüfen ob es tatsächlich Rapport ist
    for (j=frm->rapport.sr.b+1; j<=j2; j++) {
        if (!TrittfolgeEqual (frm->rapport.sr.a+(j-frm->rapport.sr.b-1)%(frm->rapport.sr.b-frm->rapport.sr.a+1), j)) {
            frm->rapport.sr.b++;
            goto g_again;
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapportImpl::CalcRapport()
{
    if (frm->rapport.overridden) {
        return;
    }

    TCursor old = frm->Cursor;
    frm->Cursor = crHourGlass;

    CalcKettrapport();
    CalcSchussrapport();

    frm->Cursor = old;
}
/*-----------------------------------------------------------------*/
bool __fastcall RpRapportImpl::TrittfolgeEqual (int _j1, int _j2)
{
    for (int i=0; i<data->MAXX2; i++) {
        char s1 = frm->trittfolge.feld.Get (i, _j1);
        char s2 = frm->trittfolge.feld.Get (i, _j2);
        if (s1<=0 && s2>0) return false;
        if (s1>0 && s2<=0) return false;
        if (s1>0 && s2>0 && s1!=s2) return false;
    }
    if (frm->kette.a!=-1 && frm->kette.b!=-1)
        for (int i=frm->kette.a; i<=frm->kette.b; i++) {
            char s1 = frm->gewebe.feld.Get(i, _j1);
            char s2 = frm->gewebe.feld.Get(i, _j2);
            if (s1<=0 && s2>0) return false;
            if (s1>0 && s2<=0) return false;
            if (s1>0 && s2>0 && s1!=s2) return false;
        }
    return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall RpRapportImpl::EinzugEqual (int _i1, int _i2)
{
    if (frm->einzug.feld.Get(_i1)!=frm->einzug.feld.Get(_i2))
        return false;
    if (frm->schuesse.a!=-1 && frm->schuesse.b!=-1)
        for (int j=frm->schuesse.a; j<=frm->schuesse.b; j++) {
            char s1 = frm->gewebe.feld.Get(_i1, j);
            char s2 = frm->gewebe.feld.Get(_i2, j);
            if (s1<=0 && s2>0) return false;
            if (s1>0 && s2<=0) return false;
            if (s1>0 && s2>0 && s1!=s2) return false;
        }
    return true;
}
/*-----------------------------------------------------------------*/
RpRapport* __fastcall RpRapport::CreateInstance (TDBWFRM* _frm, TData* _data)
{
    RpRapport* p = NULL;
    try {
        p = new RpRapportImpl (_frm, _data);
    } catch (...) {
    }
    return p;
}
/*-----------------------------------------------------------------*/
void __fastcall RpRapport::ReleaseInstance (RpRapport* _rapport)
{
    delete _rapport;
}
/*-----------------------------------------------------------------*/

