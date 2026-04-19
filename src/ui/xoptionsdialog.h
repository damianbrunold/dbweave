/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy/xoptions_form.cpp (TXOptionsForm). Tabbed
    modal exposing per-document view / symbol / setting / grid /
    size preferences. Driven by TDBWFRM::XOptionsClick (apply to
    the current document only) and XOptionsGlobalClick (also
    persists via Settings for new documents).
*/

#ifndef DBWEAVE_UI_XOPTIONSDIALOG_H
#define DBWEAVE_UI_XOPTIONSDIALOG_H

#include <QDialog>

class QCheckBox;
class QComboBox;
class QRadioButton;
class QSpinBox;
class QTabWidget;

class XOptionsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit XOptionsDialog (QWidget* _parent = nullptr);

	/*  Public accessors used by TDBWFRM::ShowOptions to pull the
	    dialog state back after OK. Organised by tab.           */

	/* Sizes tab */
	QSpinBox*     upSchaefte    = nullptr;
	QSpinBox*     upTritte      = nullptr;
	QSpinBox*     upKette       = nullptr;
	QSpinBox*     upSchuesse    = nullptr;
	QSpinBox*     upSchaefteVis = nullptr;
	QSpinBox*     upTritteVis   = nullptr;

	/* Grid tab */
	QSpinBox*     SchenienHorz  = nullptr;
	QSpinBox*     SchenienVert  = nullptr;

	/* Settings tab */
	QRadioButton* Einzeltritt   = nullptr;
	QRadioButton* Multitritt    = nullptr;
	QRadioButton* RisingShed    = nullptr;
	QRadioButton* SinkingShed   = nullptr;

	/* Symbols tab */
	QComboBox*    cbEinzug        = nullptr;
	QComboBox*    cbAufknuepfung  = nullptr;
	QComboBox*    cbTrittfolge    = nullptr;
	QComboBox*    cbSchlagpatrone = nullptr;
	QComboBox*    cbAushebung     = nullptr;
	QComboBox*    cbAnbindung     = nullptr;
	QComboBox*    cbAbbindung     = nullptr;

	/* View tab */
	QCheckBox*    EinzugUnten     = nullptr;
	QCheckBox*    RightToLeft     = nullptr;
	QCheckBox*    TopToBottom     = nullptr;
	QCheckBox*    FEWithRaster    = nullptr;
	QCheckBox*    AltFarbpalette  = nullptr;
	QCheckBox*    AltLiftplanstyle = nullptr;

private:
	QTabWidget* tabs = nullptr;

	void loadCombo (QComboBox* _cb, bool _withNumber = false);
};

#endif
