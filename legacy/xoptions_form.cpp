// DB-WEAVE, textile CAD/CAM software
// Copyright (c) 1998-2005 Brunold Software, Switzerland

#include <vcl.h>
#pragma hdrstop

#include "xoptions_form.h"
#include "cursor.h"
#include "dbw3_form.h"
#include "dbw3_strings.h"
#include "datamodule.h"
#include "settings.h"
#include "undoredo.h"
#include "palette.h"
#include "language.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

extern int CURSOR_LARGE_SKIP_X;
extern int CURSOR_LARGE_SKIP_Y;

__fastcall TXOptionsForm::TXOptionsForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();

    OptOptions->ActivePage = OptAusmasse;
}

void __fastcall TXOptionsForm::LoadCombo(TComboBox* _cb, bool _withnumber/*=false*/)
{
    _cb->Items->Strings[0] = LANG_STR("Filled", "Ausgefüllt");
    _cb->Items->Strings[1] = LANG_STR("Vertical", "Strich");
    _cb->Items->Strings[2] = LANG_STR("Cross", "Kreuz");
    _cb->Items->Strings[3] = LANG_STR("Point", "Punkt");
    _cb->Items->Strings[4] = LANG_STR("Circle", "Kreis");
    _cb->Items->Strings[5] = LANG_STR("Rising", "Steigend");
    _cb->Items->Strings[6] = LANG_STR("Falling", "Fallend");
    _cb->Items->Strings[7] = LANG_STR("Small cross", "Kleines Kreuz");
    _cb->Items->Strings[8] = LANG_STR("Small circle", "Kleiner Kreis");
    if (_withnumber)
        _cb->Items->Strings[9] = LANG_STR("Number", "Zahl");
}

void __fastcall TXOptionsForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Options", "")
    LANG_C_H(this, GE, "Optionen", "")

    LANG_C_H(OptDivers, EN, "View", "")
    LANG_C_H(OptDivers, GE, "Ansicht", "")
    LANG_C_H(EinzugUnten, EN, "&Threading below pattern", "")
    LANG_C_H(EinzugUnten, GE, "&Einzug unterhalb Gewebe darstellen", "")
    LANG_C_H(RightToLeft, EN, "&Work direction in threading and pattern from right to left", "")
    LANG_C_H(RightToLeft, GE, "A&rbeitsweise in Gewebe und Einzug von rechts nach links", "")
    LANG_C_H(TopToBottom, EN, "W&ork direction in threading and tie-up from top to bottom", "")
    LANG_C_H(TopToBottom, GE, "Ar&beitsweise in Einzug und Aufknüpfung von oben nach unten", "")
    LANG_C_H(FEWithRaster, EN, "&Color effect with grid", "")
    LANG_C_H(FEWithRaster, GE, "&Farbeffekt mit Raster", "")
    LANG_C_H(AltFarbpalette, EN, "&Use alternate color palette", "")
    LANG_C_H(AltFarbpalette, GE, "&Alternative Standardfarbpalette benutzen", "")
    LANG_C_H(AltLiftplanstyle, EN, "U&se alternate pegplan view", "")
    LANG_C_H(AltLiftplanstyle, GE, "Alternative &Schlagpatronendarstellung benutzen", "")

    LANG_C_H(OptSymbols, EN, "Symbols", "")
    LANG_C_H(OptSymbols, GE, "Symbole", "")
    LANG_C_H(lEinzug, EN, "&Threading", "")
    LANG_C_H(lEinzug, GE, "&Einzug", "")
    LANG_C_H(lTrittfolge, EN, "Tre&adling", "")
    LANG_C_H(lTrittfolge, GE, "&Trittfolge", "")
    LANG_C_H(lAufknuepfung, EN, "Tie-&up", "")
    LANG_C_H(lAufknuepfung, GE, "&Aufknüpfung", "")
    LANG_C_H(lSchlagpatrone, EN, "&Pegplan", "")
    LANG_C_H(lSchlagpatrone, GE, "&Schlagpatrone", "")
    LANG_C_H(lAushebung, EN, "&Lift out", "")
    LANG_C_H(lAushebung, GE, "A&ushebung", "")
    LANG_C_H(lAnbindung, EN, "&Binding", "")
    LANG_C_H(lAnbindung, GE, "A&nbindung", "")
    LANG_C_H(lAbbindung, EN, "U&nbinding", "")
    LANG_C_H(lAbbindung, GE, "A&bbindung", "")
    LoadCombo(cbEinzug, true);
    LoadCombo(cbTrittfolge);
    LoadCombo(cbAufknuepfung, true);
    LoadCombo(cbSchlagpatrone, true);
    LoadCombo(cbAushebung);
    LoadCombo(cbAnbindung);
    LoadCombo(cbAbbindung);

    LANG_C_H(OptSettings, EN, "Settings", "")
    LANG_C_H(OptSettings, GE, "Einstellungen", "")
    LANG_C_H(gbTrittvergabe, EN, "Treadle mode", "")
    LANG_C_H(gbTrittvergabe, GE, "Trittvergabe", "")
    LANG_C_H(Einzeltritt, EN, "&Single treadle", "")
    LANG_C_H(Einzeltritt, GE, "&Einzeltritt", "")
    LANG_C_H(Multitritt, EN, "&Multi treadle", "")
    LANG_C_H(Multitritt, GE, "&Mehrfachtritt", "")
    LANG_C_H(gbRisingSinking, EN, "Harness mode", "")
    LANG_C_H(gbRisingSinking, GE, "Schaftmodus", "")
    LANG_C_H(RisingShed, EN, "&Rising shed", "")
    LANG_C_H(RisingShed, GE, "&Hebende Schäfte", "")
    LANG_C_H(SinkingShed, EN, "S&inking shed", "")
    LANG_C_H(SinkingShed, GE, "&Sinkende Schäfte", "")

    LANG_C_H(OptSchenien, EN, "Grid", "")
    LANG_C_H(OptSchenien, GE, "Schenien", "")
    LANG_C_H(gbSchenien, EN, "Gridsetting", "")
    LANG_C_H(gbSchenien, GE, "Schenienteilung", "")
    LANG_C_H(lHorizontal, EN, "&Horizontal", "")
    LANG_C_H(lHorizontal, GE, "&Horizontal", "")
    LANG_C_H(lVertikal, EN, "&Vertical", "")
    LANG_C_H(lVertikal, GE, "&Vertikal", "")

    LANG_C_H(OptAusmasse, EN, "Sizes", "")
    LANG_C_H(OptAusmasse, GE, "Ausmasse", "")
    LANG_C_H(gbFeldgroessen, EN, "Size", "")
    LANG_C_H(gbFeldgroessen, GE, "Grösse", "")
    LANG_C_H(lSchaefte, EN, "&Harnesses", "")
    LANG_C_H(lSchaefte, GE, "&Schäfte", "")
    LANG_C_H(lTritte, EN, "&Treadles", "")
    LANG_C_H(lTritte, GE, "&Tritte", "")
    LANG_C_H(lSchuesse, EN, "&Warp threads", "")
    LANG_C_H(lSchuesse, GE, "&Kettfäden", "")
    LANG_C_H(lKettfaeden, EN, "W&eft threads", "")
    LANG_C_H(lKettfaeden, GE, "S&chusse", "")

    LANG_C_H(gbVisibility, EN, "Visibility", "")
    LANG_C_H(gbVisibility, GE, "Sichtbarkeit", "")
    LANG_C_H(lSchaefteVis, EN, "&Visible harnesses", "")
    LANG_C_H(lSchaefteVis, GE, "S&ichtbare Schäfte", "")
    LANG_C_H(lTritteVis, EN, "V&isible treadles", "")
    LANG_C_H(lTritteVis, GE, "Sic&htbare Tritte", "")

    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
}

void __fastcall TDBWFRM::XOptionsGlobalClick(TObject *Sender)
{
    ShowOptions(true);
}

void __fastcall TDBWFRM::XOptionsClick(TObject *Sender)
{
    ShowOptions(false);
}

void __fastcall TDBWFRM::ShowOptions (bool _global/*=false*/)
{
    try {
        TXOptionsForm* frm = new TXOptionsForm (this);
        // Aktuelle Einstellungen übertragen
        // Ausmasse
        frm->upSchaefte->Position = short(Data->MAXY1);
        frm->upTritte->Position = short(Data->MAXX2);
        frm->upKette->Position = short(Data->MAXX1);
        frm->upSchuesse->Position = short(Data->MAXY2);
        frm->upSchaefteVis->Position = short(hvisible);
        frm->upTritteVis->Position = short(wvisible);
        // Schenien
        frm->upSchenienHorz->Position = short(einzug.pos.strongline_x);
        frm->upSchenienVert->Position = short(einzug.pos.strongline_y);
        frm->SchenienHorz->Text = IntToStr (einzug.pos.strongline_x);
        frm->SchenienVert->Text = IntToStr (einzug.pos.strongline_y);
        // Einstellungen
        if (trittfolge.einzeltritt) frm->Einzeltritt->Checked = true;
        else frm->Multitritt->Checked = true;
        if (sinkingshed) frm->SinkingShed->Checked = true;
        else frm->RisingShed->Checked = true;
        // Symbole
        frm->cbEinzug->ItemIndex = einzug.darstellung;
        frm->cbAufknuepfung->ItemIndex = aufknuepfung.darstellung;
        frm->cbTrittfolge->ItemIndex = trittfolge.darstellung;
        frm->cbSchlagpatrone->ItemIndex = schlagpatronendarstellung;
        frm->cbAushebung->ItemIndex = darst_aushebung;
        frm->cbAnbindung->ItemIndex = darst_anbindung;
        frm->cbAbbindung->ItemIndex = darst_abbindung;
        // Ansicht
        frm->EinzugUnten->Checked = einzugunten;
        frm->RightToLeft->Checked = righttoleft;
        frm->TopToBottom->Checked = toptobottom;
        frm->FEWithRaster->Checked = fewithraster;
        frm->AltLiftplanstyle->Checked = aufknuepfung.pegplanstyle;
        frm->AltFarbpalette->Checked = palette2;
        cursorhandler->DisableCursor();
        if (frm->ShowModal()==mrOk) {
            // Einstellungen auf Änderungen überprüfen und ggf. setzen/speichern
            Settings settings;
            // Ausmasse
            int x1 = atoi (frm->Kette->Text.c_str());
            int y1 = atoi (frm->Schaefte->Text.c_str());
            int x2 = atoi (frm->Tritte->Text.c_str());
            int y2 = atoi (frm->Schuesse->Text.c_str());
            int vy1 = atoi (frm->SchaefteVis->Text.c_str());
            int vx2 = atoi (frm->TritteVis->Text.c_str());
            SetAusmasse (x1, y1, x2, y2, vx2, vy1);
            if (_global) {
				settings.SetCategory ("Size");
				settings.Save ("ShaftsVisible", hvisible);
				settings.Save ("TreadlesVisible", wvisible);
			}
            // Schenien
            einzug.pos.strongline_x = aufknuepfung.pos.strongline_x =
                trittfolge.pos.strongline_x = gewebe.pos.strongline_x =
                atoi (frm->SchenienHorz->Text.c_str());
            einzug.pos.strongline_y = aufknuepfung.pos.strongline_y =
                trittfolge.pos.strongline_y = gewebe.pos.strongline_y =
                atoi (frm->SchenienVert->Text.c_str());
            CURSOR_LARGE_SKIP_X = einzug.pos.strongline_x*2;
            CURSOR_LARGE_SKIP_Y = einzug.pos.strongline_y*2;
            if (_global) {
	            settings.SetCategory ("Grid");
	            settings.Save ("Horizontal", einzug.pos.strongline_x);
	            settings.Save ("Vertical", einzug.pos.strongline_y);
			}
            // Einstellungen
            trittfolge.einzeltritt = frm->Einzeltritt->Checked;
            sinkingshed = frm->SinkingShed->Checked;
            if (_global) {
	            settings.SetCategory ("Settings");
	            settings.Save ("SingleTreadle", trittfolge.einzeltritt);
	            settings.Save ("SinkingShed", sinkingshed);
			}
            // Symbole
            einzug.darstellung = (DARSTELLUNG)frm->cbEinzug->ItemIndex;
            aufknuepfung.darstellung = (DARSTELLUNG)frm->cbAufknuepfung->ItemIndex;
            trittfolge.darstellung = (DARSTELLUNG)frm->cbTrittfolge->ItemIndex;
            schlagpatronendarstellung = (DARSTELLUNG)frm->cbSchlagpatrone->ItemIndex;
            darst_aushebung = (DARSTELLUNG)frm->cbAushebung->ItemIndex;
            darst_anbindung = (DARSTELLUNG)frm->cbAnbindung->ItemIndex;
            darst_abbindung = (DARSTELLUNG)frm->cbAbbindung->ItemIndex;
            if (_global) {
	            settings.SetCategory ("Display");
	            settings.Save ("Threading", (int)einzug.darstellung);
	            settings.Save ("Tie-up", (int)aufknuepfung.darstellung);
	            settings.Save ("Treadling", (int)trittfolge.darstellung);
	            settings.Save ("Pegplan", (int)schlagpatronendarstellung);
                settings.Save ("LiftOut", (int)darst_aushebung);
                settings.Save ("Binding", (int)darst_anbindung);
                settings.Save ("Unbinding", (int)darst_abbindung);
			}
            // Ansicht
            einzugunten = frm->EinzugUnten->Checked;
            righttoleft = frm->RightToLeft->Checked;
            if (!righttoleft) {
                sb_horz1->Position = scroll_x1;
                UpdateScrollbars();
            } else {
                sb_horz1->Position = sb_horz1->Max - scroll_x1;
                UpdateScrollbars();
            }
            toptobottom = frm->TopToBottom->Checked;
            if (toptobottom) {
                sb_vert1->Position = scroll_y1;
                UpdateScrollbars();
            } else {
                sb_vert1->Position = sb_vert1->Max - scroll_y1;
                UpdateScrollbars();
            }
            fewithraster = frm->FEWithRaster->Checked;
            bool pal2 = frm->AltFarbpalette->Checked;
			if (pal2!=palette2) {
				palette2 = pal2;
	            Data->palette->SetPaletteType (palette2);
			}
            aufknuepfung.pegplanstyle = frm->AltLiftplanstyle->Checked;
            if (_global) {
	            settings.SetCategory ("View");
	            settings.Save ("ThreadingDown", einzugunten);
	            settings.Save ("RightToLeft", righttoleft);
                settings.Save ("TopToBottom", toptobottom);
	            settings.Save ("ColorWithRaster", fewithraster);
	            settings.Save ("AltColorpalette", palette2);
	            settings.Save ("AltPegplan", aufknuepfung.pegplanstyle);
			}
            RecalcDimensions();
            Invalidate();
            SetModified();
            undo->Snapshot();
        }
        cursorhandler->EnableCursor();
        delete frm;
    } catch (...) {
    }
}

void __fastcall TDBWFRM::SetAusmasse (int x1, int y1, int x2, int y2, int vx2, int vy1)
{
    // #Schäfte==#Tritte
    if (y1>x2) x2 = y1;
    if (x2>y1) y1 = x2;

    // Felder u. U. redimensionieren!
    if (x1!=Data->MAXX1) {
        blatteinzug.feld.Resize (x1, 0);
        kettfarben.feld.Resize (x1, Data->defcolorh);
        einzug.feld.Resize (x1, 0);
        einzug.maxy = y1;
    }
    if (x1!=Data->MAXX1 || y2!=Data->MAXY2) gewebe.feld.Resize (x1, y2, 0);
    if (x2!=Data->MAXX2 || y1!=Data->MAXY1) aufknuepfung.feld.Resize (x2, y1, 0);
    if (x2!=Data->MAXX2 || y2!=Data->MAXY2) {
        trittfolge.feld.Resize (x2, y2, 0);
        trittfolge.isempty.Resize (y2, true);
    }
    if (y2!=Data->MAXY2) {
        schussfarben.feld.Resize (y2, Data->defcolorv);
    }
    bool reallocbufs = x1!=Data->MAXX1 || y2!=Data->MAXY2 || x2!=Data->MAXX2 || y1!=Data->MAXY1;

    Data->MAXX1 = x1;
    Data->MAXY1 = y1;
    Data->MAXX2 = x2;
    Data->MAXY2 = y2;

    if (reallocbufs) AllocBuffers(false);

    if (hvisible==0 && vy1!=0) { ViewEinzug->Checked = true; ViewEinzugPopup->Checked = true; }
    if (wvisible==0 && vx2!=0) { ViewTrittfolge->Checked = true; ViewTrittfolgePopup->Checked = true; }

    // Maximal sichtbare Schäfte/Tritte am Bildschirm
    hvisible = vy1;
    wvisible = vx2;

    // Wenn weniger Schäfte/Tritte vorhanden:
    // Sichtbarer Bereich runterschrauben...
    if (Data->MAXY1<hvisible) hvisible = Data->MAXY1;
    if (Data->MAXX2<wvisible) wvisible = Data->MAXX2;

    if (hvisible==0) { ViewEinzug->Checked = false; ViewEinzugPopup->Checked = false; }
    if (wvisible==0) { ViewTrittfolge->Checked = false; ViewTrittfolgePopup->Checked = false; }

    RecalcDimensions();
    CalcRange();
}



