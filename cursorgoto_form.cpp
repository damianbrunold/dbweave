/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File cursorgoto_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "cursorgoto_form.h"
#include "dbw3_form.h"
#include "cursor.h"
#include "assert.h"
#include "dbw3_strings.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TCursorGotoForm::TCursorGotoForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::CursorGotoClick(TObject *Sender)
{
    try {
        TCursorGotoForm* frm = new TCursorGotoForm (this);
        if (!ViewSchlagpatrone->Checked) {
            frm->GotoTrittfolge->Caption = GOTO_TRITTFOLGE;
        } else {
            frm->GotoTrittfolge->Caption = GOTO_SCHLAGPATRONE;
            frm->GotoAufknuepfung->Enabled = false; 
        }
        switch (kbd_field) {
            case EINZUG:
                frm->GotoEinzug->Checked = true;
                break;
            case TRITTFOLGE:
                frm->GotoTrittfolge->Checked = true;
                break;
            case AUFKNUEPFUNG:
                frm->GotoAufknuepfung->Checked = true;
                break;
            case KETTFARBEN:
                frm->GotoKettfarben->Checked = true;
                break;
            case SCHUSSFARBEN:
                frm->GotoSchussfarben->Checked = true;
                break;
            case GEWEBE:
            default:
                frm->GotoBindung->Checked = true;
                break;
        }
        if (frm->ShowModal()==mrOk) {
            dbw3_assert (cursorhandler);
            if (frm->GotoBindung->Checked)
                cursorhandler->GotoField(GEWEBE);
            else if (frm->GotoEinzug->Checked)
                cursorhandler->GotoField(EINZUG);
            else if (frm->GotoTrittfolge->Checked)
                cursorhandler->GotoField(TRITTFOLGE);
            else if (frm->GotoAufknuepfung->Checked)
                cursorhandler->GotoField(AUFKNUEPFUNG);
            else if (frm->GotoKettfarben->Checked)
                cursorhandler->GotoField(KETTFARBEN);
            else if (frm->GotoSchussfarben->Checked)
                cursorhandler->GotoField(SCHUSSFARBEN);
            UpdateStatusBar();
        }
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TCursorGotoForm::FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
    if (Key==VK_ESCAPE) ModalResult = mrCancel;
}
/*-----------------------------------------------------------------*/
void __fastcall TCursorGotoForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Cursor Go to", "")
    LANG_C_H(this, GE, "Cursor Gehe zu", "")
    LANG_C_H(label, EN, "Choose field:", "")
    LANG_C_H(label, GE, "Feld auswählen:", "")
    LANG_C_H(GotoEinzug, EN, "&Threading", "")
    LANG_C_H(GotoEinzug, GE, "&Einzug", "")
    LANG_C_H(GotoTrittfolge, EN, "Tre&adling", "")
    LANG_C_H(GotoTrittfolge, GE, "&Trittfolge", "")
    LANG_C_H(GotoBindung, EN, "&Pattern", "")
    LANG_C_H(GotoBindung, GE, "&Bindung", "")
    LANG_C_H(GotoAufknuepfung, EN, "Tie-&up", "")
    LANG_C_H(GotoAufknuepfung, GE, "&Aufknüpfung", "")
    LANG_C_H(GotoKettfarben, EN, "&Warp colors", "")
    LANG_C_H(GotoKettfarben, GE, "&Kettfarben", "")
    LANG_C_H(GotoSchussfarben, EN, "We&ft colors", "")
    LANG_C_H(GotoSchussfarben, GE, "Schuss&farben", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
}
/*-----------------------------------------------------------------*/

