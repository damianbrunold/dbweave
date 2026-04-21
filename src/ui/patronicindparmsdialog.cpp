/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "patronicindparmsdialog.h"

#include "language.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>

PatronicIndParmsDialog::PatronicIndParmsDialog(QWidget* _parent, int _initialCount)
    : QDialog(_parent)
{
    setWindowTitle(
        LANG_STR("ARM Patronic indirect parameter", "ARM Patronic Indirekt Parameter"));
    setModal(true);

    sbPos = new QSpinBox(this);
    sbPos->setRange(1, 220);
    sbPos->setValue(1);

    sbCount = new QSpinBox(this);
    sbCount->setRange(1, 220);
    sbCount->setValue(std::max(1, std::min(220, _initialCount)));

    auto* form = new QFormLayout;
    form->addRow(LANG_STR("&Starting Position:", "&Speichern ab Position:"), sbPos);
    form->addRow(LANG_STR("&Max. number of picks:", "&Maximale Anzahl Schüsse:"), sbCount);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(btns);

    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

int PatronicIndParmsDialog::startingPosition() const { return sbPos->value(); }
int PatronicIndParmsDialog::maxPicks() const { return sbCount->value(); }
