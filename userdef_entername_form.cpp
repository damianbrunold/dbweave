/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File userdef_entername_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "userdef_entername_form.h"
#include "dbw3_form.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TUserdefEnternameForm::TUserdefEnternameForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
}
/*-----------------------------------------------------------------*/
String __fastcall GetUserdefName (String _default)
{
    String name = _default;

    try {
        TUserdefEnternameForm* frm = new TUserdefEnternameForm(DBWFRM);
        frm->Name->Text = name;
        if (frm->ShowModal()==mrOk)
            name = frm->Name->Text;
        else
            name = "";
        delete frm;
    } catch (...) {
    }

    return name;
}
/*-----------------------------------------------------------------*/
void __fastcall TUserdefEnternameForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Pattern name", "")
    LANG_C_H(this, GE, "Mustername", "")
    LANG_C_H(labName, EN, "&Enter the pattern name:", "")
    LANG_C_H(labName, GE, "&Geben Sie den Musternamen ein:", "")
    LANG_C_H(bOK, EN, "OK", "")
    LANG_C_H(bOK, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/

