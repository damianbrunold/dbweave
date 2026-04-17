/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File envoptions_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "dbw3_form.h"
#include "envoptions_form.h"
#include "settings.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TEnvironmentOptionsForm::TEnvironmentOptionsForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
    Settings settings;
    settings.SetCategory ("Environment");
    int lang = settings.Load ("Language", -1);
    if (lang==-1) { // Windows-Spracheinstellung abfragen
        char loc[4];
        if (GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME, loc, 4)!=0) {
            if ((loc[0]=='D' || loc[0]=='d') && (loc[1]=='E' || loc[1]=='e'))
                lang = 1;
            else
                lang = 0;
        } else lang = 0;
    }
    ShowSplash->Checked = settings.Load ("ShowSplash", 1)==1;
    cbLanguage->ItemIndex = lang;
}
/*-----------------------------------------------------------------*/
void __fastcall TEnvironmentOptionsForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Environment options", "")
    LANG_C_H(this, GE, "Umgebungsoptionen", "")
    LANG_C_H(lLanguage, EN, "&Language", "")
    LANG_C_H(lLanguage, GE, "&Sprache", "")
    cbLanguage->Items->Strings[0] = LANG_STR("English", "Englisch");
    cbLanguage->Items->Strings[1] = LANG_STR("German", "Deutsch");
    LANG_C_H(ShowSplash, EN, "&Display splashscreen", "")
    LANG_C_H(ShowSplash, GE, "Splashscreen &anzeigen", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}
/*-----------------------------------------------------------------*/
void __fastcall TEnvironmentOptionsForm::bOkClick(TObject *Sender)
{
    LANGUAGES language;
    switch (cbLanguage->ItemIndex) {
        case 0: language = EN; break;
        case 1: language = GE; break;
    }

    SwitchLanguage (language);

    Settings settings;
    settings.SetCategory ("Environment");
    settings.Save ("ShowSplash", ShowSplash->Checked ? 1 : 0);
    settings.Save ("Language", cbLanguage->ItemIndex);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::OptEnvironmentClick(TObject *Sender)
{
    try {
        TEnvironmentOptionsForm* pFrm = new TEnvironmentOptionsForm(this);
        pFrm->ShowModal();
        delete pFrm;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/

