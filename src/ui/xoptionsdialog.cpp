/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "xoptionsdialog.h"
#include "language.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

void XOptionsDialog::loadCombo(QComboBox* _cb, bool _withNumber)
{
    _cb->clear();
    _cb->addItem(LANG_STR("Filled", "Ausgefüllt"));
    _cb->addItem(LANG_STR("Vertical", "Vertikal"));
    _cb->addItem(LANG_STR("Cross", "Kreuz"));
    _cb->addItem(LANG_STR("Point", "Punkt"));
    _cb->addItem(LANG_STR("Circle", "Kreis"));
    _cb->addItem(LANG_STR("Rising", "Steigend"));
    _cb->addItem(LANG_STR("Falling", "Fallend"));
    _cb->addItem(LANG_STR("Small cross", "Kleines Kreuz"));
    _cb->addItem(LANG_STR("Small circle", "Kleiner Kreis"));
    if (_withNumber)
        _cb->addItem(LANG_STR("Number", "Nummer"));
}

XOptionsDialog::XOptionsDialog(QWidget* _parent)
    : QDialog(_parent)
{
    setWindowTitle(LANG_STR("Options", "Optionen"));
    setModal(true);

    tabs = new QTabWidget(this);

    auto makeSpin = [this](int _min, int _max) {
        auto* sb = new QSpinBox(this);
        sb->setRange(_min, _max);
        return sb;
    };

    /*  --- Sizes tab (default / active page) ---------------------- */
    auto* tabSizes = new QWidget(this);
    upSchaefte = makeSpin(1, 100);
    upTritte = makeSpin(1, 100);
    upKette = makeSpin(1, 10000);
    upSchuesse = makeSpin(1, 10000);
    upSchaefteVis = makeSpin(0, 100);
    upTritteVis = makeSpin(0, 100);
    auto* gbSize = new QGroupBox(LANG_STR("Size", "Grösse"), tabSizes);
    {
        auto* f = new QFormLayout(gbSize);
        f->addRow(LANG_STR("&Harnesses:", "&Schäfte:"), upSchaefte);
        f->addRow(LANG_STR("&Treadles:", "&Tritte:"), upTritte);
        f->addRow(LANG_STR("&Warp threads:", "&Kettfäden:"), upKette);
        f->addRow(LANG_STR("W&eft threads:", "Schussfäd&en:"), upSchuesse);
    }
    auto* gbVis = new QGroupBox(LANG_STR("Visibility", "Sichtbarkeit"), tabSizes);
    {
        auto* f = new QFormLayout(gbVis);
        f->addRow(LANG_STR("&Visible harnesses:", "Sichtbare &Schäfte:"), upSchaefteVis);
        f->addRow(LANG_STR("V&isible treadles:", "Sichtbare T&ritte:"), upTritteVis);
    }
    auto* sizesLay = new QVBoxLayout(tabSizes);
    sizesLay->addWidget(gbSize);
    sizesLay->addWidget(gbVis);
    sizesLay->addStretch();
    tabs->addTab(tabSizes, LANG_STR("Si&zes", "&Grössen"));

    /*  --- View tab ---------------------------------------------- */
    auto* tabView = new QWidget(this);
    EinzugUnten = new QCheckBox(LANG_STR("&Threading below pattern", "&Einzug unter Patrone"),
                                tabView);
    RightToLeft = new QCheckBox(
        LANG_STR("&Work direction in threading and pattern from right to left",
                 "&Arbeitsrichtung in Einzug und Patrone von rechts nach links"),
        tabView);
    TopToBottom = new QCheckBox(
        LANG_STR("W&ork direction in threading and tie-up from top to bottom",
                 "Arbeits&richtung in Einzug und Aufknüpfung von oben nach unten"),
        tabView);
    FEWithRaster = new QCheckBox(LANG_STR("&Color effect with grid", "&Farbeffekt mit Raster"),
                                 tabView);
    AltFarbpalette = new QCheckBox(
        LANG_STR("&Use alternate color palette", "Alternative Farb&palette verwenden"), tabView);
    AltLiftplanstyle = new QCheckBox(
        LANG_STR("U&se alternate pegplan view",
                 "Alternative Sc&hlagpatronenansicht verwenden"),
        tabView);
    auto* viewLay = new QVBoxLayout(tabView);
    viewLay->addWidget(EinzugUnten);
    viewLay->addWidget(RightToLeft);
    viewLay->addWidget(TopToBottom);
    viewLay->addWidget(FEWithRaster);
    viewLay->addWidget(AltFarbpalette);
    viewLay->addWidget(AltLiftplanstyle);
    viewLay->addStretch();
    tabs->addTab(tabView, LANG_STR("&View", "&Ansicht"));

    /*  --- Symbols tab ------------------------------------------- */
    auto* tabSymbols = new QWidget(this);
    cbEinzug = new QComboBox(tabSymbols);
    loadCombo(cbEinzug, true);
    cbTrittfolge = new QComboBox(tabSymbols);
    loadCombo(cbTrittfolge);
    cbAufknuepfung = new QComboBox(tabSymbols);
    loadCombo(cbAufknuepfung, true);
    cbSchlagpatrone = new QComboBox(tabSymbols);
    loadCombo(cbSchlagpatrone, true);
    cbAushebung = new QComboBox(tabSymbols);
    loadCombo(cbAushebung);
    cbAnbindung = new QComboBox(tabSymbols);
    loadCombo(cbAnbindung);
    cbAbbindung = new QComboBox(tabSymbols);
    loadCombo(cbAbbindung);
    auto* symLay = new QFormLayout(tabSymbols);
    symLay->addRow(LANG_STR("&Threading:", "&Einzug:"), cbEinzug);
    symLay->addRow(LANG_STR("Tre&adling:", "&Trittfolge:"), cbTrittfolge);
    symLay->addRow(LANG_STR("Tie-&up:", "A&ufknüpfung:"), cbAufknuepfung);
    symLay->addRow(LANG_STR("&Pegplan:", "&Schlagpatrone:"), cbSchlagpatrone);
    symLay->addRow(LANG_STR("&Lift out:", "A&ushebung:"), cbAushebung);
    symLay->addRow(LANG_STR("&Binding:", "A&nbindung:"), cbAnbindung);
    symLay->addRow(LANG_STR("U&nbinding:", "Abbi&ndung:"), cbAbbindung);
    tabs->addTab(tabSymbols, LANG_STR("S&ymbols", "&Symbole"));

    /*  --- Settings tab ------------------------------------------ */
    auto* tabSettings = new QWidget(this);
    auto* gbTritt = new QGroupBox(LANG_STR("Treadle mode", "Trittmodus"), tabSettings);
    Einzeltritt = new QRadioButton(LANG_STR("&Single treadle", "&Einzeltritt"), gbTritt);
    Multitritt = new QRadioButton(LANG_STR("&Multi treadle", "&Multitritt"), gbTritt);
    {
        auto* l = new QVBoxLayout(gbTritt);
        l->addWidget(Einzeltritt);
        l->addWidget(Multitritt);
    }
    auto* gbShed = new QGroupBox(LANG_STR("Harness mode", "Schaftmodus"), tabSettings);
    RisingShed = new QRadioButton(LANG_STR("&Rising shed", "&Obenhebend"), gbShed);
    SinkingShed = new QRadioButton(LANG_STR("S&inking shed", "U&ntenhebend"), gbShed);
    {
        auto* l = new QVBoxLayout(gbShed);
        l->addWidget(RisingShed);
        l->addWidget(SinkingShed);
    }
    auto* setLay = new QVBoxLayout(tabSettings);
    setLay->addWidget(gbTritt);
    setLay->addWidget(gbShed);
    setLay->addStretch();
    tabs->addTab(tabSettings, LANG_STR("Se&ttings", "&Einstellungen"));

    /*  --- Grid tab ----------------------------------------------- */
    auto* tabGrid = new QWidget(this);
    SchenienHorz = makeSpin(0, 100);
    SchenienVert = makeSpin(0, 100);
    auto* gbGrid = new QGroupBox(LANG_STR("Gridsetting", "Rastereinstellung"), tabGrid);
    {
        auto* f = new QFormLayout(gbGrid);
        f->addRow(LANG_STR("&Horizontal:", "&Horizontal:"), SchenienHorz);
        f->addRow(LANG_STR("&Vertical:", "&Vertikal:"), SchenienVert);
    }
    auto* gridLay = new QVBoxLayout(tabGrid);
    gridLay->addWidget(gbGrid);
    gridLay->addStretch();
    tabs->addTab(tabGrid, LANG_STR("&Grid", "&Raster"));

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->addWidget(tabs);
    root->addWidget(btns);

    tabs->setCurrentIndex(0); /* Sizes as default page, legacy OptAusmasse */
}

/*-----------------------------------------------------------------*/
#include "envoptionsdialog.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "palette.h"
#include "patterncanvas.h"
#include "cursor.h"
#include "undoredo.h"
#include "settings.h"

void TDBWFRM::OptEnvironmentClick()
{
    EnvOptionsDialog dlg(this);
    dlg.exec();
    refresh();
}

void TDBWFRM::XOptionsClick()
{
    ShowOptions(false);
}
void TDBWFRM::XOptionsGlobalClick()
{
    ShowOptions(true);
}

void TDBWFRM::ShowOptions(bool _global)
{
    XOptionsDialog frm(this);

    /*  Ausmasse */
    frm.upSchaefte->setValue(Data->MAXY1);
    frm.upTritte->setValue(Data->MAXX2);
    frm.upKette->setValue(Data->MAXX1);
    frm.upSchuesse->setValue(Data->MAXY2);
    frm.upSchaefteVis->setValue(hvisible);
    frm.upTritteVis->setValue(wvisible);
    /*  Schenien */
    frm.SchenienHorz->setValue(einzug.pos.strongline_x);
    frm.SchenienVert->setValue(einzug.pos.strongline_y);
    /*  Einstellungen */
    if (trittfolge.einzeltritt)
        frm.Einzeltritt->setChecked(true);
    else
        frm.Multitritt->setChecked(true);
    if (sinkingshed)
        frm.SinkingShed->setChecked(true);
    else
        frm.RisingShed->setChecked(true);
    /*  Symbole */
    frm.cbEinzug->setCurrentIndex(einzug.darstellung);
    frm.cbAufknuepfung->setCurrentIndex(aufknuepfung.darstellung);
    frm.cbTrittfolge->setCurrentIndex(trittfolge.darstellung);
    frm.cbSchlagpatrone->setCurrentIndex(schlagpatronendarstellung);
    frm.cbAushebung->setCurrentIndex(darst_aushebung);
    frm.cbAnbindung->setCurrentIndex(darst_anbindung);
    frm.cbAbbindung->setCurrentIndex(darst_abbindung);
    /*  Ansicht */
    frm.EinzugUnten->setChecked(einzugunten);
    frm.RightToLeft->setChecked(righttoleft);
    frm.TopToBottom->setChecked(toptobottom);
    frm.FEWithRaster->setChecked(fewithraster);
    frm.AltLiftplanstyle->setChecked(aufknuepfung.pegplanstyle);
    frm.AltFarbpalette->setChecked(palette2);

    if (cursorhandler)
        cursorhandler->DisableCursor();
    if (frm.exec() != QDialog::Accepted) {
        if (cursorhandler)
            cursorhandler->EnableCursor();
        return;
    }

    Settings settings;

    /*  Ausmasse */
    const int x1 = frm.upKette->value();
    const int y1 = frm.upSchaefte->value();
    const int x2 = frm.upTritte->value();
    const int y2 = frm.upSchuesse->value();
    const int vy1 = frm.upSchaefteVis->value();
    const int vx2 = frm.upTritteVis->value();
    SetAusmasse(x1, y1, x2, y2, vx2, vy1);
    if (_global) {
        settings.SetCategory(QString("Size"));
        settings.Save(QString("ShaftsVisible"), hvisible);
        settings.Save(QString("TreadlesVisible"), wvisible);
    }

    /*  Schenien */
    einzug.pos.strongline_x = aufknuepfung.pos.strongline_x = trittfolge.pos.strongline_x
        = gewebe.pos.strongline_x = frm.SchenienHorz->value();
    einzug.pos.strongline_y = aufknuepfung.pos.strongline_y = trittfolge.pos.strongline_y
        = gewebe.pos.strongline_y = frm.SchenienVert->value();
    if (_global) {
        settings.SetCategory(QString("Grid"));
        settings.Save(QString("Horizontal"), einzug.pos.strongline_x);
        settings.Save(QString("Vertical"), einzug.pos.strongline_y);
    }

    /*  Einstellungen */
    trittfolge.einzeltritt = frm.Einzeltritt->isChecked();
    sinkingshed = frm.SinkingShed->isChecked();
    if (_global) {
        settings.SetCategory(QString("Settings"));
        settings.Save(QString("SingleTreadle"), trittfolge.einzeltritt ? 1 : 0);
        settings.Save(QString("SinkingShed"), sinkingshed ? 1 : 0);
    }

    /*  Symbole */
    einzug.darstellung = (DARSTELLUNG)frm.cbEinzug->currentIndex();
    aufknuepfung.darstellung = (DARSTELLUNG)frm.cbAufknuepfung->currentIndex();
    trittfolge.darstellung = (DARSTELLUNG)frm.cbTrittfolge->currentIndex();
    schlagpatronendarstellung = (DARSTELLUNG)frm.cbSchlagpatrone->currentIndex();
    darst_aushebung = (DARSTELLUNG)frm.cbAushebung->currentIndex();
    darst_anbindung = (DARSTELLUNG)frm.cbAnbindung->currentIndex();
    darst_abbindung = (DARSTELLUNG)frm.cbAbbindung->currentIndex();
    if (_global) {
        settings.SetCategory(QString("Display"));
        settings.Save(QString("Threading"), (int)einzug.darstellung);
        settings.Save(QString("Tie-up"), (int)aufknuepfung.darstellung);
        settings.Save(QString("Treadling"), (int)trittfolge.darstellung);
        settings.Save(QString("Pegplan"), (int)schlagpatronendarstellung);
        settings.Save(QString("LiftOut"), (int)darst_aushebung);
        settings.Save(QString("Binding"), (int)darst_anbindung);
        settings.Save(QString("Unbinding"), (int)darst_abbindung);
    }

    /*  Ansicht */
    einzugunten = frm.EinzugUnten->isChecked();
    righttoleft = frm.RightToLeft->isChecked();
    toptobottom = frm.TopToBottom->isChecked();
    fewithraster = frm.FEWithRaster->isChecked();
    const bool pal2 = frm.AltFarbpalette->isChecked();
    if (pal2 != palette2) {
        palette2 = pal2;
        if (Data && Data->palette)
            Data->palette->SetPaletteType(palette2);
    }
    aufknuepfung.pegplanstyle = frm.AltLiftplanstyle->isChecked();
    if (_global) {
        settings.SetCategory(QString("View"));
        settings.Save(QString("ThreadingDown"), einzugunten ? 1 : 0);
        settings.Save(QString("RightToLeft"), righttoleft ? 1 : 0);
        settings.Save(QString("TopToBottom"), toptobottom ? 1 : 0);
        settings.Save(QString("ColorWithRaster"), fewithraster ? 1 : 0);
        settings.Save(QString("AltColorpalette"), palette2 ? 1 : 0);
        settings.Save(QString("AltPegplan"), aufknuepfung.pegplanstyle ? 1 : 0);
    }

    if (pattern_canvas)
        pattern_canvas->recomputeLayout();
    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
    if (cursorhandler)
        cursorhandler->EnableCursor();
}
