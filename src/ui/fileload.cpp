/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "fileload.h"
#include "mainwindow.h"
#include "patterncanvas.h"
#include "datamodule.h"
#include "blockmuster.h"
#include "fileformat.h"
#include "loadmap.h"
#include "palette.h"
#include "rapport.h"
#include "hilfslinien.h"
#include "properties.h"
#include "assert_compat.h"

#include <QFileInfo>

#include <algorithm>
#include <cstdlib>
#include <cstring>

/*-----------------------------------------------------------------*/
/*  TDBWFRM::Load wraps an FhLoader instance so callers don't have
    to know about the loader class. Matches the legacy call site:
        bool ok = DBWFRM->Load(stat, LOADALL); */
bool TDBWFRM::Load(LOADSTAT& _stat, LOADPARTS _loadparts)
{
    try {
        FhLoader loader(this);
        return loader.Load(_stat, _loadparts);
    } catch (...) {
        _stat = UNKNOWN_FAILURE;
        return false;
    }
}
/*-----------------------------------------------------------------*/
FhLoader::FhLoader(TDBWFRM* _mainfrm)
    : mainfrm(_mainfrm)
    , loadparts(LOADALL)
{
    dbw3_assert(_mainfrm);
}
/*-----------------------------------------------------------------*/
bool FhLoader::Need(LOADOPTION _part) const
{
    return ((unsigned int)(loadparts & _part)) == (unsigned int)_part;
}
/*-----------------------------------------------------------------*/
bool FhLoader::Load(LOADSTAT& _stat, LOADPARTS _loadparts)
{
    loadparts = _loadparts;
    if (mainfrm->filename.isEmpty()) {
        _stat = FILE_DOES_NOT_EXIST;
        return false;
    }
    if (!QFileInfo::exists(mainfrm->filename)) {
        _stat = FILE_DOES_NOT_EXIST;
        return false;
    }

    try {
        const QByteArray fn = mainfrm->filename.toLocal8Bit();
        if (!mainfrm->file->IsOpen()) {
            mainfrm->file->Open(fn.constData(), FfOpenRead | FfOpenWrite);
        } else {
            mainfrm->file->SeekBegin();
        }
        if (!mainfrm->file->IsOpen()) {
            _stat = FILE_ALREADY_OPEN;
            return false;
        }

        FfReader reader;
        reader.Assign(mainfrm->file);

        if (!LoadSignatur(&reader)) {
            _stat = FILE_WITHOUT_SIGNATURE;
            mainfrm->file->Close();
            return false;
        }

        FfReader* _reader = &reader;
        BEGIN_LOAD_MAP
        BEGIN_FIELD_MAP
        NO_FIELDS
        BEGIN_SECTION_MAP
        _SECTION_MAP("version", LoadVersion)
        SECTION_MAP("properties", LoadProperties)
        SECTION_MAP("data", LoadData)
        SECTION_MAP("view", LoadView)
        SECTION_MAP("printsettings", LoadPrint)
        DEFAULT_SECTION
        BEGIN_DEFAULT_MAP
        END_LOAD_MAP
    } catch (...) {
        _stat = FILE_CORRUPT;
        mainfrm->file->Close();
        return false;
    }

    /*  After a successful load, rebuild the derived state so the
        next paintEvent renders something coherent. */
    mainfrm->RecalcGewebe();
    mainfrm->CalcRangeKette();
    mainfrm->CalcRangeSchuesse();
    if (mainfrm->rapporthandler)
        mainfrm->rapporthandler->CalcRapport();

    /*  RecalcFreieSchaefte/Tritte from the loaded data -- walk the
        einzug to find which shafts are used, ditto aufknuepfung
        columns for the treadles.                                  */
    for (int j = 0; j < Data->MAXY1; j++)
        mainfrm->freieschaefte[j] = true;
    for (int i = 0; i < Data->MAXX1; i++) {
        const short s = mainfrm->einzug.feld.Get(i);
        if (s > 0 && s - 1 < Data->MAXY1)
            mainfrm->freieschaefte[s - 1] = false;
    }
    for (int i = 0; i < Data->MAXX2; i++) {
        bool used = false;
        for (int j = 0; j < Data->MAXY1 && !used; j++)
            if (mainfrm->aufknuepfung.feld.Get(i, j) > 0)
                used = true;
        mainfrm->freietritte[i] = !used;
    }

    /*  LoadViewGeneral sets ViewSchlagpatrone->setChecked() directly,
        which bypasses the triggered signal — so trittfolge.einzeltritt
        never flips, and the Trittfolge / Pegplan menu visibility
        doesn't update. Sync both here.                              */
    const bool pegplan
        = mainfrm->ViewSchlagpatrone && mainfrm->ViewSchlagpatrone->isChecked();
    mainfrm->trittfolge.einzeltritt = !pegplan;
    mainfrm->UpdateSchlagpatroneMode();

    /*  Re-apply layout-dependent toggles. hvisible / wvisible and the
        ViewEinzug / ViewTrittfolge / ViewFarbe / ViewBlatteinzug /
        ViewHlines visibilities all feed into PatternCanvas's strip
        sizing; without a recomputeLayout() they'd keep the stale
        dimensions from before the load.                              */
    if (mainfrm->pattern_canvas)
        mainfrm->pattern_canvas->recomputeLayout();
    mainfrm->refresh();

    mainfrm->file->Close();
    _stat = FILE_LOADED;
    return true;
}
/*-----------------------------------------------------------------*/
bool FhLoader::LoadSignatur(FfReader* _reader)
{
    dbw3_assert(_reader);
    FfToken* token = _reader->GetToken();
    const bool success = token && token->GetType() == FfSignature;
    delete token;
    return success;
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadVersion(FfReader* _reader)
{
    /*  Swallow the section. Real "version" is two fmt/ver fields;
        we don't act on them. Used as a stand-in for "properties" /
        "view" / "printsettings" which we also skip in this slice. */
    char* fmt = 0;
    char* ver = 0;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_STRING("fmt", fmt)
    FIELD_MAP_STRING("ver", ver)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    delete[] fmt;
    delete[] ver;
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadProperties(FfReader* _reader)
{
    if (Data->properties)
        Data->properties->Load(_reader);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadData(FfReader* _reader)
{
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    NO_FIELDS
    BEGIN_SECTION_MAP
    _SECTION_MAP("size", LoadDataSize)
    SECTION_MAP("fields", LoadDataFields)
    SECTION_MAP("palette", LoadDataPalette)
    SECTION_MAP("hilfslinien", LoadDataHilfslinien)
    SECTION_MAP("webstuhl", LoadDataWebstuhl)
    SECTION_MAP("blockmuster", LoadDataBlockmuster)
    SECTION_MAP("bereichmuster", LoadDataBereichmuster)
    DEFAULT_SECTION
        BEGIN_DEFAULT_MAP END_LOAD_MAP
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataSize(FfReader* _reader)
{
    int maxx1 = 300;
    int maxy1 = 30;
    int maxx2 = 30;
    int maxy2 = 300;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("maxx1", maxx1, int)
    FIELD_MAP_INT("maxy1", maxy1, int)
    FIELD_MAP_INT("maxx2", maxx2, int)
    FIELD_MAP_INT("maxy2", maxy2, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP

    if (Need(LOADALL)) {
        Data->MAXX1 = maxx1;
        Data->MAXY1 = maxy1;
        Data->MAXX2 = maxx2;
        Data->MAXY2 = maxy2;
        if (mainfrm->hvisible > Data->MAXY1)
            mainfrm->hvisible = Data->MAXY1;
        if (mainfrm->wvisible > Data->MAXX2)
            mainfrm->wvisible = Data->MAXX2;
        mainfrm->AllocBuffers(true);
    }
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataFields(FfReader* _reader)
{
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    NO_FIELDS
    BEGIN_SECTION_MAP
    _SECTION_MAP("einzug", LoadDataEinzug)
    SECTION_MAP("aufknuepfung", LoadDataAufknuepfung)
    SECTION_MAP("trittfolge", LoadDataTrittfolge)
    SECTION_MAP("schussfarben", LoadDataSchussfarben)
    SECTION_MAP("kettfarben", LoadDataKettfarben)
    SECTION_MAP("blatteinzug", LoadDataBlatteinzug)
    SECTION_MAP("fixeinzug", LoadDataFixeinzug)
    DEFAULT_SECTION /* fixeinzug skipped */
        BEGIN_DEFAULT_MAP END_LOAD_MAP
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataEinzug(FfReader* _reader)
{
    if (Need(LOADEINZUG))
        mainfrm->einzug.feld.Read(_reader, 0);
    else
        _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataAufknuepfung(FfReader* _reader)
{
    if (Need(LOADAUFKNUEPFUNG))
        mainfrm->aufknuepfung.feld.Read(_reader, 0);
    else
        _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataTrittfolge(FfReader* _reader)
{
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    NO_FIELDS
    BEGIN_SECTION_MAP
    _SECTION_MAP("trittfolge", LoadDataTrittfolgeTrittfolge)
    SECTION_MAP("isempty", LoadDataTrittfolgeIsEmpty)
    DEFAULT_SECTION
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataTrittfolgeTrittfolge(FfReader* _reader)
{
    if (Need(LOADTRITTFOLGE))
        mainfrm->trittfolge.feld.Read(_reader, 0);
    else
        _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataTrittfolgeIsEmpty(FfReader* _reader)
{
    if (Need(LOADTRITTFOLGE))
        mainfrm->trittfolge.isempty.Read(_reader, 1);
    else
        _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataSchussfarben(FfReader* _reader)
{
    if (Need(LOADSCHUSSFARBEN))
        mainfrm->schussfarben.feld.Read(_reader, Data->defcolorv);
    else
        _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataKettfarben(FfReader* _reader)
{
    if (Need(LOADKETTFARBEN))
        mainfrm->kettfarben.feld.Read(_reader, Data->defcolorh);
    else
        _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataBlatteinzug(FfReader* _reader)
{
    if (Need(LOADBLATTEINZUG))
        mainfrm->blatteinzug.feld.Read(_reader, 0);
    else
        _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataPalette(FfReader* _reader)
{
    if (Need(LOADPALETTE))
        Data->palette->Load(_reader);
    else
        _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataHilfslinien(FfReader* _reader)
{
    if (!Need(LOADALL)) {
        _reader->SkipSection();
        return;
    }

    int count = 0;
    char* list = nullptr;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("count", count, int)
    FIELD_MAP_BINARY("list", list)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP

    mainfrm->hlines.DeleteAll();
    if (list && count > 0) {
        const Hilfslinie* entries = reinterpret_cast<const Hilfslinie*>(list);
        for (int i = 0; i < count; i++)
            mainfrm->hlines.Add(entries[i].typ, entries[i].feld, entries[i].pos);
    }
    delete[] list;
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataFixeinzug(FfReader* _reader)
{
    char* data = nullptr;
    int dataBytes = 0;
    int firstfree = 1;
    int fixsize = Data ? Data->MAXX1 : 0;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_BINARY_SIZED("fixeinzug", data, dataBytes)
    FIELD_MAP_INT("firstfree", firstfree, int)
    FIELD_MAP_INT("fixsize", fixsize, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    mainfrm->firstfree = short(firstfree);
    mainfrm->fixsize = fixsize;
    if (data) {
        /*  Allocate a properly-typed short[] so the destructor's
            delete[] gets the correct type. Copy only as many bytes
            as the file actually stored (older files may be shorter
            than sizeof(short) * MAXX1); zero the rest.              */
        delete[] mainfrm->fixeinzug;
        const int want = int(sizeof(short)) * Data->MAXX1;
        mainfrm->fixeinzug = new short[Data->MAXX1];
        std::memset(mainfrm->fixeinzug, 0, want);
        const int copy = dataBytes < want ? dataBytes : want;
        std::memcpy(mainfrm->fixeinzug, data, copy);
        delete[] data;
    }
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataBlockmuster(FfReader* _reader)
{
    char* data[10] = { nullptr };
    int sizes[10] = { 0 };
    int einzugz = mainfrm->einzugZ ? 1 : 0;
    int trittfolgez = mainfrm->trittfolgeZ ? 1 : 0;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_BINARY_SIZED("bindung0", data[0], sizes[0])
    FIELD_MAP_BINARY_SIZED("bindung1", data[1], sizes[1])
    FIELD_MAP_BINARY_SIZED("bindung2", data[2], sizes[2])
    FIELD_MAP_BINARY_SIZED("bindung3", data[3], sizes[3])
    FIELD_MAP_BINARY_SIZED("bindung4", data[4], sizes[4])
    FIELD_MAP_BINARY_SIZED("bindung5", data[5], sizes[5])
    FIELD_MAP_BINARY_SIZED("bindung6", data[6], sizes[6])
    FIELD_MAP_BINARY_SIZED("bindung7", data[7], sizes[7])
    FIELD_MAP_BINARY_SIZED("bindung8", data[8], sizes[8])
    FIELD_MAP_BINARY_SIZED("bindung9", data[9], sizes[9])
    FIELD_MAP_INT("einzugz", einzugz, int)
    FIELD_MAP_INT("trittfolgez", trittfolgez, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    mainfrm->einzugZ = (einzugz != 0);
    mainfrm->trittfolgeZ = (trittfolgez != 0);
    const int expected = mainfrm->blockmuster[0].DataSize();
    for (int i = 0; i < 10; i++) {
        if (data[i]) {
            /*  Only apply if the file stored exactly the expected
                per-Muster byte count (12*12 = 144). Older / partial
                files may have a shorter blob; SetData would
                memcpy past the allocation and corrupt the heap, so
                we defensively skip those.                         */
            if (sizes[i] >= expected)
                mainfrm->blockmuster[i].SetData(data[i]);
            delete[] data[i];
        }
    }
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataBereichmuster(FfReader* _reader)
{
    char* data[10] = { nullptr };
    int sizes[10] = { 0 };
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_BINARY_SIZED("bindung0", data[0], sizes[0])
    FIELD_MAP_BINARY_SIZED("bindung1", data[1], sizes[1])
    FIELD_MAP_BINARY_SIZED("bindung2", data[2], sizes[2])
    FIELD_MAP_BINARY_SIZED("bindung3", data[3], sizes[3])
    FIELD_MAP_BINARY_SIZED("bindung4", data[4], sizes[4])
    FIELD_MAP_BINARY_SIZED("bindung5", data[5], sizes[5])
    FIELD_MAP_BINARY_SIZED("bindung6", data[6], sizes[6])
    FIELD_MAP_BINARY_SIZED("bindung7", data[7], sizes[7])
    FIELD_MAP_BINARY_SIZED("bindung8", data[8], sizes[8])
    FIELD_MAP_BINARY_SIZED("bindung9", data[9], sizes[9])
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    const int expected = mainfrm->bereichmuster[0].DataSize();
    for (int i = 0; i < 10; i++) {
        if (data[i]) {
            if (sizes[i] >= expected)
                mainfrm->bereichmuster[i].SetData(data[i]);
            delete[] data[i];
        }
    }
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataWebstuhlCurrent(FfReader* _reader)
{
    int pos = 0, kl = 0, rep = 0;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("position", pos, int)
    FIELD_MAP_INT("klammer", kl, int)
    FIELD_MAP_INT("repetition", rep, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    mainfrm->weave_position = pos;
    mainfrm->weave_klammer = kl;
    mainfrm->weave_repetition = rep;
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataWebstuhlLast(FfReader* _reader)
{
    int pos = 0, kl = 0, rep = 0;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("position", pos, int)
    FIELD_MAP_INT("klammer", kl, int)
    FIELD_MAP_INT("repetition", rep, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    mainfrm->last_position = pos;
    mainfrm->last_klammer = kl;
    mainfrm->last_repetition = rep;
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataWebstuhlDivers(FfReader* _reader)
{
    int schussselected = mainfrm->schussselected ? 1 : 0;
    int scrolly = mainfrm->scrolly_weben;
    int firstschuss = mainfrm->firstschuss ? 1 : 0;
    int weaving = mainfrm->weaving ? 1 : 0;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("schussselected", schussselected, int)
    FIELD_MAP_INT("scrolly", scrolly, int)
    FIELD_MAP_INT("firstschuss", firstschuss, int)
    FIELD_MAP_INT("weaving", weaving, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    mainfrm->schussselected = (schussselected != 0);
    mainfrm->scrolly_weben = scrolly;
    mainfrm->firstschuss = (firstschuss != 0);
    mainfrm->weaving = (weaving != 0);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadPrint(FfReader* _reader)
{
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    NO_FIELDS
    BEGIN_SECTION_MAP
    _SECTION_MAP("printrange", LoadPrintRange)
    DEFAULT_SECTION
        BEGIN_DEFAULT_MAP END_LOAD_MAP
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadPrintRange(FfReader* _reader)
{
    int ka = mainfrm->printkette.a;
    int kb = mainfrm->printkette.b;
    int sa = mainfrm->printschuesse.a;
    int sb = mainfrm->printschuesse.b;
    int scha = mainfrm->printschaefte.a;
    int schb = mainfrm->printschaefte.b;
    int ta = mainfrm->printtritte.a;
    int tb = mainfrm->printtritte.b;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("kettevon", ka, int)
    FIELD_MAP_INT("kettebis", kb, int)
    FIELD_MAP_INT("schuessevon", sa, int)
    FIELD_MAP_INT("schuessebis", sb, int)
    FIELD_MAP_INT("schaeftevon", scha, int)
    FIELD_MAP_INT("schaeftebis", schb, int)
    FIELD_MAP_INT("trittevon", ta, int)
    FIELD_MAP_INT("trittebis", tb, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    mainfrm->printkette = SZ(ka, kb);
    mainfrm->printschuesse = SZ(sa, sb);
    mainfrm->printschaefte = SZ(scha, schb);
    mainfrm->printtritte = SZ(ta, tb);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataWebstuhlKlammer(FfReader* _reader, int _index)
{
    int first = 0;
    int last = 1;
    int repetitions = 0;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("first", first, int)
    FIELD_MAP_INT("last", last, int)
    FIELD_MAP_INT("repetitions", repetitions, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    if (_index >= 0 && _index < 9) {
        mainfrm->klammern[_index].first = first;
        mainfrm->klammern[_index].last = last;
        mainfrm->klammern[_index].repetitions = repetitions;
    }
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataWebstuhl(FfReader* _reader)
{
    /*  Webstuhl carries klammer0..klammer8 sections plus "current" /
        "last" / "divers" that belong to the (unported) loom-control
        window. Match section names manually because SECTION_MAP
        can't forward an index to the handler.                    */
    dbw3_assert(_reader);
    FfToken* token = _reader->GetToken();
    while (token) {
        if (token->GetType() == FfSection) {
            const char* name = (const char*)((FfTokenSection*)token)->data;
            int idx = -1;
            if (name && std::strncmp(name, "klammer", 7) == 0 && name[7] != '\0') {
                const int n = std::atoi(name + 7);
                if (n >= 0 && n < 9)
                    idx = n;
            }
            if (idx >= 0)
                LoadDataWebstuhlKlammer(_reader, idx);
            else if (name && std::strcmp(name, "current") == 0)
                LoadDataWebstuhlCurrent(_reader);
            else if (name && std::strcmp(name, "last") == 0)
                LoadDataWebstuhlLast(_reader);
            else if (name && std::strcmp(name, "divers") == 0)
                LoadDataWebstuhlDivers(_reader);
            else
                _reader->SkipSection();
        } else if (token->GetType() == FfField) {
            _reader->SkipField();
        } else if (token->GetType() == FfEndSection) {
            delete token;
            break;
        } else {
            delete token;
            throw int(0);
        }
        delete token;
        token = _reader->GetToken();
    }
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadView(FfReader* _reader)
{
    /*  Seed the darstellungen to the file-format defaults BEFORE
        parsing so missing view subsections fall back to those
        defaults rather than keeping the session's Grundeinstellung.
        Any subsection actually present in the file overrides via
        the SECTION_MAP below.                                     */
    mainfrm->einzug.darstellung = PUNKT;
    mainfrm->aufknuepfung.darstellung = KREUZ;
    mainfrm->trittfolge.darstellung = PUNKT;
    mainfrm->schlagpatronendarstellung = AUSGEFUELLT;

    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    NO_FIELDS
    BEGIN_SECTION_MAP
    _SECTION_MAP("general", LoadViewGeneral)
    SECTION_MAP("gewebe", LoadViewGewebe)
    SECTION_MAP("einzug", LoadViewEinzug)
    SECTION_MAP("aufknuepfung", LoadViewAufknuepfung)
    SECTION_MAP("trittfolge", LoadViewTrittfolge)
    SECTION_MAP("schlagpatrone", LoadViewSchlagpatrone)
    SECTION_MAP("blatteinzug", LoadViewBlatteinzug)
    SECTION_MAP("kettfarben", LoadViewKettfarben)
    SECTION_MAP("schussfarben", LoadViewSchussfarben)
    SECTION_MAP("pagesetup", LoadViewPagesetup)
    DEFAULT_SECTION
        BEGIN_DEFAULT_MAP END_LOAD_MAP
}
/*-----------------------------------------------------------------*/
static void setChecked(QAction* _a, bool _v)
{
    if (_a)
        _a->setChecked(_v);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewGeneral(FfReader* _reader)
{
    int zoom = mainfrm->currentzoom;
    int hebung = 1; /* inverse of sinkingshed */
    int color = Data ? int(Data->color) : 1;
    int viewpalette
        = mainfrm->ViewFarbpalette ? mainfrm->ViewFarbpalette->isChecked() : 1;
    int viewpegplan = mainfrm->ViewSchlagpatrone ? mainfrm->ViewSchlagpatrone->isChecked() : 0;
    int viewrapport = mainfrm->RappViewRapport ? mainfrm->RappViewRapport->isChecked() : 0;
    int viewhlines = mainfrm->ViewHlines ? mainfrm->ViewHlines->isChecked() : 1;
    int righttoleft = mainfrm->righttoleft ? 1 : 0;
    int toptobottom = mainfrm->toptobottom ? 1 : 0;
    double fk = mainfrm->faktor_kette;
    double fs = mainfrm->faktor_schuss;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("zoom", zoom, int)
    FIELD_MAP_INT("hebung", hebung, int)
    FIELD_MAP_INT("color", color, int)
    FIELD_MAP_INT("viewpalette", viewpalette, int)
    FIELD_MAP_INT("viewpegplan", viewpegplan, int)
    FIELD_MAP_INT("viewrapport", viewrapport, int)
    FIELD_MAP_INT("viewhlines", viewhlines, int)
    FIELD_MAP_INT("righttoleft", righttoleft, int)
    FIELD_MAP_INT("toptobottom", toptobottom, int)
    FIELD_MAP_DOUBLE("faktor_kette", fk)
    FIELD_MAP_DOUBLE("faktor_schuss", fs)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    mainfrm->currentzoom = (zoom < 0 ? 0 : (zoom > 9 ? 9 : zoom));
    mainfrm->sinkingshed = (hebung == 0);
    if (Data)
        Data->color = (unsigned char)color;
    mainfrm->faktor_kette = float(fk);
    mainfrm->faktor_schuss = float(fs);
    mainfrm->righttoleft = (righttoleft != 0);
    mainfrm->toptobottom = (toptobottom != 0);
    setChecked(mainfrm->ViewFarbpalette, viewpalette != 0);
    setChecked(mainfrm->ViewSchlagpatrone, viewpegplan != 0);
    setChecked(mainfrm->RappViewRapport, viewrapport != 0);
    setChecked(mainfrm->ViewHlines, viewhlines != 0);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewGewebe(FfReader* _reader)
{
    int state = 0; /* 0=Normal 1=Farbeffekt 2=Simulation 3=None */
    int withgrid = mainfrm->fewithraster ? 1 : 0;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("state", state, int)
    FIELD_MAP_INT("withgrid", withgrid, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    setChecked(mainfrm->GewebeNormal, state == 0);
    setChecked(mainfrm->GewebeFarbeffekt, state == 1);
    setChecked(mainfrm->GewebeSimulation, state == 2);
    setChecked(mainfrm->GewebeNone, state == 3);
    mainfrm->fewithraster = (withgrid != 0);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewEinzug(FfReader* _reader)
{
    /*  Hardcoded per-field defaults match legacy file-format
        expectations: when a pattern file lacks viewtype, the symbol
        resets to the historical default (PUNKT for einzug) so two
        loads of the same file always yield the same look, regardless
        of the active Grundeinstellung.                             */
    int visible = 1;
    int down = 0;
    int viewtype = int(PUNKT);
    int hvisible = mainfrm->hvisible;
    int style = 1; /* 1 = EzMinimalZ -- legacy default */
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("visible", visible, int)
    FIELD_MAP_INT("down", down, int)
    FIELD_MAP_INT("viewtype", viewtype, int)
    FIELD_MAP_INT("hvisible", hvisible, int)
    FIELD_MAP_INT("style", style, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    setChecked(mainfrm->ViewEinzug, visible != 0);
    mainfrm->einzugunten = (down != 0);
    mainfrm->einzug.darstellung = DARSTELLUNG(viewtype);
    if (hvisible > 0)
        mainfrm->hvisible = hvisible;
    /*  style selects the einzug-rearrangement radio group. Map legacy
        indices verbatim; out-of-range falls back to MinimalZ.      */
    switch (style) {
    case 0: setChecked(mainfrm->EzFixiert, true); break;
    case 1: setChecked(mainfrm->EzMinimalZ, true); break;
    case 2: setChecked(mainfrm->EzMinimalS, true); break;
    case 3: setChecked(mainfrm->EzGeradeZ, true); break;
    case 4: setChecked(mainfrm->EzGeradeS, true); break;
    case 5: setChecked(mainfrm->EzBelassen, true); break;
    case 6: setChecked(mainfrm->EzChorig2, true); break;
    case 7: setChecked(mainfrm->EzChorig3, true); break;
    default: setChecked(mainfrm->EzMinimalZ, true); break;
    }
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewAufknuepfung(FfReader* _reader)
{
    int viewtype = int(KREUZ);
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("viewtype", viewtype, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    mainfrm->aufknuepfung.darstellung = DARSTELLUNG(viewtype);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewTrittfolge(FfReader* _reader)
{
    int visible = 1;
    int viewtype = int(PUNKT);
    int einzeltritt = 1;
    int wvisible = mainfrm->wvisible;
    int style = 1;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("visible", visible, int)
    FIELD_MAP_INT("viewtype", viewtype, int)
    FIELD_MAP_INT("single", einzeltritt, int)
    FIELD_MAP_INT("wvisible", wvisible, int)
    FIELD_MAP_INT("style", style, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    setChecked(mainfrm->ViewTrittfolge, visible != 0);
    mainfrm->trittfolge.darstellung = DARSTELLUNG(viewtype);
    mainfrm->trittfolge.einzeltritt = (einzeltritt != 0);
    if (wvisible > 0)
        mainfrm->wvisible = wvisible;
    /*  Trittfolge-rearrangement radio group selector. */
    switch (style) {
    case 0: setChecked(mainfrm->TfBelassen, true); break;
    case 1: setChecked(mainfrm->TfMinimalZ, true); break;
    case 2: setChecked(mainfrm->TfMinimalS, true); break;
    case 3: setChecked(mainfrm->TfGeradeZ, true); break;
    case 4: setChecked(mainfrm->TfGeradeS, true); break;
    case 5: setChecked(mainfrm->TfGesprungen, true); break;
    default: setChecked(mainfrm->TfMinimalZ, true); break;
    }
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewSchlagpatrone(FfReader* _reader)
{
    int viewtype = int(AUSGEFUELLT);
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("viewtype", viewtype, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    mainfrm->schlagpatronendarstellung = DARSTELLUNG(viewtype);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewBlatteinzug(FfReader* _reader)
{
    int visible = mainfrm->ViewBlatteinzug ? mainfrm->ViewBlatteinzug->isChecked() : 1;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("visible", visible, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    setChecked(mainfrm->ViewBlatteinzug, visible != 0);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewKettfarben(FfReader* _reader)
{
    int visible = mainfrm->ViewFarbe ? mainfrm->ViewFarbe->isChecked() : 1;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("visible", visible, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    setChecked(mainfrm->ViewFarbe, visible != 0);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewSchussfarben(FfReader* _reader)
{
    int visible = mainfrm->ViewFarbe ? mainfrm->ViewFarbe->isChecked() : 1;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("visible", visible, int)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    setChecked(mainfrm->ViewFarbe, visible != 0);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewPagesetup(FfReader* _reader)
{
    int topborder = mainfrm->borders.range.top;
    int bottomborder = mainfrm->borders.range.bottom;
    int leftborder = mainfrm->borders.range.left;
    int rightborder = mainfrm->borders.range.right;
    int headerheight = mainfrm->header.height;
    int footerheight = mainfrm->footer.height;
    char* headertext = nullptr;
    char* footertext = nullptr;
    BEGIN_LOAD_MAP
    BEGIN_FIELD_MAP
    _FIELD_MAP_INT("topmargin", topborder, int)
    FIELD_MAP_INT("bottommargin", bottomborder, int)
    FIELD_MAP_INT("leftmargin", leftborder, int)
    FIELD_MAP_INT("rightmargin", rightborder, int)
    FIELD_MAP_INT("headerheight", headerheight, int)
    FIELD_MAP_STRING("headertext", headertext)
    FIELD_MAP_INT("footerheight", footerheight, int)
    FIELD_MAP_STRING("footertext", footertext)
    DEFAULT_FIELD
    BEGIN_SECTION_MAP
    NO_SECTIONS
    BEGIN_DEFAULT_MAP
    END_LOAD_MAP
    mainfrm->borders.range.top = topborder;
    mainfrm->borders.range.bottom = bottomborder;
    mainfrm->borders.range.left = leftborder;
    mainfrm->borders.range.right = rightborder;
    mainfrm->header.height = headerheight;
    mainfrm->footer.height = footerheight;
    if (headertext) {
        mainfrm->header.text = QString::fromUtf8(headertext);
        delete[] headertext;
    }
    if (footertext) {
        mainfrm->footer.text = QString::fromUtf8(footertext);
        delete[] footertext;
    }
}
/*-----------------------------------------------------------------*/
