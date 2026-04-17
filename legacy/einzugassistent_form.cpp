/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File einzugassistent_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "dbw3_form.h"
#include "assert.h"
#include "datamodule.h"
#include "einzugassistent_form.h"
#include "undoredo.h"
#include "dbw3_strings.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TEinzugassistentForm::TEinzugassistentForm(TDBWFRM* _frm, TComponent* Owner)
: TForm(Owner), frm(_frm)
{
    einzugarten->ActivePage = einzugarten->Pages[0];
}
/*-----------------------------------------------------------------*/
void __fastcall TEinzugassistentForm::Recalc()
{
    frm->RecalcGewebe();
    frm->CalcRangeSchuesse();
    frm->CalcRangeKette();
    frm->RecalcFreieSchaefte();
    frm->UpdateRapport();
    frm->Invalidate();
    frm->SetModified();
    // undo-snapshot wird in TDBWFRM::EinzugAssistentClick gemacht
}
/*-----------------------------------------------------------------*/
void __fastcall TEinzugassistentForm::CreateGerade (int _firstkettfaden, int _firstschaft, int _schaefte, bool _steigend)
{
    if (_steigend) {
        for (int i=_firstkettfaden-1; i<_firstkettfaden-1+_schaefte; i++) {
            short s = short(_firstschaft + (i-(_firstkettfaden-1)));
            if (s>=Data->MAXY1) frm->ExtendSchaefte();
            if (i>=Data->MAXX1) break;
            frm->einzug.feld.Set (i, s);
        }
    } else {
        for (int i=_firstkettfaden-1; i<_firstkettfaden-1+_schaefte; i++) {
            short s = short(_firstschaft + (_firstkettfaden-1+_schaefte-1) - (i-(_firstkettfaden-1)));
            if (s>=Data->MAXY1) frm->ExtendSchaefte();
            if (i>=Data->MAXX1) break;
            frm->einzug.feld.Set (i, s);
        }
    }
    Recalc();
}
/*-----------------------------------------------------------------*/
void __fastcall TEinzugassistentForm::CreateAbgesetzt (int _firstkettfaden, int _firstschaft, int _schaefte, int _gratlen, int _versatz)
{
    int i = _firstkettfaden-1;
    int j = _firstschaft-1;
    while(1) {
        for (int ii=0; ii<_gratlen; ii++) {
            short s = short(j+ii+1);
            if (s-_firstschaft >= _schaefte) goto g_break;
            if (s<=0) goto g_break;
            if (i+ii>=Data->MAXX1) goto g_break;
            frm->einzug.feld.Set (i+ii, s);
            if (s-_firstschaft+1==_schaefte) goto g_break;
        }
        i += _gratlen;
        j += _versatz;
    }
g_break:
    Recalc();
}
/*-----------------------------------------------------------------*/
void __fastcall TEinzugassistentForm::bOKClick(TObject *Sender)
{
    if (einzugarten->ActivePage==einzugarten->Pages[0]) { // Geradedurch
        int firstkf;
        try { firstkf = StrToInt(FirstKettfaden1->Text); } catch (...) { firstkf = 0; }
        if (firstkf<1 || firstkf>Data->MAXX1) {
            char msg[100];
            wsprintf (msg, "Der erste Kettfaden muss im Bereich 1-%d liegen", Data->MAXX1);
            Application->MessageBox(msg, "DB-WEBEN", MB_OK);
            ModalResult = mrNone;
            return;
        }
        int firstschuss;
        try { firstschuss = StrToInt(FirstSchaft1->Text); } catch (...) { firstschuss = 0; }
        if (firstschuss<1 || firstschuss>Data->MAXY1) {
            char msg[100];
            wsprintf (msg, "Der erste Schuss muss im Bereich 1-%d liegen", Data->MAXY1);
            Application->MessageBox(msg, APP_TITLE, MB_OK);
            ModalResult = mrNone;
            return;
        }
        int schaefte;
        try { schaefte = StrToInt(Schaefte1->Text); } catch (...) { schaefte = 0; }
        if (schaefte<1 || schaefte>Data->MAXY1) {
            char msg[100];
            wsprintf (msg, "Die Anzahl Schäfte muss im Bereich 1-%d liegen", Data->MAXY1);
            Application->MessageBox(msg, APP_TITLE, MB_OK);
            ModalResult = mrNone;
            return;
        }
        CreateGerade (firstkf, firstschuss, schaefte, ZGrat1->Checked);
     } else if (einzugarten->ActivePage==einzugarten->Pages[1]) { // Abgesetzt
        int firstkf;
        try { firstkf = StrToInt(FirstKettfaden2->Text); } catch (...) { firstkf = 0; }
        if (firstkf<1 || firstkf>Data->MAXX1) {
            char msg[100];
            wsprintf (msg, "Der erste Kettfaden muss im Bereich 1-%d liegen", Data->MAXX1);
            Application->MessageBox(msg, APP_TITLE, MB_OK);
            ModalResult = mrNone;
            return;
        }
        int firstschuss;
        try { firstschuss = StrToInt(FirstSchaft2->Text); } catch (...) { firstschuss = 0; }
        if (firstschuss<1 || firstschuss>Data->MAXY1) {
            char msg[100];
            wsprintf (msg, "Der erste Schuss muss im Bereich 1-%d liegen", Data->MAXY1);
            Application->MessageBox(msg, APP_TITLE, MB_OK);
            ModalResult = mrNone;
            return;
        }
        int schaefte;
        try { schaefte = StrToInt(Schaefte2->Text); } catch (...) { schaefte = 0; }
        if (schaefte<1 || schaefte>Data->MAXY1) {
            char msg[100];
            wsprintf (msg, "Die Anzahl Schäfte muss im Bereich 1-%d liegen", Data->MAXY1);
            Application->MessageBox(msg, APP_TITLE, MB_OK);
            ModalResult = mrNone;
            return;
        }
        int glen;
        try { glen = StrToInt(Gratlen->Text); } catch (...) { glen = 0; }
        if (glen<1 || glen>schaefte) {
            Application->MessageBox("Die Gratlänge muss zwischen 1 und der Anzahl Schäfte liegen", APP_TITLE, MB_OK);
            ModalResult = mrNone;
            return;
        }
        int versatz;
        try { versatz = StrToInt(Versatz->Text); } catch (...) { versatz=0; }
        if (abs(versatz)<1 || abs(versatz)>schaefte) {
            Application->MessageBox ("Der Versatz muss zwischen 1 und der Anzahl Schäfte liegen", APP_TITLE, MB_OK);
            ModalResult = mrNone;
            return;
        }
        CreateAbgesetzt (firstkf, firstschuss, schaefte, glen, versatz);
    } else {
        //
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EinzugAssistentClick(TObject *Sender)
{
    try {
        TEinzugassistentForm* frm = new TEinzugassistentForm(this, this);
        if (frm->ShowModal()==mrOk) {
            dbw3_assert(undo!=0);
            undo->Snapshot();
        }
        delete frm;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/


