/* DB-WEAVE, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File entervv_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "entervv_form.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TEnterVVForm::TEnterVVForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
}
/*-----------------------------------------------------------------*/
void __fastcall TEnterVVForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Thread ratio", "")
    LANG_C_H(this, GE, "Fadenverhältnis", "")
    LANG_C_H(labVV, EN, "&Enter the thread ratio:", "")
    LANG_C_H(labVV, GE, "&Geben Sie das Fadenverhältnis ein:", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/
void __fastcall TEnterVVForm::bOkClick(TObject *Sender)
{
    //xxx Eingaben auf Vernunft überprüfen
}
/*-----------------------------------------------------------------*/
void __fastcall TEnterVVForm::VV1Change(TObject *Sender)
{
    int vv = atoi(VV1->Text.c_str());
    if (vv==0) {
        VV2->Text = ""; VV2->Enabled = false;
        VV3->Text = ""; VV3->Enabled = false;
        VV4->Text = ""; VV4->Enabled = false;
        VV5->Text = ""; VV5->Enabled = false;
        VV6->Text = ""; VV6->Enabled = false;
    } else
        VV2->Enabled = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TEnterVVForm::VV2Change(TObject *Sender)
{
    int vv = atoi(VV2->Text.c_str());
    if (vv==0) {
        VV3->Text = ""; VV3->Enabled = false;
        VV4->Text = ""; VV4->Enabled = false;
        VV5->Text = ""; VV5->Enabled = false;
        VV6->Text = ""; VV6->Enabled = false;
    } else
        VV3->Enabled = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TEnterVVForm::VV3Change(TObject *Sender)
{
    int vv = atoi(VV3->Text.c_str());
    if (vv==0) {
        VV4->Text = ""; VV4->Enabled = false;
        VV5->Text = ""; VV5->Enabled = false;
        VV6->Text = ""; VV6->Enabled = false;
    } else
        VV4->Enabled = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TEnterVVForm::VV4Change(TObject *Sender)
{
    int vv = atoi(VV4->Text.c_str());
    if (vv==0) {
        VV5->Text = ""; VV5->Enabled = false;
        VV6->Text = ""; VV6->Enabled = false;
    } else
        VV5->Enabled = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TEnterVVForm::VV5Change(TObject *Sender)
{
    int vv = atoi(VV5->Text.c_str());
    if (vv==0) {
        VV6->Text = ""; VV6->Enabled = false;
    } else
        VV6->Enabled = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TEnterVVForm::VV6Change(TObject *Sender)
{
    //
}
/*-----------------------------------------------------------------*/

