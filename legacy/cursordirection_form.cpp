/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File cursordirection_form.cpp
 * Copyright (C) 1999-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "cursordirection_form.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TCursordirectionForm::TCursordirectionForm(TComponent* Owner, CURSORDIRECTION _cd)
: TForm(Owner), cursordirection(_cd)
{
    if (_cd&CD_UP) cdUp->Down = true;
    if (_cd&CD_DOWN) cdDown->Down = true;
    if (_cd&CD_LEFT) cdLeft->Down = true;
    if (_cd&CD_RIGHT) cdRight->Down = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TCursordirectionForm::bOkClick(TObject *Sender)
{
    cursordirection = CD_NONE;
    if (cdUp->Down) cursordirection |= CD_UP;
    if (cdDown->Down) cursordirection |= CD_DOWN;
    if (cdLeft->Down) cursordirection |= CD_LEFT;
    if (cdRight->Down) cursordirection |= CD_RIGHT;
}
/*-----------------------------------------------------------------*/
void __fastcall TCursordirectionForm::bCancelClick(TObject *Sender)
{
    // nichts ändern
}
/*-----------------------------------------------------------------*/
void __fastcall TCursordirectionForm::cdUpClick(TObject *Sender)
{
    if (GetKeyState(VK_MENU)&0x8000) {
        cdDown->Down = false;
        cdUp->Down = !cdUp->Down;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TCursordirectionForm::cdLeftClick(TObject *Sender)
{
    if (GetKeyState(VK_MENU)&0x8000) {
        cdRight->Down = false;
        cdLeft->Down = !cdLeft->Down;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TCursordirectionForm::cdRightClick(TObject *Sender)
{
    if (GetKeyState(VK_MENU)&0x8000) {
        cdLeft->Down = false;
        cdRight->Down = !cdRight->Down;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TCursordirectionForm::cdDownClick(TObject *Sender)
{
    if (GetKeyState(VK_MENU)&0x8000) {
        cdUp->Down = false;
        cdDown->Down = !cdDown->Down;
    }
}
/*-----------------------------------------------------------------*/

