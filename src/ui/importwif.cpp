/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  WIF importer — port of legacy/import.cpp, extended to also read
    palette, warp/weft colours, notes, blatteinzug, sinking/rising
    shed, and liftplan so the export/import pair can round-trip. */

#include "mainwindow.h"
#include "datamodule.h"
#include "cursor.h"
#include "undoredo.h"
#include "rapport.h"
#include "palette.h"
#include "properties.h"
#include "language.h"

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QVector>

/*-----------------------------------------------------------------*/
namespace
{

struct WifWeaving {
    int shafts = 0;
    int treadles = 0;
    bool risingshed = true;
    bool risingshedSeen = false;
};
struct WifWeftWarp {
    int threads = 0;
    int color = 1;
};
struct WifColorPalette {
    int entries = 0;
    int beginrange = 0;
    int endrange = 65535;
};
struct WifColor {
    int r = 0, g = 0, b = 0;
    bool set = false;
};
struct WifThreadingEntry {
    QVector<int> harnesses;
};
struct WifTieupEntry {
    QVector<int> harnesses;
};
struct WifTreadlingEntry {
    QVector<int> treadles;
};
struct WifText {
    QString title;
    QString author;
    bool haveTitle = false;
    bool haveAuthor = false;
};

class WifReader
{
public:
    explicit WifReader(TDBWFRM* _frm)
        : frm(_frm)
    {
    }
    bool read(const QString& _filename);

private:
    TDBWFRM* frm = nullptr;
    QString filename;

    WifWeaving weaving;
    WifWeftWarp weft;
    WifWeftWarp warp;
    WifColorPalette colorpalette;
    WifText text;
    QString note1;
    bool haveNote1 = false;
    bool haveLiftplan = false;

    QVector<WifColor> colortable;
    QVector<WifThreadingEntry> threading;
    QVector<WifTieupEntry> tieup;
    QVector<WifTreadlingEntry> treadling;
    QVector<int> warpColors; /* 1-based palette indices, -1 == unset */
    QVector<int> weftColors;
    QVector<int> blatteinzug;

    void firstPass();
    void secondPass();
    void thirdPass();
    void copyData();

public:
    static QString nextToken(QTextStream& _strm);
    static QString fieldName(const QString& _line);
    static QString fieldValue(const QString& _line);
    static int fieldInt(const QString& _line);
    static bool fieldBool(const QString& _line);
    static QVector<int> splitInts(const QString& _line);
};

/*-----------------------------------------------------------------*/
QString WifReader::nextToken(QTextStream& _strm)
{
    while (!_strm.atEnd()) {
        const QString line = _strm.readLine();
        const QString trim = line.trimmed();
        if (trim.isEmpty())
            continue;
        if (trim.startsWith('[')) {
            const int end = trim.indexOf(']');
            if (end < 0)
                return QString();
            return trim.left(end + 1).toLower();
        }
    }
    return QString();
}

QString WifReader::fieldName(const QString& _line)
{
    const int eq = _line.indexOf('=');
    if (eq < 0)
        return QString();
    return _line.left(eq).trimmed().toLower();
}

QString WifReader::fieldValue(const QString& _line)
{
    const int eq = _line.indexOf('=');
    if (eq < 0)
        return QString();
    return _line.mid(eq + 1).trimmed();
}

int WifReader::fieldInt(const QString& _line)
{
    return fieldValue(_line).toInt();
}

bool WifReader::fieldBool(const QString& _line)
{
    const QString v = fieldValue(_line).toUpper();
    return v == QStringLiteral("TRUE") || v == QStringLiteral("YES") || v == QStringLiteral("1")
           || v == QStringLiteral("ON");
}

QVector<int> WifReader::splitInts(const QString& _line)
{
    QVector<int> out;
    const QStringList parts = _line.split(',', Qt::SkipEmptyParts);
    for (const QString& p : parts)
        out.append(p.trimmed().toInt());
    return out;
}

/*-----------------------------------------------------------------*/
/*  Each read<Section> helper scans until it sees the next bracketed
    section header, returning that header. */
static QString readWeaving(QTextStream& _s, WifWeaving& _w)
{
    while (!_s.atEnd()) {
        const QString line = _s.readLine();
        const QString t = line.trimmed();
        if (t.startsWith('['))
            return t.toLower();
        const QString f = WifReader::fieldName(line);
        if (f == "shafts")
            _w.shafts = WifReader::fieldInt(line);
        else if (f == "treadles")
            _w.treadles = WifReader::fieldInt(line);
        else if (f == "rising shed") {
            _w.risingshed = WifReader::fieldBool(line);
            _w.risingshedSeen = true;
        }
    }
    return QString();
}

static QString readWeftWarp(QTextStream& _s, WifWeftWarp& _w)
{
    while (!_s.atEnd()) {
        const QString line = _s.readLine();
        const QString t = line.trimmed();
        if (t.startsWith('['))
            return t.toLower();
        const QString f = WifReader::fieldName(line);
        if (f == "color")
            _w.color = WifReader::fieldInt(line);
        else if (f == "threads")
            _w.threads = WifReader::fieldInt(line);
    }
    return QString();
}

static QString readColorPalette(QTextStream& _s, WifColorPalette& _p)
{
    while (!_s.atEnd()) {
        const QString line = _s.readLine();
        const QString t = line.trimmed();
        if (t.startsWith('['))
            return t.toLower();
        const QString f = WifReader::fieldName(line);
        if (f == "entries")
            _p.entries = WifReader::fieldInt(line);
        else if (f == "range") {
            const QVector<int> r = WifReader::splitInts(WifReader::fieldValue(line));
            if (r.size() >= 2) {
                _p.beginrange = r[0];
                _p.endrange = r[1];
            }
        }
    }
    return QString();
}

static QString readText(QTextStream& _s, WifText& _t)
{
    while (!_s.atEnd()) {
        const QString line = _s.readLine();
        const QString t = line.trimmed();
        if (t.startsWith('['))
            return t.toLower();
        const QString f = WifReader::fieldName(line);
        if (f == "title") {
            _t.title = WifReader::fieldValue(line);
            _t.haveTitle = true;
        } else if (f == "author") {
            _t.author = WifReader::fieldValue(line);
            _t.haveAuthor = true;
        }
    }
    return QString();
}

static QString readNotes(QTextStream& _s, QString& _note1, bool& _have)
{
    while (!_s.atEnd()) {
        const QString line = _s.readLine();
        const QString t = line.trimmed();
        if (t.startsWith('['))
            return t.toLower();
        const QString f = WifReader::fieldName(line);
        if (f == "1") {
            _note1 = WifReader::fieldValue(line);
            _have = true;
        }
    }
    return QString();
}

/*  Indexed colour table: "<i>=r,g,b". */
static QString readColorTable(QTextStream& _s, QVector<WifColor>& _colors)
{
    while (!_s.atEnd()) {
        const QString line = _s.readLine();
        const QString t = line.trimmed();
        if (t.startsWith('['))
            return t.toLower();
        const int idx = WifReader::fieldName(line).toInt() - 1;
        if (idx < 0)
            continue;
        if (idx >= _colors.size())
            _colors.resize(idx + 1);
        const QVector<int> v = WifReader::splitInts(WifReader::fieldValue(line));
        if (v.size() >= 3) {
            _colors[idx].r = v[0];
            _colors[idx].g = v[1];
            _colors[idx].b = v[2];
            _colors[idx].set = true;
        }
    }
    return QString();
}

/*  Threading / treadling / tieup: "<index>=<csv>". */
template <typename Entry, typename Assign>
static QString readIndexed(QTextStream& _s, QVector<Entry>& _entries, Assign _assign)
{
    while (!_s.atEnd()) {
        const QString line = _s.readLine();
        const QString t = line.trimmed();
        if (t.startsWith('['))
            return t.toLower();
        const int n = WifReader::fieldName(line).toInt() - 1;
        if (n < 0 || n >= _entries.size())
            continue;
        Entry entry;
        _assign(entry, WifReader::splitInts(WifReader::fieldValue(line)));
        _entries[n] = entry;
    }
    return QString();
}

/*  "<index>=<int>". Negative / out-of-range indices are silently
    dropped so a larger WIF does not stomp unrelated array slots. */
static QString readIndexedInt(QTextStream& _s, QVector<int>& _out)
{
    while (!_s.atEnd()) {
        const QString line = _s.readLine();
        const QString t = line.trimmed();
        if (t.startsWith('['))
            return t.toLower();
        const int n = WifReader::fieldName(line).toInt() - 1;
        if (n < 0)
            continue;
        if (n >= _out.size())
            _out.resize(n + 1, -1);
        _out[n] = WifReader::fieldValue(line).toInt();
    }
    return QString();
}

/*-----------------------------------------------------------------*/
void WifReader::firstPass()
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream s(&f);

    QString tok = nextToken(s);
    while (!tok.isEmpty()) {
        if (tok == "[weaving]")
            tok = readWeaving(s, weaving);
        else if (tok == "[color palette]")
            tok = readColorPalette(s, colorpalette);
        else
            tok = nextToken(s);
    }
}

void WifReader::secondPass()
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream s(&f);

    QString tok = nextToken(s);
    while (!tok.isEmpty()) {
        if (tok == "[weft]")
            tok = readWeftWarp(s, weft);
        else if (tok == "[warp]")
            tok = readWeftWarp(s, warp);
        else if (tok == "[color table]")
            tok = readColorTable(s, colortable);
        else if (tok == "[text]")
            tok = readText(s, text);
        else if (tok == "[notes]")
            tok = readNotes(s, note1, haveNote1);
        else
            tok = nextToken(s);
    }
}

void WifReader::thirdPass()
{
    threading.resize(warp.threads);
    treadling.resize(weft.threads);
    tieup.resize(weaving.treadles);
    warpColors.fill(-1, warp.threads);
    weftColors.fill(-1, weft.threads);
    blatteinzug.fill(-1, warp.threads);

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream s(&f);

    QString tok = nextToken(s);
    while (!tok.isEmpty()) {
        if (tok == "[threading]") {
            tok = readIndexed(s, threading,
                              [](WifThreadingEntry& e, const QVector<int>& v) { e.harnesses = v; });
        } else if (tok == "[treadling]") {
            tok = readIndexed(s, treadling,
                              [](WifTreadlingEntry& e, const QVector<int>& v) { e.treadles = v; });
        } else if (tok == "[liftplan]") {
            /*  LIFTPLAN rows enumerate shafts lifted for each weft pick.
                In DB-WEAVE terms that's "direct" treadling: one virtual
                treadle per shaft, with an identity tieup. We record the
                raised shafts as treadle indices so copyData() can
                install them directly into trittfolge. */
            haveLiftplan = true;
            tok = readIndexed(s, treadling,
                              [](WifTreadlingEntry& e, const QVector<int>& v) { e.treadles = v; });
        } else if (tok == "[tieup]") {
            tok = readIndexed(s, tieup,
                              [](WifTieupEntry& e, const QVector<int>& v) { e.harnesses = v; });
        } else if (tok == "[warp colors]") {
            tok = readIndexedInt(s, warpColors);
        } else if (tok == "[weft colors]") {
            tok = readIndexedInt(s, weftColors);
        } else if (tok == "[private dbweave blatteinzug]") {
            tok = readIndexedInt(s, blatteinzug);
        } else {
            tok = nextToken(s);
        }
    }
}

/*-----------------------------------------------------------------*/
void WifReader::copyData()
{
    /*  Palette (COLOR TABLE). */
    for (int i = 0; i < colortable.size() && i < MAX_PAL_ENTRY; i++) {
        if (!colortable[i].set)
            continue;
        Data->palette->SetColor(i, RGB(colortable[i].r, colortable[i].g, colortable[i].b));
    }

    /*  Threading / einzug. */
    frm->einzug.Clear();
    for (int i = 0; i < threading.size(); i++) {
        if (threading[i].harnesses.isEmpty())
            continue;
        if (i >= Data->MAXX1)
            break;
        frm->einzug.feld.Set(i, short(threading[i].harnesses.first()));
    }

    /*  Tie-up. LIFTPLAN mode uses an identity tie-up. */
    frm->aufknuepfung.Clear();
    if (haveLiftplan) {
        const int n = qMin(Data->MAXX2, Data->MAXY1);
        for (int i = 0; i < n; i++)
            frm->aufknuepfung.feld.Set(i, i, 1);
    } else {
        for (int i = 0; i < tieup.size(); i++) {
            const QVector<int>& hs = tieup[i].harnesses;
            for (int v : hs) {
                if (i < Data->MAXX2 && v - 1 >= 0 && v - 1 < Data->MAXY1)
                    frm->aufknuepfung.feld.Set(i, v - 1, 1);
            }
        }
    }

    /*  Treadling / trittfolge. */
    frm->trittfolge.Clear();
    for (int j = 0; j < treadling.size(); j++) {
        const QVector<int>& ts = treadling[j].treadles;
        for (int v : ts) {
            if (v - 1 >= 0 && v - 1 < Data->MAXX2 && j < Data->MAXY2)
                frm->trittfolge.feld.Set(v - 1, j, 1);
        }
    }

    /*  Warp/weft colours. Values are 1-based palette indices. */
    for (int i = 0; i < warpColors.size() && i < Data->MAXX1; i++) {
        if (warpColors[i] < 1)
            continue;
        frm->kettfarben.feld.Set(i, char(warpColors[i] - 1));
    }
    for (int j = 0; j < weftColors.size() && j < Data->MAXY2; j++) {
        if (weftColors[j] < 1)
            continue;
        frm->schussfarben.feld.Set(j, char(weftColors[j] - 1));
    }

    /*  Blatteinzug (private extension). */
    for (int i = 0; i < blatteinzug.size() && i < Data->MAXX1; i++) {
        if (blatteinzug[i] < 0)
            continue;
        frm->blatteinzug.feld.Set(i, char(blatteinzug[i]));
    }

    /*  Sinking shed: WIF's "Rising Shed" is the inverse. Only apply
        when the source file actually declared it. */
    if (weaving.risingshedSeen)
        frm->sinkingshed = !weaving.risingshed;

    /*  Properties. */
    if (Data->properties) {
        if (text.haveAuthor)
            Data->properties->SetAuthor(text.author.toUtf8().constData());
        if (haveNote1)
            Data->properties->SetRemarks(note1.toUtf8().constData());
    }

    frm->RecalcGewebe();
    frm->SetModified();
}

/*-----------------------------------------------------------------*/
bool WifReader::read(const QString& _filename)
{
    filename = _filename;
    firstPass();
    secondPass();
    thirdPass();
    copyData();
    return true;
}

} /* anonymous namespace */

/*-----------------------------------------------------------------*/
bool TDBWFRM::DateiImportieren(const QString& _filename)
{
    WifReader reader(this);
    const bool ok = reader.read(_filename);
    CalcRangeKette();
    CalcRangeSchuesse();
    if (rapporthandler)
        rapporthandler->CalcRapport();
    return ok;
}

/*-----------------------------------------------------------------*/
void TDBWFRM::ImportWIFClick()
{
    const QString fn = QFileDialog::getOpenFileName(
        this, LANG_STR("Import WIF file", "WIF-Datei importieren"), QString(),
        LANG_STR("WIF files (*.wif);;All files (*)", "WIF-Dateien (*.wif);;Alle Dateien (*)"));
    if (fn.isEmpty())
        return;

    /*  Legacy AskSave gate is not yet ported; the main window asks
        before replacing an unsaved document when that slice lands. */

    QApplication::setOverrideCursor(Qt::WaitCursor);
    DateiImportieren(fn);
    QApplication::restoreOverrideCursor();

    filename.clear();
    SetAppTitle();
    if (undo) {
        undo->Clear();
        undo->Snapshot();
    }
    refresh();
}
