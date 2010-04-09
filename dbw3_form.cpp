// DB-WEAVE, textile CAD/CAM software
// Copyright (c) 1998-2005 Brunold Software, Switzerland

#include <vcl\vcl.h>
#include <vcl\printers.hpp>
#include <mem.h>
#include <shellapi.h>
#pragma hdrstop

#include "assert.h"
#include "dbw3_form.h"
#include "about.h"
#include "felddef.h"
#include "dbw3_strings.h"
#include "datamodule.h"
#include "toolpalette_form.h"
#include "farbpalette_form.h"
#include "palette.h"
#include "properties_form.h"
#include "properties.h"
#include "fileformat.h"
#include "undoredo.h"
#include "blockmuster.h"
#include "einstellverh_form.h"
#include "cursordir_form.h"
#include "cursor.h"
#include "rapport.h"
#include "einzug.h"
#include "settings.h"
#include "overview_form.h"
#include "rangecolors.h"
#include "splash_form.h"
#include "language.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

TDBWFRM *DBWFRM;
TSplashscreen* Splashscreen = NULL;

extern int CURSOR_LARGE_SKIP_X;
extern int CURSOR_LARGE_SKIP_Y;

__fastcall TDBWFRM::TDBWFRM(TComponent* Owner)
:   TForm(Owner),
    bInitialized(false), rangepopupactive(false)
{
    // OS ermitteln
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx (&osvi))
        IsNT = osvi.dwPlatformId == VER_PLATFORM_WIN32_NT;
    else
        IsNT = false; // Annahme von Win 95

    InitRangeColors (Canvas);

    dohighlight = false;

	try {
		undo = new UrUndo (this, 100);
    } catch (...) {
		undo = 0;
    }

    try {
        blockundo = new BlockUndo(&blockmuster, currentbm);
        bereichundo = new BlockUndo(&bereichmuster, currentbm);
    } catch (...) {
        blockundo = 0;
        bereichundo = 0;
    }

    CURSOR_LARGE_SKIP_X  = 8;
    CURSOR_LARGE_SKIP_Y  = 8;

    for (int i=0; i<9; i++) {
        klammern[i].first = 0;
        klammern[i].last = 1;
        klammern[i].repetitions = 0;
    }

    cursorhandler = CrCursorHandler::CreateInstance (this, Data);
    rapporthandler = RpRapport::CreateInstance (this, Data);
    einzughandler = EinzugRearrange::CreateInstance (this, Data);

    savedtrittfolgenstyle = TfMinimalZ;

    weaving = false;

    InitDimensions();

    // Muss nach InitDimensions sein!
    xbuf = ybuf = NULL;
    fixeinzug = NULL;
    firstfree = 0;
    freieschaefte = NULL;
    freietritte = NULL;
    AllocBuffers(false);
    InitTools();

    CursorLocked->Enabled = RappViewRapport->Checked;
    CursorLocked->Checked = false;

    showweaveposition = false;

    einzugunten = false;
    righttoleft = false;
    toptobottom = false;
    fewithraster = false;
    palette2 = false;
    Data->color     = DEFAULT_COLOR;
    Data->defcolorh = DEFAULT_COLORH;
    Data->defcolorv = DEFAULT_COLORV;

    hDragging = vDragging = false;

    hlinedrag = false;
    hlinepredrag = false;
    hline = NULL;

    bSelectionCleared = false;

    mousedown = false;
    InitFelder();

    InitPrintRange();

    rapport.kr = SZ(0,-1);
    rapport.sr = SZ(0,-1);

    kette = SZ (-1, -1);
    schuesse = SZ (-1, -1);

    currentrange = 1;
    Range1->Checked = true;
    PopupRange1->Checked = true;

    InitZoom();
    faktor_schuss = faktor_kette = 1.0;
    CalcGrid();

    strongclr = clBlack;

    sinkingshed = false;

    ViewBlatteinzug->Checked = true;
    ViewBlatteinzugPopup->Checked = true;
    ViewFarbe->Checked = true;
    ViewFarbePopup->Checked = true;
    ViewEinzug->Checked = true;
    ViewEinzugPopup->Checked = true;
    ViewTrittfolge->Checked = true;
    ViewTrittfolgePopup->Checked = true;

    einzugunten = false;

    SelectGewebeNormal();

    ViewSchlagpatrone->Checked = false;

    scroll_x1 = scroll_x2 = scroll_y1 = scroll_y2 = 0;

    sb_horz1->Max = Data->MAXX1;
    sb_horz2->Max = Data->MAXX2;
    sb_vert1->Max = Data->MAXY1;
    sb_vert2->Max = Data->MAXY2;

    sb_horz1->Position = 0;
    sb_horz2->Position = 0;
    sb_vert1->Position = Data->MAXY1;
    sb_vert2->Position = Data->MAXY2;

    RecalcDimensions();

    kbd_field = GEWEBE;

    Settings settings;

    settings.SetCategory ("Environment");
    LANGUAGES language;
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
    language = lang==0 ? EN : GE;
    if (active_language==language) active_language = language==EN ? GE : EN; // Update erzwingen
    SwitchLanguage (language);

    if (settings.Load("ShowSplash", 1)==1) {
        try {
            Splashtimer->Enabled = false;
            Splashscreen = new TSplashscreen(this);
            Splashscreen->Show();
            Splashtimer->Interval = 2000;
            Splashtimer->Enabled = true;
        } catch (...) {
        }
    }

    // Einstellungen laden
    LoadOptions();
    LoadUserdefMenu();

    bInitialized = true;
    Invalidate();

	Statusbar->OnDrawPanel = OnDrawStatusBar;
    UpdateStatusBar();
    Application->OnHint = DisplayHint;

    file = new FfFile;
    LoadNormalTemplate();
    modified = false;
	saved = false;
    filename = "";
    Data->SaveDialog->FileName = DATEI_UNBENANNT;
    SetAppTitle();

    LoadMRU();
    UpdateMRU();

    controller = 0;

	InitBorders();

    Application->OnIdle = IdleHandler;

    handlecommandline = true;

    dbw3_assert(undo);
    undo->Snapshot();

    dbw3_assert (blockundo);
    blockundo->Snapshot();

    dbw3_assert (bereichundo);
    bereichundo->Snapshot();
}

__fastcall TDBWFRM::~TDBWFRM()
{
    CloseDataFile();
    delete file;
    delete undo;
    CrCursorHandler::Release (cursorhandler);
    RpRapport::ReleaseInstance (rapporthandler);
    EinzugRearrange::ReleaseInstance (einzughandler);
    delete blockundo;
    delete bereichundo;
    delete[] freieschaefte;
    delete[] freietritte;
    delete[] fixeinzug;
    delete[] xbuf;
    delete[] ybuf;
}

void __fastcall TDBWFRM::CloseDataFile()
{
    if (file) file->Close();
}

void __fastcall TDBWFRM::AllocBuffersX1()
{
    if (xbuf) { delete[] xbuf; xbuf = NULL; }
    if (fixeinzug) { delete[] fixeinzug; fixeinzug = NULL; }
    try {
        xbuf = new char[Data->MAXX1];
#ifdef _DEBUG
        xbufsize = Data->MAXX1;
#endif
        fixeinzug = new short[Data->MAXX1];
#ifdef _DEBUG
        fixeinzugsize = Data->MAXX1;
#endif
        memset (xbuf, 0, Data->MAXX1);
        memset (fixeinzug, 0, Data->MAXX1*sizeof(short));
    } catch (...) {
    }
    firstfree = 0;
    fixsize = 0;
}

void __fastcall TDBWFRM::AllocBuffersX2()
{
    if (freietritte) { delete[] freietritte; freietritte = NULL; }
    try {
        freietritte = new bool[Data->MAXX2];
#ifdef _DEBUG
        freietrittesize = Data->MAXX2;
#endif
    } catch (...) {
    }
}

void __fastcall TDBWFRM::AllocBuffersY1()
{
    if (freieschaefte) { delete[] freieschaefte; freieschaefte = NULL; }
    try {
        freieschaefte = new bool[Data->MAXY1];
#ifdef _DEBUG
        freieschaeftesize = Data->MAXY1;
#endif
    } catch (...) {
    }
}

void __fastcall TDBWFRM::AllocBuffersY2()
{
    if (ybuf) { delete[] ybuf; ybuf = NULL; }
    try {
        ybuf = new char[Data->MAXY2];
#ifdef _DEBUG
        ybufsize = Data->MAXY2;
#endif
        memset (ybuf, 0, Data->MAXY2);
    } catch (...) {
    }
}

void __fastcall TDBWFRM::AllocBuffers (bool _norecalc)
{
    if (xbuf) { delete[] xbuf; xbuf = NULL; }
    if (ybuf) { delete[] ybuf; ybuf = NULL; }
    if (fixeinzug) { delete[] fixeinzug; fixeinzug = NULL; }
    if (freieschaefte) { delete[] freieschaefte; freieschaefte = NULL; }
//    if (freietritte) { /*delete[] freietritte;*/ freietritte = NULL; }
    if (freietritte) { delete[] freietritte; freietritte = NULL; }
    try {
        xbuf = new char[Data->MAXX1];
#ifdef _DEBUG
        xbufsize = Data->MAXX1;
#endif

        ybuf = new char[Data->MAXY2];
#ifdef _DEBUG
        ybufsize = Data->MAXY2;
#endif

        fixeinzug = new short[Data->MAXX1];
#ifdef _DEBUG
        fixeinzugsize = Data->MAXX1;
#endif

        freieschaefte = new bool[Data->MAXY1];
#ifdef _DEBUG
        freieschaeftesize = Data->MAXY1;
#endif

        freietritte = new bool[Data->MAXX2];
#ifdef _DEBUG
        freietrittesize = Data->MAXX2;
#endif

        memset (xbuf, 0, Data->MAXX1);
        memset (ybuf, 0, Data->MAXY2);
        memset (fixeinzug, 0, Data->MAXX1*sizeof(short));
    } catch (...) {
    }
    firstfree = 0;
    fixsize = 0;
    if (!_norecalc) {
        RecalcFreieSchaefte();
        RecalcFreieTritte();
    }
}

void __fastcall TDBWFRM::InitDimensions()
{
    Data->MAXX1 = DEFAULT_MAXX1;
    Data->MAXY1 = DEFAULT_MAXY1;
    Data->MAXX2 = DEFAULT_MAXX2;
    Data->MAXY2 = DEFAULT_MAXY2;
    hvisible = 12; //xxxx
    wvisible = 12; //xxxx
    faktor_kette = faktor_schuss = 1.0;
}

// ClearFelder: Diese Funktion 'leert' alle Felder und initalisiert
// die komplexeren Felder wie Blatteinzug neu.
void __fastcall TDBWFRM::ClearFelder()
{
    //blatteinzug wird in InitFelder initialisiert!
    kettfarben.Clear();
    einzug.Clear();
    gewebe.Clear();
    aufknuepfung.Clear();
    trittfolge.Clear();
    schussfarben.Clear();

    InitFelder();

    rapport.kr = SZ(0,-1);
    rapport.sr = SZ(0,-1);
}

// InitFelder: Hier werden Felder, die komplexe Initialisierungen
// benoetigen, initialisiert.
void __fastcall TDBWFRM::InitFelder()
{
	blatteinzug.Clear();
}

void __fastcall TDBWFRM::CalcGrid()
{
    if (faktor_schuss>faktor_kette) {
        gridheight = (int)((double)zoom[currentzoom]*faktor_schuss/faktor_kette);
        gridwidth = zoom[currentzoom];
        einzug.gw = einzug.gh = gridwidth;
        aufknuepfung.gw = aufknuepfung.gh = gridwidth;
        kettfarben.gw = kettfarben.gh = gridwidth;
        blatteinzug.gw = blatteinzug.gh = gridwidth;
        trittfolge.gw = gridwidth;
        trittfolge.gh = gridheight;
        schussfarben.gw = gridwidth;
        schussfarben.gh = gridheight;
        gewebe.gw = gridwidth;
        gewebe.gh = gridheight;
    } else {
        gridheight = zoom[currentzoom];
        gridwidth = (int)((double)zoom[currentzoom]*faktor_kette/faktor_schuss);
        einzug.gw = gridwidth;
        einzug.gh = gridheight;
        aufknuepfung.gw = aufknuepfung.gh = gridheight;
        kettfarben.gw = gridwidth;
        kettfarben.gh = gridheight;
        blatteinzug.gw = gridwidth;
        blatteinzug.gh = gridheight;
        trittfolge.gw = trittfolge.gh = gridheight;
        schussfarben.gw = schussfarben.gh = gridheight;
        gewebe.gw = gridwidth;
        gewebe.gh = gridheight;
    }
    // Die Hilfslinienbars sind immer gleich breit und hoch
    hlinehorz1.gh = hlinehorz2.gh = hlinevert1.gw = hlinevert2.gw = zoom[3];
    hlinehorz1.gw = einzug.gw;
    hlinehorz2.gw = trittfolge.gw;
    hlinevert1.gh = einzug.gh;
    hlinevert2.gh = gewebe.gh;
    if (currentzoom>=3) divider = zoom[3]*3/4;
    else divider = zoom[currentzoom]*3/4;
}

void __fastcall TDBWFRM::ToggleBlatteinzug()
{
    ViewBlatteinzug->Checked = !ViewBlatteinzug->Checked;
    ViewBlatteinzugPopup->Checked = ViewBlatteinzug->Checked;
    if (cursorhandler && !ViewBlatteinzug->Checked)
        cursorhandler->SetInvisible (BLATTEINZUG);
    SetModified();
}

void __fastcall TDBWFRM::ToggleFarbe()
{
    ViewFarbe->Checked = !ViewFarbe->Checked;
    ViewFarbePopup->Checked = ViewFarbe->Checked;
    if (cursorhandler && !ViewFarbe->Checked) {
        cursorhandler->SetInvisible (KETTFARBEN);
        cursorhandler->SetInvisible (SCHUSSFARBEN);
        cursorhandler->SetInvisible (KETTFARBEN); // muss nochmals sein!
    }
    SetModified();
}

void __fastcall TDBWFRM::ToggleEinzug()
{
    ViewEinzug->Checked = !ViewEinzug->Checked;
    ViewEinzugPopup->Checked = ViewEinzug->Checked;
    if (cursorhandler && !ViewEinzug->Checked) {
        cursorhandler->SetInvisible (EINZUG);
        cursorhandler->SetInvisible (AUFKNUEPFUNG);
    }
    SetModified();
}

void __fastcall TDBWFRM::ToggleTrittfolge()
{
    ViewTrittfolge->Checked = !ViewTrittfolge->Checked;
    ViewTrittfolgePopup->Checked = ViewTrittfolge->Checked;
    if (cursorhandler && !ViewTrittfolge->Checked) {
        cursorhandler->SetInvisible (TRITTFOLGE);
        cursorhandler->SetInvisible (AUFKNUEPFUNG);
    }
    SetModified();
}

void __fastcall TDBWFRM::SelectGewebeNormal()
{
    GewebeNormal->Checked = true;
    GewebeNormalPopup->Checked = true;
    SetModified();
}

void __fastcall TDBWFRM::SelectGewebeFarbeffekt()
{
    GewebeFarbeffekt->Checked = true;
    GewebeFarbeffektPopup->Checked = true;
    SetModified();
}

void __fastcall TDBWFRM::SelectGewebeSimulation()
{
    GewebeSimulation->Checked = true;
    GewebeSimulationPopup->Checked = true;
    SetModified();
}

void __fastcall TDBWFRM::SelectGewebeNone()
{
    GewebeNone->Checked = true;
    GewebeNonePopup->Checked = true;
    SetModified();
}

void __fastcall TDBWFRM::ToggleSchlagpatrone()
{
    ViewSchlagpatrone->Checked = !ViewSchlagpatrone->Checked;
    trittfolge.einzeltritt = !ViewSchlagpatrone->Checked;
    if (!ViewSchlagpatrone->Checked) {
        savedtrittfolgenstyle->Checked = true;
        RecalcTrittfolgeAufknuepfung();
    } else
        RecalcSchlagpatrone();
    SetModified();
    if (kbd_field==AUFKNUEPFUNG) {
        cursorhandler->GotoNextField();
        UpdateStatusBar();
    }
    dbw3_assert(undo);
    undo->Snapshot();

    // Popup nachführen...
    if (ViewSchlagpatrone->Checked) {
      LANG_C_H(ViewTrittfolgePopup, EN, "Pegplan", "Toggles pegplan")
      LANG_C_H(ViewTrittfolgePopup, GE, "Schlagpatrone", "Schlagpatrone sichtbar/unsichtbar")
    } else {
      LANG_C_H(ViewTrittfolgePopup, EN, "Treadling", "Toggles treadling")
      LANG_C_H(ViewTrittfolgePopup, GE, "Trittfolge", "Trittfolge sichtbar/unsichtbar")
    }
    // ... und Menü
    if (ViewSchlagpatrone->Checked) {
      LANG_C_H(ViewTrittfolge, EN, "Pegpl&an", "Toggles pegplan")
      LANG_C_H(ViewTrittfolge, GE, "Schlagpa&trone", "Schlagpatrone sichtbar/unsichtbar")
    } else {
      LANG_C_H(ViewTrittfolge, EN, "Tre&adling", "Toggles treadling")
      LANG_C_H(ViewTrittfolge, GE, "&Trittfolge", "Trittfolge sichtbar/unsichtbar")
    }
    CopyEinzugTrittfolge->Visible = !ViewSchlagpatrone->Checked;
}

void __fastcall TDBWFRM::ViewBlatteinzugClick(TObject *Sender)
{
    ToggleBlatteinzug();
    RecalcDimensions();
    Invalidate();
}

void __fastcall TDBWFRM::ViewFarbeClick(TObject *Sender)
{
    ToggleFarbe();
    RecalcDimensions();
    Invalidate();
}

void __fastcall TDBWFRM::ViewEinzugClick(TObject *Sender)
{
    ToggleEinzug();
    if (hvisible==0) hvisible = 12;
    if (Data->MAXY1<hvisible) hvisible = Data->MAXY1;
    RecalcDimensions();
    Invalidate();
}

void __fastcall TDBWFRM::ViewTrittfolgeClick(TObject *Sender)
{
    ToggleTrittfolge();
    if (wvisible==0) wvisible = 12;
    if (Data->MAXX2<wvisible) wvisible = Data->MAXX2;
    RecalcDimensions();
    Invalidate();
}

void __fastcall TDBWFRM::GewebeFarbeffektClick(TObject *Sender)
{
    SelectGewebeFarbeffekt();
    InvalidateFeld (gewebe.pos);
    if (!ViewSchlagpatrone->Checked) InvalidateFeld (aufknuepfung.pos);
    else InvalidateFeld (trittfolge.pos);
}

void __fastcall TDBWFRM::GewebeNormalClick(TObject *Sender)
{
    SelectGewebeNormal();
    InvalidateFeld (gewebe.pos);
    if (!ViewSchlagpatrone->Checked) InvalidateFeld (aufknuepfung.pos);
    else InvalidateFeld (trittfolge.pos);
}

void __fastcall TDBWFRM::GewebeSimulationClick(TObject *Sender)
{
    SelectGewebeSimulation();
    InvalidateFeld (gewebe.pos);
    if (!ViewSchlagpatrone->Checked) InvalidateFeld (aufknuepfung.pos);
    else InvalidateFeld (trittfolge.pos);
}

void __fastcall TDBWFRM::GewebeNoneClick(TObject *Sender)
{
    SelectGewebeNone();
    InvalidateFeld (gewebe.pos);
    if (!ViewSchlagpatrone->Checked) InvalidateFeld (aufknuepfung.pos);
    else InvalidateFeld (trittfolge.pos);
}

void __fastcall TDBWFRM::ViewSchlagpatroneClick(TObject *Sender)
{
    ToggleSchlagpatrone();
    InvalidateFeld (aufknuepfung.pos);
    InvalidateFeld (trittfolge.pos);
    Settings settings;
    settings.SetCategory ("Divers");
    settings.Save ("PegplanMode", ViewSchlagpatrone->Checked);
}

void __fastcall TDBWFRM::RecalcDimensions()
{
    // Zuerst Statusbar ausrichten
    Statusbar->Left = 0;
    Statusbar->Width = ClientWidth;
    Statusbar->Top = ClientHeight - Statusbar->Height;

    int clientwidth = ClientWidth - sb_vert1->Width - 4;
    int clientheight = ClientHeight - sb_horz1->Height - Statusbar->Height - Toolbar->Height - 4;

    int hlh_div = ViewHlines->Checked ? 3 : 0;
    int fh_div = ViewFarbe->Checked ? divider : 0;
    int e_div = ViewEinzug->Checked && !einzugunten ? divider : 0;
    int b_div = ViewBlatteinzug->Checked ? divider : 0;
    int g_div = ViewEinzug->Checked && einzugunten ? divider : 0;

    int hlv_div = ViewHlines->Checked ? 3 : 0;
    int fv_div = ViewFarbe->Checked ? divider : 0;
    int t_div = ViewTrittfolge->Checked ? divider : 0;

    hlinehorz1.y0 = hlinehorz2.y0 = Toolbar->Height;
    hlinehorz1.height = ViewHlines->Checked ? hlinehorz1.gh : 0;
    hlinehorz2.height = ViewHlines->Checked && ViewTrittfolge->Checked ? hlinehorz2.gh : 0;

    kettfarben.pos.height = ViewFarbe->Checked ? kettfarben.gh : 0;
    einzug.pos.height = ViewEinzug->Checked ? hvisible*einzug.gh : 0;
    blatteinzug.pos.height = ViewBlatteinzug->Checked ? blatteinzug.gh+1 : 0;
    gewebe.pos.height = clientheight-blatteinzug.pos.height-kettfarben.pos.height-
                einzug.pos.height-hlinehorz1.height-b_div-fh_div-e_div-g_div-hlh_div;
    int rest = gewebe.pos.height % gewebe.gh;
    gewebe.pos.height -= rest;
    if (gewebe.pos.height<0) gewebe.pos.height = 0;
    if (gewebe.pos.height>Data->MAXY2*gewebe.gh) gewebe.pos.height = Data->MAXY2*gewebe.gh;

    gewebe.pos.y0 = einzugunten ?
                hlinehorz1.y0+hlinehorz1.height+hlh_div
              : Toolbar->Height + clientheight - gewebe.pos.height - rest;

    kettfarben.pos.y0 = einzugunten ?
                Toolbar->Height + clientheight - kettfarben.pos.height - rest
              : hlinehorz1.y0 + hlinehorz1.height + hlh_div;
    einzug.pos.y0 = einzugunten ?
                kettfarben.pos.y0 - einzug.pos.height - fh_div
              : kettfarben.pos.y0+kettfarben.pos.height+fh_div;
    blatteinzug.pos.y0 = einzugunten ?
                einzug.pos.y0 - blatteinzug.pos.height
              : einzug.pos.y0 + e_div + einzug.pos.height;
    if (einzugunten && ViewEinzug->Checked)
        blatteinzug.pos.y0 -= b_div;


    hlinevert1.width = ViewHlines->Checked && ViewEinzug->Checked ? hlinevert1.gw : 0;
    hlinevert2.width = ViewHlines->Checked ? hlinevert2.gw : 0;
    hlinevert1.height = einzug.pos.height;
    hlinevert2.height = gewebe.pos.height;
    hlinevert1.y0 = einzug.pos.y0;
    hlinevert2.y0 = gewebe.pos.y0;

    schussfarben.pos.width = ViewFarbe->Checked ? schussfarben.gw : 0;
    schussfarben.pos.y0 = gewebe.pos.y0;
    schussfarben.pos.height = gewebe.pos.height;

    trittfolge.pos.width = ViewTrittfolge->Checked ? wvisible*trittfolge.gw : 0;
    aufknuepfung.pos.width = trittfolge.pos.width;
    gewebe.pos.width = clientwidth-trittfolge.pos.width-schussfarben.pos.width-hlinevert2.width-hlv_div-t_div-fv_div;
    rest = gewebe.pos.width % gewebe.gw;
    gewebe.pos.width -= rest;
    if (gewebe.pos.width<0) gewebe.pos.width = 0;
    if (gewebe.pos.width>Data->MAXX1*gewebe.gw) gewebe.pos.width = Data->MAXX1*gewebe.gw;

    hlinevert1.x0 = hlinevert2.x0 = clientwidth - hlinevert2.width - rest;
    schussfarben.pos.x0 = hlinevert1.x0 - hlv_div -schussfarben.pos.width;
    trittfolge.pos.x0 = schussfarben.pos.x0-fv_div-trittfolge.pos.width;
    trittfolge.pos.y0 = gewebe.pos.y0;
    trittfolge.pos.height = gewebe.pos.height;

    aufknuepfung.pos.y0 = einzug.pos.y0;
    aufknuepfung.pos.height = einzug.pos.height;
    aufknuepfung.pos.x0 = trittfolge.pos.x0;

    blatteinzug.pos.width = kettfarben.pos.width = einzug.pos.width = gewebe.pos.width;
    hlinehorz1.width = gewebe.pos.width;
    hlinehorz2.width = trittfolge.pos.width;
    hlinehorz1.x0 = gewebe.pos.x0;
    hlinehorz2.x0 = trittfolge.pos.x0;

    blatteinzug.pos.x0 = kettfarben.pos.x0 = einzug.pos.x0 = hlinehorz1.x0 =
        gewebe.pos.x0 = trittfolge.pos.x0-gewebe.pos.width-t_div;

    // Scrollbars repositionieren
    sb_horz1->Top    = ClientHeight-Statusbar->Height-sb_horz1->Height;
    sb_horz2->Top    = sb_horz1->Top;
    sb_horz1->Left   = gewebe.pos.x0+1;
    sb_horz1->Width  = gewebe.pos.width-1;
    sb_horz2->Left   = trittfolge.pos.x0+1;
    sb_horz2->Width  = trittfolge.pos.width-1;

    sb_vert1->Left   = ClientWidth-sb_vert1->Width;
    sb_vert2->Left   = sb_vert1->Left;
    sb_vert1->Top    = aufknuepfung.pos.y0+1;
    sb_vert1->Height = aufknuepfung.pos.height-1;
    sb_vert2->Top    = trittfolge.pos.y0+1;
    sb_vert2->Height = trittfolge.pos.height-1;

    UpdateScrollbars();
}

void __fastcall TDBWFRM::SetScrollbar (TScrollBar* _sb, int _max, int _i, int _pos, bool _invers/*=false*/)
{
    _sb->LargeChange = (short)(_i>_max ? _max : _i);
    _sb->PageSize = (short)(_i>_max ? _max : _i);
    _sb->Max  = _i>_max ? 0 : _max - _i;
    if (!_invers) _sb->Position = _sb->Kind==sbHorizontal ? _pos : _sb->Max - _pos;
    else _sb->Position = _sb->Kind==sbHorizontal ? _sb->Max - _pos : _pos;

    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask  = SIF_PAGE|SIF_RANGE|SIF_POS|SIF_DISABLENOSCROLL;
    si.nPage  = _i>_max ? _max : _i+1;
    if (!_invers) si.nPos = _sb->Kind==sbHorizontal ? _pos : (_i>_max ? 0 : _max - _i - _pos);
    else si.nPos = _sb->Kind==sbHorizontal ? (_i>_max ? 0 : _max - _i - _pos) : _pos;
    si.nMin = 0;
    si.nMax = _max;
    SetScrollInfo (_sb->Handle, SB_CTL, &si, true);
}

void __fastcall TDBWFRM::FormResize(TObject *Sender)
{
    if (!Application->Terminated) {
        RecalcDimensions();
        cursorhandler->CheckCursorPos();
        UpdateStatusBar();
        Invalidate();
    }
}

void __fastcall TDBWFRM::RecalcTrittfolgeEmpty (int _j)
{
    for (int i=0; i<Data->MAXX2; i++)
        if (trittfolge.feld.Get(i,_j)>0) {
            trittfolge.isempty.Set(_j, false);
            return;
        }
    trittfolge.isempty.Set(_j, true);
}

bool __fastcall TDBWFRM::IsEmptyEinzug (int _i)
{
    return einzug.feld.Get(_i)==0;
}

bool __fastcall TDBWFRM::IsEmptyTrittfolge (int _j)
{
    return trittfolge.isempty.Get(_j);
}

void __fastcall TDBWFRM::SetAppTitle()
{
    String file;
    if (filename=="") file = DATEI_UNBENANNT;
    else file = ExtractFileName (filename);
    if (modified) file += "*";

    Caption = file + " - " + APP_TITLE;
}

void __fastcall TDBWFRM::FileExitClick(TObject *Sender)
{
    if (!AskSave()) return;
    Application->Terminate();
}

void __fastcall TDBWFRM::SetModified()
{
    modified = true;
    SetAppTitle();
}

void __fastcall TDBWFRM::HelpAboutClick(TObject *Sender)
{
    cursorhandler->DisableCursor();
    TAboutBox* box = new TAboutBox (this);
    Statusbar->Panels->Items[0]->Text = LANG_STR("This is the status bar", "Dies ist die Statuszeile");
    box->ShowModal();
    delete box;
    UpdateStatusBar();
    cursorhandler->EnableCursor();
}

void __fastcall TDBWFRM::FormCloseQuery(TObject *Sender, bool &CanClose)
{
    CanClose = AskSave();
}

void __fastcall TDBWFRM::HelpBsoftOnlineClick(TObject *Sender)
{
    ShellExecute (NULL, "open", HOMEPAGE,
                  NULL, NULL, SW_SHOWNORMAL);
}

void __fastcall TDBWFRM::ViewRedrawClick(TObject *Sender)
{
    Canvas->Brush->Color = clBtnFace;
    Canvas->Pen->Color = clBtnFace;
    Canvas->Rectangle (0, 0, ClientWidth, ClientHeight);
    Invalidate();
}

void __fastcall TDBWFRM::WMEraseBkgnd (TWMEraseBkgnd& _msg)
{
    // Ich zeichne den Hintergrund nicht neu,
    // sage aber, dass Windows ihn nicht löschen soll!
    // Dafür muss meine OnPaint-Funktion ALLES (innerhalb
    // des ClipRects) neuzeichnen.
    _msg.Result = 1;
}

void __fastcall TDBWFRM::OptionsLockGewebeClick(TObject *Sender)
{
    OptionsLockGewebe->Checked = !OptionsLockGewebe->Checked;
    SetModified();
}

void __fastcall TDBWFRM::ViewFarbpaletteClick(TObject *Sender)
{
    FarbPalette->Visible = !ViewFarbpalette->Checked;
    ViewFarbpalette->Checked = FarbPalette->Visible;
}

void __fastcall TDBWFRM::RappViewRapportClick(TObject *Sender)
{
    RappViewRapport->Checked = !RappViewRapport->Checked;
    CalcRapport();
    Invalidate();
    if (!RappViewRapport->Checked && CursorLocked->Checked)
        CursorLockedClick (this);
    CursorLocked->Enabled = RappViewRapport->Checked;
    SetModified();
}

void __fastcall TDBWFRM::FilePropsClick(TObject *Sender)
{
	if (Data->properties==0) return;
    cursorhandler->DisableCursor();
    TFilePropertiesForm* frm = new TFilePropertiesForm (this);
    frm->author->Text = Data->properties->Author();
    frm->organization->Text = Data->properties->Organization();
    frm->remarks->Text = Data->properties->Remarks();
    if (frm->ShowModal()==mrOk) {
        Data->properties->SetAuthor (frm->author->Text.c_str());
        Data->properties->SetOrganization (frm->organization->Text.c_str());
        Data->properties->SetRemarks (frm->remarks->Text.c_str());
        SetModified();
    }
    delete frm;
    cursorhandler->EnableCursor();
}

void __fastcall TDBWFRM::FormActivate(TObject *Sender)
{
    CursorTimer->Enabled = true;
}

void __fastcall TDBWFRM::FormDeactivate(TObject *Sender)
{
    CursorTimer->Enabled = false;
    DeleteCursor();
}

void __fastcall TDBWFRM::ViewOnlyGewebeClick(TObject *Sender)
{
    ViewOnlyGewebe->Checked = !ViewOnlyGewebe->Checked;
    Invalidate();
}

void __fastcall TDBWFRM::InitPrintRange()
{
    printkette.a = 0;
    printkette.b = 39;
    printschuesse.a = 0;
    printschuesse.b = 39;
    printschaefte.a = 0;
    printschaefte.b = 7;
    printtritte.a = 0;
    printtritte.b = 7;
}

void __fastcall TDBWFRM::EditCursordirectionClick(TObject *Sender)
{
    dbw3_assert (cursorhandler!=NULL);
    cursorhandler->DisableCursor();
    TCursorDirForm* pForm = new TCursorDirForm (this, cursorhandler->GetCursorDirection(), 0);
    if (pForm->ShowModal()==mrOk) {
        cursorhandler->SetCursorDirection (pForm->cursordirection);
    }
    delete pForm;
    cursorhandler->EnableCursor();
}

void __fastcall TDBWFRM::CursorLockedClick(TObject *Sender)
{
    CursorLocked->Checked = !CursorLocked->Checked;
    cursorhandler->SetCursorLocked (CursorLocked->Checked);
}

void __fastcall TDBWFRM::ToggleGewebe (int _i, int _j)
{
	char s = gewebe.feld.Get (_i, _j);
 	gewebe.feld.Set (_i, _j, char(s<=0 ? currentrange : -s));
}

void __fastcall TDBWFRM::ToggleAufknuepfung (int _i, int _j)
{
	char s = aufknuepfung.feld.Get (_i, _j);
   	aufknuepfung.feld.Set (_i, _j, char(s<=0 ? currentrange : -s));
}

void __fastcall TDBWFRM::OverviewClick(TObject *Sender)
{
    try {
        TOverviewForm* pFrm = new TOverviewForm (this, this);
        pFrm->ShowModal();
        delete pFrm;
    } catch (...) {
    }
}

void __fastcall TDBWFRM::KettfarbenWieSchussfarbenClick(TObject *Sender)
{
    for (int j=0; j<min(Data->MAXY2,Data->MAXX1); j++)
        kettfarben.feld.Set (j, schussfarben.feld.Get(j));
    if (Data->MAXY2>Data->MAXX1)
        for (int j=Data->MAXX1; j<Data->MAXY2; j++)
            kettfarben.feld.Set (j, DEFAULT_COLORV);

    InvalidateFeld (kettfarben.pos);
    if (GewebeFarbeffekt->Checked || GewebeSimulation->Checked)
        InvalidateFeld (gewebe.pos);

    SetModified();
    undo->Snapshot();
}

void __fastcall TDBWFRM::SchussfarbenWieKettfarbenClick(TObject *Sender)
{
    for (int i=0; i<min(Data->MAXX1,Data->MAXY2); i++)
        schussfarben.feld.Set (i, kettfarben.feld.Get(i));
    if (Data->MAXX1>Data->MAXY2)
        for (int i=Data->MAXY2; i<Data->MAXY1; i++)
            schussfarben.feld.Set (i, DEFAULT_COLORH);

    InvalidateFeld (schussfarben.pos);
    if (GewebeFarbeffekt->Checked || GewebeSimulation->Checked)
        InvalidateFeld (gewebe.pos);

    SetModified();
    undo->Snapshot();
}

void __fastcall TDBWFRM::_ExtendTritte (int _max)
{
    if (Data->MAXX2<_max) {
        Data->MAXX2 = _max;
        aufknuepfung.feld.Resize (Data->MAXX2, Data->MAXY1, 0);
        trittfolge.feld.Resize (Data->MAXX2, Data->MAXY2, 0);
        bool* pNew = NULL;
        try { pNew = new bool[Data->MAXX2]; } catch (...) { dbw3_assert(false); }
        for (int i=0; i<Data->MAXX2-10; i++) pNew[i] = freietritte[i];
        for (int i=Data->MAXX2-10; i<Data->MAXX2; i++) pNew[i] = true;
        delete[] freietritte;
        freietritte = pNew;
    }
}

void __fastcall TDBWFRM::_ExtendSchaefte (int _max)
{
    if (Data->MAXY1<_max) {
        Data->MAXY1 = _max;
        aufknuepfung.feld.Resize (Data->MAXX2, Data->MAXY1, 0);
        bool* pNew = NULL;
        try { pNew = new bool[Data->MAXY1]; } catch (...) {}
        for (int i=0; i<Data->MAXY1-10; i++) pNew[i] = freieschaefte[i];
        for (int i=Data->MAXY1-10; i<Data->MAXY1; i++) pNew[i] = true;
        delete[] freieschaefte;
        freieschaefte = pNew;
    }
}

void __fastcall TDBWFRM::ExtendTritte()
{
    int newmax = max(Data->MAXX2+10, int(Data->MAXY1));
    _ExtendTritte (newmax);
    _ExtendSchaefte (newmax);
    if (undo) undo->UpdateSize();
    UpdateScrollbars();
}

void __fastcall TDBWFRM::ExtendSchaefte()
{
    int newmax = max(int(Data->MAXX2), Data->MAXY1+10);
    _ExtendTritte (newmax);
    _ExtendSchaefte (newmax);
    if (undo) undo->UpdateSize();
    UpdateScrollbars();
}

void __fastcall TDBWFRM::UpdateScrollbars()
{
    SetScrollbar (sb_horz1, Data->MAXX1, gewebe.pos.width/gewebe.gw, scroll_x1, righttoleft);
    SetScrollbar (sb_horz2, Data->MAXX2, trittfolge.pos.width/trittfolge.gw, scroll_x2);
    SetScrollbar (sb_vert1, Data->MAXY1, einzug.pos.height/einzug.gh, scroll_y1, toptobottom);
    SetScrollbar (sb_vert2, Data->MAXY2, gewebe.pos.height/gewebe.gh, scroll_y2);
}

void __fastcall TDBWFRM::InitTools()
{
    tool = TOOL_NORMAL;
    linesize = LINE_SIZE1;
    if (ToolpaletteForm) {
        ToolpaletteForm->toolNormal->Down = true;
        ToolpaletteForm->toolLinesize1->Down = true;
    }
}

void __fastcall TDBWFRM::ViewToolpaletteClick(TObject *Sender)
{
    ToolpaletteForm->Visible = !ViewToolpalette->Checked;
    ViewToolpalette->Checked = ToolpaletteForm->Visible;
    // Zeichnen kann ich nur bei normalem Gewebe mit Schlagpatrone
    if (ViewToolpalette->Checked) {
        if (!GewebeNormal->Checked) {
            SelectGewebeNormal();
            Invalidate();
        }
        if (!ViewSchlagpatrone->Checked)
            ToggleSchlagpatrone();
    }
    // Darum auch die Gewebeansicht ausschalten
    bool tools = ViewToolpalette->Checked;
    GewebeNone->Enabled = !tools;
    GewebeFarbeffekt->Enabled = !tools;
    GewebeSimulation->Enabled = !tools;
    GewebeNonePopup->Enabled = !tools;
    GewebeFarbeffektPopup->Enabled = !tools;
    GewebeSimulationPopup->Enabled = !tools;
}

int __fastcall TDBWFRM::GetFirstSchaft()
{
    for (int j=0; j<Data->MAXY1; j++)
        if (!freieschaefte[j]) return j;
    return Data->MAXY1-1;
}

int __fastcall TDBWFRM::GetLastSchaft()
{
    for (int j=Data->MAXY1-1; j>=0; j--)
        if (!freieschaefte[j]) return j;
    return 0;
}

int __fastcall TDBWFRM::GetFirstTritt()
{
    for (int i=0; i<Data->MAXX2; i++)
        if (!freietritte[i]) return i;
    return Data->MAXX2-1;
}

int __fastcall TDBWFRM::GetLastTritt()
{
    for (int i=Data->MAXX2-1; i>=0; i--)
        if (!freietritte[i]) return i;
    return 0;
}

void __fastcall TDBWFRM::OptSwissClick(TObject *Sender)
{
    cursorhandler->DisableCursor();

    // Darstellung
    einzug.darstellung = STRICH;
    aufknuepfung.darstellung = KREUZ;
    trittfolge.darstellung = PUNKT;
    schlagpatronendarstellung = AUSGEFUELLT;

    // Linksnachrechts
    righttoleft = false;
    toptobottom = false;
    sb_vert1->Position = sb_vert1->Max - scroll_y1;
    sb_horz1->Position = scroll_x1;
    UpdateScrollbars();

    // Hebende Schäfte
    sinkingshed = false;

    // Einzug oben
    einzugunten = false;

    // Blatteinzug sichtbar
    ViewBlatteinzug->Checked = true;
    ViewBlatteinzugPopup->Checked = true;

    Settings settings;
    settings.SetCategory ("Display");
    settings.Save ("Threading", (int)einzug.darstellung);
    settings.Save ("Tie-up", (int)aufknuepfung.darstellung);
    settings.Save ("Treadling", (int)trittfolge.darstellung);
    settings.Save ("Pegplan", (int)schlagpatronendarstellung);
    settings.SetCategory ("View");
    settings.Save ("ThreadingDown", einzugunten);
    settings.Save ("RightToLeft", righttoleft);
    settings.Save ("TopToBottom", toptobottom);
    settings.SetCategory ("Settings");
    settings.Save ("SinkingShed", sinkingshed);
    settings.SetCategory ("Divers");
    settings.Save ("ViewReedthreading", ViewBlatteinzug->Checked);

    RecalcDimensions();

    cursorhandler->EnableCursor();
    if (undo) undo->Snapshot();
    Invalidate();
    SetModified();
}

void __fastcall TDBWFRM::OptSkandinavischClick(TObject *Sender)
{
    cursorhandler->DisableCursor();

    // Darstellung
    einzug.darstellung = AUSGEFUELLT;
    aufknuepfung.darstellung = AUSGEFUELLT;
    trittfolge.darstellung = AUSGEFUELLT;
    schlagpatronendarstellung = AUSGEFUELLT;

    // Linksnachrechts
    righttoleft = true;
    toptobottom = true;
    sb_vert1->Position = scroll_y1;
    sb_horz1->Position = sb_horz1->Max - scroll_x1;
    UpdateScrollbars();

    // Einzug oben
    einzugunten = true;

    // Hebende Schäfte
    sinkingshed = true;

    // Blatteinzug sichtbar
    ViewBlatteinzug->Checked = false;
    ViewBlatteinzugPopup->Checked = false;

    Settings settings;
    settings.SetCategory ("Display");
    settings.Save ("Threading", (int)einzug.darstellung);
    settings.Save ("Tie-up", (int)aufknuepfung.darstellung);
    settings.Save ("Treadling", (int)trittfolge.darstellung);
    settings.Save ("Pegplan", (int)schlagpatronendarstellung);
    settings.SetCategory ("View");
    settings.Save ("ThreadingDown", einzugunten);
    settings.Save ("RightToLeft", righttoleft);
    settings.Save ("TopToBottom", toptobottom);
    settings.SetCategory ("Settings");
    settings.Save ("SinkingShed", sinkingshed);
    settings.SetCategory ("Divers");
    settings.Save ("ViewReedthreading", ViewBlatteinzug->Checked);

    RecalcDimensions();

    cursorhandler->EnableCursor();
    if (undo) undo->Snapshot();
    Invalidate();
    SetModified();
}

void __fastcall TDBWFRM::OptAmericanClick(TObject *Sender)
{
    cursorhandler->DisableCursor();

    // Darstellung
    einzug.darstellung = AUSGEFUELLT;
    aufknuepfung.darstellung = AUSGEFUELLT;
    trittfolge.darstellung = AUSGEFUELLT;
    schlagpatronendarstellung = AUSGEFUELLT;

    // Linksnachrechts
    righttoleft = true;
    toptobottom = false;
    sb_vert1->Position = sb_vert1->Max - scroll_y1;
    sb_horz1->Position = sb_horz1->Max - scroll_x1;
    UpdateScrollbars();

    // Einzug oben
    einzugunten = false;

    // Hebende Schäfte
    sinkingshed = false;

    // Blatteinzug sichtbar
    ViewBlatteinzug->Checked = true;
    ViewBlatteinzugPopup->Checked = true;

    Settings settings;
    settings.SetCategory ("Display");
    settings.Save ("Threading", (int)einzug.darstellung);
    settings.Save ("Tie-up", (int)aufknuepfung.darstellung);
    settings.Save ("Treadling", (int)trittfolge.darstellung);
    settings.Save ("Pegplan", (int)schlagpatronendarstellung);
    settings.SetCategory ("View");
    settings.Save ("ThreadingDown", einzugunten);
    settings.Save ("RightToLeft", righttoleft);
    settings.Save ("TopToBottom", toptobottom);
    settings.SetCategory ("Settings");
    settings.Save ("SinkingShed", sinkingshed);
    settings.SetCategory ("Divers");
    settings.Save ("ViewReedthreading", ViewBlatteinzug->Checked);

    RecalcDimensions();

    cursorhandler->EnableCursor();
    if (undo) undo->Snapshot();
    Invalidate();
    SetModified();
}

void __fastcall TDBWFRM::LoadOptions()
{
    Settings settings;

    settings.SetCategory ("Size");
    hvisible = settings.Load ("ShaftsVisible", 12);
    wvisible = settings.Load ("TreadlesVisible", 12);

    settings.SetCategory ("Grid");
    einzug.pos.strongline_x = aufknuepfung.pos.strongline_x =
        trittfolge.pos.strongline_x = gewebe.pos.strongline_x =
        settings.Load ("Horizontal", 4);
    einzug.pos.strongline_y = aufknuepfung.pos.strongline_y =
        trittfolge.pos.strongline_y = gewebe.pos.strongline_y =
        settings.Load ("Vertical", 4);
    CURSOR_LARGE_SKIP_X = einzug.pos.strongline_x*2;
    CURSOR_LARGE_SKIP_Y = einzug.pos.strongline_y*2;

    settings.SetCategory ("Settings");
    trittfolge.einzeltritt = settings.Load ("SingleTreadle", true);
    sinkingshed = settings.Load ("SinkingShed", false);

    settings.SetCategory ("Display");
    einzug.darstellung = (DARSTELLUNG)settings.Load ("Threading", (int)STRICH);
    aufknuepfung.darstellung = (DARSTELLUNG)settings.Load ("Tie-up", (int)KREUZ);
    trittfolge.darstellung = (DARSTELLUNG)settings.Load ("Treadling", (int)PUNKT);
    schlagpatronendarstellung = (DARSTELLUNG)settings.Load ("Pegplan", (int)AUSGEFUELLT);
    darst_aushebung = (DARSTELLUNG)settings.Load ("LiftOut", (int)STEIGEND);
    darst_anbindung = (DARSTELLUNG)settings.Load ("Binding", (int)SMALLKREUZ);
    darst_abbindung = (DARSTELLUNG)settings.Load ("Unbinding", (int)SMALLKREIS);

    settings.SetCategory ("View");
    einzugunten = settings.Load ("ThreadingDown", false);
    righttoleft = settings.Load ("RightToLeft", false);
    if (!righttoleft) {
        sb_horz1->Position = scroll_x1;
        UpdateScrollbars();
    } else {
        sb_horz1->Position = sb_horz1->Max - scroll_x1;
        UpdateScrollbars();
    }
    toptobottom = settings.Load ("TopToBottom", false);
    if (!toptobottom) {
        sb_vert1->Position = sb_vert1->Max - scroll_y1;
        UpdateScrollbars();
    } else {
        sb_vert1->Position = scroll_y1;
        UpdateScrollbars();
    }
    fewithraster = settings.Load ("ColorWithRaster", false);
    palette2 = settings.Load ("AltColorpalette", false);
    Data->palette->SetPaletteType (palette2);
    aufknuepfung.pegplanstyle = settings.Load ("AltPegplan", false);

    settings.SetCategory ("Divers");
    bool viewblatteinzug = settings.Load ("ViewReedthreading", true);
    ViewBlatteinzug->Checked = viewblatteinzug;
    ViewBlatteinzugPopup->Checked = ViewBlatteinzug->Checked;
    if (cursorhandler && !ViewBlatteinzug->Checked)
        cursorhandler->SetInvisible (BLATTEINZUG);
    bool pegplan = settings.Load ("PegplanMode", false);
    if (pegplan!=ViewSchlagpatrone->Checked)
        ToggleSchlagpatrone();

    RecalcDimensions();
    Invalidate();
}

void __fastcall TDBWFRM::UpdateIsEmpty (int _from, int _to)
{
    dbw3_assert(_from>=0 && _from<Data->MAXY2);
    dbw3_assert(_to>=_from && _to<Data->MAXY2);
    for (int j=_from; j<=_to; j++) {
        bool empty = true;
        for (int i=0; i<Data->MAXX2; i++)
            if (trittfolge.feld.Get (i, j)>0) {
                empty = false;
                break;
            }
        trittfolge.isempty.Set (j, empty);
    }
}

void __fastcall TDBWFRM::InverserepeatClick(TObject *Sender)
{
    Inverserepeat->Checked = !Inverserepeat->Checked;
    Invalidate();
}

void __fastcall TDBWFRM::SplashtimerTimer(TObject *Sender)
{
    Splashtimer->Enabled = false;
    if (Splashscreen) {
        Splashscreen->Hide();
        delete Splashscreen;
        Splashscreen = NULL;
    }
}

void __fastcall TDBWFRM::FormMouseWheel(TObject *Sender, TShiftState Shift,
                                        int WheelDelta, TPoint &MousePos, bool &Handled)
{
    WheelDelta = WheelDelta>0 ? 3 : -3; //xxx
    bool redraw;
    bool ez;
    MousePos.y -= Toolbar->Height + GetSystemMetrics(SM_CYMENU);
    if (!einzugunten) {
        if (MousePos.y>einzug.pos.y0+einzug.pos.height) {
            int oldscroll = scroll_y2;
            scroll_y2 += WheelDelta;
            if (scroll_y2>sb_vert2->Max) scroll_y2 = sb_vert2->Max;
            if (scroll_y2<0) scroll_y2 = 0;
            redraw = oldscroll!=scroll_y2;
            ez = false;
        } else {
            int oldscroll = scroll_y1;
            scroll_y1 += WheelDelta;
            if (scroll_y1>sb_vert1->Max) scroll_y1 = sb_vert1->Max;
            if (scroll_y1<0) scroll_y1 = 0;
            redraw = oldscroll!=scroll_y1;
            ez = true;
        }
    } else {
        if (MousePos.y>gewebe.pos.y0+gewebe.pos.height) {
            int oldscroll = scroll_y1;
            scroll_y1 += WheelDelta;
            if (scroll_y1>sb_vert1->Max) scroll_y1 = sb_vert1->Max;
            if (scroll_y1<0) scroll_y1 = 0;
            redraw = oldscroll!=scroll_y1;
            ez = true;
        } else {
            int oldscroll = scroll_y2;
            scroll_y2 += WheelDelta;
            if (scroll_y2>sb_vert2->Max) scroll_y2 = sb_vert2->Max;
            if (scroll_y2<0) scroll_y2 = 0;
            redraw = oldscroll!=scroll_y2;
            ez = false;
        }
    }

    if (redraw) {
        if (ez) {
            InvalidateFeld (einzug.pos);
            InvalidateFeld (aufknuepfung.pos);
            InvalidateFeld (hlinevert1);
            sb_vert1->Position = Data->MAXY1-einzug.pos.height/einzug.gh - scroll_y1;
        } else {
            InvalidateFeld (gewebe.pos);
            InvalidateFeld (trittfolge.pos);
            InvalidateFeld (schussfarben.pos);
            InvalidateFeld (hlinevert2);
            sb_vert2->Position = Data->MAXY2-gewebe.pos.height/gewebe.gh - scroll_y2;
        }
    }

    Handled = true;
}

void __fastcall TDBWFRM::PopupRangeCancelClick(TObject *Sender)
{
    //
}

void __fastcall TDBWFRM::PopupRange1Click(TObject *Sender)
{
    Range1Click(Sender);
}

void __fastcall TDBWFRM::PopupRange2Click(TObject *Sender)
{
    Range2Click(Sender);
}

void __fastcall TDBWFRM::PopupRange3Click(TObject *Sender)
{
    Range3Click(Sender);
}

void __fastcall TDBWFRM::PopupRange4Click(TObject *Sender)
{
    Range4Click(Sender);
}

void __fastcall TDBWFRM::PopupRange5Click(TObject *Sender)
{
    Range5Click(Sender);
}

void __fastcall TDBWFRM::PopupRange6Click(TObject *Sender)
{
    Range6Click(Sender);
}

void __fastcall TDBWFRM::PopupRange7Click(TObject *Sender)
{
    Range7Click(Sender);
}

void __fastcall TDBWFRM::PopupRange8Click(TObject *Sender)
{
    Range8Click(Sender);
}

void __fastcall TDBWFRM::PopupRange9Click(TObject *Sender)
{
    Range9Click(Sender);
}

void __fastcall TDBWFRM::PopupRangeBindingClick(TObject *Sender)
{
    RangeAnbindungClick(Sender);
}

void __fastcall TDBWFRM::PopupRangeUnbindingClick(TObject *Sender)
{
    RangeAbbindungClick(Sender);
}

void __fastcall TDBWFRM::PopupRangeLiftoutClick(TObject *Sender)
{
    RangeAushebungClick(Sender);
}

void __fastcall TDBWFRM::StatusbarMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
    int x1 = Statusbar->Panels->Items[0]->Width + Statusbar->Panels->Items[1]->Width;
    int x2 = x1 + Statusbar->Panels->Items[2]->Width;
    int y1 = 0;
    int y2 = y1 + Statusbar->Height;
    if (X>=x1 && X<=x2 && Y>=y1 && Y<=y2) { // Aktuelle Farbe
        Statusbar->Panels->Items[0]->Text = LANG_STR("Active color: ", "Aktive Farbe: ")+IntToStr(Data->color);
    } else {
        UpdateStatusBar();
    }
}


