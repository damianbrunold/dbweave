/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File loadparts_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "loadparts_form.h"
#include "dbw3_form.h"
#include "datamodule.h"
#include "undoredo.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TLoadPartsForm::TLoadPartsForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
}
/*-----------------------------------------------------------------*/
void __fastcall TLoadPartsForm::AlleClick(TObject *Sender)
{
    bool enable = !Alle->Checked;
    Einzug->Enabled = enable;
    Trittfolge->Enabled = enable;
    Aufknuepfung->Enabled = enable;
    Kettfarben->Enabled = enable;
    Schussfarben->Enabled = enable;
    Blatteinzug->Enabled = enable;
    Hilfslinien->Enabled = enable;
    Farbpalette->Enabled = enable;
    Bendefeinzug->Enabled = enable;
    Blockmuster->Enabled = enable;
    Bereichmuster->Enabled = enable;

    Einzug->Checked = !enable;
    Trittfolge->Checked = !enable;
    Aufknuepfung->Checked = !enable;
    Kettfarben->Checked = !enable;
    Schussfarben->Checked = !enable;
    Blatteinzug->Checked = !enable;
    Hilfslinien->Checked = !enable;
    Farbpalette->Checked = !enable;
    Bendefeinzug->Checked = !enable;
    Blockmuster->Checked = !enable;
    Bereichmuster->Checked = !enable;
}
/*-----------------------------------------------------------------*/
LOADPARTS TLoadPartsForm::GetLoadParts()
{
    if (Alle->Checked) return LOADALL;
    else {
        LOADPARTS parts = 0;
        if (Einzug->Checked) parts |= LOADEINZUG;
        if (Trittfolge->Checked) parts |= LOADTRITTFOLGE;
        if (Aufknuepfung->Checked) parts |= LOADAUFKNUEPFUNG;
        if (Kettfarben->Checked) parts |= LOADKETTFARBEN;
        if (Schussfarben->Checked) parts |= LOADSCHUSSFARBEN;
        if (Blatteinzug->Checked) parts |= LOADBLATTEINZUG;
        if (Hilfslinien->Checked) parts |= LOADHILFSLINIEN;
        if (Farbpalette->Checked) parts |= LOADPALETTE;
        if (Bendefeinzug->Checked) parts |= LOADFIXEINZUG;
        if (Blockmuster->Checked) parts |= LOADBLOCKMUSTER;
        if (Bereichmuster->Checked) parts |= LOADBEREICHMUSTER;
        return parts;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TLoadPartsForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Load parts", "")
    LANG_C_H(this, GE, "Teile laden", "")
    LANG_C_H(gbParts, EN, "Which parts should be loaded?", "")
    LANG_C_H(gbParts, GE, "Welche Teile sollen geladen werden?", "")
    LANG_C_H(Einzug, EN, "&Threading", "")
    LANG_C_H(Einzug, GE, "&Einzug", "")
    LANG_C_H(Trittfolge, EN, "Tre&adling", "")
    LANG_C_H(Trittfolge, GE, "&Trittfolge", "")
    LANG_C_H(Aufknuepfung, EN, "Tie-&up", "")
    LANG_C_H(Aufknuepfung, GE, "&Aufknüpfung", "")
    LANG_C_H(Kettfarben, EN, "&Warp colors", "")
    LANG_C_H(Kettfarben, GE, "&Schussfarben", "")
    LANG_C_H(Schussfarben, EN, "We&ft colors", "")
    LANG_C_H(Schussfarben, GE, "&Kettfarben", "")
    LANG_C_H(Hilfslinien, EN, "&Support lines", "")
    LANG_C_H(Hilfslinien, GE, "&Hilfslinien", "")
    LANG_C_H(Farbpalette, EN, "&Color palette", "")
    LANG_C_H(Farbpalette, GE, "&Farbpalette", "")
    LANG_C_H(Blatteinzug, EN, "&Reed threading", "")
    LANG_C_H(Blatteinzug, GE, "&Blatteinzug", "")
    LANG_C_H(Bereichmuster, EN, "Ra&nge patterns", "")
    LANG_C_H(Bereichmuster, GE, "Be&reichsmuster", "")
    LANG_C_H(Blockmuster, EN, "&Block patterns", "")
    LANG_C_H(Blockmuster, GE, "Bl&ockmuster", "")
    LANG_C_H(Bendefeinzug, EN, "&Userdef. threading", "")
    LANG_C_H(Bendefeinzug, GE, "Be&nutzerdef. Einzug", "")
    LANG_C_H(Alle, EN, "&All", "")
    LANG_C_H(Alle, GE, "A&lle", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::LoadPartsClick(TObject *Sender)
{
    try {
        TLoadPartsForm* frm = new TLoadPartsForm(this);
        if (frm->ShowModal()==mrOk) {
            TCursor oldcursor = Cursor;
            Cursor = crHourGlass;
            String fname = filename;
            filename = "";
            String savefn = Data->OpenDialog->FileName;
            Data->OpenDialog->FileName = "";
            Data->OpenDialog->FilterIndex = 1;
            Data->SaveDialog->FilterIndex = 1;
            if (Data->OpenDialog->Execute()) {
                filename = Data->OpenDialog->FileName;
                CloseDataFile();
				LOADSTAT stat;
                Load(stat, frm->GetLoadParts());
                Data->SaveDialog->FilterIndex = Data->OpenDialog->FilterIndex;
            }
            Data->OpenDialog->FileName = savefn;
            filename = fname;
            CloseDataFile();
            ResizeFelder();
            FinalizeLoad();
            if (!righttoleft) sb_horz1->Position = scroll_x1;
            else sb_horz1->Position = sb_horz1->Max - scroll_x1;
            if (toptobottom) sb_vert1->Position = scroll_y1;
            else sb_vert1->Position = sb_vert1->Max - scroll_y1;
            UpdateScrollbars();
            RecalcFreieSchaefte();
            RecalcFreieTritte();
            Cursor = oldcursor;
            Invalidate();
            SetModified();
            undo->Snapshot();
        }
        delete frm;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ResizeFelder()
{
    gewebe.feld.Resize(Data->MAXX1, Data->MAXY2, 0);
    einzug.feld.Resize(Data->MAXX1, 0);
    aufknuepfung.feld.Resize(Data->MAXX2, Data->MAXY1, 0);
    trittfolge.feld.Resize(Data->MAXX2, Data->MAXY2, 0);
    kettfarben.feld.Resize(Data->MAXX1, Data->defcolorh);
    schussfarben.feld.Resize(Data->MAXY2, Data->defcolorv);
    blatteinzug.feld.Resize(Data->MAXX1, 0);
}
/*-----------------------------------------------------------------*/

