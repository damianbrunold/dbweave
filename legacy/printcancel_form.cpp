/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File printcancel_form.cpp
 * Copyright (C) 1998-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "printcancel_form.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TPrintCancelForm::TPrintCancelForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintCancelForm::cancelClick(TObject *Sender)
{
    Hide();
}
/*-----------------------------------------------------------------*/
void __fastcall TPrintCancelForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Printing", "")
    LANG_C_H(this, GE, "Drucken", "")
    LANG_C_H(cancel, EN, "Cancel", "")
    LANG_C_H(cancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/

