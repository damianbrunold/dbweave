/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "rapportdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

RapportDialog::RapportDialog (QWidget* _parent)
	: QDialog(_parent)
{
	setWindowTitle(QStringLiteral("Repeat"));
	setModal(true);

	rappHorz   = new QLineEdit(this);
	rappVert   = new QLineEdit(this);
	rappAll    = new QCheckBox(QStringLiteral("Repeat &all"),    this);
	rappColors = new QCheckBox(QStringLiteral("Repeat &colors"), this);

	rappHorz->setValidator(new QIntValidator(0, 9999, rappHorz));
	rappVert->setValidator(new QIntValidator(0, 9999, rappVert));

	auto* form = new QFormLayout();
	form->addRow(QStringLiteral("&Horizontal:"), rappHorz);
	form->addRow(QStringLiteral("&Vertical:"),   rappVert);

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	auto* root = new QVBoxLayout(this);
	root->addLayout(form);
	root->addWidget(rappAll);
	root->addWidget(rappColors);
	root->addWidget(btns);

	connect(btns,     &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(btns,     &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(rappAll,  &QCheckBox::toggled,         this, [this](bool on) {
		/*  Legacy RappAllClick: "Repeat all" disables both numeric
		    fields so the user can't set a conflicting count. */
		rappHorz->setEnabled(!on);
		rappVert->setEnabled(!on);
	});
}

int RapportDialog::horz() const
{
	return rappHorz->text().toInt();
}

int RapportDialog::vert() const
{
	return rappVert->text().toInt();
}

bool RapportDialog::repeatAll() const   { return rappAll->isChecked();    }
bool RapportDialog::repeatColors() const { return rappColors->isChecked(); }

void RapportDialog::setHorz (int _n)
{
	rappHorz->setText(_n > 0 ? QString::number(_n) : QString());
}

void RapportDialog::setVert (int _n)
{
	rappVert->setText(_n > 0 ? QString::number(_n) : QString());
}

void RapportDialog::setRepeatAll (bool _on)
{
	rappAll->setChecked(_on);
	rappHorz->setEnabled(!_on);
	rappVert->setEnabled(!_on);
}

void RapportDialog::setRepeatColors (bool _on)
{
	rappColors->setChecked(_on);
}
