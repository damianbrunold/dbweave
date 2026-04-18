/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Writer half of the round-trip. Scope matches the loader's scope:
      signature
      version
      data/size
      data/fields (einzug, aufknuepfung, trittfolge+isempty,
                   kettfarben, schussfarben, blatteinzug)
      data/palette
      data/hilfslinien (count + binary list)

    The legacy also writes webstuhl / blockmuster / bereichmuster /
    fixeinzug / view / printsettings / properties; those are skipped
    here because the port does not yet carry their state. A save-
    then-load round-trip is therefore non-destructive only for the
    sections above; reading a legacy-written file still works
    because our loader skips the unmapped sections via DEFAULT_SECTION.
*/

#include "mainwindow.h"
#include "datamodule.h"
#include "fileformat.h"
#include "palette.h"
#include "hilfslinien.h"

/*  @dbw3:file format version tag -- matches legacy FILEFORMATVERSION
    for 3.7+ files. "0001" is the older style; we always emit "0002". */
#define FILEFORMATVERSION "0002"
#define DBWEAVE_PORT_VERSION "0.1.0 (Qt 6 port)"

bool __fastcall TDBWFRM::Save ()
{
	if (filename.isEmpty()) return false;

	try {
		const QByteArray fn = filename.toLocal8Bit();
		if (!file->IsOpen()) {
			file->Open(fn.constData(), FfOpenRead | FfOpenWrite | FfOpenOverwrite);
		} else {
			file->SeekBegin();
		}
		if (!file->IsOpen()) return false;

		FfWriter writer;
		writer.BreakFields(true);
		writer.Assign(file);

		writer.WriteSignature();

		writer.BeginSection("version", "Dateiformat und Applikationsversion");
		writer.WriteField("fmt", FILEFORMATVERSION);
		writer.WriteField("ver", DBWEAVE_PORT_VERSION);
		writer.EndSection();

		writer.BeginSection("data", "Daten");

		writer.BeginSection("size");
		writer.WriteFieldInt("maxx1", Data->MAXX1);
		writer.WriteFieldInt("maxy1", Data->MAXY1);
		writer.WriteFieldInt("maxx2", Data->MAXX2);
		writer.WriteFieldInt("maxy2", Data->MAXY2);
		writer.EndSection();

		writer.BeginSection("fields");
		einzug.feld.Write       ("einzug",       &writer);
		aufknuepfung.feld.Write ("aufknuepfung", &writer);
		writer.BeginSection("trittfolge");
			trittfolge.feld.Write    ("trittfolge", &writer);
			trittfolge.isempty.Write ("isempty",    &writer);
		writer.EndSection();
		kettfarben.feld.Write   ("kettfarben",   &writer);
		schussfarben.feld.Write ("schussfarben", &writer);
		blatteinzug.feld.Write  ("blatteinzug",  &writer);
		writer.EndSection();

		if (Data->palette) Data->palette->Save(&writer, /*format37=*/true);

		writer.BeginSection("hilfslinien");
		writer.WriteFieldInt   ("count", hlines.GetCount());
		writer.WriteFieldBinary("list",  hlines.Data(), hlines.DataSize());
		writer.EndSection();

		writer.EndSection();   /* data */

		file->SetEndOfFile();
		file->Close();
		return true;

	} catch (...) {
		if (file->IsOpen()) file->Close();
		return false;
	}
}
