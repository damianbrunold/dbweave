/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * rapport_form.cpp
 * Copyright (C) 1998-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "rapport_form.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TRapportForm::TRapportForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
}
/*-----------------------------------------------------------------*/
void __fastcall TRapportForm::RappAllClick(TObject *Sender)
{
    rapp_horz->Enabled = !RappAll->Checked;
    rapp_vert->Enabled = !RappAll->Checked;
}
/*-----------------------------------------------------------------*/
void __fastcall TRapportForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Repeat", "")
    LANG_C_H(this, GE, "Rapportieren", "")
    LANG_C_H(RappAll, EN, "Repeat &all", "")
    LANG_C_H(RappAll, GE, "&Gesamtes Feld ausfüllen", "")
    LANG_C_H(RappColors, EN, "Repeat &colors", "")
    LANG_C_H(RappColors, GE, "&Farben rapportieren", "")
    LANG_C_H(labelHorz, EN, "&Horizontal:", "")
    LANG_C_H(labelHorz, GE, "&Horizontal:", "")
    LANG_C_H(labelVert, EN, "&Vertical:", "")
    LANG_C_H(labelVert, GE, "&Vertikal:", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/

