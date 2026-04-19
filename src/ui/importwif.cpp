/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  WIF importer — port of legacy/import.cpp. The legacy reader does
    three passes over the file to resolve size fields before
    populating threading / tieup / treadling. We keep the same
    approach, each pass closing the stream and reopening.         */

#include "mainwindow.h"
#include "datamodule.h"
#include "cursor.h"
#include "undoredo.h"
#include "rapport.h"

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
    bool risingshed = false;
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
struct WifThreadingEntry {
    QVector<int> harnesses;
};
struct WifTieupEntry {
    QVector<int> harnesses;
};
struct WifTreadlingEntry {
    QVector<int> treadles;
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

    QVector<WifThreadingEntry> threading;
    QVector<WifTieupEntry> tieup;
    QVector<WifTreadlingEntry> treadling;

    void firstPass();
    void secondPass();
    void thirdPass();
    void copyData();

public:
    /*  Returns the next "[section]" token or "" at EOF. */
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
/*  Each Read<Section> helper scans until it sees the next bracketed
    section header, returning that header. The parent loop then
    dispatches on it. This matches the legacy control flow. */
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
        else if (f == "rising shed")
            _w.risingshed = WifReader::fieldBool(line);
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
        else
            tok = nextToken(s);
    }
}

void WifReader::thirdPass()
{
    threading.resize(warp.threads);
    treadling.resize(weft.threads);
    tieup.resize(weaving.treadles);

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
        } else if (tok == "[tieup]") {
            tok = readIndexed(s, tieup,
                              [](WifTieupEntry& e, const QVector<int>& v) { e.harnesses = v; });
        } else {
            tok = nextToken(s);
        }
    }
}

/*-----------------------------------------------------------------*/
void WifReader::copyData()
{
    frm->einzug.Clear();
    for (int i = 0; i < threading.size(); i++) {
        if (threading[i].harnesses.isEmpty())
            continue;
        if (i >= Data->MAXX1)
            break;
        frm->einzug.feld.Set(i, short(threading[i].harnesses.first()));
    }

    frm->aufknuepfung.Clear();
    for (int i = 0; i < tieup.size(); i++) {
        const QVector<int>& hs = tieup[i].harnesses;
        for (int v : hs) {
            if (i < Data->MAXX2 && v - 1 >= 0 && v - 1 < Data->MAXY1)
                frm->aufknuepfung.feld.Set(i, v - 1, 1);
        }
    }

    frm->trittfolge.Clear();
    for (int j = 0; j < treadling.size(); j++) {
        const QVector<int>& ts = treadling[j].treadles;
        for (int v : ts) {
            if (v - 1 >= 0 && v - 1 < Data->MAXX2 && j < Data->MAXY2)
                frm->trittfolge.feld.Set(v - 1, j, 1);
        }
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
void TDBWFRM::ImportWIFClick()
{
    const QString fn
        = QFileDialog::getOpenFileName(this, QStringLiteral("Import WIF file"), QString(),
                                       QStringLiteral("WIF files (*.wif);;All files (*)"));
    if (fn.isEmpty())
        return;

    /*  Legacy AskSave gate is not yet ported; the main window asks
        before replacing an unsaved document when that slice lands. */

    QApplication::setOverrideCursor(Qt::WaitCursor);
    WifReader reader(this);
    reader.read(fn);
    QApplication::restoreOverrideCursor();

    filename.clear();
    SetAppTitle();
    CalcRangeKette();
    CalcRangeSchuesse();
    if (rapporthandler)
        rapporthandler->CalcRapport();
    for (int j = 0; j < Data->MAXY1; j++)
        freieschaefte[j] = true;
    for (int i = 0; i < Data->MAXX1; i++) {
        const short s = einzug.feld.Get(i);
        if (s > 0 && s - 1 < Data->MAXY1)
            freieschaefte[s - 1] = false;
    }
    for (int i = 0; i < Data->MAXX2; i++) {
        bool used = false;
        for (int j = 0; j < Data->MAXY1 && !used; j++)
            if (aufknuepfung.feld.Get(i, j) > 0)
                used = true;
        freietritte[i] = !used;
    }
    if (undo) {
        undo->Clear();
        undo->Snapshot();
    }
    refresh();
}
