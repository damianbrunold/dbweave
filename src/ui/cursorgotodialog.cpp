/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "cursorgotodialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

CursorGotoDialog::CursorGotoDialog(QWidget* _parent, FELD _initial, bool _pegplan)
    : QDialog(_parent)
{
    setWindowTitle(QStringLiteral("Cursor Go to"));
    setModal(true);

    rBindung = new QRadioButton(QStringLiteral("&Pattern"), this);
    rEinzug = new QRadioButton(QStringLiteral("&Threading"), this);
    rTrittfolge = new QRadioButton(
        _pegplan ? QStringLiteral("Pegp&lan") : QStringLiteral("Tre&adling"), this);
    rAufknuepfung = new QRadioButton(QStringLiteral("Tie-&up"), this);
    rKettfarben = new QRadioButton(QStringLiteral("&Warp colors"), this);
    rSchussfarben = new QRadioButton(QStringLiteral("We&ft colors"), this);

    /*  Legacy: in pegplan (ViewSchlagpatrone) mode the
        aufknuepfung field is hidden, so the Goto button is
        disabled. */
    if (_pegplan)
        rAufknuepfung->setEnabled(false);

    switch (_initial) {
    case EINZUG:
        rEinzug->setChecked(true);
        break;
    case TRITTFOLGE:
        rTrittfolge->setChecked(true);
        break;
    case AUFKNUEPFUNG:
        rAufknuepfung->setChecked(true);
        break;
    case KETTFARBEN:
        rKettfarben->setChecked(true);
        break;
    case SCHUSSFARBEN:
        rSchussfarben->setChecked(true);
        break;
    case GEWEBE:
    default:
        rBindung->setChecked(true);
        break;
    }

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* root = new QVBoxLayout(this);
    root->addWidget(new QLabel(QStringLiteral("Choose field:"), this));
    root->addWidget(rBindung);
    root->addWidget(rEinzug);
    root->addWidget(rTrittfolge);
    root->addWidget(rAufknuepfung);
    root->addWidget(rKettfarben);
    root->addWidget(rSchussfarben);
    root->addWidget(btns);

    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

FELD CursorGotoDialog::selected() const
{
    if (rEinzug->isChecked())
        return EINZUG;
    if (rTrittfolge->isChecked())
        return TRITTFOLGE;
    if (rAufknuepfung->isChecked())
        return AUFKNUEPFUNG;
    if (rKettfarben->isChecked())
        return KETTFARBEN;
    if (rSchussfarben->isChecked())
        return SCHUSSFARBEN;
    return GEWEBE;
}
