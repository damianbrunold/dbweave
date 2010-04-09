// DB-WEAVE, textile CAD/CAM software
// Copyright (c) 1998-2005 Brunold Software, Switzerland

#include <vcl.h>
#pragma hdrstop

#include "steuerung_form.h"
#include "dbw3_form.h"
#include "assert.h"
#include "steuerung.h"
#include "settings.h"
#include "dbw3_strings.h"
#include "datamodule.h"
#include "strggoto_form.h"
#include "cursor.h"
#include "language.h"

#pragma package(smart_init)
#pragma link "ComPort"
#pragma resource "*.dfm"

TSTRGFRM* STRGFRM;
bool strgfrm_loaded = false;

void __fastcall TDBWFRM::WeaveClick(TObject *Sender)
{
    // Steuerungsform anlegen und Modal anzeigen
    TIdleEvent save_idle = Application->OnIdle;
    TNotifyEvent save_hint = Application->OnHint;
    try {
        // Grösse wie Hauptfenster
        STRGFRM->Left = Left;          
        STRGFRM->Top = Top;
        STRGFRM->Width = Width;
        STRGFRM->Height = Height;

        // Daten abfüllen
        bool filldata = true;
        for (int i=0; i<MAXKLAMMERN; i++)
            if (klammern[i].repetitions>0) filldata = false;
        if (filldata) {
            // Defaultklammer setzen
            if (schuesse.count()!=0) {
                klammern[0].repetitions = 1;
                klammern[0].first = schuesse.a;
                klammern[0].last = schuesse.b;
            } else {
                klammern[0].repetitions = 1;
                klammern[0].first = 0;
                klammern[0].last = 8;
            }
        }
        for (int i=0; i<MAXKLAMMERN; i++)
            STRGFRM->klammern[i] = klammern[i];
        for (int i=0; i<10; i++)
            STRGFRM->zoom[i] = zoom[i];
        STRGFRM->currentzoom = currentzoom;
        STRGFRM->schlagpatrone = ViewSchlagpatrone->Checked;
        STRGFRM->schlagpatronendarstellung = schlagpatronendarstellung;
        STRGFRM->rapportx = rapport.kr.b-rapport.kr.a+1;
        STRGFRM->rapporty = rapport.sr.b-rapport.sr.a+1;
        STRGFRM->einzug = &einzug;
        STRGFRM->trittfolge = &trittfolge;
        STRGFRM->gewebe = &gewebe;
        STRGFRM->aufknuepfung = &aufknuepfung;
        STRGFRM->schussfarben = &schussfarben;
        STRGFRM->kettfarben = &kettfarben;
        STRGFRM->gridsize = min(gridwidth, gridheight);
        STRGFRM->tritte = wvisible;
        STRGFRM->fewithraster = fewithraster;
        STRGFRM->filename = filename;
        STRGFRM->ValidateWeavePosition();
        STRGFRM->SetWeaving(weaving);

        // Darstellung bestimmen
        STRGFRM->ViewPatrone->Checked = GewebeNormal->Checked;
        STRGFRM->ViewPatronePopup->Checked = GewebeNormal->Checked;
        STRGFRM->ViewFarbeffekt->Checked = GewebeFarbeffekt->Checked;
        STRGFRM->ViewFarbeffektPopup->Checked = GewebeFarbeffekt->Checked;
        STRGFRM->ViewGewebesimulation->Checked = GewebeSimulation->Checked;
        STRGFRM->ViewGewebesimulationPopup->Checked = GewebeSimulation->Checked;

        // Anzeigen
        STRGFRM->ShowModal();
        weaving = STRGFRM->Weaving();
        if (STRGFRM->weave_position>=scroll_y2 && STRGFRM->weave_position<scroll_y2+gewebe.pos.height/gewebe.gh) {
            if (ViewTrittfolge->Checked) cursorhandler->SetCursor (TRITTFOLGE, scroll_x2, STRGFRM->weave_position, true);
            else cursorhandler->SetCursor (GEWEBE, scroll_x1, STRGFRM->weave_position, true);
            showweaveposition = true;
            weave_position = STRGFRM->weave_position;
        } else {
            int gj = gewebe.pos.height/gewebe.gh;
            int y2 = STRGFRM->weave_position - gj/2;
            if (y2<0) y2 = 0;
            if (y2+gj>=Data->MAXY2) y2 = Data->MAXY2 - 1 - gj;
            scroll_y2 = y2;
            UpdateScrollbars();
            sb_vert2->Position = Data->MAXY2 - gj - scroll_y2;
            if (ViewTrittfolge->Checked) cursorhandler->SetCursor (TRITTFOLGE, scroll_x2, STRGFRM->weave_position, true);
            else cursorhandler->SetCursor (GEWEBE, scroll_x1, STRGFRM->weave_position, true);
            showweaveposition = true;
            weave_position = STRGFRM->weave_position;
        }
        for (int i=0; i<MAXKLAMMERN; i++)
            klammern[i] = STRGFRM->klammern[i];
        if (STRGFRM->Modified()) SetModified();
    } catch (...) {
    }
    Application->OnIdle = save_idle;
    Application->OnHint = save_hint;
}

__fastcall TSTRGFRM::TSTRGFRM(TComponent* Owner)
: TForm(Owner)
{
    strgfrm_loaded = true;
    ReloadLanguage();
    
    port = P_COM1;
    lpt = LP_LPT1;
    delay = 3;
    intrf = intrf_arm_patronic;
    modified = false;
    reverse = false;
    
    LoadSettings();

    einzug = NULL;
    aufknuepfung = NULL;
    trittfolge = NULL;
    gewebe = NULL;

    fewithraster = false;

    maxweave = 0;

    weave_position = 0;
    weave_klammer = 0;
    weave_repetition = 1;
    current_klammer = 0;

    dragging = false;

    weaving = false;

    scrolly = 0;
    scrollbar->SetParams (Data->MAXY2, 0, Data->MAXY2);

    _ResetCurrentPos();

    schussselected = true;

    controller = NULL;
    AllocInterface();

    stopit = true;
}

__fastcall TSTRGFRM::~TSTRGFRM()
{
    if (controller) {
        controller->Terminate();
        delete controller;
    }
}

bool __fastcall TSTRGFRM::Weaving()
{
    return weaving;
}

void __fastcall TSTRGFRM::SetWeaving (bool _weaving/*=true*/)
{
    weaving = _weaving;
}

void __fastcall TSTRGFRM::LoadSettings()
{
    Settings settings;
    settings.SetCategory ("Loom");
    intrf  = (LOOMINTERFACE)settings.Load ("Interface", (int)intrf_arm_patronic);
    port   = (PORT)settings.Load ("Port", (int)P_COM1);
    lpt    = (LPT)settings.Load ("Lpt", (int)LP_LPT1);
    delay  = (int)settings.Load ("Delay", (int)3);
    Loop->Checked = (bool)settings.Load ("Endless", (int)true);
    ReverseSchaft->Checked = (bool)settings.Load ("ShaftsReversed", (int)false);
    reverse = ReverseSchaft->Checked;

    settings.SetCategory ("Loom");
    int harnesses = settings.Load ("NumberOfShafts", 24);

    switch (harnesses) {
        case 4: Schafts4->Checked = true; break;
        case 8: Schafts8->Checked = true; break;
        case 12: Schafts12->Checked = true; break;
        case 16: Schafts16->Checked = true; break;
        case 20: Schafts20->Checked = true; break;
        case 24: Schafts24->Checked = true; break;
        case 28: Schafts28->Checked = true; break;
        case 32: Schafts32->Checked = true; break;
        default: Schafts24->Checked = true; break;
    }
}

void __fastcall TSTRGFRM::SaveSettings()
{
    Settings settings;
    settings.SetCategory ("Loom");
    settings.Save ("Interface", (int)intrf);
    settings.Save ("Port", (int)port);
    settings.Save ("Lpt", (int)lpt);
    settings.Save ("Delay", (int)delay);
    settings.Save ("Endless", (int)Loop->Checked);
    settings.Save ("ShaftsReversed", (int)ReverseSchaft->Checked);
}

void __fastcall TSTRGFRM::WebenBeendenClick(TObject *Sender)
{
    if (!stopit) {
        WeaveTempQuit();
        tempquit = true;
    } else {
        ModalResult = mrOk;
    }
}

void __fastcall TSTRGFRM::FormClose(TObject *Sender, TCloseAction &Action)
{
    if (!stopit) {
        WeaveStopClick(this);
        weaving = true;
    }
    SaveSettings();
}

void __fastcall TSTRGFRM::WMSwitchShortcuts(TMessage& _message)
{
    TShortCut sc = ShortCut (VK_F5, TShiftState());
    if (WeaveStop->ShortCut == sc) {
        WeaveStop->ShortCut = 0;
        WeaveStart->ShortCut = sc;
    } else {
        WeaveStop->ShortCut = sc;
        WeaveStart->ShortCut = 0;
    }
}

void __fastcall TSTRGFRM::WMAutoStartWeave(TMessage& _message)
{
    if (intrf!=intrf_arm_patronic_indirect)
        WeaveStartClick(this);
}

void __fastcall TSTRGFRM::UpdateStatusbar()
{
    try {
        if (schussselected) {
            if (!IsValidWeavePosition()) {
                statusbar->SimpleText = String(WEB_PREFIXINVALIDPOS) + WEB_SCHUSSFADEN+IntToStr(weave_position+1);
            } else {
                statusbar->SimpleText = WEB_SCHUSSFADEN+IntToStr(weave_position+1)+
                                        WEB_KLAMMER1+IntToStr(weave_klammer+1)+
                                        WEB_REPEAT+IntToStr (weave_repetition);
            }
        } else {
            statusbar->SimpleText = WEB_KLAMMER+IntToStr(current_klammer+1)+
                                    WEB_KL_LENGTH+IntToStr(klammern[current_klammer].last-klammern[current_klammer].first+1)+
                                    WEB_KL_START+IntToStr(klammern[current_klammer].first+1)+
                                    WEB_KL_STOP+IntToStr(klammern[current_klammer].last+1)+
                                    WEB_KL_REPEAT+IntToStr(klammern[current_klammer].repetitions);
        }
    } catch (...) {
    }
}

void __fastcall TSTRGFRM::AllocInterface()
{
    if (controller) delete controller;

    WeaveBackwards->Visible = true;
    wb->Visible = true;

    try {
        switch (intrf) {
            case intrf_arm_patronic:
                controller = new StPatronicController (comport);
                break;

            case intrf_arm_patronic_indirect:
                controller = new StPatronicIndirectController (comport);
                break;

            case intrf_arm_designer:
                controller = new StDesignerController (comport);
                break;

            case intrf_slips:
                controller = new StSlipsController (comport);
                WeaveBackwards->Visible = false;
                wb->Visible = false;
                if (Schafts4->Checked || Schafts8->Checked) controller->SetBytes(1);
                if (Schafts12->Checked || Schafts16->Checked) controller->SetBytes(2);
                if (Schafts20->Checked || Schafts24->Checked) controller->SetBytes(3);
                if (Schafts28->Checked || Schafts32->Checked) controller->SetBytes(4);
                break;

            case intrf_avl_cd_iii:
                controller = new StAvlCdIIIController(comport);
                break;

            default:
                controller = new StDummyController();
        }
    } catch (...) {
        controller = NULL;
    }
}

void __fastcall TSTRGFRM::OptionsLoomClick(TObject *Sender)
{
    try {
        // Webstuhl auswählen
        LOOMINTERFACE oldintrf = intrf;
        PORT oldport = port;
        LPT  oldlpt = lpt;
        int  olddelay = delay;
        TStrgOptLoomForm* pFrm = new TStrgOptLoomForm(this);
        pFrm->IsNT = DBWFRM->IsNT;
        pFrm->intrf = intrf;
        pFrm->port = port;
        pFrm->lpt = lpt;
        pFrm->delay = delay;
        if (pFrm->ShowModal()==mrOk) {
            intrf = pFrm->intrf;
            port = pFrm->port;
            lpt = pFrm->lpt;
            delay = pFrm->delay;
            // Aktualisieren
            if (oldintrf!=intrf || oldport!=port || oldlpt!=lpt || olddelay!=delay) {
                SaveSettings();
                AllocInterface();
            }
        }
        delete pFrm;

    } catch (...) {
    }
}

void __fastcall TSTRGFRM::FormResize(TObject *Sender)
{
    Invalidate();
}

void __fastcall TSTRGFRM::WeaveBackwardsClick(TObject *Sender)
{
    WeaveBackwards->Checked = !WeaveBackwards->Checked;
    ReverseWeben->Checked = !ReverseWeben->Checked;
    SBReverse->Down = !SBReverse->Down;
}

void __fastcall TSTRGFRM::_ResetCurrentPos()
{
    for (weave_klammer=0; weave_klammer<MAXKLAMMERN; weave_klammer++) {
        if (klammern[weave_klammer].repetitions>0) {
            last_position = weave_position = klammern[weave_klammer].first;
            last_repetition = weave_repetition = 1;
            last_klammer = current_klammer = weave_klammer;
            return;
        }
    }
    weave_klammer = last_klammer = current_klammer = 0;
    weave_position = last_position = 0;
    weave_repetition = last_repetition = 1;
}

void __fastcall TSTRGFRM::scrollbarScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos)
{
    switch (ScrollCode) {
        case scLineUp:
        case scLineDown:
        case scPageUp:
        case scPageDown:
        case scPosition: {
            int oldscrolly = scrolly;
            scrolly = Data->MAXY2 - maxj - ScrollPos;
            if (oldscrolly!=scrolly) {
                weave_position += (scrolly - oldscrolly);
                if (weave_position<0) {
                    weave_position = 0;
                    scrollbar->Position = Data->MAXY2 - maxj;
                }
                dbw3_assert(weave_position<Data->MAXY2);
                Invalidate();
            }
            break;
        }
    }
}

void __fastcall TSTRGFRM::SBStartClick(TObject *Sender)
{
    WeaveStartClick(this);
}

void __fastcall TSTRGFRM::SBStopClick(TObject *Sender)
{
    WeaveStopClick(this);
}

void __fastcall TSTRGFRM::SBReverseClick(TObject *Sender)
{
    WeaveBackwards->Checked = !WeaveBackwards->Checked;
    ReverseWeben->Checked = !ReverseWeben->Checked;
}

void __fastcall TSTRGFRM::SBOptionsClick(TObject *Sender)
{
    OptionsLoomClick(this);
}

void __fastcall TSTRGFRM::FormShow(TObject *Sender)
{
    // Titel setzen
    String title = APP_TITLE_WEBEN + APP_TITLE;
    String file;
    if (filename=="") file = DATEI_UNBENANNT;
    else file = ExtractFileName (filename);
    Caption = file + title;

    // Scrollbar nachführen
    UpdateScrollbar();
    if (scrolly>Data->MAXY2-maxj) {
        scrolly = 0;
        UpdateScrollbar();
    }

    // Idle-Handler
    Application->OnIdle = IdleHandler;

    // Hint-Handler
    Application->OnHint = DisplayHint;

    // Falls Weben unterbrochen: Direkt starten...
    if (weaving && stopit)
        PostMessage (Handle, WM_AUTOSTARTWEAVE, 0, 0);
}

void __fastcall TSTRGFRM::ViewPatroneClick(TObject *Sender)
{
    ViewPatrone->Checked = true;
    ViewPatronePopup->Checked = true;
    Invalidate();
}

void __fastcall TSTRGFRM::ViewFarbeffektClick(TObject *Sender)
{
    ViewFarbeffekt->Checked = true;
    ViewFarbeffektPopup->Checked = true;
    Invalidate();
}

void __fastcall TSTRGFRM::ViewGewebesimulationClick(TObject *Sender)
{
    ViewGewebesimulation->Checked = true;
    ViewGewebesimulationPopup->Checked = true;
    Invalidate();
}

void __fastcall TSTRGFRM::AutoScroll()
{
    // Wenn current_tritt näher als auf 1/10 der Fäden
    // an den Rand kommt, so wird versucht, ihn
    // per Scrolling zu zentrieren.

    int onetenth = maxj / 10;

    if (weave_position-scrolly>maxj-onetenth ||
        weave_position-scrolly<onetenth)  {
        int idealscrolly = weave_position - maxj/2;
        if (idealscrolly<0) idealscrolly = 0;
        if (idealscrolly>Data->MAXY2-maxj) idealscrolly = Data->MAXY2-maxj;

        if (scrolly!=idealscrolly) {
            scrollbar->Position = Data->MAXY2 - maxj - idealscrolly;
            scrolly = idealscrolly;
            Invalidate();
            Application->ProcessMessages();
        }
    }
}

void __fastcall TSTRGFRM::LoopClick(TObject *Sender)
{
    Loop->Checked = !Loop->Checked;
}

void __fastcall TSTRGFRM::SetCurrentPosClick(TObject *Sender)
{
    try {
        TStrgGotoForm* frm = new TStrgGotoForm(this);
        // Daten setzen
        frm->upSchuss->Position = short(weave_position+1);
        frm->cbKlammer->ItemIndex = weave_klammer;
        frm->cbRepeat->ItemIndex = weave_repetition-1;
        if (frm->ShowModal()==mrOk) {
            // Position setzen
            ClearSelection();
            ClearPositionSelection();
            weave_position = frm->upSchuss->Position-1;
            weave_klammer = current_klammer = frm->cbKlammer->ItemIndex;
            weave_repetition = frm->cbRepeat->ItemIndex+1;
            ValidateWeavePosition();
            DrawSelection();
            DrawPositionSelection();
            UpdateStatusbar();
        }
    } catch (...) {
    }
}

void __fastcall TSTRGFRM::SetModified()
{
    modified = true;
}

bool __fastcall TSTRGFRM::Modified()
{
    return modified;
}

void __fastcall TSTRGFRM::IdleHandler (TObject* Sender, bool& Done)
{
    // Goto Klammer Menüs aktivieren/deaktivieren
    GotoKlammer1->Enabled = klammern[0].repetitions!=0;
    GotoKlammer2->Enabled = klammern[1].repetitions!=0;
    GotoKlammer3->Enabled = klammern[2].repetitions!=0;
    GotoKlammer4->Enabled = klammern[3].repetitions!=0;
    GotoKlammer5->Enabled = klammern[4].repetitions!=0;
    GotoKlammer6->Enabled = klammern[5].repetitions!=0;
    GotoKlammer7->Enabled = klammern[6].repetitions!=0;
    GotoKlammer8->Enabled = klammern[7].repetitions!=0;
    GotoKlammer9->Enabled = klammern[8].repetitions!=0;

    // Ebenso Toolbarbuttons
    sbGoto1->Enabled = klammern[0].repetitions!=0;
    sbGoto2->Enabled = klammern[1].repetitions!=0;
    sbGoto3->Enabled = klammern[2].repetitions!=0;
    sbGoto4->Enabled = klammern[3].repetitions!=0;
    sbGoto5->Enabled = klammern[4].repetitions!=0;
    sbGoto6->Enabled = klammern[5].repetitions!=0;
    sbGoto7->Enabled = klammern[6].repetitions!=0;
    sbGoto8->Enabled = klammern[7].repetitions!=0;
    sbGoto9->Enabled = klammern[8].repetitions!=0;

    // Und Kontextmenü
    GotoKlammer1Popup->Enabled = klammern[0].repetitions!=0;
    GotoKlammer2Popup->Enabled = klammern[1].repetitions!=0;
    GotoKlammer3Popup->Enabled = klammern[2].repetitions!=0;
    GotoKlammer4Popup->Enabled = klammern[3].repetitions!=0;
    GotoKlammer5Popup->Enabled = klammern[4].repetitions!=0;
    GotoKlammer6Popup->Enabled = klammern[5].repetitions!=0;
    GotoKlammer7Popup->Enabled = klammern[6].repetitions!=0;
    GotoKlammer8Popup->Enabled = klammern[7].repetitions!=0;
    GotoKlammer9Popup->Enabled = klammern[8].repetitions!=0;

    // Statusbar updaten
    UpdateStatusbar();
}

void __fastcall TSTRGFRM::DisplayHint (TObject* Sender)
{
    statusbar->SimpleText = Application->Hint;
}

void __fastcall TSTRGFRM::UpdateScrollbar()
{
    scrollbar->LargeChange = (short)maxj;
    scrollbar->Max  = Data->MAXY2 - maxj;
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask  = SIF_PAGE|SIF_RANGE|SIF_POS|SIF_DISABLENOSCROLL;
    si.nPage  = maxj+1;
    si.nPos   = Data->MAXY2 - maxj - scrolly;
    si.nMin   = 0;
    si.nMax   = Data->MAXY2;
    SetScrollInfo (scrollbar->Handle, SB_CTL, &si, true);
}

void __fastcall TSTRGFRM::ZoomInClick(TObject *Sender)
{
    int maxzoom = sizeof(zoom)/sizeof(int)-1;
    if (currentzoom<maxzoom) {
        currentzoom++;
        CalcSizes();
        Invalidate();
    }
}

void __fastcall TSTRGFRM::ZoomNormalClick(TObject *Sender)
{
    currentzoom = 3;
    CalcSizes();
    Invalidate();
}

void __fastcall TSTRGFRM::ZoomOutClick(TObject *Sender)
{
    if (currentzoom>0) {
        currentzoom--;
        CalcSizes();
        Invalidate();
    }
}

void __fastcall TSTRGFRM::ReverseSchaftClick(TObject *Sender)
{
    ReverseSchaft->Checked = !ReverseSchaft->Checked;
    reverse = ReverseSchaft->Checked;
}

void __fastcall TSTRGFRM::Schafts4Click(TObject *Sender)
{
    Schafts4->Checked = true;
    Settings settings;
    settings.SetCategory ("Loom");
    settings.Save ("NumberOfShafts", 4);
}

void __fastcall TSTRGFRM::Schafts8Click(TObject *Sender)
{
    Schafts8->Checked = true;
    Settings settings;
    settings.SetCategory ("Loom");
    settings.Save ("NumberOfShafts", 8);
}

void __fastcall TSTRGFRM::Schafts12Click(TObject *Sender)
{
    Schafts12->Checked = true;
    Settings settings;
    settings.SetCategory ("Loom");
    settings.Save ("NumberOfShafts", 12);
}

void __fastcall TSTRGFRM::Schafts16Click(TObject *Sender)
{
    Schafts16->Checked = true;
    Settings settings;
    settings.SetCategory ("Loom");
    settings.Save ("NumberOfShafts", 16);
}

void __fastcall TSTRGFRM::Schafts20Click(TObject *Sender)
{
    Schafts20->Checked = true;
    Settings settings;
    settings.SetCategory ("Loom");
    settings.Save ("NumberOfShafts", 20);
}

void __fastcall TSTRGFRM::Schafts24Click(TObject *Sender)
{
    Schafts24->Checked = true;
    Settings settings;
    settings.SetCategory ("Loom");
    settings.Save ("NumberOfShafts", 24);
}

void __fastcall TSTRGFRM::Schafts28Click(TObject *Sender)
{
    Schafts28->Checked = true;
    Settings settings;
    settings.SetCategory ("Loom");
    settings.Save ("NumberOfShafts", 28);
}

void __fastcall TSTRGFRM::Schafts32Click(TObject *Sender)
{
    Schafts32->Checked = true;
    Settings settings;
    settings.SetCategory ("Loom");
    settings.Save ("NumberOfShafts", 32);
}

int __fastcall TSTRGFRM::MaxSchaefte()
{
    if (Schafts4->Checked) return 4;
    if (Schafts8->Checked) return 8;
    if (Schafts12->Checked) return 12;
    if (Schafts16->Checked) return 16;
    if (Schafts20->Checked) return 20;
    if (Schafts24->Checked) return 24;
    if (Schafts28->Checked) return 28;
    if (Schafts32->Checked) return 32;
    return 24;
}

void __fastcall TSTRGFRM::ReloadLanguage()
{
    if (!strgfrm_loaded) return;

    // Menü
    // Menü Weben
    LANG_C_H(MenuWeave, EN, "&Weave", "")
    LANG_C_H(MenuWeave, GE, "&Weben", "")
    LANG_C_H(WeaveStart, EN, "&Start weaving", "Starts weaving")
    LANG_C_H(WeaveStart, GE, "&Start Weben", "Startet das Weben")
    LANG_C_H(WeaveStop, EN, "St&op weaving", "Stops weaving")
    LANG_C_H(WeaveStop, GE, "St&opp Weben", "Stoppt das Weben")
    LANG_C_H(WeaveBackwards, EN, "Weave &backwards", "Weaves backwards")
    LANG_C_H(WeaveBackwards, GE, "&Rückwärts weben", "Webt rückwärts")
    LANG_C_H(WebenBeenden, EN, "&Close", "Exits the weave mode")
    LANG_C_H(WebenBeenden, GE, "S&chliessen", "Verlässt den Webenmodus")

    // Menü Position
    LANG_C_H(MenuPosition, EN, "&Position", "")
    LANG_C_H(MenuPosition, GE, "&Position", "")
    LANG_C_H(SetCurrentPos, EN, "&Set current position...", "Sets the current weaving position")
    LANG_C_H(SetCurrentPos, GE, "&Aktuelle Position setzen...", "Setzt die aktuelle Webposition")
    LANG_C_H(GotoLastPos, EN, "&Last woven pick", "Goes to the last woven weft thread")
    LANG_C_H(GotoLastPos, GE, "&Zuletzt gewobener Schuss", "Geht zum letzten gewobenen Schuss")
    LANG_C_H(MenuGotoKlammer, EN, "&Goto", "")
    LANG_C_H(MenuGotoKlammer, GE, "&Gehe zu", "")
    LANG_C_H(GotoKlammer1, EN, "Brace &1", "Goes to brace 1")
    LANG_C_H(GotoKlammer1, GE, "Klammer &1", "Geht zur Klammer 1")
    LANG_C_H(GotoKlammer2, EN, "Brace &2", "Goes to brace 2")
    LANG_C_H(GotoKlammer2, GE, "Klammer &2", "Geht zur Klammer 2")
    LANG_C_H(GotoKlammer3, EN, "Brace &3", "Goes to brace 3")
    LANG_C_H(GotoKlammer3, GE, "Klammer &3", "Geht zur Klammer 3")
    LANG_C_H(GotoKlammer4, EN, "Brace &4", "Goes to brace 4")
    LANG_C_H(GotoKlammer4, GE, "Klammer &4", "Geht zur Klammer 4")
    LANG_C_H(GotoKlammer5, EN, "Brace &5", "Goes to brace 5")
    LANG_C_H(GotoKlammer5, GE, "Klammer &5", "Geht zur Klammer 5")
    LANG_C_H(GotoKlammer6, EN, "Brace &6", "Goes to brace 6")
    LANG_C_H(GotoKlammer6, GE, "Klammer &6", "Geht zur Klammer 6")
    LANG_C_H(GotoKlammer7, EN, "Brace &7", "Goes to brace 7")
    LANG_C_H(GotoKlammer7, GE, "Klammer &7", "Geht zur Klammer 7")
    LANG_C_H(GotoKlammer8, EN, "Brace &8", "Goes to brace 8")
    LANG_C_H(GotoKlammer8, GE, "Klammer &8", "Geht zur Klammer 8")
    LANG_C_H(GotoKlammer9, EN, "Brace &9", "Goes to brace 9")
    LANG_C_H(GotoKlammer9, GE, "Klammer &9", "Geht zur Klammer 9")

    // Menü Ansicht
    LANG_C_H(MenuAnsicht, EN, "&View", "")
    LANG_C_H(MenuAnsicht, GE, "&Ansicht", "")
    LANG_C_H(ViewPatrone, EN, "&Draft", "Displays the pattern in draft mode")
    LANG_C_H(ViewPatrone, GE, "&Patrone", "Zeigt das Muster im Patronenmodus an")
    LANG_C_H(ViewFarbeffekt, EN, "&Color", "Displays the pattern in color mode")
    LANG_C_H(ViewFarbeffekt, GE, "&Farbeffekt", "Zeigt das Muster im Farbeffektmodus an")
    LANG_C_H(ViewGewebesimulation, EN, "&Fabric", "Displays the pattern in fabric mode")
    LANG_C_H(ViewGewebesimulation, GE, "&Gewebe", "Zeigt das Muster im Gewebesimulationsmodus an")
    LANG_C_H(ZoomIn, EN, "&Zoom in", "Zooms in")
    LANG_C_H(ZoomIn, GE, "&Vergrössern", "Vergrössert die Ansicht")
    LANG_C_H(ZoomNormal, EN, "&Normal", "Normal view size")
    LANG_C_H(ZoomNormal, GE, "&Normal", "Normale Ansichtsgrösse")
    LANG_C_H(ZoomOut, EN, "Zoom &out", "Zooms out")
    LANG_C_H(ZoomOut, GE, "Ver&kleinern", "Verkleinert die Ansicht")

    // Menü Optionen
    LANG_C_H(MenuOptions, EN, "&Options", "")
    LANG_C_H(MenuOptions, GE, "&Optionen", "")
    LANG_C_H(OptionsLoom, EN, "&Loom...", "Sets loom type and options")
    LANG_C_H(OptionsLoom, GE, "&Webstuhl...", "Setzt Webstuhltyp und -optionen")
    LANG_C_H(Loop, EN, "&Endless", "Endless weaving mode")
    LANG_C_H(Loop, GE, "&Endlos", "Endloser Webmodus")
    LANG_C_H(ReverseSchaft, EN, "&Reversed harnesses", "Harnesses are reversed")
    LANG_C_H(ReverseSchaft, GE, "&Umgekehrte Schaftreihenfolge", "Schäfte sind umgekehrt")
    LANG_C_H(MenuSchaefte, EN, "&Number of harnesses", "")
    LANG_C_H(MenuSchaefte, GE, "&Anzahl Schäfte", "")
    LANG_C_H(Schafts4, EN, "4 harnesses", "")
    LANG_C_H(Schafts4, GE, "4 Schäfte", "")
    LANG_C_H(Schafts8, EN, "8 harnesses", "")
    LANG_C_H(Schafts8, GE, "8 Schäfte", "")
    LANG_C_H(Schafts12, EN, "12 harnesses", "")
    LANG_C_H(Schafts12, GE, "12 Schäfte", "")
    LANG_C_H(Schafts16, EN, "16 harnesses", "")
    LANG_C_H(Schafts16, GE, "16 Schäfte", "")
    LANG_C_H(Schafts20, EN, "20 harnesses", "")
    LANG_C_H(Schafts20, GE, "20 Schäfte", "")
    LANG_C_H(Schafts24, EN, "24 harnesses", "")
    LANG_C_H(Schafts24, GE, "24 Schäfte", "")
    LANG_C_H(Schafts28, EN, "28 harnesses", "")
    LANG_C_H(Schafts28, GE, "28 Schäfte", "")
    LANG_C_H(Schafts32, EN, "32 harnesses", "")
    LANG_C_H(Schafts32, GE, "32 Schäfte", "")

    // Popupmenü
    LANG_C_H(Cancel, EN, "Cancel", "Exits this menu")
    LANG_C_H(Cancel, GE, "Abbrechen", "Verlässt dieses Menü")
    LANG_C_H(StartWeben, EN, "Start weaving", "Starts weaving")
    LANG_C_H(StartWeben, GE, "Start Weben", "Startet das Weben")
    LANG_C_H(StopWeben, EN, "Stop weaving", "Stops weaving")
    LANG_C_H(StopWeben, GE, "Stop Weben", "Stoppt das Weben")
    LANG_C_H(ReverseWeben, EN, "Backwards", "Weaves backwards")
    LANG_C_H(ReverseWeben, GE, "Rückwärts", "Webt rückwärts")
    LANG_C_H(GotoLast, EN, "Last woven pick", "Goes to the last woven warp thread")
    LANG_C_H(GotoLast, GE, "Zuletzt gewobener Schuss", "Geht zum letzten gewobenen Schuss")
    LANG_C_H(GotoMenu, EN, "Goto", "")
    LANG_C_H(GotoMenu, GE, "Gehe zu", "")
    LANG_C_H(ViewMenu, EN, "View", "")
    LANG_C_H(ViewMenu, GE, "Ansicht", "")
    LANG_C_H(ViewPatronePopup, EN, "Draft", "Displays the pattern in draft mode")
    LANG_C_H(ViewPatronePopup, GE, "Patrone", "Zeigt das Muster im Patronenmodus an")
    LANG_C_H(ViewFarbeffektPopup, EN, "Color", "Displays the pattern in color mode")
    LANG_C_H(ViewFarbeffektPopup, GE, "Farbeffekt", "Zeigt das Muster im Farbeffektmodus an")
    LANG_C_H(ViewGewebesimulationPopup, EN, "Fabric", "Displays the pattern in fabric mode")
    LANG_C_H(ViewGewebesimulationPopup, GE, "Gewebe", "Zeigt das Muster im Gewebesimulationsmodus an")
    LANG_C_H(ZoomInPopup, EN, "Zoom in", "Zooms in")
    LANG_C_H(ZoomInPopup, GE, "Vergrössern", "Vergrössert die Ansicht")
    LANG_C_H(NormalPopup, EN, "Normal", "Normal size")
    LANG_C_H(NormalPopup, GE, "Normal", "Normale Ansichtsgrösse")
    LANG_C_H(ZoomOutPopup, EN, "Zoom out", "Zooms out")
    LANG_C_H(ZoomOutPopup, GE, "Verkleinern", "Verkleinert die Ansicht")
	LANG_C_H(GotoKlammer1Popup, EN, "Brace 1", "Goes to brace 1")
	LANG_C_H(GotoKlammer1Popup, GE, "Klammer 1", "Gehe zu Klammer 1")
	LANG_C_H(GotoKlammer2Popup, EN, "Brace 2", "Goes to brace 2")
	LANG_C_H(GotoKlammer2Popup, GE, "Klammer 2", "Gehe zu Klammer 2")
	LANG_C_H(GotoKlammer3Popup, EN, "Brace 3", "Goes to brace 3")
	LANG_C_H(GotoKlammer3Popup, GE, "Klammer 3", "Gehe zu Klammer 3")
	LANG_C_H(GotoKlammer4Popup, EN, "Brace 4", "Goes to brace 4")
	LANG_C_H(GotoKlammer4Popup, GE, "Klammer 4", "Gehe zu Klammer 4")
	LANG_C_H(GotoKlammer5Popup, EN, "Brace 5", "Goes to brace 5")
	LANG_C_H(GotoKlammer5Popup, GE, "Klammer 5", "Gehe zu Klammer 5")
	LANG_C_H(GotoKlammer6Popup, EN, "Brace 6", "Goes to brace 6")
	LANG_C_H(GotoKlammer6Popup, GE, "Klammer 6", "Gehe zu Klammer 6")
	LANG_C_H(GotoKlammer7Popup, EN, "Brace 7", "Goes to brace 7")
	LANG_C_H(GotoKlammer7Popup, GE, "Klammer 7", "Gehe zu Klammer 7")
	LANG_C_H(GotoKlammer8Popup, EN, "Brace 8", "Goes to brace 8")
	LANG_C_H(GotoKlammer8Popup, GE, "Klammer 8", "Gehe zu Klammer 8")
	LANG_C_H(GotoKlammer9Popup, EN, "Brace 9", "Goes to brace 9")
	LANG_C_H(GotoKlammer9Popup, GE, "Klammer 9", "Gehe zu Klammer 9")
}

