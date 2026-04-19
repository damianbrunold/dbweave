/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  WIF (Weaving Information Format) exporter — verbatim port of
    legacy/export.cpp. WIF is a plain-text INI-style format; the
    writer emits sections (WIF, CONTENTS, COLOR PALETTE, TEXT,
    WEAVING, WARP, WEFT, NOTES, COLOR TABLE, THREADING, WARP COLORS,
    TREADLING or LIFTPLAN + TIEUP, WEFT COLORS, PRIVATE DBWEAVE
    BLATTEINZUG).                                                  */

#include "mainwindow.h"
#include "datamodule.h"
#include "palette.h"
#include "properties.h"
#include "language.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextStream>

/*-----------------------------------------------------------------*/
class WifWriter
{
public:
    explicit WifWriter(TDBWFRM* _frm)
        : frm(_frm)
    {
    }

    bool write(const QString& _filename);

private:
    TDBWFRM* frm = nullptr;
    QFile file;
    QTextStream out;

    int firstschaft = 0, lastschaft = 0;
    int firsttritt = 0, lasttritt = 0;
    int firstschussfaden = 0, lastschussfaden = 0;
    int firstkettfaden = 0, lastkettfaden = 0;

    void calcDimensions();

    void section(const char* _name)
    {
        out << '[' << _name << "]\n";
    }
    void endsec()
    {
        out << '\n';
    }
    void entry(const char* _k, const char* _v)
    {
        out << _k << '=' << _v << '\n';
    }
    void entry(const char* _k, const QString& _v)
    {
        out << _k << '=' << _v << '\n';
    }
    void entry(const char* _k, int _v)
    {
        out << _k << '=' << _v << '\n';
    }
    void entry(int _i, const QString& _v)
    {
        out << _i << '=' << _v << '\n';
    }
    void entry(int _i, int _v)
    {
        out << _i << '=' << _v << '\n';
    }

    void writeColorpalette();
    void writeText();
    void writeWeaving();
    void writeWarp();
    void writeWeft();
    void writeNotes();
    void writeTieup();
    void writeColortable();
    void writeThreading();
    void writeWarpcolors();
    void writeTreadling();
    void writeLiftplan();
    void writeWeftcolors();
    void writeBlatteinzug();
};

/*-----------------------------------------------------------------*/
bool WifWriter::write(const QString& _filename)
{
    file.setFileName(_filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return false;
    out.setDevice(&file);

    calcDimensions();

    section("WIF");
    entry("Version", "1.1");
    entry("Date", "April 20, 1997");
    entry("Developers", "wif@mhsoft.com");
    entry("Source Program", "DB-WEAVE");
    entry("Source Version", "Qt 6 port");
    endsec();

    const bool schlagpatrone = frm->ViewSchlagpatrone && frm->ViewSchlagpatrone->isChecked();
    section("CONTENTS");
    entry("COLOR PALETTE", "yes");
    entry("TEXT", "yes");
    entry("WEAVING", "yes");
    entry("WARP", "yes");
    entry("WEFT", "yes");
    entry("NOTES", "yes");
    if (!schlagpatrone)
        entry("TIEUP", "yes");
    entry("COLOR TABLE", "yes");
    entry("THREADING", "yes");
    entry("WARP COLORS", "yes");
    if (schlagpatrone)
        entry("LIFTPLAN", "yes");
    else
        entry("TREADLING", "yes");
    entry("WEFT COLORS", "yes");
    endsec();

    writeColorpalette();
    writeText();
    writeWeaving();
    writeWarp();
    writeWeft();
    writeNotes();
    writeColortable();
    writeThreading();
    writeWarpcolors();
    if (schlagpatrone)
        writeLiftplan();
    else {
        writeTreadling();
        writeTieup();
    }
    writeWeftcolors();
    writeBlatteinzug();

    out.flush();
    file.close();
    return true;
}

/*-----------------------------------------------------------------*/
void WifWriter::calcDimensions()
{
    firstschaft = lastschaft = 0;
    for (int i = 0; i < Data->MAXY1; i++)
        if (!frm->freieschaefte[i]) {
            firstschaft = i;
            break;
        }
    for (int i = Data->MAXY1 - 1; i >= 0; i--)
        if (!frm->freieschaefte[i]) {
            lastschaft = i;
            break;
        }

    firsttritt = lasttritt = 0;
    for (int i = 0; i < Data->MAXX2; i++)
        if (!frm->freietritte[i]) {
            firsttritt = i;
            break;
        }
    for (int i = Data->MAXX2 - 1; i >= 0; i--)
        if (!frm->freietritte[i]) {
            lasttritt = i;
            break;
        }

    firstkettfaden = frm->kette.a;
    lastkettfaden = frm->kette.b;
    firstschussfaden = frm->schuesse.a;
    lastschussfaden = frm->schuesse.b;
}

/*-----------------------------------------------------------------*/
void WifWriter::writeColorpalette()
{
    section("COLOR PALETTE");
    entry("Entries", MAX_PAL_ENTRY);
    entry("Range", "0,255");
    endsec();
}

void WifWriter::writeText()
{
    section("TEXT");
    entry("Title", frm->filename);
    entry("Author", QString::fromUtf8(Data->properties ? Data->properties->Author() : ""));
    endsec();
}

void WifWriter::writeWeaving()
{
    section("WEAVING");
    entry("Shafts", lastschaft - firstschaft + 1);
    entry("Treadles", lasttritt - firsttritt + 1);
    entry("Rising Shed", frm->sinkingshed ? "no" : "yes");
    endsec();
}

void WifWriter::writeWarp()
{
    section("WARP");
    entry("Threads", lastkettfaden - firstkettfaden + 1);
    entry("Color", int(DEFAULT_COLORV));
    endsec();
}

void WifWriter::writeWeft()
{
    section("WEFT");
    entry("Threads", lastschussfaden - firstschussfaden + 1);
    entry("Color", int(DEFAULT_COLORH));
    endsec();
}

void WifWriter::writeNotes()
{
    section("NOTES");
    entry("1", QString::fromUtf8(Data->properties ? Data->properties->Remarks() : ""));
    endsec();
}

void WifWriter::writeTieup()
{
    section("TIEUP");
    for (int i = firsttritt; i <= lasttritt; i++) {
        QString line;
        for (int j = firstschaft; j <= lastschaft; j++)
            if (frm->aufknuepfung.feld.Get(i, j) > 0)
                line += QStringLiteral(",%1").arg(j - firstschaft + 1);
        if (!line.isEmpty())
            entry(i - firsttritt + 1, line.mid(1));
    }
    endsec();
}

void WifWriter::writeColortable()
{
    section("COLOR TABLE");
    for (int i = 0; i < MAX_PAL_ENTRY; i++) {
        const COLORREF col = Data->palette->GetColor(i);
        entry(
            i + 1,
            QStringLiteral("%1,%2,%3").arg(GetRValue(col)).arg(GetGValue(col)).arg(GetBValue(col)));
    }
    endsec();
}

void WifWriter::writeThreading()
{
    section("THREADING");
    for (int i = firstkettfaden; i <= lastkettfaden; i++)
        entry(i - firstkettfaden + 1, int(frm->einzug.feld.Get(i)));
    endsec();
}

void WifWriter::writeWarpcolors()
{
    section("WARP COLORS");
    for (int i = firstkettfaden; i <= lastkettfaden; i++)
        entry(i - firstkettfaden + 1, int(frm->kettfarben.feld.Get(i)) + 1);
    endsec();
}

void WifWriter::writeTreadling()
{
    section("TREADLING");
    for (int j = firstschussfaden; j <= lastschussfaden; j++) {
        QString tritte;
        for (int i = firsttritt; i <= lasttritt; i++)
            if (frm->trittfolge.feld.Get(i, j) > 0)
                tritte += QStringLiteral(",%1").arg(i + 1);
        entry(j - firstschussfaden + 1, tritte.isEmpty() ? QString() : tritte.mid(1));
    }
    endsec();
}

void WifWriter::writeLiftplan()
{
    section("LIFTPLAN");
    for (int j = firstschussfaden; j <= lastschussfaden; j++) {
        QString tritte;
        for (int i = firsttritt; i <= lasttritt; i++)
            if (frm->trittfolge.feld.Get(i, j) > 0)
                tritte += QStringLiteral(",%1").arg(i - firsttritt + 1);
        entry(j - firstschussfaden + 1, tritte.isEmpty() ? QString() : tritte.mid(1));
    }
    endsec();
}

void WifWriter::writeWeftcolors()
{
    section("WEFT COLORS");
    for (int i = firstschussfaden; i <= lastschussfaden; i++)
        entry(i - firstschussfaden + 1, int(frm->schussfarben.feld.Get(i)) + 1);
    endsec();
}

void WifWriter::writeBlatteinzug()
{
    section("PRIVATE DBWEAVE BLATTEINZUG");
    for (int i = firstkettfaden; i <= lastkettfaden; i++)
        entry(i - firstkettfaden + 1, int(frm->blatteinzug.feld.Get(i)));
    endsec();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::DateiExportieren(const QString& _filename)
{
    WifWriter writer(this);
    writer.write(_filename);
}

/*-----------------------------------------------------------------*/
/*  TDBWFRM::DateiExportClick — pops a Save-as dialog with BMP /
    WIF / PNG filter options and dispatches based on suffix, matching
    legacy export.cpp semantics (minus the dbw-3.5 "old format"
    branch, which is essentially a legacy-Save-As and will land when
    the file-I/O slice rounds out).                                */
void TDBWFRM::DateiExportClick()
{
    const QString fn = QFileDialog::getSaveFileName(
        this, LANG_STR("Export", "Exportieren"), QString(),
        LANG_STR("Bitmap (*.bmp);;PNG (*.png);;WIF (*.wif)",
                 "Bitmap (*.bmp);;PNG (*.png);;WIF (*.wif)"));
    if (fn.isEmpty())
        return;

    const QString suffix = QFileInfo(fn).suffix().toLower();
    if (suffix == QStringLiteral("wif")) {
        DateiExportieren(fn);
    } else {
        DoExportBitmap(fn);
    }
}
