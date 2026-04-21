/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "verhaeltnisdialog.h"
#include "language.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

VerhaeltnisDialog::VerhaeltnisDialog(QWidget* _parent, double _faktor_kette, double _faktor_schuss)
    : QDialog(_parent)
{
    setWindowTitle(LANG_STR("Weft/warp ratio", "Schuss/Kett-Verhältnis"));
    setModal(true);

    auto makeSpin = [this](double v) {
        auto* s = new QDoubleSpinBox(this);
        s->setDecimals(2);
        s->setRange(0.01, 10.0);
        s->setSingleStep(0.1);
        s->setValue(v);
        return s;
    };

    sbKette = makeSpin(_faktor_kette);
    sbSchuss = makeSpin(_faktor_schuss);

    auto* form = new QFormLayout;
    form->addRow(new QLabel(LANG_STR("&Warp factor:", "Faktor &Kette:"), this), sbKette);
    form->addRow(new QLabel(LANG_STR("W&eft factor:", "Faktor &Schuss:"), this), sbSchuss);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(btns);

    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    sbKette->setFocus();
    sbKette->selectAll();
}

double VerhaeltnisDialog::faktorKette() const { return sbKette->value(); }
double VerhaeltnisDialog::faktorSchuss() const { return sbSchuss->value(); }
