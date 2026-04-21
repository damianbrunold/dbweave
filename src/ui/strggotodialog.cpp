/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "strggotodialog.h"

#include "datamodule.h"
#include "language.h"
#include "steuerung.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

StrgGotoDialog::StrgGotoDialog(TSTRGFRM* _frm)
    : QDialog(_frm)
    , frm(_frm)
{
    setWindowTitle(LANG_STR("Set current weave position", "Aktuelle Webposition festlegen"));
    setModal(true);

    sbSchuss = new QSpinBox(this);
    sbSchuss->setRange(1, Data ? Data->MAXY2 : 10000);
    sbSchuss->setValue(frm ? frm->weave_position + 1 : 1);

    cbKlammer = new QComboBox(this);
    for (int i = 0; i < TSTRGFRM::MAXKLAMMERN; i++) {
        cbKlammer->addItem(QString::number(i + 1));
        if (frm && frm->klammern[i].repetitions == 0)
            cbKlammer->setItemData(i, false, Qt::UserRole - 1); /* grey-out */
    }
    cbKlammer->setCurrentIndex(frm ? frm->weave_klammer : 0);

    cbRepeat = new QComboBox(this);
    refreshRepeatOptions();
    cbRepeat->setCurrentIndex(frm ? std::max(0, frm->weave_repetition - 1) : 0);

    connect(cbKlammer, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int) { refreshRepeatOptions(); });

    auto* form = new QFormLayout;
    form->addRow(LANG_STR("&Pick:", "&Schuss:"), sbSchuss);
    form->addRow(LANG_STR("&Brace:", "&Klammer:"), cbKlammer);
    form->addRow(LANG_STR("&Repetition:", "&Wiederholung:"), cbRepeat);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(btns);

    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void StrgGotoDialog::refreshRepeatOptions()
{
    const int k = cbKlammer ? cbKlammer->currentIndex() : 0;
    const int reps = (frm && k >= 0 && k < TSTRGFRM::MAXKLAMMERN) ? frm->klammern[k].repetitions
                                                                  : 1;
    const int want = std::max(1, reps);
    const int prev = cbRepeat->currentIndex();
    cbRepeat->blockSignals(true);
    cbRepeat->clear();
    for (int i = 0; i < want; i++)
        cbRepeat->addItem(QString::number(i + 1));
    cbRepeat->setCurrentIndex(std::min(prev, want - 1));
    cbRepeat->blockSignals(false);
}

int StrgGotoDialog::schuss() const { return sbSchuss->value() - 1; }
int StrgGotoDialog::klammer() const { return cbKlammer->currentIndex(); }
int StrgGotoDialog::repetition() const { return cbRepeat->currentIndex() + 1; }
