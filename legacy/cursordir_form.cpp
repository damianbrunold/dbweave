/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File cursordir_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "cursordir_form.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TCursorDirForm::TCursorDirForm(TComponent* Owner, CURSORDIRECTION _cd, int /*_dummy*/)
: TForm(Owner), cursordirection(_cd)
{
    LoadLanguage();
    if (_cd&CD_UP) cdUp->Down = true;
    if (_cd&CD_DOWN) cdDown->Down = true;
    if (_cd&CD_LEFT) cdLeft->Down = true;
    if (_cd&CD_RIGHT) cdRight->Down = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TCursorDirForm::cdUpClick(TObject *Sender)
{
    if (GetKeyState(VK_MENU)&0x8000) {
        cdDown->Down = false;
        cdUp->Down = !cdUp->Down;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TCursorDirForm::cdLeftClick(TObject *Sender)
{
    if (GetKeyState(VK_MENU)&0x8000) {
        cdRight->Down = false;
        cdLeft->Down = !cdLeft->Down;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TCursorDirForm::cdRightClick(TObject *Sender)
{
    if (GetKeyState(VK_MENU)&0x8000) {
        cdLeft->Down = false;
        cdRight->Down = !cdRight->Down;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TCursorDirForm::cdDownClick(TObject *Sender)
{
    if (GetKeyState(VK_MENU)&0x8000) {
        cdUp->Down = false;
        cdDown->Down = !cdDown->Down;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TCursorDirForm::FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    CURSORDIRECTION cd = CD_NONE;
    if (X>cdLeft->Left && X<(cdLeft->Left+cdLeft->Width)) cd |= CD_LEFT;
    if (X>cdRight->Left && X<(cdRight->Left+cdRight->Width)) cd |= CD_RIGHT;

    if (Y>cdUp->Top && Y<(cdUp->Top+cdUp->Height)) cd |= CD_UP;
    if (Y>cdDown->Top && Y<(cdDown->Top+cdDown->Height)) cd |= CD_DOWN;

    if ((cd!=CD_NONE && cd!=CD_LEFT && cd!=CD_RIGHT && cd!=CD_UP && cd!=CD_DOWN) ||
        X>cdUp->Left && X<(cdUp->Left+cdUp->Width) && Y>cdLeft->Top && Y<(cdLeft->Top+cdLeft->Height))
    {
        cdUp->Down = false;
        cdDown->Down = false;
        cdLeft->Down = false;
        cdRight->Down = false;
    }

    if (cd!=CD_NONE && cd!=CD_LEFT && cd!=CD_RIGHT && cd!=CD_UP && cd!=CD_DOWN) {
        if (cd&CD_UP) cdUp->Down = true;
        if (cd&CD_DOWN) cdDown->Down = true;
        if (cd&CD_LEFT) cdLeft->Down = true;
        if (cd&CD_RIGHT) cdRight->Down = true;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TCursorDirForm::bOkClick(TObject *Sender)
{
    cursordirection = CD_NONE;
    if (cdUp->Down) cursordirection |= CD_UP;
    if (cdDown->Down) cursordirection |= CD_DOWN;
    if (cdLeft->Down) cursordirection |= CD_LEFT;
    if (cdRight->Down) cursordirection |= CD_RIGHT;
}
/*-----------------------------------------------------------------*/
void __fastcall TCursorDirForm::bCancelClick(TObject *Sender)
{
    // Nichts ändern
}
/*-----------------------------------------------------------------*/
void __fastcall TCursorDirForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Cursor movement", "")
    LANG_C_H(this, GE, "Cursorbewegung", "")

    LANG_C_H(cdUp, EN, "&Up", "")
    LANG_C_H(cdUp, GE, "&Oben", "")

    LANG_C_H(cdDown, EN, "&Down", "")
    LANG_C_H(cdDown, GE, "&Unten", "")

    LANG_C_H(cdLeft, EN, "&Left", "")
    LANG_C_H(cdLeft, GE, "&Links", "")

    LANG_C_H(cdRight, EN, "&Right", "")
    LANG_C_H(cdRight, GE, "&Rechts", "")

    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")

    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/

