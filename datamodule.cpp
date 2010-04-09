// DB-WEAVE, textile CAD/CAM software
// Copyright (c) 1998-2005 Brunold Software, Switzerland

#include <vcl\vcl.h>
#pragma hdrstop

#include "assert.h"
#include "datamodule.h"
#include "palette.h"
#include "properties.h"
#include "language.h"
#include "dbw3_strings.h"

#pragma resource "*.dfm"

TData *Data;

#define ALLOC_OBJ(a,cls) {try{a=new cls;}catch(...){a=0;}}

__fastcall TData::TData(TComponent* Owner)
: TDataModule(Owner)
{
	ALLOC_OBJ(palette, Palette);
	ALLOC_OBJ(properties, FileProperties);
}

__fastcall TData::~TData()
{
	delete palette;
	delete properties;
}

void __fastcall TData::ReloadLanguage()
{
    OpenDialog->Filter = LANG_STR("DB-WEAVE Pattern|*.dbw|DB-WEAVE Template|*.dbv|All Files|*.*", "DB-WEAVE Muster|*.dbw|DB-WEAVE Vorlage|*.dbv|Alle Dateien|*.*");
    SaveDialog->Filter = LANG_STR("DB-WEAVE Pattern|*.dbw|DB-WEAVE Template|*.dbv|All Files|*.*", "DB-WEAVE Muster|*.dbw|DB-WEAVE Vorlage|*.dbv|Alle Dateien|*.*");
    if (SaveDialog->FileName=="unbenannt" || SaveDialog->FileName=="unnamed")
        SaveDialog->FileName = DATEI_UNBENANNT;
    ImportBmpDialog->Filter = LANG_STR("All (*.bmp;*.ico;*.emf;*.wmf)|*.bmp;*.ico;*.emf;*.wmf|Bitmaps (*.bmp)|*.bmp|Icons (*.ico)|*.ico|Enhanced Metafiles (*.emf)|*.emf|Metafiles (*.wmf)|*.wmf", "Alle (*.bmp;*.ico;*.emf;*.wmf)|*.bmp;*.ico;*.emf;*.wmf|Bitmaps (*.bmp)|*.bmp|Icons (*.ico)|*.ico|Enhanced Metafiles (*.emf)|*.emf|Metafiles (*.wmf)|*.wmf");
    ImportBmpDialog->Title = LANG_STR("Choose Picture", "Bild wählen");
    ExportDialog->Title = LANG_STR("Export pattern", "Muster exportieren");
    ExportDialog->Filter = LANG_STR("Bitmap (*.bmp)|*.bmp|Weave Information File (*.WIF)|*.wif|DB-WEAVE 3.5 Pattern|*.dbw", "Bitmap (*.bmp)|*.bmp|Weave Information File (*.wif)|*.wif|DB-WEAVE 3.5 Muster|*.dbw");
    if (ExportDialog->FileName=="unbenannt" || ExportDialog->FileName=="unnamed")
        ExportDialog->FileName = DATEI_UNBENANNT;
    ImportDialog->Title = LANG_STR("Import pattern", "Muster importieren");
    ImportDialog->Filter = LANG_STR("Weave Information File (*.WIF)|*.wif", "Weave Information File (*.wif)|*.wif");
    if (ImportDialog->FileName=="unbenannt" || ImportDialog->FileName=="unnamed")
        ImportDialog->FileName = DATEI_UNBENANNT;
}



