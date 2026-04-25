/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "userdefdialogs.h"
#include "language.h"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>

/*-----------------------------------------------------------------*/
UserdefEnterNameDialog::UserdefEnterNameDialog(const QString& _default, QWidget* _parent)
    : QDialog(_parent)
{
    setWindowTitle(LANG_STR("Pattern name", "Mustername"));
    setModal(true);

    edName = new QLineEdit(this);
    edName->setText(_default);
    edName->selectAll();

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    auto* label = new QLabel(LANG_STR("&Enter the pattern name:", "&Mustername eingeben:"), this);
    label->setBuddy(edName);
    root->addWidget(label);
    root->addWidget(edName);
    root->addWidget(btns);

    resize(320, 100);
}

QString UserdefEnterNameDialog::name() const
{
    return edName->text();
}

QString getUserdefName(QWidget* _parent, const QString& _default)
{
    UserdefEnterNameDialog dlg(_default, _parent);
    if (dlg.exec() != QDialog::Accepted)
        return QString();
    return dlg.name();
}

/*-----------------------------------------------------------------*/
UserdefSelectDialog::UserdefSelectDialog(const UserdefPattern _slots[MAXUSERDEF],
                                         const QString& _title, QWidget* _parent)
    : QDialog(_parent)
{
    setWindowTitle(_title.isEmpty() ? LANG_STR("Select pattern", "Muster auswählen") : _title);
    setModal(true);

    auto* root = new QVBoxLayout(this);
    auto* group = new QButtonGroup(this);

    for (int i = 0; i < MAXUSERDEF; i++) {
        const QString mnemonic = (i < 9) ? QStringLiteral("&%1").arg(i + 1) : QStringLiteral("1&0");
        const QString desc = _slots[i].description.isEmpty()
                                 ? LANG_STR("<free>", "<frei>")
                                 : _slots[i].description;
        radios[i] = new QRadioButton(QStringLiteral("%1 - %2").arg(mnemonic, desc), this);
        group->addButton(radios[i], i);
        root->addWidget(radios[i]);
    }
    radios[0]->setChecked(true);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(btns);
}

int UserdefSelectDialog::selectedIndex() const
{
    for (int i = 0; i < MAXUSERDEF; i++)
        if (radios[i]->isChecked())
            return i;
    return -1;
}
