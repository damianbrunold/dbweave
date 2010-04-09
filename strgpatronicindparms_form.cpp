/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File strgpatronicindparms_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "strgpatronicindparms_form.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TStrgPatronicIndParmsForm::TStrgPatronicIndParmsForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
}
/*-----------------------------------------------------------------*/
void __fastcall TStrgPatronicIndParmsForm::LoadLanguage()
{
    LANG_C_H(this, EN, "ARM Patronc indirect parameter", "")
    LANG_C_H(this, GE, "ARM Patronic Indirekt Parameter", "")
    LANG_C_H(lPos, EN, "&Starting Position:", "")
    LANG_C_H(lPos, GE, "&Speichern ab Position:", "")
    LANG_C_H(lCount, EN, "&Max. number of picks:", "")
    LANG_C_H(lCount, GE, "&Maximale Anzahl Schüsse:", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/

