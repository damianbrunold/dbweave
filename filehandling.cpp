/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <vcl\vcl.h>
#include <mem.h>
#pragma hdrstop

#include "assert.h"
#include "dbw3_form.h"
#include "felddef.h"
#include "dbw3_strings.h"
#include "farbpalette_form.h"
#include "datamodule.h"
#include "palette.h"
#include "properties.h"
#include "undoredo.h"
#include "cursor.h"

extern int CURSOR_LARGE_SKIP_X;
extern int CURSOR_LARGE_SKIP_Y;

void __fastcall TDBWFRM::DateiNeu()
{
    // Eventuelle Änderungen speichern
    if (!AskSave()) return;

    TCursor oldcursor = Cursor;
    Cursor = crHourGlass;

    // Datei schliessen
    CloseDataFile();

    // Grundeinstellungen
    modified = false;
	saved = false;
    mousedown = false;
    weaving = false;
	InitBorders();
    InitPrintRange();
    filename = "";
    scroll_x1 = scroll_y1 = scroll_x2 = scroll_y2 = 0;
    Data->properties->Init();
    Data->SaveDialog->FileName = DATEI_UNBENANNT;
    Data->OpenDialog->FilterIndex = 1;
    Data->SaveDialog->FilterIndex = 1;
    EzMinimalZ->Checked = true;
    TfMinimalZ->Checked = true;
    RappViewRapport->Checked = false;
    GewebeNormal->Checked = true;
    GewebeNormalPopup->Checked = true;
    fewithraster = false;
    SetAppTitle();

    // Feldgrössen anpassen
    InitDimensions();
    blatteinzug.feld.Resize (Data->MAXX1, 0);
    kettfarben.feld.Resize (Data->MAXX1, Data->defcolorh);
    einzug.feld.Resize (Data->MAXX1, 0);
    gewebe.feld.Resize (Data->MAXX1, Data->MAXY2, 0);
    aufknuepfung.feld.Resize (Data->MAXX2, Data->MAXY1, 0);
    trittfolge.feld.Resize (Data->MAXX2, Data->MAXY2, 0);
	trittfolge.isempty.Resize (Data->MAXY2, 1);
    schussfarben.feld.Resize (Data->MAXY2, Data->defcolorv);

    // Felder initialisieren!
    if (undo) undo->Clear();
    ClearFelder();
    for (int i=0; i<10; i++) blockmuster[i].Clear();
    for (int i=0; i<10; i++) bereichmuster[i].Clear();
    ViewHlines->Checked = true;
    hlines.DeleteAll();
    AllocBuffers(false);
    InitTools();
    InitZoom();
    CalcGrid();
    Data->palette->InitPalette();
    Data->color = DEFAULT_COLOR;
    kette.a = kette.b = -1;
    schuesse.a = schuesse.b = -1;
    rapport.kr = SZ(0, -1);
    rapport.sr = SZ(0, -1);
    currentrange = 1;
    Range1->Checked = true;
    RecalcDimensions();
    ViewSchlagpatrone->Checked = false;
    trittfolge.einzeltritt = true;
	trittfolge.darstellung = PUNKT;
    OptionsLockGewebe->Checked = false;
    if (cursorhandler) {
        cursorhandler->GotoField (GEWEBE);
        cursorhandler->SetCursorDirection(CD_UP);
        cursorhandler->CheckCursorPos();
    }

    gewebe.kbd.Init();
    einzug.kbd.Init();
    trittfolge.kbd.Init();
    aufknuepfung.kbd.Init();
    schussfarben.kbd.Init();
    kettfarben.kbd.Init();
    blatteinzug.kbd.Init();
    if (cursorhandler) cursorhandler->Init();

    gewebe.pos.Init();
    einzug.pos.Init();
    trittfolge.pos.Init();
    aufknuepfung.pos.Init();
    schussfarben.pos.Init();
    kettfarben.pos.Init();
    blatteinzug.pos.Init();

    for (int i=0; i<9; i++) {
        klammern[i].first = 0;
        klammern[i].last = 1;
        klammern[i].repetitions = 0;
    }

    LoadOptions();

    Cursor = oldcursor;
    Invalidate();
    if (undo) undo->Snapshot();
}

void __fastcall TDBWFRM::DateiSpeichern()
{
    SetAppTitle();
    if (/*!modified || */filename=="") return; //xxxx wird immer gespeichert!

    TCursor oldcursor = Cursor;
    Cursor = crHourGlass;

    if (!Save()) {
        Application->MessageBox (CANNOTSAVE, APP_TITLE, MB_OK);
    } else {
        modified = false;
        saved = true;
    }

    Cursor = oldcursor;
    SetAppTitle();
}

void __fastcall TDBWFRM::DateiLaden (const String& _filename, bool _save, bool _addtomru/*=true*/)
{
    if (_save && !AskSave()) return;

    TCursor oldcursor = Cursor;
    Cursor = crHourGlass;

	// Defaults
    InitDimensions();
	InitBorders();
    InitPrintRange();
    InitZoom();
    InitTools();
    for (int i=0; i<10; i++) blockmuster[i].Clear();
    for (int i=0; i<10; i++) bereichmuster[i].Clear();
    hlines.DeleteAll();
    ViewHlines->Checked = true;
    for (int i=0; i<9; i++) {
        klammern[i].first = 0;
        klammern[i].last = 1;
        klammern[i].repetitions = 0;
    }
    currentrange = 1;
    Range1->Checked = true;

    // Datei schliessen
    CloseDataFile();

    filename = _filename;
	LOADSTAT stat = FILE_LOADED;
    if (!Load(stat)) {
		String msg;
		switch (stat) {
			case UNKNOWN_FAILURE:
				msg = LANG_STR("Unknown Error on loading file ",
							   "Unbekannter Fehler beim Laden von Datei ")
					  + ExtractFileName(filename);
				break;
			case FILE_DOES_NOT_EXIST:
				msg = LANG_STR("File ", "Datei ") + ExtractFileName(filename)
					  + LANG_STR(" does not exist!", "existiert nicht!");
				break;
			case FILE_ALREADY_OPEN:
				msg = LANG_STR("File ", "Datei ") + ExtractFileName(filename)
					  + LANG_STR(" is already opened by another program!",
							" ist bereits von einem anderen Programm geöffnet!");
				break;
			case FILE_WITHOUT_SIGNATURE:
				msg = LANG_STR("File ", "Datei ") + ExtractFileName(filename)
					  + LANG_STR(" is not a DB-WEAVE file!",
							" ist keine DB-WEAVE-Datei!");
				break;
			case FILE_CORRUPT:
				msg = LANG_STR("File ", "Datei ") + ExtractFileName(filename)
					  + LANG_STR(" is corrupt and cannot be correctly loaded!",
							" ist korrupt und kann nicht korrekt geladen werden!");
				break;
		}
        Application->MessageBox (msg.c_str(), APP_TITLE, MB_OK);
        filename = "";
        InitDimensions();
    }
    modified = false;
	saved = false;
    mousedown = false;
    SetAppTitle();
    FinalizeLoad();
    if (!righttoleft) sb_horz1->Position = scroll_x1;
    else sb_horz1->Position = sb_horz1->Max - scroll_x1;
    if (toptobottom) sb_vert1->Position = scroll_y1;
    else sb_vert1->Position = sb_vert1->Max - scroll_y1;
    UpdateScrollbars();
    dbw3_assert(cursorhandler);
    cursorhandler->CheckCursorPos();
    if (undo) undo->Clear();
    Cursor = oldcursor;
    Invalidate();
    if (_addtomru) AddToMRU (filename);
    if (undo) undo->Snapshot();
}

void __fastcall TDBWFRM::FinalizeLoad()
{
    gewebe.feld.Resize(Data->MAXX1, Data->MAXY2, 0);
    RecalcGewebe();
    CalcGrid();
    CalcRangeSchuesse();
    CalcRangeKette();
    CalcRapport();
    RecalcDimensions();
    CURSOR_LARGE_SKIP_X = gewebe.pos.strongline_x*2;
    CURSOR_LARGE_SKIP_Y = gewebe.pos.strongline_y*2;
    if (currentzoom==9) {
        ViewZoomIn->Enabled  = false;
        ViewZoomInPopup->Enabled = false;
        SBZoomIn->Enabled = false;
    } else if (currentzoom==0) {
        ViewZoomOut->Enabled  = false;
        ViewZoomOutPopup->Enabled = false;
        SBZoomOut->Enabled = false;
    }
}

bool __fastcall TDBWFRM::LoadNormalTemplate()
{
    // Falls die Datei normal.dbv im Datenverzeichnis
    // oder im Progverzeichnis existiert, so wird sie
    // als Vorlage benutzt. Analog zum normal.dot bei
    // Word oder so.
    String temp = ExtractFilePath(ParamStr(0));
//    String normaltpl1 = temp+DATEN_VERZEICHNIS1+"normal.dbv";   //xxxy Vorlagen verzeichnis
    String normaltpl2 = temp+"normal.dbv";
//    if (FileExists (normaltpl1)) {
//        DateiLaden (normaltpl1, true, false);
//        CloseDataFile();
//        return true;
//    } else
    if (FileExists (normaltpl2)) {
        DateiLaden (normaltpl2, true, false);
        CloseDataFile();
        return true;
    }
    return false;
}

void __fastcall TDBWFRM::FileNewClick(TObject *Sender)
{
    if (!LoadNormalTemplate()) {
        DateiNeu();
    } else {
        // Rest der Initialisierung durchführen,
        // die nicht von LoadNormalTemplate
        // gemacht wurde.
        filename = "";
        Data->SaveDialog->FileName = DATEI_UNBENANNT;
        Data->OpenDialog->FilterIndex = 1;
        Data->SaveDialog->FilterIndex = 1;
        SetAppTitle();
    }
}

void __fastcall TDBWFRM::FileNewTemplateClick(TObject *Sender)
{
    // Datei laden und dann Dateinamen zurücksetzen
    // Damit wird beim Speichern ein anderer Dateiname
    // vorgeschlagen.
    Data->OpenDialog->FilterIndex = 2;
    Data->SaveDialog->FilterIndex = 1;
    Data->OpenDialog->FileName = "*.dbv";
    if (filename!="") Data->OpenDialog->InitialDir = ExtractFilePath (filename);
    else Data->OpenDialog->InitialDir = ""; //xxxy eigene Dateien
    if (Data->OpenDialog->Execute()) {
        DateiLaden (Data->OpenDialog->FileName, true, false);
        CloseDataFile();
        filename = "";
        Data->OpenDialog->FileName = "*.dbw";
        Data->SaveDialog->FileName = DATEI_UNBENANNT;
        SetAppTitle();
    }
}

static bool IsTemplate (const String& _filename)
{
    int pos = _filename.Pos (".dbv");
    return pos!=0 && pos==(_filename.Length()-4+1);
}

void __fastcall TDBWFRM::FileOpenClick(TObject *Sender)
{
    Data->OpenDialog->FilterIndex = 1;
    Data->SaveDialog->FilterIndex = 1;
    if (filename!="") {
        Data->OpenDialog->InitialDir = ExtractFilePath (filename);
        Data->OpenDialog->FileName = ExtractFileName (filename);
    }
    if (Data->OpenDialog->Execute()) {
        DateiLaden (Data->OpenDialog->FileName, true, !IsTemplate(Data->OpenDialog->FileName));
        Data->SaveDialog->FilterIndex = Data->OpenDialog->FilterIndex;
    }
}

void __fastcall TDBWFRM::FileSaveClick(TObject *Sender)
{
    if (filename=="") {
        if (Data->SaveDialog->Execute()) {
            if (!CheckExistence (Data->SaveDialog->FileName)) return;
            filename = Data->SaveDialog->FileName;
            modified = true;
        }
    }

    if (filename!="") {
        DateiSpeichern();
        if (!IsTemplate(filename)) AddToMRU (filename);
    }
}

void __fastcall TDBWFRM::FileSaveasClick(TObject *Sender)
{
    if (IsTemplate(filename)) Data->SaveDialog->FilterIndex = 2;
    else Data->SaveDialog->FilterIndex = 1;

    if (!filename.IsEmpty()) {
        Data->SaveDialog->InitialDir = ExtractFilePath (filename);
        Data->SaveDialog->FileName = ExtractFileName (filename);
    } else {
        Data->SaveDialog->InitialDir = ""; //xxxy Eigene Dateien
        Data->SaveDialog->FileName = DATEI_UNBENANNT;
    }
    if (Data->SaveDialog->Execute()) {
        if (!CheckExistence (Data->SaveDialog->FileName)) return;
        filename = Data->SaveDialog->FileName;
        modified = true;
        CloseDataFile();
        DateiSpeichern();
        if (!IsTemplate(filename)) AddToMRU (filename);
    }
}

void __fastcall TDBWFRM::FileRevertClick(TObject *Sender)
{
    // Zurück zur gespeicherten Version
    //xxxx Ohne Abfrage?!
    if (filename!="") {
        // Laden ohne Änderungen speichern
        DateiLaden (filename, false);
    }
}

bool __fastcall TDBWFRM::AskSave()
{
    // Falls geändert, eventuell zuerst speichern
    if (modified) {
        if (filename!="") {
            int result = Application->MessageBox (SAVE_CHANGES, APP_TITLE, MB_YESNOCANCEL);
            if (result==IDCANCEL) return false;
            if (result==IDYES)
                DateiSpeichern();
        } else if (kette.b!=-1 || schuesse.b!=-1 ||
                   GetFirstSchaft()<=GetLastSchaft() || GetFirstTritt()<=GetLastTritt())
        {
            int result = Application->MessageBox (SAVE_CHANGES, APP_TITLE, MB_YESNOCANCEL);
            if (result==IDCANCEL) return false;
            if (result==IDYES) {
                if (!Data->SaveDialog->Execute()) return false;
                if (!CheckExistence (Data->SaveDialog->FileName)) return false;
                filename = Data->SaveDialog->FileName;
                DateiSpeichern();
            }
        }
    }
    return true;
}

bool __fastcall TDBWFRM::CheckExistence(const String& _filename)
{
    if (FileExists(_filename)) {
        char msg[1024];
        wsprintf (msg, CAN_OVERWRITE, _filename.c_str());
        int result = Application->MessageBox (msg, APP_TITLE, MB_YESNO);
        return result==IDYES;
    } else {
        return true;
    }
}

