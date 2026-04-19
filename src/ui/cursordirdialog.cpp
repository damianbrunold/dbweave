/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "cursordirdialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QMouseEvent>
#include <QToolButton>
#include <QVBoxLayout>

CursorDirDialog::CursorDirDialog(QWidget* _parent, CURSORDIRECTION _cd)
    : QDialog(_parent)
    , cursordirection(_cd)
{
    setWindowTitle(QStringLiteral("Cursor movement"));
    setModal(true);

    auto makeBtn = [this](const QString& label) {
        auto* b = new QToolButton(this);
        b->setText(label);
        b->setCheckable(true);
        b->setAutoRaise(false);
        b->setMinimumSize(48, 48);
        return b;
    };
    cdUp = makeBtn(QStringLiteral("\xe2\x86\x91"));    /* ↑ */
    cdDown = makeBtn(QStringLiteral("\xe2\x86\x93"));  /* ↓ */
    cdLeft = makeBtn(QStringLiteral("\xe2\x86\x90"));  /* ← */
    cdRight = makeBtn(QStringLiteral("\xe2\x86\x92")); /* → */

    cdUp->setChecked((_cd & CD_UP) != 0);
    cdDown->setChecked((_cd & CD_DOWN) != 0);
    cdLeft->setChecked((_cd & CD_LEFT) != 0);
    cdRight->setChecked((_cd & CD_RIGHT) != 0);

    /*  Cross-shape grid so "click between two buttons" makes sense. */
    auto* grid = new QGridLayout();
    grid->addWidget(cdUp, 0, 1);
    grid->addWidget(cdLeft, 1, 0);
    grid->addWidget(cdRight, 1, 2);
    grid->addWidget(cdDown, 2, 1);
    grid->setSpacing(4);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* root = new QVBoxLayout(this);
    root->addLayout(grid);
    root->addWidget(btns);

    connect(btns, &QDialogButtonBox::accepted, this, [this] {
        readFromButtons();
        accept();
    });
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    /*  Alt-click on an axis button un-presses the opposite one
        (legacy cdUpClick / cdDownClick / cdLeftClick / cdRightClick).
        Bare clicks just toggle that single button. */
    auto wireAlt = [this](QToolButton* self, QToolButton* opp) {
        connect(self, &QToolButton::clicked, this, [self, opp] {
            if (QApplication::keyboardModifiers() & Qt::AltModifier)
                opp->setChecked(false);
        });
    };
    wireAlt(cdUp, cdDown);
    wireAlt(cdDown, cdUp);
    wireAlt(cdLeft, cdRight);
    wireAlt(cdRight, cdLeft);
}

void CursorDirDialog::readFromButtons()
{
    cursordirection = CD_NONE;
    if (cdUp->isChecked())
        cursordirection |= CD_UP;
    if (cdDown->isChecked())
        cursordirection |= CD_DOWN;
    if (cdLeft->isChecked())
        cursordirection |= CD_LEFT;
    if (cdRight->isChecked())
        cursordirection |= CD_RIGHT;
}

/*  Diagonal-click: release in the gap between two adjacent buttons
    activates both. Port of legacy TCursorDirForm::FormMouseUp.    */
void CursorDirDialog::mouseReleaseEvent(QMouseEvent* _e)
{
    const int x = _e->position().toPoint().x();
    const int y = _e->position().toPoint().y();
    auto inside = [&](const QToolButton* b, bool xAxis) {
        if (!b)
            return false;
        const QRect r = b->geometry();
        if (xAxis)
            return x > r.left() && x < r.right();
        return y > r.top() && y < r.bottom();
    };

    CURSORDIRECTION cd = CD_NONE;
    if (inside(cdLeft, true))
        cd |= CD_LEFT;
    if (inside(cdRight, true))
        cd |= CD_RIGHT;
    if (inside(cdUp, false))
        cd |= CD_UP;
    if (inside(cdDown, false))
        cd |= CD_DOWN;

    /*  The release is only a "diagonal" when it hits more than one
        pair (or the centre). Ignore clicks that land on a single
        button -- Qt's own QAbstractButton::clicked path handles
        those. */
    const bool singleAxis = (cd == CD_LEFT || cd == CD_RIGHT || cd == CD_UP || cd == CD_DOWN);
    const QRect upRect = cdUp ? cdUp->geometry() : QRect();
    const QRect leftRect = cdLeft ? cdLeft->geometry() : QRect();
    const bool overCentre
        = (x > upRect.left() && x < upRect.right() && y > leftRect.top() && y < leftRect.bottom());

    if ((cd != CD_NONE && !singleAxis) || overCentre) {
        cdUp->setChecked(false);
        cdDown->setChecked(false);
        cdLeft->setChecked(false);
        cdRight->setChecked(false);
    }
    if (cd != CD_NONE && !singleAxis) {
        if (cd & CD_UP)
            cdUp->setChecked(true);
        if (cd & CD_DOWN)
            cdDown->setChecked(true);
        if (cd & CD_LEFT)
            cdLeft->setChecked(true);
        if (cd & CD_RIGHT)
            cdRight->setChecked(true);
    }

    QDialog::mouseReleaseEvent(_e);
}
