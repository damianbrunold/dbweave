/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File einstellverh_form.cpp
 * Copyright (C) 1998-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "einstellverh_form.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
TEinstellVerhaeltnisForm *EinstellVerhaeltnisForm;
/*-----------------------------------------------------------------*/
__fastcall TEinstellVerhaeltnisForm::TEinstellVerhaeltnisForm(TComponent* Owner)
    : TForm(Owner)
{
    LoadLanguage();
}
/*-----------------------------------------------------------------*/
void __fastcall TEinstellVerhaeltnisForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Weft/warp ratio", "")
    LANG_C_H(this, GE, "Schuss/Kett-Verhältnis", "")
    LANG_C_H(lKette, EN, "Weft &factor:", "")
    LANG_C_H(lKette, GE, "Faktor &Schuss:", "")
    LANG_C_H(lSchuss, EN, "&Warp factor:", "")
    LANG_C_H(lSchuss, GE, "Faktor &Kette:", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/
