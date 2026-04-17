/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File strggoto_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "strggoto_form.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TStrgGotoForm::TStrgGotoForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
}
/*-----------------------------------------------------------------*/
void __fastcall TStrgGotoForm::SchussChange(TObject *Sender)
{
    //
}
/*-----------------------------------------------------------------*/
void __fastcall TStrgGotoForm::cbKlammerChange(TObject *Sender)
{
    //
}
/*-----------------------------------------------------------------*/
void __fastcall TStrgGotoForm::cbRepeatChange(TObject *Sender)
{
    //
}
/*-----------------------------------------------------------------*/
void __fastcall TStrgGotoForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Set current weave position", "")
    LANG_C_H(this, GE, "Aktuelle Webposition festlegen", "")
    LANG_C_H(lSchuss, EN, "&Pick:", "")
    LANG_C_H(lSchuss, GE, "&Schuss:", "")
    LANG_C_H(lKlammer, EN, "&Brace:", "")
    LANG_C_H(lKlammer, GE, "&Klammer:", "")
    LANG_C_H(lRepeat, EN, "&Repetition:", "")
    LANG_C_H(lRepeat, GE, "&Wiederholung:", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/

