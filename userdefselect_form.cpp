/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File userdefselect_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "userdefselect_form.h"
#include "dbw3_form.h"
#include "dbw3_strings.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TUserdefSelectForm::TUserdefSelectForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
}
/*-----------------------------------------------------------------*/
void __fastcall TUserdefSelectForm::LoadLanguage()
{
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/
int __fastcall TDBWFRM::SelectUserdef (String _title/*=""*/)
{
    int result = -1;
    try {
        TUserdefSelectForm* frm = new TUserdefSelectForm(this);
        if (_title!="") frm->Caption = _title;
        if (userdef[0].description!="") frm->Userdef1->Caption = String("&1 - ")+ userdef[0].description;
        else frm->Userdef1->Caption = String("&1 - ")+USERDEF_FREE;
        if (userdef[1].description!="") frm->Userdef2->Caption = String("&2 - ")+ userdef[1].description;
        else frm->Userdef2->Caption = String("&2 - ")+USERDEF_FREE;
        if (userdef[2].description!="") frm->Userdef3->Caption = String("&3 - ")+ userdef[2].description;
        else frm->Userdef3->Caption = String("&3 - ")+USERDEF_FREE;
        if (userdef[3].description!="") frm->Userdef4->Caption = String("&4 - ")+ userdef[3].description;
        else frm->Userdef4->Caption = String("&4 - ")+USERDEF_FREE;
        if (userdef[4].description!="") frm->Userdef5->Caption = String("&5 - ")+ userdef[4].description;
        else frm->Userdef5->Caption = String("&5 - ")+USERDEF_FREE;
        if (userdef[5].description!="") frm->Userdef6->Caption = String("&6 - ")+ userdef[5].description;
        else frm->Userdef6->Caption = String("&6 - ")+USERDEF_FREE;
        if (userdef[6].description!="") frm->Userdef7->Caption = String("&7 - ")+ userdef[6].description;
        else frm->Userdef7->Caption = String("&7 - ")+USERDEF_FREE;
        if (userdef[7].description!="") frm->Userdef8->Caption = String("&8 - ")+ userdef[7].description;
        else frm->Userdef8->Caption = String("&8 - ")+USERDEF_FREE;
        if (userdef[8].description!="") frm->Userdef9->Caption = String("&9 - ")+ userdef[8].description;
        else frm->Userdef9->Caption = String("&9 - ")+USERDEF_FREE;
        if (userdef[9].description!="") frm->Userdef10->Caption = String("1&0 - ")+ userdef[9].description;
        else frm->Userdef10->Caption = String("1&0 - ")+USERDEF_FREE;
        frm->Userdef1->Checked = true;
        if (frm->ShowModal()==mrOk) {
            if (frm->Userdef1->Checked) result = 0;
            else if (frm->Userdef2->Checked) result = 1;
            else if (frm->Userdef3->Checked) result = 2;
            else if (frm->Userdef4->Checked) result = 3;
            else if (frm->Userdef5->Checked) result = 4;
            else if (frm->Userdef6->Checked) result = 5;
            else if (frm->Userdef7->Checked) result = 6;
            else if (frm->Userdef8->Checked) result = 7;
            else if (frm->Userdef9->Checked) result = 8;
            else if (frm->Userdef10->Checked) result = 9;
        }
    } catch (...) {
    }
    return result;
}
/*-----------------------------------------------------------------*/

