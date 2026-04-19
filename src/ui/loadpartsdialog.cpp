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
#include "language.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QVBoxLayout>

LoadPartsDialog::LoadPartsDialog(QWidget* _parent)
    : QDialog(_parent)
{
    setWindowTitle(LANG_STR("Load parts", "Teile laden"));
    setModal(true);

    auto* gb = new QGroupBox(LANG_STR("Which parts should be loaded?",
                                      "Welche Teile sollen geladen werden?"),
                             this);
    auto* gblay = new QVBoxLayout(gb);

    auto mk = [this, gblay](const QString& label) {
        auto* c = new QCheckBox(label, this);
        gblay->addWidget(c);
        return c;
    };
    einzug = mk(LANG_STR("&Threading", "&Einzug"));
    trittfolge = mk(LANG_STR("Tre&adling", "&Trittfolge"));
    aufknuepfung = mk(LANG_STR("Tie-&up", "A&ufknüpfung"));
    kettfarben = mk(LANG_STR("&Warp colors", "&Kettfarben"));
    schussfarben = mk(LANG_STR("We&ft colors", "Sch&ussfarben"));
    blatteinzug = mk(LANG_STR("&Reed threading", "&Blatteinzug"));
    hilfslinien = mk(LANG_STR("&Support lines", "&Hilfslinien"));
    farbpalette = mk(LANG_STR("&Color palette", "Farb&palette"));
    benedefeinzug = mk(LANG_STR("&Userdef. threading", "Ben&utzerdef. Einzug"));
    blockmuster = mk(LANG_STR("&Block patterns", "Bloc&kmuster"));
    bereichmuster = mk(LANG_STR("Ra&nge patterns", "Bereic&hmuster"));

    alle = new QCheckBox(LANG_STR("&All", "&Alle"), this);

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
        QCheckBox* cb[]
            = { einzug,      trittfolge,  aufknuepfung,  kettfarben,  schussfarben, blatteinzug,
                hilfslinien, farbpalette, benedefeinzug, blockmuster, bereichmuster };
        for (QCheckBox* c : cb) {
            c->setEnabled(!on);
            c->setChecked(on);
        }
    });
}

LOADPARTS LoadPartsDialog::getLoadParts() const
{
    if (alle->isChecked())
        return LOADALL;
    LOADPARTS p = 0;
    if (einzug->isChecked())
        p |= LOADEINZUG;
    if (trittfolge->isChecked())
        p |= LOADTRITTFOLGE;
    if (aufknuepfung->isChecked())
        p |= LOADAUFKNUEPFUNG;
    if (kettfarben->isChecked())
        p |= LOADKETTFARBEN;
    if (schussfarben->isChecked())
        p |= LOADSCHUSSFARBEN;
    if (blatteinzug->isChecked())
        p |= LOADBLATTEINZUG;
    if (hilfslinien->isChecked())
        p |= LOADHILFSLINIEN;
    if (farbpalette->isChecked())
        p |= LOADPALETTE;
    if (benedefeinzug->isChecked())
        p |= LOADFIXEINZUG;
    if (blockmuster->isChecked())
        p |= LOADBLOCKMUSTER;
    if (bereichmuster->isChecked())
        p |= LOADBEREICHMUSTER;
    return p;
}
