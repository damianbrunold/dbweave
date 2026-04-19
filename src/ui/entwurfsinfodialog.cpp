/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "entwurfsinfodialog.h"

#include "mainwindow.h"
#include "datamodule.h"
#include "palette.h"

#include <QApplication>
#include <QColor>
#include <QCursor>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>

#include <cmath>

/*  Helper matching sprintf("%1.3g", ...) used throughout the legacy
    report text. QString::number(v, 'g', 3) produces the same short
    3-significant-digit formatting. */
static QString fmt3g(double _v)
{
    return QString::number(_v, 'g', 3);
}

EntwurfsinfoDialog::EntwurfsinfoDialog(TDBWFRM* _frm, QWidget* _parent)
    : QDialog(_parent)
    , frm(_frm)
{
    setWindowTitle(QStringLiteral("Pattern information"));
    setModal(true);

    categories = new QListWidget(this);
    categories->addItem(QStringLiteral("General"));
    categories->addItem(QStringLiteral("Colors"));
    categories->addItem(QStringLiteral("Floats"));
    categories->addItem(QStringLiteral("Heddles"));
    categories->setMaximumWidth(160);

    text = new QTextEdit(this);
    text->setReadOnly(true);
    text->setFont(QFont(QStringLiteral("monospace")));

    auto* bExport = new QPushButton(QStringLiteral("&Save..."), this);
    auto* bPrint = new QPushButton(QStringLiteral("&Print"), this);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    btns->addButton(bPrint, QDialogButtonBox::ActionRole);
    btns->addButton(bExport, QDialogButtonBox::ActionRole);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bPrint, &QPushButton::clicked, this, &EntwurfsinfoDialog::onPrint);
    connect(bExport, &QPushButton::clicked, this, &EntwurfsinfoDialog::onExport);

    auto* content = new QHBoxLayout();
    content->addWidget(categories);
    content->addWidget(text, 1);

    auto* root = new QVBoxLayout(this);
    root->addLayout(content, 1);
    root->addWidget(btns);

    connect(categories, &QListWidget::currentRowChanged, this,
            &EntwurfsinfoDialog::onCategoryChanged);

    resize(640, 480);
}

void EntwurfsinfoDialog::initInfos()
{
    categories->setCurrentRow(0);
    onAusmasse();
}

void EntwurfsinfoDialog::onCategoryChanged(int _row)
{
    switch (_row) {
    case 0:
        onAusmasse();
        break;
    case 1:
        onFarben();
        break;
    case 2:
        onFlottierungen();
        break;
    case 3:
        onLitzen();
        break;
    }
}

void EntwurfsinfoDialog::insertHeader()
{
    insertHeading(QStringLiteral("DB-WEAVE - Pattern information"));
    if (!frm->filename.isEmpty())
        text->append(QStringLiteral("Pattern: ") + frm->filename);
    text->append(QString());
}

void EntwurfsinfoDialog::insertHeading(const QString& _heading)
{
    QTextCursor c = text->textCursor();
    c.movePosition(QTextCursor::End);
    QTextCharFormat bold;
    bold.setFontWeight(QFont::Bold);
    if (!text->document()->isEmpty())
        c.insertBlock();
    c.insertText(_heading, bold);
    c.insertBlock();
    text->setTextCursor(c);
}

/*-----------------------------------------------------------------*/
void EntwurfsinfoDialog::buildAusmasse()
{
    ausmasse.clear();
    if (frm->kette.a == -1 || frm->kette.b == -1 || frm->schuesse.a == -1 || frm->schuesse.b == -1)
        return;

    if (!frm->filename.isEmpty())
        ausmasse << QStringLiteral("Pattern name: ") + QFileInfo(frm->filename).fileName();

    ausmasse << QStringLiteral("Pattern size: %1 x %2")
                    .arg(frm->kette.b - frm->kette.a + 1)
                    .arg(frm->schuesse.b - frm->schuesse.a + 1);

    if (frm->rapport.kr.b != -1 && frm->rapport.sr.b != -1)
        ausmasse << QStringLiteral("Pattern repeat: %1 x %2")
                        .arg(frm->rapport.kr.b - frm->rapport.kr.a + 1)
                        .arg(frm->rapport.sr.b - frm->rapport.sr.a + 1);

    int nschaefte = 0;
    for (int i = 0; i < Data->MAXY1; i++)
        if (!frm->freieschaefte[i])
            nschaefte++;
    ausmasse << QStringLiteral("Number of shafts: %1").arg(nschaefte);

    int ntritte = 0;
    for (int i = 0; i < Data->MAXX2; i++)
        if (!frm->freietritte[i])
            ntritte++;
    ausmasse << QStringLiteral("Number of treadles: %1").arg(ntritte);

    const int total = (frm->kette.b - frm->kette.a + 1) * (frm->schuesse.b - frm->schuesse.a + 1);
    int hebungen = 0;
    int senkungen = 0;
    for (int i = frm->kette.a; i <= frm->kette.b; i++)
        for (int j = frm->schuesse.a; j <= frm->schuesse.b; j++)
            if (frm->gewebe.feld.Get(i, j) > 0)
                hebungen++;
            else
                senkungen++;
    if (hebungen < senkungen)
        ausmasse << QStringLiteral("The pattern is weft sided (") + fmt3g(100.0 * senkungen / total)
                        + QStringLiteral("% sinking binding points)");
    else if (hebungen > senkungen)
        ausmasse << QStringLiteral("The pattern is warp sided (") + fmt3g(100.0 * hebungen / total)
                        + QStringLiteral("% rising binding points)");
    else
        ausmasse << QStringLiteral("The pattern is balanced");

    ausmasse << QString();
}

/*-----------------------------------------------------------------*/
void EntwurfsinfoDialog::buildFarben()
{
    farben.clear();

    int farbcount = 0;
    int kfarbcount = 0;
    int sfarbcount = 0;
    bool coltable[MAX_PAL_ENTRY + 1];
    int ktable[MAX_PAL_ENTRY + 1];
    int stable[MAX_PAL_ENTRY + 1];
    for (int i = 0; i <= MAX_PAL_ENTRY; i++) {
        coltable[i] = false;
        ktable[i] = stable[i] = 0;
    }
    for (int i = frm->kette.a; i <= frm->kette.b; i++) {
        unsigned char c = frm->kettfarben.feld.Get(i);
        if (!coltable[c]) {
            farbcount++;
            kfarbcount++;
        }
        coltable[c] = true;
        ktable[c]++;
    }
    for (int j = frm->schuesse.a; j <= frm->schuesse.b; j++) {
        unsigned char c = frm->schussfarben.feld.Get(j);
        if (!coltable[c]) {
            farbcount++;
            sfarbcount++;
        }
        coltable[c] = true;
        stable[c]++;
    }
    farben << QStringLiteral("Number of colors:");
    farben << QStringLiteral("Total: %1").arg(farbcount);
    farben << QStringLiteral("In warp: %1").arg(kfarbcount);
    farben << QStringLiteral("In weft: %1").arg(sfarbcount);
    farben << QString();

    auto emitPalette = [&](const char* _label, int* _table, int _total, const char* _fadenLabel) {
        farben << QString::fromLatin1(_label);
        int count = 1;
        for (int i = 0; i < MAX_PAL_ENTRY; i++) {
            if (_table[i] != 0) {
                const COLORREF rgb = Data->palette->GetColor(i);
                const int r = GetRValue(rgb);
                const int g = GetGValue(rgb);
                const int b = GetBValue(rgb);
                QColor qc(r, g, b);
                float h, s, v;
                qc.getHsvF(&h, &s, &v);
                /*  Legacy reports H in 0..360 and S/V in 0..1; when Qt
                    returns h==-1 (achromatic) we show 0 to mimic the
                    "UNDEFINED" sentinel the legacy shows as a large
                    number from the %1.3g formatter. */
                double hDeg = (h < 0.0) ? 0.0 : h * 360.0;
                farben << QStringLiteral("Color %1: HSV=(%2,%3,%4), "
                                         "RGB=(%5,%6,%7), %8")
                              .arg(count)
                              .arg(fmt3g(hDeg))
                              .arg(fmt3g(s))
                              .arg(fmt3g(v))
                              .arg(r)
                              .arg(g)
                              .arg(b)
                              .arg(QStringLiteral("%1%2%3%)")
                                       .arg(_table[i])
                                       .arg(QString::fromLatin1(_fadenLabel))
                                       .arg(fmt3g(100.0 * _table[i] / _total)));
                count++;
            }
        }
        farben << QString();
    };

    emitPalette("Warp colors:", ktable, frm->kette.b - frm->kette.a + 1, " warp ends  (");
    emitPalette("Weft colors:", stable, frm->schuesse.b - frm->schuesse.a + 1, " weft picks  (");
}

/*-----------------------------------------------------------------*/
void EntwurfsinfoDialog::buildLitzen()
{
    litzen.clear();
    int litzennr = 0;
    for (int i = 0; i < Data->MAXX1; i++)
        if (frm->einzug.feld.Get(i) != 0)
            litzennr++;
    litzen << QStringLiteral("Number of heddles: %1").arg(litzennr);

    litzen << QStringLiteral("Distribution across the shafts:");
    int schaft = 0;
    for (int j = 0; j < Data->MAXY1; j++) {
        if (!frm->freieschaefte[j]) {
            schaft++;
            int count = 0;
            for (int i = 0; i < Data->MAXX1; i++)
                if (frm->einzug.feld.Get(i) == j + 1)
                    count++;
            litzen << QStringLiteral("Shaft %1:  %2 Heddles  (%3%)")
                          .arg(schaft)
                          .arg(count)
                          .arg(litzennr ? fmt3g(100.0 * count / litzennr) : QStringLiteral("0"));
        }
    }
    litzen << QString();
}

/*-----------------------------------------------------------------*/
static void buildFloatDir(const QStringList::iterator&, QStringList& /*unused*/) { }

void EntwurfsinfoDialog::buildFlottierungen()
{
    flottierungen.clear();

    /*  Weft floats. Walks each weft row left-to-right; a run of
        non-rising (<=0) gewebe cells bounded by a rising cell is a
        "float" whose length is tallied by i-startflot. An open trailing
        run is counted at kette.b+1-startflot.                          */
    try {
        int maxflot = frm->kette.b - frm->kette.a + 1 + 1;
        int* table = new int[maxflot];
        for (int i = 0; i < maxflot; i++)
            table[i] = 0;
        int biggest = 0;
        int count = 0;
        for (int j = frm->schuesse.a; j <= frm->schuesse.b; j++) {
            bool inflot = frm->gewebe.feld.Get(frm->kette.a, j) <= 0;
            int startflot = -1;
            if (inflot)
                startflot = frm->kette.a;
            for (int i = frm->kette.a; i <= frm->kette.b; i++) {
                if (inflot) {
                    if (frm->gewebe.feld.Get(i, j) > 0) {
                        table[i - startflot]++;
                        count++;
                        if (i - startflot > biggest)
                            biggest = i - startflot;
                        inflot = false;
                        startflot = -1;
                    }
                } else if (frm->gewebe.feld.Get(i, j) <= 0) {
                    inflot = true;
                    startflot = i;
                }
            }
            if (inflot) {
                int flot = frm->kette.b + 1 - startflot;
                table[flot]++;
                count++;
                if (flot > biggest)
                    biggest = flot;
            }
        }
        flottierungen << QStringLiteral("Weft floats:");
        flottierungen << QStringLiteral("Longest float: %1").arg(biggest);
        double mittel = 0.0;
        for (int i = 1; i < maxflot; i++)
            if (table[i] != 0)
                mittel = mittel + double(table[i]) * i / count;
        flottierungen << QStringLiteral("Average float: ") + fmt3g(mittel);
        flottierungen << QStringLiteral("Distribution:");
        for (int i = 1; i < maxflot; i++)
            if (table[i] != 0)
                flottierungen << QStringLiteral("Float %1:  %2 floats  (%3%)")
                                     .arg(i)
                                     .arg(table[i])
                                     .arg(fmt3g(100.0 * table[i] / count));
        delete[] table;
    } catch (...) {
    }
    flottierungen << QString();

    /*  Warp floats — same algorithm, axes swapped. */
    try {
        int maxflot = frm->schuesse.b - frm->schuesse.a + 1 + 1;
        int* table = new int[maxflot];
        for (int i = 0; i < maxflot; i++)
            table[i] = 0;
        int biggest = 0;
        int count = 0;
        for (int i = frm->kette.a; i <= frm->kette.b; i++) {
            bool inflot = frm->gewebe.feld.Get(i, frm->schuesse.a) > 0;
            int startflot = -1;
            if (inflot)
                startflot = frm->schuesse.a;
            for (int j = frm->schuesse.a; j <= frm->schuesse.b; j++) {
                if (inflot) {
                    if (frm->gewebe.feld.Get(i, j) <= 0) {
                        table[j - startflot]++;
                        count++;
                        if (j - startflot > biggest)
                            biggest = j - startflot;
                        inflot = false;
                        startflot = -1;
                    }
                } else if (frm->gewebe.feld.Get(i, j) > 0) {
                    inflot = true;
                    startflot = j;
                }
            }
            if (inflot) {
                int flot = frm->schuesse.b + 1 - startflot;
                table[flot]++;
                count++;
                if (flot > biggest)
                    biggest = flot;
            }
        }
        flottierungen << QStringLiteral("Warp floats:");
        flottierungen << QStringLiteral("Longest float: %1").arg(biggest);
        double mittel = 0.0;
        for (int i = 1; i < maxflot; i++)
            if (table[i] != 0)
                mittel = mittel + double(table[i]) * i / count;
        flottierungen << QStringLiteral("Average float: ") + fmt3g(mittel);
        flottierungen << QStringLiteral("Distribution:");
        for (int i = 1; i < maxflot; i++)
            if (table[i] != 0)
                flottierungen << QStringLiteral("Float %1:  %2 floats  (%3%)")
                                     .arg(i)
                                     .arg(table[i])
                                     .arg(fmt3g(100.0 * table[i] / count));
        delete[] table;
    } catch (...) {
    }
    flottierungen << QString();
}

/*-----------------------------------------------------------------*/
static void appendLines(QTextEdit* _text, const QStringList& _lines)
{
    for (const QString& line : _lines)
        _text->append(line);
}

void EntwurfsinfoDialog::onAusmasse(bool _clear)
{
    if (_clear)
        text->clear();
    if (!read_ausmasse) {
        buildAusmasse();
        read_ausmasse = true;
    }
    appendLines(text, ausmasse);
}

void EntwurfsinfoDialog::onFarben(bool _clear)
{
    if (_clear)
        text->clear();
    if (!read_farben) {
        buildFarben();
        read_farben = true;
    }
    appendLines(text, farben);
}

void EntwurfsinfoDialog::onLitzen(bool _clear)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (_clear)
        text->clear();
    if (!read_litzen) {
        buildLitzen();
        read_litzen = true;
    }
    appendLines(text, litzen);
    QApplication::restoreOverrideCursor();
}

void EntwurfsinfoDialog::onFlottierungen(bool _clear)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (_clear)
        text->clear();
    if (!read_flottierungen) {
        buildFlottierungen();
        read_flottierungen = true;
    }
    appendLines(text, flottierungen);
    QApplication::restoreOverrideCursor();
}

/*-----------------------------------------------------------------*/
void EntwurfsinfoDialog::onPrint()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const int prev = categories->currentRow();

    text->clear();
    insertHeader();
    insertHeading(QStringLiteral("General"));
    onAusmasse(false);
    insertHeading(QStringLiteral("Colors"));
    onFarben(false);
    insertHeading(QStringLiteral("Floats"));
    onFlottierungen(false);
    insertHeading(QStringLiteral("Heddles"));
    onLitzen(false);

    QPrinter printer;
    QPrintDialog dlg(&printer, this);
    if (dlg.exec() == QDialog::Accepted)
        text->document()->print(&printer);

    /*  Restore selected category view. */
    text->clear();
    categories->setCurrentRow(prev);
    onCategoryChanged(prev);
    QApplication::restoreOverrideCursor();
}

void EntwurfsinfoDialog::onExport()
{
    QString suggested;
    if (!frm->filename.isEmpty()) {
        QFileInfo fi(frm->filename);
        suggested = fi.completeBaseName() + QStringLiteral(".rtf");
    } else {
        suggested = QStringLiteral("unnamed.rtf");
    }

    const QString fn
        = QFileDialog::getSaveFileName(this, QStringLiteral("Save pattern info as"), suggested,
                                       QStringLiteral("Word-Format (*.rtf)"));
    if (fn.isEmpty())
        return;

    const int prev = categories->currentRow();
    QApplication::setOverrideCursor(Qt::WaitCursor);

    text->clear();
    insertHeader();
    insertHeading(QStringLiteral("General"));
    onAusmasse(false);
    insertHeading(QStringLiteral("Colors"));
    onFarben(false);
    insertHeading(QStringLiteral("Floats"));
    onFlottierungen(false);
    insertHeading(QStringLiteral("Heddles"));
    onLitzen(false);

    /*  Legacy writes TRichEdit::Lines->SaveToFile which despite the
        .rtf filter only writes plain text. Match that. */
    QFile f(fn);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts(&f);
        ts << text->toPlainText();
        f.close();
        QDesktopServices::openUrl(QUrl::fromLocalFile(fn));
    }

    text->clear();
    categories->setCurrentRow(prev);
    onCategoryChanged(prev);
    QApplication::restoreOverrideCursor();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::ViewInfosClick()
{
    EntwurfsinfoDialog dlg(this, this);
    dlg.initInfos();
    dlg.exec();
}
