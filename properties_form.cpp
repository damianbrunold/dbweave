/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File properties_form.cpp
 * Copyright (C) 1998-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "properties_form.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
TFilePropertiesForm *FilePropertiesForm;
/*-----------------------------------------------------------------*/
__fastcall TFilePropertiesForm::TFilePropertiesForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
}
/*-----------------------------------------------------------------*/
void __fastcall TFilePropertiesForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Properties", "")
    LANG_C_H(this, GE, "Eigenschaften", "")
    LANG_C_H(l_author, EN, "&Author:", "")
    LANG_C_H(l_author, GE, "&Autor:", "")
    LANG_C_H(l_organzation, EN, "&Organisation:", "")
    LANG_C_H(l_organzation, GE, "&Organisation:", "")
    LANG_C_H(l_remarks, EN, "&Notes:", "")
    LANG_C_H(l_remarks, GE, "&Bemerkungen:", "")
    LANG_C_H(b_ok, EN, "OK", "")
    LANG_C_H(b_ok, GE, "OK", "")
    LANG_C_H(b_cancel, EN, "Cancel", "")
    LANG_C_H(b_cancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/

