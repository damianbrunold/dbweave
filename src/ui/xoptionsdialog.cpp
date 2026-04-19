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

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

void XOptionsDialog::loadCombo (QComboBox* _cb, bool _withNumber)
{
	_cb->clear();
	_cb->addItem(QStringLiteral("Filled"));
	_cb->addItem(QStringLiteral("Vertical"));
	_cb->addItem(QStringLiteral("Cross"));
	_cb->addItem(QStringLiteral("Point"));
	_cb->addItem(QStringLiteral("Circle"));
	_cb->addItem(QStringLiteral("Rising"));
	_cb->addItem(QStringLiteral("Falling"));
	_cb->addItem(QStringLiteral("Small cross"));
	_cb->addItem(QStringLiteral("Small circle"));
	if (_withNumber) _cb->addItem(QStringLiteral("Number"));
}

XOptionsDialog::XOptionsDialog (QWidget* _parent)
	: QDialog(_parent)
{
	setWindowTitle(QStringLiteral("Options"));
	setModal(true);

	tabs = new QTabWidget(this);

	auto makeSpin = [this](int _min, int _max) {
		auto* sb = new QSpinBox(this);
		sb->setRange(_min, _max);
		return sb;
	};

	/*  --- Sizes tab (default / active page) ---------------------- */
	auto* tabSizes = new QWidget(this);
	upSchaefte    = makeSpin(1, 100);
	upTritte      = makeSpin(1, 100);
	upKette       = makeSpin(1, 10000);
	upSchuesse    = makeSpin(1, 10000);
	upSchaefteVis = makeSpin(0, 100);
	upTritteVis   = makeSpin(0, 100);
	auto* gbSize = new QGroupBox(QStringLiteral("Size"), tabSizes);
	{
		auto* f = new QFormLayout(gbSize);
		f->addRow(QStringLiteral("&Harnesses:"),    upSchaefte);
		f->addRow(QStringLiteral("&Treadles:"),     upTritte);
		f->addRow(QStringLiteral("&Warp threads:"), upKette);
		f->addRow(QStringLiteral("W&eft threads:"), upSchuesse);
	}
	auto* gbVis = new QGroupBox(QStringLiteral("Visibility"), tabSizes);
	{
		auto* f = new QFormLayout(gbVis);
		f->addRow(QStringLiteral("&Visible harnesses:"), upSchaefteVis);
		f->addRow(QStringLiteral("V&isible treadles:"),  upTritteVis);
	}
	auto* sizesLay = new QVBoxLayout(tabSizes);
	sizesLay->addWidget(gbSize);
	sizesLay->addWidget(gbVis);
	sizesLay->addStretch();
	tabs->addTab(tabSizes, QStringLiteral("Si&zes"));

	/*  --- View tab ---------------------------------------------- */
	auto* tabView = new QWidget(this);
	EinzugUnten      = new QCheckBox(QStringLiteral("&Threading below pattern"),            tabView);
	RightToLeft      = new QCheckBox(QStringLiteral("&Work direction in threading and pattern from right to left"), tabView);
	TopToBottom      = new QCheckBox(QStringLiteral("W&ork direction in threading and tie-up from top to bottom"),  tabView);
	FEWithRaster     = new QCheckBox(QStringLiteral("&Color effect with grid"),             tabView);
	AltFarbpalette   = new QCheckBox(QStringLiteral("&Use alternate color palette"),        tabView);
	AltLiftplanstyle = new QCheckBox(QStringLiteral("U&se alternate pegplan view"),         tabView);
	auto* viewLay = new QVBoxLayout(tabView);
	viewLay->addWidget(EinzugUnten);
	viewLay->addWidget(RightToLeft);
	viewLay->addWidget(TopToBottom);
	viewLay->addWidget(FEWithRaster);
	viewLay->addWidget(AltFarbpalette);
	viewLay->addWidget(AltLiftplanstyle);
	viewLay->addStretch();
	tabs->addTab(tabView, QStringLiteral("&View"));

	/*  --- Symbols tab ------------------------------------------- */
	auto* tabSymbols = new QWidget(this);
	cbEinzug        = new QComboBox(tabSymbols);  loadCombo(cbEinzug, true);
	cbTrittfolge    = new QComboBox(tabSymbols);  loadCombo(cbTrittfolge);
	cbAufknuepfung  = new QComboBox(tabSymbols);  loadCombo(cbAufknuepfung, true);
	cbSchlagpatrone = new QComboBox(tabSymbols);  loadCombo(cbSchlagpatrone, true);
	cbAushebung     = new QComboBox(tabSymbols);  loadCombo(cbAushebung);
	cbAnbindung     = new QComboBox(tabSymbols);  loadCombo(cbAnbindung);
	cbAbbindung     = new QComboBox(tabSymbols);  loadCombo(cbAbbindung);
	auto* symLay = new QFormLayout(tabSymbols);
	symLay->addRow(QStringLiteral("&Threading:"),     cbEinzug);
	symLay->addRow(QStringLiteral("Tre&adling:"),     cbTrittfolge);
	symLay->addRow(QStringLiteral("Tie-&up:"),        cbAufknuepfung);
	symLay->addRow(QStringLiteral("&Pegplan:"),       cbSchlagpatrone);
	symLay->addRow(QStringLiteral("&Lift out:"),      cbAushebung);
	symLay->addRow(QStringLiteral("&Binding:"),       cbAnbindung);
	symLay->addRow(QStringLiteral("U&nbinding:"),     cbAbbindung);
	tabs->addTab(tabSymbols, QStringLiteral("S&ymbols"));

	/*  --- Settings tab ------------------------------------------ */
	auto* tabSettings = new QWidget(this);
	auto* gbTritt = new QGroupBox(QStringLiteral("Treadle mode"), tabSettings);
	Einzeltritt = new QRadioButton(QStringLiteral("&Single treadle"), gbTritt);
	Multitritt  = new QRadioButton(QStringLiteral("&Multi treadle"),  gbTritt);
	{
		auto* l = new QVBoxLayout(gbTritt);
		l->addWidget(Einzeltritt);
		l->addWidget(Multitritt);
	}
	auto* gbShed = new QGroupBox(QStringLiteral("Harness mode"), tabSettings);
	RisingShed  = new QRadioButton(QStringLiteral("&Rising shed"),  gbShed);
	SinkingShed = new QRadioButton(QStringLiteral("S&inking shed"), gbShed);
	{
		auto* l = new QVBoxLayout(gbShed);
		l->addWidget(RisingShed);
		l->addWidget(SinkingShed);
	}
	auto* setLay = new QVBoxLayout(tabSettings);
	setLay->addWidget(gbTritt);
	setLay->addWidget(gbShed);
	setLay->addStretch();
	tabs->addTab(tabSettings, QStringLiteral("Se&ttings"));

	/*  --- Grid tab ----------------------------------------------- */
	auto* tabGrid = new QWidget(this);
	SchenienHorz = makeSpin(0, 100);
	SchenienVert = makeSpin(0, 100);
	auto* gbGrid = new QGroupBox(QStringLiteral("Gridsetting"), tabGrid);
	{
		auto* f = new QFormLayout(gbGrid);
		f->addRow(QStringLiteral("&Horizontal:"), SchenienHorz);
		f->addRow(QStringLiteral("&Vertical:"),   SchenienVert);
	}
	auto* gridLay = new QVBoxLayout(tabGrid);
	gridLay->addWidget(gbGrid);
	gridLay->addStretch();
	tabs->addTab(tabGrid, QStringLiteral("&Grid"));

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto* root = new QVBoxLayout(this);
	root->addWidget(tabs);
	root->addWidget(btns);

	tabs->setCurrentIndex(0);   /* Sizes as default page, legacy OptAusmasse */
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

void __fastcall TDBWFRM::OptEnvironmentClick ()
{
	EnvOptionsDialog dlg(this);
	dlg.exec();
	refresh();
}

void __fastcall TDBWFRM::XOptionsClick ()       { ShowOptions(false); }
void __fastcall TDBWFRM::XOptionsGlobalClick () { ShowOptions(true);  }

void __fastcall TDBWFRM::ShowOptions (bool _global)
{
	XOptionsDialog frm(this);

	/*  Ausmasse */
	frm.upSchaefte   ->setValue(Data->MAXY1);
	frm.upTritte     ->setValue(Data->MAXX2);
	frm.upKette      ->setValue(Data->MAXX1);
	frm.upSchuesse   ->setValue(Data->MAXY2);
	frm.upSchaefteVis->setValue(hvisible);
	frm.upTritteVis  ->setValue(wvisible);
	/*  Schenien */
	frm.SchenienHorz->setValue(einzug.pos.strongline_x);
	frm.SchenienVert->setValue(einzug.pos.strongline_y);
	/*  Einstellungen */
	if (trittfolge.einzeltritt) frm.Einzeltritt->setChecked(true);
	else                        frm.Multitritt ->setChecked(true);
	if (sinkingshed)            frm.SinkingShed->setChecked(true);
	else                        frm.RisingShed ->setChecked(true);
	/*  Symbole */
	frm.cbEinzug       ->setCurrentIndex(einzug.darstellung);
	frm.cbAufknuepfung ->setCurrentIndex(aufknuepfung.darstellung);
	frm.cbTrittfolge   ->setCurrentIndex(trittfolge.darstellung);
	frm.cbSchlagpatrone->setCurrentIndex(schlagpatronendarstellung);
	frm.cbAushebung    ->setCurrentIndex(darst_aushebung);
	frm.cbAnbindung    ->setCurrentIndex(darst_anbindung);
	frm.cbAbbindung    ->setCurrentIndex(darst_abbindung);
	/*  Ansicht */
	frm.EinzugUnten     ->setChecked(einzugunten);
	frm.RightToLeft     ->setChecked(righttoleft);
	frm.TopToBottom     ->setChecked(toptobottom);
	frm.FEWithRaster    ->setChecked(fewithraster);
	frm.AltLiftplanstyle->setChecked(aufknuepfung.pegplanstyle);
	frm.AltFarbpalette  ->setChecked(palette2);

	if (cursorhandler) cursorhandler->DisableCursor();
	if (frm.exec() != QDialog::Accepted) {
		if (cursorhandler) cursorhandler->EnableCursor();
		return;
	}

	Settings settings;

	/*  Ausmasse */
	const int x1 = frm.upKette      ->value();
	const int y1 = frm.upSchaefte   ->value();
	const int x2 = frm.upTritte     ->value();
	const int y2 = frm.upSchuesse   ->value();
	const int vy1 = frm.upSchaefteVis->value();
	const int vx2 = frm.upTritteVis  ->value();
	SetAusmasse(x1, y1, x2, y2, vx2, vy1);
	if (_global) {
		settings.SetCategory(AnsiString("Size"));
		settings.Save(AnsiString("ShaftsVisible"),   hvisible);
		settings.Save(AnsiString("TreadlesVisible"), wvisible);
	}

	/*  Schenien */
	einzug.pos.strongline_x = aufknuepfung.pos.strongline_x =
	    trittfolge.pos.strongline_x = gewebe.pos.strongline_x =
	    frm.SchenienHorz->value();
	einzug.pos.strongline_y = aufknuepfung.pos.strongline_y =
	    trittfolge.pos.strongline_y = gewebe.pos.strongline_y =
	    frm.SchenienVert->value();
	if (_global) {
		settings.SetCategory(AnsiString("Grid"));
		settings.Save(AnsiString("Horizontal"), einzug.pos.strongline_x);
		settings.Save(AnsiString("Vertical"),   einzug.pos.strongline_y);
	}

	/*  Einstellungen */
	trittfolge.einzeltritt = frm.Einzeltritt->isChecked();
	sinkingshed            = frm.SinkingShed->isChecked();
	if (_global) {
		settings.SetCategory(AnsiString("Settings"));
		settings.Save(AnsiString("SingleTreadle"), trittfolge.einzeltritt ? 1 : 0);
		settings.Save(AnsiString("SinkingShed"),   sinkingshed ? 1 : 0);
	}

	/*  Symbole */
	einzug.darstellung        = (DARSTELLUNG)frm.cbEinzug       ->currentIndex();
	aufknuepfung.darstellung  = (DARSTELLUNG)frm.cbAufknuepfung ->currentIndex();
	trittfolge.darstellung    = (DARSTELLUNG)frm.cbTrittfolge   ->currentIndex();
	schlagpatronendarstellung = (DARSTELLUNG)frm.cbSchlagpatrone->currentIndex();
	darst_aushebung           = (DARSTELLUNG)frm.cbAushebung    ->currentIndex();
	darst_anbindung           = (DARSTELLUNG)frm.cbAnbindung    ->currentIndex();
	darst_abbindung           = (DARSTELLUNG)frm.cbAbbindung    ->currentIndex();
	if (_global) {
		settings.SetCategory(AnsiString("Display"));
		settings.Save(AnsiString("Threading"), (int)einzug.darstellung);
		settings.Save(AnsiString("Tie-up"),    (int)aufknuepfung.darstellung);
		settings.Save(AnsiString("Treadling"), (int)trittfolge.darstellung);
		settings.Save(AnsiString("Pegplan"),   (int)schlagpatronendarstellung);
		settings.Save(AnsiString("LiftOut"),   (int)darst_aushebung);
		settings.Save(AnsiString("Binding"),   (int)darst_anbindung);
		settings.Save(AnsiString("Unbinding"), (int)darst_abbindung);
	}

	/*  Ansicht */
	einzugunten  = frm.EinzugUnten ->isChecked();
	righttoleft  = frm.RightToLeft ->isChecked();
	toptobottom  = frm.TopToBottom ->isChecked();
	fewithraster = frm.FEWithRaster->isChecked();
	const bool pal2 = frm.AltFarbpalette->isChecked();
	if (pal2 != palette2) {
		palette2 = pal2;
		if (Data && Data->palette) Data->palette->SetPaletteType(palette2);
	}
	aufknuepfung.pegplanstyle = frm.AltLiftplanstyle->isChecked();
	if (_global) {
		settings.SetCategory(AnsiString("View"));
		settings.Save(AnsiString("ThreadingDown"),   einzugunten ? 1 : 0);
		settings.Save(AnsiString("RightToLeft"),     righttoleft ? 1 : 0);
		settings.Save(AnsiString("TopToBottom"),     toptobottom ? 1 : 0);
		settings.Save(AnsiString("ColorWithRaster"), fewithraster ? 1 : 0);
		settings.Save(AnsiString("AltColorpalette"), palette2 ? 1 : 0);
		settings.Save(AnsiString("AltPegplan"),      aufknuepfung.pegplanstyle ? 1 : 0);
	}

	if (pattern_canvas) pattern_canvas->recomputeLayout();
	SetModified();
	refresh();
	if (undo) undo->Snapshot();
	if (cursorhandler) cursorhandler->EnableCursor();
}
