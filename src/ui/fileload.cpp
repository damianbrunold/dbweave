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
#include "datamodule.h"
#include "fileformat.h"
#include "loadmap.h"
#include "palette.h"
#include "rapport.h"
#include "hilfslinien.h"
#include "assert_compat.h"

#include <QFileInfo>

#include <algorithm>
#include <cstdlib>
#include <cstring>

/*-----------------------------------------------------------------*/
/*  TDBWFRM::Load wraps an FhLoader instance so callers don't have
    to know about the loader class. Matches the legacy call site:
        bool ok = DBWFRM->Load(stat, LOADALL); */
bool __fastcall TDBWFRM::Load (LOADSTAT& _stat, LOADPARTS _loadparts)
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
FhLoader::FhLoader (TDBWFRM* _mainfrm)
: mainfrm(_mainfrm)
, loadparts(LOADALL)
{
	dbw3_assert(_mainfrm);
}
/*-----------------------------------------------------------------*/
bool FhLoader::Need (LOADOPTION _part) const
{
	return ((unsigned int)(loadparts & _part)) == (unsigned int)_part;
}
/*-----------------------------------------------------------------*/
bool FhLoader::Load (LOADSTAT& _stat, LOADPARTS _loadparts)
{
	loadparts = _loadparts;
	if (mainfrm->filename.isEmpty()) { _stat = FILE_DOES_NOT_EXIST; return false; }
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
				SECTION_MAP ("properties", LoadVersion)   /* SkipSection-compatible: reads fields, discards */
				SECTION_MAP ("data", LoadData)
				SECTION_MAP ("view", LoadView)
				SECTION_MAP ("printsettings", LoadVersion)
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
	if (mainfrm->rapporthandler) mainfrm->rapporthandler->CalcRapport();

	/*  RecalcFreieSchaefte/Tritte from the loaded data -- walk the
	    einzug to find which shafts are used, ditto aufknuepfung
	    columns for the treadles.                                  */
	for (int j = 0; j < Data->MAXY1; j++) mainfrm->freieschaefte[j] = true;
	for (int i = 0; i < Data->MAXX1; i++) {
		const short s = mainfrm->einzug.feld.Get(i);
		if (s > 0 && s - 1 < Data->MAXY1)
			mainfrm->freieschaefte[s - 1] = false;
	}
	for (int i = 0; i < Data->MAXX2; i++) {
		bool used = false;
		for (int j = 0; j < Data->MAXY1 && !used; j++)
			if (mainfrm->aufknuepfung.feld.Get(i, j) > 0) used = true;
		mainfrm->freietritte[i] = !used;
	}

	mainfrm->file->Close();
	_stat = FILE_LOADED;
	return true;
}
/*-----------------------------------------------------------------*/
bool FhLoader::LoadSignatur (FfReader* _reader)
{
	dbw3_assert(_reader);
	FfToken* token = _reader->GetToken();
	const bool success = token && token->GetType() == FfSignature;
	delete token;
	return success;
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadVersion (FfReader* _reader)
{
	/*  Swallow the section. Real "version" is two fmt/ver fields;
	    we don't act on them. Used as a stand-in for "properties" /
	    "view" / "printsettings" which we also skip in this slice. */
	char* fmt = 0;
	char* ver = 0;
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_STRING("fmt", fmt)
			FIELD_MAP_STRING ("ver", ver)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
	delete[] fmt;
	delete[] ver;
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadData (FfReader* _reader)
{
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			NO_FIELDS
		BEGIN_SECTION_MAP
			_SECTION_MAP("size",   LoadDataSize)
			SECTION_MAP ("fields", LoadDataFields)
			SECTION_MAP ("palette", LoadDataPalette)
			SECTION_MAP ("hilfslinien", LoadDataHilfslinien)
			SECTION_MAP ("webstuhl",    LoadDataWebstuhl)
			DEFAULT_SECTION   /* blockmuster / bereichmuster -- skipped
			                     until the block-pattern state is
			                     ported. */
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataSize (FfReader* _reader)
{
	int maxx1 = 300;
	int maxy1 = 30;
	int maxx2 = 30;
	int maxy2 = 300;
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("maxx1", maxx1, int)
			FIELD_MAP_INT ("maxy1", maxy1, int)
			FIELD_MAP_INT ("maxx2", maxx2, int)
			FIELD_MAP_INT ("maxy2", maxy2, int)
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
		if (mainfrm->hvisible > Data->MAXY1) mainfrm->hvisible = Data->MAXY1;
		if (mainfrm->wvisible > Data->MAXX2) mainfrm->wvisible = Data->MAXX2;
		mainfrm->AllocBuffers(true);
	}
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataFields (FfReader* _reader)
{
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			NO_FIELDS
		BEGIN_SECTION_MAP
			_SECTION_MAP("einzug",       LoadDataEinzug)
			SECTION_MAP ("aufknuepfung", LoadDataAufknuepfung)
			SECTION_MAP ("trittfolge",   LoadDataTrittfolge)
			SECTION_MAP ("schussfarben", LoadDataSchussfarben)
			SECTION_MAP ("kettfarben",   LoadDataKettfarben)
			SECTION_MAP ("blatteinzug",  LoadDataBlatteinzug)
			DEFAULT_SECTION  /* fixeinzug skipped */
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataEinzug (FfReader* _reader)
{
	if (Need(LOADEINZUG)) mainfrm->einzug.feld.Read(_reader, 0);
	else                  _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataAufknuepfung (FfReader* _reader)
{
	if (Need(LOADAUFKNUEPFUNG)) mainfrm->aufknuepfung.feld.Read(_reader, 0);
	else                        _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataTrittfolge (FfReader* _reader)
{
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			NO_FIELDS
		BEGIN_SECTION_MAP
			_SECTION_MAP("trittfolge", LoadDataTrittfolgeTrittfolge)
			SECTION_MAP ("isempty",    LoadDataTrittfolgeIsEmpty)
			DEFAULT_SECTION
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataTrittfolgeTrittfolge (FfReader* _reader)
{
	if (Need(LOADTRITTFOLGE)) mainfrm->trittfolge.feld.Read(_reader, 0);
	else                      _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataTrittfolgeIsEmpty (FfReader* _reader)
{
	if (Need(LOADTRITTFOLGE)) mainfrm->trittfolge.isempty.Read(_reader, 1);
	else                      _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataSchussfarben (FfReader* _reader)
{
	if (Need(LOADSCHUSSFARBEN)) mainfrm->schussfarben.feld.Read(_reader, Data->defcolorv);
	else                        _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataKettfarben (FfReader* _reader)
{
	if (Need(LOADKETTFARBEN)) mainfrm->kettfarben.feld.Read(_reader, Data->defcolorh);
	else                      _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataBlatteinzug (FfReader* _reader)
{
	if (Need(LOADBLATTEINZUG)) mainfrm->blatteinzug.feld.Read(_reader, 0);
	else                       _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataPalette (FfReader* _reader)
{
	if (Need(LOADPALETTE)) Data->palette->Load(_reader);
	else                   _reader->SkipSection();
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataHilfslinien (FfReader* _reader)
{
	if (!Need(LOADALL)) { _reader->SkipSection(); return; }

	int   count = 0;
	char* list  = nullptr;
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
void FhLoader::LoadDataWebstuhlKlammer (FfReader* _reader, int _index)
{
	int first       = 0;
	int last        = 1;
	int repetitions = 0;
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("first",       first,       int)
			FIELD_MAP_INT ("last",        last,        int)
			FIELD_MAP_INT ("repetitions", repetitions, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
	if (_index >= 0 && _index < 9) {
		mainfrm->klammern[_index].first       = first;
		mainfrm->klammern[_index].last        = last;
		mainfrm->klammern[_index].repetitions = repetitions;
	}
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadDataWebstuhl (FfReader* _reader)
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
				if (n >= 0 && n < 9) idx = n;
			}
			if (idx >= 0) LoadDataWebstuhlKlammer(_reader, idx);
			else          _reader->SkipSection();
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
void FhLoader::LoadView (FfReader* _reader)
{
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			NO_FIELDS
		BEGIN_SECTION_MAP
			_SECTION_MAP("general",    LoadViewGeneral)
			SECTION_MAP ("gewebe",     LoadViewGewebe)
			SECTION_MAP ("einzug",     LoadViewEinzug)
			SECTION_MAP ("trittfolge", LoadViewTrittfolge)
			DEFAULT_SECTION   /* aufknuepfung, schlagpatrone, blatteinzug,
			                     kettfarben, schussfarben, pagesetup --
			                     skipped until those toggles + metadata
			                     are wired up in the port.          */
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
}
/*-----------------------------------------------------------------*/
static void setChecked (QAction* _a, bool _v)
{
	if (_a) _a->setChecked(_v);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewGeneral (FfReader* _reader)
{
	int zoom        = mainfrm->currentzoom;
	int hebung      = 1;           /* inverse of sinkingshed */
	int viewpegplan = mainfrm->ViewSchlagpatrone ? mainfrm->ViewSchlagpatrone->isChecked() : 0;
	int viewrapport = mainfrm->RappViewRapport   ? mainfrm->RappViewRapport  ->isChecked() : 0;
	int viewhlines  = mainfrm->ViewHlines        ? mainfrm->ViewHlines       ->isChecked() : 1;
	int righttoleft = mainfrm->righttoleft ? 1 : 0;
	int toptobottom = mainfrm->toptobottom ? 1 : 0;
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("zoom",        zoom,        int)
			FIELD_MAP_INT ("hebung",      hebung,      int)
			FIELD_MAP_INT ("viewpegplan", viewpegplan, int)
			FIELD_MAP_INT ("viewrapport", viewrapport, int)
			FIELD_MAP_INT ("viewhlines",  viewhlines,  int)
			FIELD_MAP_INT ("righttoleft", righttoleft, int)
			FIELD_MAP_INT ("toptobottom", toptobottom, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
	mainfrm->currentzoom = (zoom < 0 ? 0 : (zoom > 9 ? 9 : zoom));
	mainfrm->sinkingshed = (hebung == 0);
	mainfrm->righttoleft = (righttoleft != 0);
	mainfrm->toptobottom = (toptobottom != 0);
	setChecked(mainfrm->ViewSchlagpatrone, viewpegplan != 0);
	setChecked(mainfrm->RappViewRapport,   viewrapport != 0);
	setChecked(mainfrm->ViewHlines,        viewhlines  != 0);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewGewebe (FfReader* _reader)
{
	int state = 0;   /* 0=Normal 1=Farbeffekt 2=Simulation 3=None */
	int withgrid = mainfrm->fewithraster ? 1 : 0;
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("state",    state,    int)
			FIELD_MAP_INT ("withgrid", withgrid, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
	setChecked(mainfrm->GewebeNormal,     state == 0);
	setChecked(mainfrm->GewebeFarbeffekt, state == 1);
	setChecked(mainfrm->GewebeSimulation, state == 2);
	setChecked(mainfrm->GewebeNone,       state == 3);
	mainfrm->fewithraster = (withgrid != 0);
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewEinzug (FfReader* _reader)
{
	int visible  = mainfrm->ViewEinzug ? mainfrm->ViewEinzug->isChecked() : 1;
	int hvisible = mainfrm->hvisible;
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("visible",  visible,  int)
			FIELD_MAP_INT ("hvisible", hvisible, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
	setChecked(mainfrm->ViewEinzug, visible != 0);
	if (hvisible > 0) mainfrm->hvisible = hvisible;
}
/*-----------------------------------------------------------------*/
void FhLoader::LoadViewTrittfolge (FfReader* _reader)
{
	int visible  = mainfrm->ViewTrittfolge ? mainfrm->ViewTrittfolge->isChecked() : 1;
	int wvisible = mainfrm->wvisible;
	BEGIN_LOAD_MAP
		BEGIN_FIELD_MAP
			_FIELD_MAP_INT("visible",  visible,  int)
			FIELD_MAP_INT ("wvisible", wvisible, int)
			DEFAULT_FIELD
		BEGIN_SECTION_MAP
			NO_SECTIONS
		BEGIN_DEFAULT_MAP
	END_LOAD_MAP
	setChecked(mainfrm->ViewTrittfolge, visible != 0);
	if (wvisible > 0) mainfrm->wvisible = wvisible;
}
/*-----------------------------------------------------------------*/
