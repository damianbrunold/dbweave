/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File entwurfsinfo_form.cpp
 * Copyright (C) 1999-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#include <printers.hpp>
#include <stdio.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "entwurfsinfo_form.h"
#include "assert.h"
#include "dbw3_form.h"
#include "dbw3_strings.h"
#include "datamodule.h"
#include "palette.h"
#include "colors.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TEntwurfsinfoForm::TEntwurfsinfoForm(TDBWFRM* _frm, TComponent* Owner)
: TForm(Owner), frm(_frm)
{
    LoadLanguage();

    read_ausmasse = false;
    read_farben = false;
    read_litzen = false;
    read_flottierungen = false;

    ausmasse = rAusmasse->Lines;
    farben = rFarben->Lines;
    litzen = rLitzen->Lines;
    flottierungen = rFlottierungen->Lines;
}
/*-----------------------------------------------------------------*/
void __fastcall TEntwurfsinfoForm::CopyStrings (TStrings* _dest, TStrings* _src, bool _append/*=false*/)
{
    if (!_append) _dest->Clear();
    _dest->AddStrings (_src);
}
/*-----------------------------------------------------------------*/
void __fastcall TEntwurfsinfoForm::categoriesChange(TObject *Sender,TTreeNode *Node)
{
    if (Node==categories->Items->Item[0]) OnAusmasse();
    else if (Node==categories->Items->Item[1]) OnFarben();
    else if (Node==categories->Items->Item[2]) OnFlottierungen();
    else if (Node==categories->Items->Item[3]) OnLitzen();
}
/*-----------------------------------------------------------------*/
void __fastcall TEntwurfsinfoForm::InsertHeader()
{
    InsertHeading (String(APP_TITLE)+ENTWURFSINFO);
    if (frm->filename!="")
        text->Lines->Add (String(MUSTER)+frm->filename);
    text->Lines->Add ("");
}
/*-----------------------------------------------------------------*/
void __fastcall TEntwurfsinfoForm::InsertHeading (const String& _heading)
{
    dummy->DefAttributes->Assign(text->DefAttributes);
    text->DefAttributes->Style = text->DefAttributes->Style << fsBold;
    text->Lines->Add (_heading);
    text->DefAttributes->Assign(dummy->DefAttributes);
}
/*-----------------------------------------------------------------*/
void __fastcall TEntwurfsinfoForm::bPrintClick(TObject *Sender)
{
    TCursor oldcursor = Cursor;
    Cursor = crHourGlass;
    TTreeNode* pItem = categories->Selected;

    // Ausgabe aufbauen
    text->Clear();
    text->Visible = false;
    text->Color = clWhite;
    InsertHeader();
    InsertHeading (ENTW_ALLGEMEIN);
    OnAusmasse(false);
    InsertHeading (ENTW_FARBEN);
    OnFarben(false);
    InsertHeading (ENTW_FLOTTIERUNGEN);
    OnFlottierungen(false);
    InsertHeading (ENTW_LITZEN);
    OnLitzen(false);

    // Ausdrucken
    text->Print (String(APP_TITLE)+ENTWURFSINFO);

    // Zustand wiederherstellen
    text->Clear();
    categories->Selected = pItem;
    categoriesChange (this, pItem);
    Cursor = oldcursor;
    text->Visible = true;
    ::MessageBeep(MB_OK);
}
/*-----------------------------------------------------------------*/
void __fastcall TEntwurfsinfoForm::bExportClick(TObject *Sender)
{
    if (frm->filename!="") {
        String fn = ExtractFileName (frm->filename);
        int i = fn.Length();
        while (fn[i]!='.' && i>0) i--;
        if (i>0) fn = fn.SubString (1, i)+"rtf";
        SaveDialog->FileName = fn;
    } else
        SaveDialog->FileName = DATEI_UNBENANNT;
    if (!SaveDialog->Execute()) return;

    TTreeNode* pItem = categories->Selected;
    TCursor oldcursor = Cursor;
    Cursor = crHourGlass;

    // Ausgabe aufbauen
    text->Clear();
    text->Visible = false;
    InsertHeader();
    InsertHeading (ENTW_ALLGEMEIN);
    OnAusmasse(false);
    InsertHeading (ENTW_FARBEN);
    OnFarben(false);
    InsertHeading (ENTW_FLOTTIERUNGEN);
    OnFlottierungen(false);
    InsertHeading (ENTW_LITZEN);
    OnLitzen(false);

    // Speichern
    text->Lines->SaveToFile (SaveDialog->FileName);

    // Zustand wiederherstellen
    text->Clear();
    categories->Selected = pItem;
    categoriesChange (this, pItem);
    text->Visible = true;
    Cursor = oldcursor;

    // Datei öffnen
    ShellExecute (NULL, "open", SaveDialog->FileName.c_str(), "", "", SW_SHOW);
}
/*-----------------------------------------------------------------*/
void __fastcall TEntwurfsinfoForm::InitInfos()
{
    categories->Selected = categories->Items->Item[0];
    OnAusmasse();
}
/*-----------------------------------------------------------------*/
void __fastcall TEntwurfsinfoForm::OnAusmasse (bool _clear/*=true*/)
{
    if (_clear) text->Lines->Clear();

    if (!read_ausmasse) {
        if (frm->kette.a==-1 || frm->kette.b==-1 || frm->schuesse.a==-1 || frm->schuesse.b==-1)
            return;

        ausmasse->Clear();

        // Mustername
        if (frm->filename!="")
            ausmasse->Add (String(ENTW_MUSTERNAME)+ExtractFileName(frm->filename));

        // Bindungsgrösse
        ausmasse->Add (String(ENTW_BINDUNGSGROESSE)+
            IntToStr(frm->kette.b-frm->kette.a+1)+String(" x ")+
            IntToStr(frm->schuesse.b-frm->schuesse.a+1));

        // Bindungsrapport
        if (frm->rapport.kr.b!=-1 && frm->rapport.sr.b!=-1)
            ausmasse->Add (String(ENTW_BINDUNGSRAPPORT)+
                IntToStr(frm->rapport.kr.b-frm->rapport.kr.a+1)+String(" x ")+
                IntToStr(frm->rapport.sr.b-frm->rapport.sr.a+1));

        // Farbrapport
        //xxxx

        // Anzahl Schäfte
        int nschaefte = 0;
        for (int i=0; i<Data->MAXY1; i++)
            if (!frm->freieschaefte[i]) nschaefte++;
        ausmasse->Add(String(ENTW_SCHAFTCOUNT)+IntToStr(nschaefte));

        // Anzahl Tritte
        int ntritte = 0;
        for (int i=0; i<Data->MAXX2; i++)
            if (!frm->freietritte[i]) ntritte++;
        ausmasse->Add(String(ENTW_TRITTCOUNT)+IntToStr(ntritte));

        // Gleichseitig oder Kett-/Schusslastig
        int total = (frm->kette.b-frm->kette.a+1)*(frm->schuesse.b-frm->schuesse.a+1);
        int hebungen = 0;
        int senkungen = 0;
        for (int i=frm->kette.a; i<=frm->kette.b; i++)
            for (int j=frm->schuesse.a; j<=frm->schuesse.b; j++)
                if (frm->gewebe.feld.Get(i, j)>0) hebungen++;
                else senkungen++;
        if (hebungen<senkungen) {
            char prz[10];
            sprintf (prz, "%1.3g", double(100)*senkungen/total);
            ausmasse->Add (String(ENTW_SCHUSSSEITIG1)+prz+
                ENTW_SCHUSSSEITIG2);
        } else if (hebungen>senkungen) {
            char prz[10];
            sprintf (prz, "%1.3g", double(100)*hebungen/total);
            ausmasse->Add (String(ENTW_KETTSEITIG1)+prz+
                ENTW_KETTSEITIG2);
        } else {
            ausmasse->Add (ENTW_GLEICHSEITIG);
        }

        ausmasse->Add("");
        read_ausmasse = true;
    }

    CopyStrings (text->Lines, ausmasse, !_clear);
}
/*-----------------------------------------------------------------*/
void __fastcall TEntwurfsinfoForm::OnFarben (bool _clear/*=true*/)
{
    if (_clear) text->Lines->Clear();

    if (!read_farben) {
        farben->Clear();
        // Anzahl Farben
        int farbcount = 0;
        int kfarbcount = 0;
        int sfarbcount = 0;
        bool coltable[MAX_PAL_ENTRY];
        int  ktable[MAX_PAL_ENTRY];
        int  stable[MAX_PAL_ENTRY];
        for (int i=0; i<=MAX_PAL_ENTRY; i++) {
            coltable[i] = false;
            ktable[i] = stable[i] = 0;
        }
        for (int i=frm->kette.a; i<=frm->kette.b; i++) {
            if (!coltable[frm->kettfarben.feld.Get(i)]) { farbcount++; kfarbcount++; }
            coltable[frm->kettfarben.feld.Get(i)] = true;
            ktable[frm->kettfarben.feld.Get(i)]++;
        }
        for (int j=frm->schuesse.a; j<=frm->schuesse.b; j++) {
            if (!coltable[frm->schussfarben.feld.Get(j)]) { farbcount++; sfarbcount++; }
            coltable[frm->schussfarben.feld.Get(j)] = true;
            stable[frm->schussfarben.feld.Get(j)]++;
        }
        // Anzahl Farben
        farben->Add (ENTW_FARBENCOUNT);
        farben->Add (String(ENTW_FARBENGESAMT)+IntToStr(farbcount));
        farben->Add (String(ENTW_FARBENKETTE)+IntToStr(kfarbcount));
        farben->Add (String(ENTW_FARBENSCHUSS)+IntToStr(sfarbcount));
        farben->Add ("");

        // Kettfarben
        farben->Add (ENTW_KETTFARBEN);
        int count = 1;
        int ktotal = frm->kette.b-frm->kette.a+1;
        for (int i=0; i<MAX_PAL_ENTRY; i++) {
            if (ktable[i]!=0) {
                char prz[10];
                sprintf (prz, "%1.3g", double(100)*ktable[i]/ktotal);
                char col[20];
                float h, s, v;
                RGB2HSV (GetRValue(Data->palette->GetColor(i)),
                         GetGValue(Data->palette->GetColor(i)),
                         GetBValue(Data->palette->GetColor(i)), h, s, v);
                sprintf (col, "%1.3g,%1.3g,%1.3g", h, s, v);
                farben->Add(String(ENTW_FARBE)+IntToStr(count)+
                    ": HSV=("+col+
                    "), "+
                    "RGB=("+IntToStr(GetRValue(Data->palette->GetColor(i)))+
                    ","+IntToStr(GetGValue(Data->palette->GetColor(i)))+
                    ","+IntToStr(GetBValue(Data->palette->GetColor(i)))+
                    "), "+IntToStr(ktable[i])+ENTW_KETTFAEDEN+
                    prz+"%)");
                count++;
            }
        }
        farben->Add("");

        // Schussfarben
        farben->Add (ENTW_SCHUSSFARBEN);
        count = 1;
        int stotal = frm->schuesse.b-frm->schuesse.a+1;
        for (int i=0; i<MAX_PAL_ENTRY; i++) {
            if (stable[i]!=0) {
                char prz[10];
                sprintf (prz, "%1.3g", double(100)*stable[i]/stotal);
                char col[20];
                float h, s, v;
                RGB2HSV (GetRValue(Data->palette->GetColor(i)),
                         GetGValue(Data->palette->GetColor(i)),
                         GetBValue(Data->palette->GetColor(i)), h, s, v);
                sprintf (col, "%1.3g,%1.3g,%1.3g", h, s, v);
                farben->Add(String(ENTW_FARBE)+IntToStr(count)+
                    ": HSV=("+col+
                    "), "+
                    "RGB=("+IntToStr(GetRValue(Data->palette->GetColor(i)))+
                    ","+IntToStr(GetGValue(Data->palette->GetColor(i)))+
                    ","+IntToStr(GetBValue(Data->palette->GetColor(i)))+
                    "), "+IntToStr(stable[i])+ENTW_SCHUSSFAEDEN+
                    prz+"%)");
                count++;
            }
        }

        farben->Add ("");
        read_farben = true;
    }

    CopyStrings (text->Lines, farben, !_clear);
}
/*-----------------------------------------------------------------*/
void __fastcall TEntwurfsinfoForm::OnLitzen (bool _clear/*=true*/)
{
    TCursor oldcursor = Cursor;
    Cursor = crHourGlass;
    if (_clear) text->Lines->Clear();

    if (!read_litzen) {
        litzen->Clear();
        // Anzahl Litzen
        int litzennr = 0;
        for (int i=0; i<Data->MAXX1; i++)
            if (frm->einzug.feld.Get(i)!=0)
                litzennr++;
        litzen->Add (String(ENTW_LITZENCOUNT)+IntToStr(litzennr));

        // Verteilung auf die Schäfte
        litzen->Add (ENTW_LITZENAUFTEILUNG);
        int schaft = 0;
        for (int j=0; j<Data->MAXY1; j++) {
            if (!frm->freieschaefte[j]) {
                schaft++;
                int count = 0;
                for (int i=0; i<Data->MAXX1; i++)
                    if (frm->einzug.feld.Get(i)==j+1)
                        count++;
                char prz[10];
                sprintf (prz, "%1.3g", double(100)*count/litzennr);
                litzen->Add (String(ENTW_LITZENSCHAFT)+IntToStr(schaft)+
                    String(":  ")+IntToStr(count)+String(ENTW_LITZEN2)+
                    String(prz)+
                    String ("%)"));
            }
        }
        litzen->Add("");
        read_litzen = true;
    }
    Cursor = oldcursor;

    CopyStrings (text->Lines, litzen, !_clear);
}
/*-----------------------------------------------------------------*/
void __fastcall TEntwurfsinfoForm::OnFlottierungen (bool _clear/*=true*/)
{
    TCursor oldcursor = Cursor;
    Cursor = crHourGlass;
    if (_clear) text->Lines->Clear();

    if (!read_flottierungen) {
        flottierungen->Clear();
        // Schussflottierungen ermitteln
        try {
            int maxflot = frm->kette.b-frm->kette.a+1+1;
            // Flottierungstabelle initialisieren
            int* table = new int[maxflot];
            for (int i=0; i<maxflot; i++)
                table[i] = 0;
            int biggest = 0;
            int count = 0;
            // Flottierungsdaten ermitteln
            for (int j=frm->schuesse.a; j<=frm->schuesse.b; j++) {
                bool inflot = frm->gewebe.feld.Get(frm->kette.a, j)<=0;
                int startflot = -1;
                if (inflot) startflot = frm->kette.a;
                for (int i=frm->kette.a; i<=frm->kette.b; i++) {
                    if (inflot) {
                        if (frm->gewebe.feld.Get(i, j)>0) {
                            table[i-startflot]++;
                            count++;
                            if (i-startflot>biggest) biggest = i-startflot;
                            inflot = false;
                            startflot = -1;
                        }
                    } else if (frm->gewebe.feld.Get (i, j)<=0) {
                        inflot = true;
                        startflot = i;
                    }
                }
                if (inflot) {
                    int flot = frm->kette.b+1-startflot;
                    table[flot]++;
                    count++;
                    if (flot>biggest) biggest = flot;
                }
            }
            // Daten ausgeben
            flottierungen->Add (ENTW_SCHUSSFLOTT);
            flottierungen->Add (String(ENTW_MAXFLOTT)+IntToStr(biggest));
            double mittel = 0.0;
            for (int i=1; i<maxflot; i++)
                if (table[i]!=0)
                    mittel = mittel + double(table[i])*i/count;
            char prz[10];
            sprintf (prz, "%1.3g", mittel);
            flottierungen->Add (String(ENTW_AVGFLOTT)+String(prz));
            flottierungen->Add (ENTW_FLOTTDIST);
            for (int i=1; i<maxflot; i++)
                if (table[i]!=0) {
                    char prz[10];
                    sprintf (prz, "%1.3g", double(100)*table[i]/count);
                    flottierungen->Add (String(ENTW_FLOTT1)+IntToStr(i)+String(":  ")+
                        IntToStr(table[i])+String(ENTW_FLOTT2)+
                        String(prz)+String("%)"));
                }
            delete[] table;
        } catch(...) {
        }

        flottierungen->Add ("");

        // Kettflottierungen ermitteln
        try {
            int maxflot = frm->schuesse.b-frm->schuesse.a+1+1;
            // Flottierungstabelle initialisieren
            int* table = new int[maxflot];
            for (int i=0; i<maxflot; i++)
                table[i] = 0;
            int biggest = 0;
            int count = 0;
            // Flottierungsdaten ermitteln
            for (int i=frm->kette.a; i<=frm->kette.b; i++) {
                bool inflot = frm->gewebe.feld.Get(i, frm->schuesse.a)>0;
                int startflot = -1;
                if (inflot) startflot = frm->schuesse.a;
                for (int j=frm->schuesse.a; j<=frm->schuesse.b; j++) {
                    if (inflot) {
                        if (frm->gewebe.feld.Get(i, j)<=0) {
                            table[j-startflot]++;
                            count++;
                            if (j-startflot>biggest) biggest = j-startflot;
                            inflot = false;
                            startflot = -1;
                        }
                    } else if (frm->gewebe.feld.Get (i, j)>0) {
                        inflot = true;
                        startflot = j;
                    }
                }
                if (inflot) {
                    int flot = frm->schuesse.b+1-startflot;
                    table[flot]++;
                    count++;
                    if (flot>biggest) biggest = flot;
                }
            }
            // Daten ausgeben
            flottierungen->Add (ENTW_KETTFLOTT);
            flottierungen->Add (String(ENTW_MAXFLOTT)+IntToStr(biggest));
            double mittel = 0.0;
            for (int i=1; i<maxflot; i++)
                if (table[i]!=0)
                    mittel = mittel + double(table[i])*i/count;
            char prz[10];
            sprintf (prz, "%1.3g", mittel);
            flottierungen->Add (String(ENTW_AVGFLOTT)+String(prz));
            flottierungen->Add (ENTW_FLOTTDIST);
            for (int i=1; i<maxflot; i++)
                if (table[i]!=0) {
                    char prz[10];
                    sprintf (prz, "%1.3g", double(100)*table[i]/count);
                    flottierungen->Add (String(ENTW_FLOTT1)+IntToStr(i)+String(":  ")+
                        IntToStr(table[i])+String(ENTW_FLOTT2)+
                        String(prz)+String("%)"));
                }

            delete[] table;
        } catch(...) {
        }
        flottierungen->Add("");
    }
    Cursor = oldcursor;

    CopyStrings (text->Lines, flottierungen, !_clear);
}
/*-----------------------------------------------------------------*/
void __fastcall TEntwurfsinfoForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Pattern information", "")
    LANG_C_H(this, GE, "Musterinformationen", "")
    LANG_C_H(bExport, EN, "&Save...", "")
    LANG_C_H(bExport, GE, "&Speichern...", "")
    LANG_C_H(bPrint, EN, "&Print", "")
    LANG_C_H(bPrint, GE, "&Drucken", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")

    SaveDialog->Title = LANG_STR("Musterinfos speichern unter", "Save pattern info as");
    SaveDialog->Filter = LANG_STR("Word-Format (*.rtf)|*.rtf", "Word-Format (*.rtf)|*.rtf");
    if (SaveDialog->FileName=="unbenannt" || SaveDialog->FileName=="unnamed")
        SaveDialog->FileName = DATEI_UNBENANNT;

    categories->Items->Item[0]->Text = LANG_STR("General", "Allgemeines");
    categories->Items->Item[1]->Text = LANG_STR("Colors", "Farben");
    categories->Items->Item[2]->Text = LANG_STR("Floats", "Flottierungen");
    categories->Items->Item[3]->Text = LANG_STR("Heddles", "Litzen");
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ViewInfosClick(TObject *Sender)
{
    try {
        TEntwurfsinfoForm* frm = new TEntwurfsinfoForm(this, this);
        frm->InitInfos();
        frm->ShowModal();
        delete frm;
    } catch (...) {
        dbw3_assert(false);
    }
}
/*-----------------------------------------------------------------*/


