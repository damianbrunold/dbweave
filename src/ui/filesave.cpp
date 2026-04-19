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
#include "properties.h"
#include "blockmuster.h"

#include <vector>

/*  @dbw3:file format version tag -- matches legacy FILEFORMATVERSION
    for 3.7+ files. "0001" is the older style; we always emit "0002". */
#define FILEFORMATVERSION "0002"
#define DBWEAVE_PORT_VERSION "0.1.0 (Qt 6 port)"

bool TDBWFRM::Save()
{
    if (filename.isEmpty())
        return false;

    try {
        const QByteArray fn = filename.toLocal8Bit();
        if (!file->IsOpen()) {
            file->Open(fn.constData(), FfOpenRead | FfOpenWrite | FfOpenOverwrite);
        } else {
            file->SeekBegin();
        }
        if (!file->IsOpen())
            return false;

        FfWriter writer;
        writer.BreakFields(true);
        writer.Assign(file);

        writer.WriteSignature();

        writer.BeginSection("version", "Dateiformat und Applikationsversion");
        writer.WriteField("fmt", FILEFORMATVERSION);
        writer.WriteField("ver", DBWEAVE_PORT_VERSION);
        writer.EndSection();

        if (Data->properties)
            Data->properties->Save(&writer);

        writer.BeginSection("data", "Daten");

        writer.BeginSection("size");
        writer.WriteFieldInt("maxx1", Data->MAXX1);
        writer.WriteFieldInt("maxy1", Data->MAXY1);
        writer.WriteFieldInt("maxx2", Data->MAXX2);
        writer.WriteFieldInt("maxy2", Data->MAXY2);
        writer.EndSection();

        writer.BeginSection("fields");
        einzug.feld.Write("einzug", &writer);
        aufknuepfung.feld.Write("aufknuepfung", &writer);
        writer.BeginSection("trittfolge");
        trittfolge.feld.Write("trittfolge", &writer);
        /*  The in-memory isempty cache is gone; derive the vector
            on the fly from trittfolge.feld so the file format stays
            byte-identical and older DB-WEAVE binaries can still
            read newly-written files.                             */
        {
            FeldVectorBool isempty(Data->MAXY2, true);
            for (int j = 0; j < Data->MAXY2; j++)
                isempty.Set(j, IsEmptyTrittfolge(j));
            isempty.Write("isempty", &writer);
        }
        writer.EndSection();
        kettfarben.feld.Write("kettfarben", &writer);
        schussfarben.feld.Write("schussfarben", &writer);
        blatteinzug.feld.Write("blatteinzug", &writer);
        /*  fixeinzug: MAXX1 shorts -- written even when the table
            was never filled, so File > New state matches the legacy
            default (all zeros, firstfree=1, fixsize=0).            */
        writer.BeginSection("fixeinzug");
        {
            const int bytes = int(sizeof(short)) * Data->MAXX1;
            /*  If the user never opened the dialog we still allocate
                a zeroed buffer on the fly so the file always has the
                section.                                              */
            if (fixeinzug) {
                writer.WriteFieldBinary("fixeinzug", fixeinzug, bytes);
            } else {
                std::vector<short> zero(Data->MAXX1, 0);
                writer.WriteFieldBinary("fixeinzug", zero.data(), bytes);
            }
            writer.WriteFieldInt("fixsize", fixsize);
            writer.WriteFieldInt("firstfree", firstfree);
        }
        writer.EndSection();
        writer.EndSection();

        if (Data->palette)
            Data->palette->Save(&writer, /*format37=*/true);

        /*  Webstuhl: just the 9 klammer brace entries. The legacy
            also wrote STRGFRM weave_position / last_position /
            schussselected etc.; those belong to the loom-control
            window which isn't ported yet and are skipped here.    */
        writer.BeginSection("webstuhl");
        for (int i = 0; i < 9; i++) {
            const QByteArray name = QStringLiteral("klammer%1").arg(i).toLatin1();
            writer.BeginSection(name.constData());
            writer.WriteFieldInt("first", klammern[i].first);
            writer.WriteFieldInt("last", klammern[i].last);
            writer.WriteFieldInt("repetitions", klammern[i].repetitions);
            writer.EndSection();
        }
        writer.BeginSection("current");
        writer.WriteFieldInt("position", weave_position);
        writer.WriteFieldInt("klammer", weave_klammer);
        writer.WriteFieldInt("repetition", weave_repetition);
        writer.EndSection();
        writer.BeginSection("last");
        writer.WriteFieldInt("position", last_position);
        writer.WriteFieldInt("klammer", last_klammer);
        writer.WriteFieldInt("repetition", last_repetition);
        writer.EndSection();
        writer.BeginSection("divers");
        writer.WriteFieldInt("schussselected", schussselected ? 1 : 0);
        writer.WriteFieldInt("scrolly", scrolly_weben);
        writer.WriteFieldInt("firstschuss", firstschuss ? 1 : 0);
        writer.WriteFieldInt("weaving", weaving ? 1 : 0);
        writer.EndSection();
        writer.EndSection();

        writer.BeginSection("blockmuster");
        for (int i = 0; i < 10; i++) {
            const QByteArray name = QStringLiteral("bindung%1").arg(i).toLatin1();
            writer.WriteFieldBinary(name.constData(),
                                    const_cast<char*>(blockmuster[i].Data()),
                                    blockmuster[i].DataSize());
        }
        writer.WriteFieldInt("einzugz", einzugZ ? 1 : 0);
        writer.WriteFieldInt("trittfolgez", trittfolgeZ ? 1 : 0);
        writer.EndSection();

        writer.BeginSection("bereichmuster");
        for (int i = 0; i < 10; i++) {
            const QByteArray name = QStringLiteral("bindung%1").arg(i).toLatin1();
            writer.WriteFieldBinary(name.constData(),
                                    const_cast<char*>(bereichmuster[i].Data()),
                                    bereichmuster[i].DataSize());
        }
        writer.EndSection();

        writer.BeginSection("hilfslinien");
        writer.WriteFieldInt("count", hlines.GetCount());
        writer.WriteFieldBinary("list", hlines.Data(), hlines.DataSize());
        writer.EndSection();

        writer.EndSection(); /* data */

        /*  View state -- full round-trip matching legacy SaveView in
            filesave.cpp. Covers the symbols (viewtype) per field, the
            einzug/trittfolge rearrangement style radio group, pegplan
            state, orientation, fabric-view mode, and the page-setup
            margins / header / footer.                              */
        auto styleIndex = [](QAction* const* acts, int n, int fallback) {
            for (int i = 0; i < n; i++)
                if (acts[i] && acts[i]->isChecked())
                    return i;
            return fallback;
        };
        QAction* ezActs[8] = { EzFixiert,  EzMinimalZ, EzMinimalS, EzGeradeZ,
                               EzGeradeS,  EzBelassen, EzChorig2,  EzChorig3 };
        QAction* tfActs[6] = { TfBelassen, TfMinimalZ,   TfMinimalS,
                               TfGeradeZ,  TfGeradeS,    TfGesprungen };

        writer.BeginSection("view", "Ansicht");
        writer.BeginSection("general");
        writer.WriteFieldInt("zoom", currentzoom);
        writer.WriteFieldInt("hebung", sinkingshed ? 0 : 1);
        writer.WriteFieldInt("color", Data ? int(Data->color) : 1);
        writer.WriteFieldInt("viewpalette",
                             (ViewFarbpalette && ViewFarbpalette->isChecked()) ? 1 : 0);
        writer.WriteFieldInt("viewpegplan",
                             (ViewSchlagpatrone && ViewSchlagpatrone->isChecked()) ? 1 : 0);
        writer.WriteFieldInt("viewrapport",
                             (RappViewRapport && RappViewRapport->isChecked()) ? 1 : 0);
        writer.WriteFieldInt("viewhlines", (ViewHlines && ViewHlines->isChecked()) ? 1 : 0);
        writer.WriteFieldInt("righttoleft", righttoleft ? 1 : 0);
        writer.WriteFieldInt("toptobottom", toptobottom ? 1 : 0);
        writer.EndSection();

        writer.BeginSection("einzug");
        writer.WriteFieldInt("visible", (ViewEinzug && ViewEinzug->isChecked()) ? 1 : 0);
        writer.WriteFieldInt("down", einzugunten ? 1 : 0);
        writer.WriteFieldInt("viewtype", int(einzug.darstellung));
        writer.WriteFieldInt("hvisible", hvisible);
        writer.WriteFieldInt("style", styleIndex(ezActs, 8, 1));
        writer.EndSection();

        writer.BeginSection("aufknuepfung");
        writer.WriteFieldInt("viewtype", int(aufknuepfung.darstellung));
        writer.EndSection();

        writer.BeginSection("trittfolge");
        writer.WriteFieldInt("visible", (ViewTrittfolge && ViewTrittfolge->isChecked()) ? 1 : 0);
        writer.WriteFieldInt("viewtype", int(trittfolge.darstellung));
        writer.WriteFieldInt("single", trittfolge.einzeltritt ? 1 : 0);
        writer.WriteFieldInt("wvisible", wvisible);
        writer.WriteFieldInt("style", styleIndex(tfActs, 6, 1));
        writer.EndSection();

        writer.BeginSection("schlagpatrone");
        writer.WriteFieldInt("viewtype", int(schlagpatronendarstellung));
        writer.EndSection();

        writer.BeginSection("gewebe");
        int g = 0;
        if (GewebeFarbeffekt && GewebeFarbeffekt->isChecked())
            g = 1;
        else if (GewebeSimulation && GewebeSimulation->isChecked())
            g = 2;
        else if (GewebeNone && GewebeNone->isChecked())
            g = 3;
        writer.WriteFieldInt("state", g);
        writer.WriteFieldInt("withgrid", fewithraster ? 1 : 0);
        writer.EndSection();

        writer.BeginSection("blatteinzug");
        writer.WriteFieldInt("visible",
                             (ViewBlatteinzug && ViewBlatteinzug->isChecked()) ? 1 : 0);
        writer.EndSection();

        const int viewFarbe = (ViewFarbe && ViewFarbe->isChecked()) ? 1 : 0;
        writer.BeginSection("kettfarben");
        writer.WriteFieldInt("visible", viewFarbe);
        writer.EndSection();
        writer.BeginSection("schussfarben");
        writer.WriteFieldInt("visible", viewFarbe);
        writer.EndSection();

        writer.BeginSection("pagesetup");
        writer.WriteFieldInt("topmargin", borders.range.top);
        writer.WriteFieldInt("bottommargin", borders.range.bottom);
        writer.WriteFieldInt("leftmargin", borders.range.left);
        writer.WriteFieldInt("rightmargin", borders.range.right);
        writer.WriteFieldInt("headerheight", header.height);
        writer.WriteField("headertext", header.text.toUtf8().constData());
        writer.WriteFieldInt("footerheight", footer.height);
        writer.WriteField("footertext", footer.text.toUtf8().constData());
        writer.EndSection();
        writer.EndSection(); /* view */

        writer.BeginSection("printsettings", "Druckeinstellungen");
        writer.BeginSection("printrange");
        writer.WriteFieldInt("kettevon", printkette.a);
        writer.WriteFieldInt("kettebis", printkette.b);
        writer.WriteFieldInt("schuessevon", printschuesse.a);
        writer.WriteFieldInt("schuessebis", printschuesse.b);
        writer.WriteFieldInt("schaeftevon", printschaefte.a);
        writer.WriteFieldInt("schaeftebis", printschaefte.b);
        writer.WriteFieldInt("trittevon", printtritte.a);
        writer.WriteFieldInt("trittebis", printtritte.b);
        writer.EndSection();
        writer.EndSection();

        file->SetEndOfFile();
        file->Close();
        return true;

    } catch (...) {
        if (file->IsOpen())
            file->Close();
        return false;
    }
}
