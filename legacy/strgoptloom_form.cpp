// DB-WEAVE, textile CAD/CAM software
// Copyright (c) 1998-2005 Brunold Software, Switzerland

#include <vcl.h>
#pragma hdrstop

#include "strgoptloom_form.h"
#include "dbw3_strings.h"
#include "language.h"
#include "strginfo_form.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

__fastcall TStrgOptLoomForm::TStrgOptLoomForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
}

void __fastcall TStrgOptLoomForm::UpdateOptions()
{
    switch ((LOOMINTERFACE)loomlist->ItemIndex) {
        case intrf_arm_patronic:
        case intrf_arm_patronic_indirect:
        case intrf_arm_designer:
        case intrf_slips:
        case intrf_avl_cd_iii:
            PanelSerial->Visible = true;
            PanelParallel->Visible = false;
            break;

        case intrf_varpapuu_parallel:
            PanelSerial->Visible = false;
            PanelParallel->Visible = true;
            lDelay->Visible = true;
            Delay->Visible = true;
            break;

        case intrf_lips:
            PanelSerial->Visible = false;
            PanelParallel->Visible = true;
            lDelay->Visible = false;
            Delay->Visible = false;
            break;

        default:
            PanelSerial->Visible = false;
            PanelParallel->Visible = false;
            break;
    }
}

void __fastcall TStrgOptLoomForm::FormShow(TObject *Sender)
{
    loomlist->ItemIndex = (int)intrf;
    portlist->ItemIndex = (int)port-1;
    lptlist->ItemIndex = (int)lpt-1;
    Delay->ItemIndex = delay;
    UpdateOptions();
}

void __fastcall TStrgOptLoomForm::bOkClick(TObject *Sender)
{
    intrf = (LOOMINTERFACE)loomlist->ItemIndex;
    port = (PORT)(portlist->ItemIndex+1);
    lpt = (LPT)(lptlist->ItemIndex+1);
    delay = Delay->ItemIndex;
    ModalResult = mrOk;
}

void __fastcall TStrgOptLoomForm::bCancelClick(TObject *Sender)
{
    ModalResult = mrCancel;
}

void __fastcall TStrgOptLoomForm::loomlistChange(TObject *Sender)
{
    UpdateOptions();
}

void __fastcall TStrgOptLoomForm::InfoClick(TObject *Sender)
{
    ShowLoomInfo(this, (LOOMINTERFACE)loomlist->ItemIndex);
}

void __fastcall TStrgOptLoomForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Loom options", "")
    LANG_C_H(this, GE, "Webstuhl-Optionen", "")
	LANG_C_H (lWebstuhl, EN, "&Loom:", "")
	LANG_C_H (lWebstuhl, GE, "&Webstuhl:", "")
	LANG_C_H (bOk, EN, "OK", "")
	LANG_C_H (bOk, GE, "OK", "")
	LANG_C_H (bCancel, EN, "Cancel", "")
	LANG_C_H (bCancel, GE, "Abbrechen", "")
	LANG_C_H (lPort, EN, "&Port:", "")
	LANG_C_H (lPort, GE, "&Port:", "")
	LANG_C_H (lParallelport, EN, "&Parallel port:", "")
	LANG_C_H (lParallelport, GE, "&Parallelport:", "")
	LANG_C_H (lDelay, EN, "&Delay:", "")
	LANG_C_H (lDelay, GE, "&Verzögerung:", "")
	LANG_C_H (Info, EN, "&Info", "")
	LANG_C_H (Info, GE, "&Info", "")

    loomlist->Items->Strings[0] = LANG_STR("No loom", "Kein Webstuhl");
    loomlist->Items->Strings[1] = LANG_STR("ARM Patronic direct", "ARM Patronic Direkt");
    loomlist->Items->Strings[2] = LANG_STR("ARM Patronic indirect", "ARM Patronic Indirekt");
    loomlist->Items->Strings[3] = LANG_STR("ARM Designer electronic", "ARM Designer Electronic");
    loomlist->Items->Strings[4] = LANG_STR("Varpapuu parallel", "Varpapuu über Druckerkabel");
    loomlist->Items->Strings[5] = LANG_STR("Generic SLIPS", "Allgemeines SLIPS");
    loomlist->Items->Strings[6] = LANG_STR("Generic LIPS", "Allgemeines LIPS");
    loomlist->Items->Strings[7] = LANG_STR("AVL Compu-Dobby III", "AVL Compu-Dobby III");
}

