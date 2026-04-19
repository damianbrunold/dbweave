/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "loadpartsdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QVBoxLayout>

LoadPartsDialog::LoadPartsDialog (QWidget* _parent)
	: QDialog(_parent)
{
	setWindowTitle(QStringLiteral("Load parts"));
	setModal(true);

	auto* gb = new QGroupBox(QStringLiteral("Which parts should be loaded?"), this);
	auto* gblay = new QVBoxLayout(gb);

	auto mk = [this, gblay](const QString& label) {
		auto* c = new QCheckBox(label, this);
		gblay->addWidget(c);
		return c;
	};
	einzug        = mk(QStringLiteral("&Threading"));
	trittfolge    = mk(QStringLiteral("Tre&adling"));
	aufknuepfung  = mk(QStringLiteral("Tie-&up"));
	kettfarben    = mk(QStringLiteral("&Warp colors"));
	schussfarben  = mk(QStringLiteral("We&ft colors"));
	blatteinzug   = mk(QStringLiteral("&Reed threading"));
	hilfslinien   = mk(QStringLiteral("&Support lines"));
	farbpalette   = mk(QStringLiteral("&Color palette"));
	benedefeinzug = mk(QStringLiteral("&Userdef. threading"));
	blockmuster   = mk(QStringLiteral("&Block patterns"));
	bereichmuster = mk(QStringLiteral("Ra&nge patterns"));

	alle = new QCheckBox(QStringLiteral("&All"), this);

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	auto* root = new QVBoxLayout(this);
	root->addWidget(gb);
	root->addWidget(alle);
	root->addWidget(btns);

	connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

	/*  "All" toggles every other checkbox on AND disables them
	    (legacy AlleClick). Unchecking "All" re-enables them but
	    leaves them checked so the user can tweak from there.   */
	connect(alle, &QCheckBox::toggled, this, [this](bool on) {
		QCheckBox* cb[] = { einzug, trittfolge, aufknuepfung,
		                    kettfarben, schussfarben, blatteinzug,
		                    hilfslinien, farbpalette, benedefeinzug,
		                    blockmuster, bereichmuster };
		for (QCheckBox* c : cb) {
			c->setEnabled(!on);
			c->setChecked(on);
		}
	});
}

LOADPARTS LoadPartsDialog::getLoadParts() const
{
	if (alle->isChecked()) return LOADALL;
	LOADPARTS p = 0;
	if (einzug       ->isChecked()) p |= LOADEINZUG;
	if (trittfolge   ->isChecked()) p |= LOADTRITTFOLGE;
	if (aufknuepfung ->isChecked()) p |= LOADAUFKNUEPFUNG;
	if (kettfarben   ->isChecked()) p |= LOADKETTFARBEN;
	if (schussfarben ->isChecked()) p |= LOADSCHUSSFARBEN;
	if (blatteinzug  ->isChecked()) p |= LOADBLATTEINZUG;
	if (hilfslinien  ->isChecked()) p |= LOADHILFSLINIEN;
	if (farbpalette  ->isChecked()) p |= LOADPALETTE;
	if (benedefeinzug->isChecked()) p |= LOADFIXEINZUG;
	if (blockmuster  ->isChecked()) p |= LOADBLOCKMUSTER;
	if (bereichmuster->isChecked()) p |= LOADBEREICHMUSTER;
	return p;
}
