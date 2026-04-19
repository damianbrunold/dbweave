/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "einzugassistentdialog.h"

#include "mainwindow.h"
#include "datamodule.h"
#include "undoredo.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

#include <cstdlib>

EinzugassistentDialog::EinzugassistentDialog(TDBWFRM* _frm, QWidget* _parent)
    : QDialog(_parent)
    , frm(_frm)
{
    setWindowTitle(QStringLiteral("Threading wizard"));
    setModal(true);

    tabs = new QTabWidget(this);

    /*  --- Geradedurch (straight-through) page ------------------ */
    {
        auto* page = new QWidget(this);
        gdFirstKettfaden = new QSpinBox(page);
        gdFirstSchaft = new QSpinBox(page);
        gdSchaefte = new QSpinBox(page);
        gdFirstKettfaden->setRange(1, Data->MAXX1);
        gdFirstSchaft->setRange(1, Data->MAXY1);
        gdSchaefte->setRange(1, Data->MAXY1);
        gdFirstKettfaden->setValue(1);
        gdFirstSchaft->setValue(1);
        gdSchaefte->setValue(4);

        gdZ = new QRadioButton(QStringLiteral("&Z"), page);
        gdS = new QRadioButton(QStringLiteral("&S"), page);
        gdZ->setChecked(true);
        auto* gb = new QGroupBox(QStringLiteral("Orientation"), page);
        auto* gbl = new QVBoxLayout(gb);
        gbl->addWidget(gdZ);
        gbl->addWidget(gdS);

        auto* form = new QFormLayout();
        form->addRow(QStringLiteral("&First warp thread:"), gdFirstKettfaden);
        form->addRow(QStringLiteral("First &shaft:"), gdFirstSchaft);
        form->addRow(QStringLiteral("&Number of shafts:"), gdSchaefte);

        auto* v = new QVBoxLayout(page);
        v->addLayout(form);
        v->addWidget(gb);
        tabs->addTab(page, QStringLiteral("Straight through"));
    }

    /*  --- Abgesetzt (stepped / broken) page ------------------- */
    {
        auto* page = new QWidget(this);
        abFirstKettfaden = new QSpinBox(page);
        abFirstSchaft = new QSpinBox(page);
        abSchaefte = new QSpinBox(page);
        abGratlen = new QSpinBox(page);
        abVersatz = new QSpinBox(page);
        abFirstKettfaden->setRange(1, Data->MAXX1);
        abFirstSchaft->setRange(1, Data->MAXY1);
        abSchaefte->setRange(1, Data->MAXY1);
        abGratlen->setRange(1, Data->MAXY1);
        abVersatz->setRange(-Data->MAXY1, Data->MAXY1);
        abFirstKettfaden->setValue(1);
        abFirstSchaft->setValue(1);
        abSchaefte->setValue(4);
        abGratlen->setValue(4);
        abVersatz->setValue(2);

        auto* form = new QFormLayout(page);
        form->addRow(QStringLiteral("First &warp thread:"), abFirstKettfaden);
        form->addRow(QStringLiteral("First &shaft:"), abFirstSchaft);
        form->addRow(QStringLiteral("&Number of shafts:"), abSchaefte);
        form->addRow(QStringLiteral("&Run length (Gratlänge):"), abGratlen);
        form->addRow(QStringLiteral("&Offset (Versatz):"), abVersatz);
        tabs->addTab(page, QStringLiteral("Stepped"));
    }

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &EinzugassistentDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->addWidget(tabs);
    root->addWidget(btns);

    tabs->setCurrentIndex(0);
}

/*-----------------------------------------------------------------*/
void EinzugassistentDialog::recalc()
{
    frm->RecalcGewebe();
    frm->CalcRangeSchuesse();
    frm->CalcRangeKette();
    frm->RecalcFreieSchaefte();
    frm->UpdateRapport();
    frm->update();
    frm->SetModified();
    /*  undo snapshot is done by TDBWFRM::EinzugAssistentClick after
        the dialog accepts, matching legacy control flow. */
}

/*-----------------------------------------------------------------*/
void EinzugassistentDialog::createGerade(int _firstKettfaden, int _firstSchaft, int _schaefte,
                                         bool _steigend)
{
    if (_steigend) {
        for (int i = _firstKettfaden - 1; i < _firstKettfaden - 1 + _schaefte; i++) {
            short s = short(_firstSchaft + (i - (_firstKettfaden - 1)));
            if (s >= Data->MAXY1)
                frm->ExtendSchaefte();
            if (i >= Data->MAXX1)
                break;
            frm->einzug.feld.Set(i, s);
        }
    } else {
        for (int i = _firstKettfaden - 1; i < _firstKettfaden - 1 + _schaefte; i++) {
            short s = short(_firstSchaft + (_firstKettfaden - 1 + _schaefte - 1)
                            - (i - (_firstKettfaden - 1)));
            if (s >= Data->MAXY1)
                frm->ExtendSchaefte();
            if (i >= Data->MAXX1)
                break;
            frm->einzug.feld.Set(i, s);
        }
    }
    recalc();
}

/*-----------------------------------------------------------------*/
void EinzugassistentDialog::createAbgesetzt(int _firstKettfaden, int _firstSchaft, int _schaefte,
                                            int _gratlen, int _versatz)
{
    int i = _firstKettfaden - 1;
    int j = _firstSchaft - 1;
    while (true) {
        for (int ii = 0; ii < _gratlen; ii++) {
            short s = short(j + ii + 1);
            if (s - _firstSchaft >= _schaefte)
                goto g_break;
            if (s <= 0)
                goto g_break;
            if (i + ii >= Data->MAXX1)
                goto g_break;
            frm->einzug.feld.Set(i + ii, s);
            if (s - _firstSchaft + 1 == _schaefte)
                goto g_break;
        }
        i += _gratlen;
        j += _versatz;
    }
g_break:
    recalc();
}

/*-----------------------------------------------------------------*/
void EinzugassistentDialog::accept()
{
    if (tabs->currentIndex() == 0) {
        const int firstkf = gdFirstKettfaden->value();
        const int firstsch = gdFirstSchaft->value();
        const int schaefte = gdSchaefte->value();
        createGerade(firstkf, firstsch, schaefte, gdZ->isChecked());
    } else {
        const int firstkf = abFirstKettfaden->value();
        const int firstsch = abFirstSchaft->value();
        const int schaefte = abSchaefte->value();
        const int glen = abGratlen->value();
        const int versatz = abVersatz->value();
        if (glen < 1 || glen > schaefte) {
            QMessageBox::information(
                this, QStringLiteral("DB-WEAVE"),
                QStringLiteral("Run length must be between 1 and the number of shafts."));
            return;
        }
        if (std::abs(versatz) < 1 || std::abs(versatz) > schaefte) {
            QMessageBox::information(
                this, QStringLiteral("DB-WEAVE"),
                QStringLiteral("Offset must be between 1 and the number of shafts."));
            return;
        }
        createAbgesetzt(firstkf, firstsch, schaefte, glen, versatz);
    }
    QDialog::accept();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::EinzugAssistentClick()
{
    EinzugassistentDialog dlg(this, this);
    if (dlg.exec() == QDialog::Accepted) {
        if (undo)
            undo->Snapshot();
    }
}
