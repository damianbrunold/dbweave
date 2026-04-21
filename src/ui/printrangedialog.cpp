/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "printrangedialog.h"
#include "language.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

static QGroupBox* makeRangeGroup(QWidget* _parent, const QString& _title, QSpinBox*& _von,
                                 QSpinBox*& _bis, int _max, const SZ& _initial, bool& _skip)
{
    auto* gb = new QGroupBox(_title, _parent);
    auto* form = new QFormLayout(gb);
    _von = new QSpinBox(gb);
    _bis = new QSpinBox(gb);
    _von->setRange(1, std::max(_max, 1));
    _bis->setRange(1, std::max(_max, 1));
    const QString lblFrom = LANG_STR("From", "Von");
    const QString lblTo = LANG_STR("To", "Bis");
    form->addRow(lblFrom, _von);
    form->addRow(lblTo, _bis);

    if (_initial.a < 0 || _initial.b < 0) {
        _skip = true;
        gb->setEnabled(false);
    } else {
        _von->setValue(_initial.a + 1);
        _bis->setValue(_initial.b + 1);
    }
    return gb;
}

PrintRangeDialog::PrintRangeDialog(QWidget* _parent, const SZ& _kette, int _maxKette,
                                   const SZ& _schuesse, int _maxSchuesse, const SZ& _schaefte,
                                   int _maxSchaefte, const SZ& _tritte, int _maxTritte)
    : QDialog(_parent)
{
    setWindowTitle(LANG_STR("Print part", "Teil drucken"));
    setModal(true);

    bool skipKette = false, skipSchuesse = false;
    auto* gbKette = makeRangeGroup(this, LANG_STR("&Warp range", "&Kettfadenbereich"), ketteVon,
                                   ketteBis, _maxKette, _kette, skipKette);
    auto* gbSchuesse = makeRangeGroup(this, LANG_STR("We&ft range", "&Schussfadenbereich"),
                                      schussVon, schussBis, _maxSchuesse, _schuesse, skipSchuesse);
    auto* gbSchaefte = makeRangeGroup(this, LANG_STR("&Harness range", "S&chaftbereich"),
                                      schaftVon, schaftBis, _maxSchaefte, _schaefte, skipSchaefte);
    auto* gbTritte = makeRangeGroup(this, LANG_STR("&Treadle range", "&Trittbereich"), trittVon,
                                    trittBis, _maxTritte, _tritte, skipTritte);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* root = new QVBoxLayout(this);
    root->addWidget(gbKette);
    root->addWidget(gbSchuesse);
    root->addWidget(gbSchaefte);
    root->addWidget(gbTritte);
    root->addWidget(btns);

    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

static SZ readRange(QSpinBox* _von, QSpinBox* _bis)
{
    SZ r;
    r.a = _von->value() - 1;
    r.b = _bis->value() - 1;
    if (r.b < r.a)
        std::swap(r.a, r.b);
    return r;
}

SZ PrintRangeDialog::kette() const { return readRange(ketteVon, ketteBis); }
SZ PrintRangeDialog::schuesse() const { return readRange(schussVon, schussBis); }

SZ PrintRangeDialog::schaefte() const
{
    if (skipSchaefte)
        return SZ(-1, -1);
    return readRange(schaftVon, schaftBis);
}

SZ PrintRangeDialog::tritte() const
{
    if (skipTritte)
        return SZ(-1, -1);
    return readRange(trittVon, trittBis);
}
