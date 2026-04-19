/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "entervvdialog.h"
#include "language.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

EnterVVDialog::EnterVVDialog(QWidget* _parent)
    : QDialog(_parent)
{
    setWindowTitle(LANG_STR("Thread ratio", "Fadenverhältnis"));
    setModal(true);

    /*  Six integer slots laid out as
            [VV1] : [VV2] : [VV3] : [VV4] : [VV5] : [VV6]
        Only VV1 is enabled initially; the rest enable as the user
        fills in previous slots.                                  */
    auto* row = new QHBoxLayout();
    for (int i = 0; i < 6; i++) {
        vv[i] = new QLineEdit(this);
        vv[i]->setValidator(new QIntValidator(0, 999, vv[i]));
        vv[i]->setFixedWidth(48);
        if (i > 0)
            vv[i]->setEnabled(false);
        row->addWidget(vv[i]);
        if (i < 5)
            row->addWidget(new QLabel(QStringLiteral(":"), this));
        connect(vv[i], &QLineEdit::textChanged, this, [this, i](const QString&) { onChange(i); });
    }

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* root = new QVBoxLayout(this);
    root->addWidget(
        new QLabel(LANG_STR("&Enter the thread ratio:", "&Fadenverhältnis eingeben:"), this));
    root->addLayout(row);
    root->addWidget(btns);

    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void EnterVVDialog::onChange(int _slot)
{
    const int v = value(_slot);
    if (v == 0) {
        /*  Clear and disable every slot after this one. */
        for (int k = _slot + 1; k < 6; k++) {
            vv[k]->clear();
            vv[k]->setEnabled(false);
        }
    } else if (_slot + 1 < 6) {
        vv[_slot + 1]->setEnabled(true);
    }
}

int EnterVVDialog::value(int _slot) const
{
    if (_slot < 0 || _slot >= 6)
        return 0;
    return vv[_slot]->text().toInt();
}
