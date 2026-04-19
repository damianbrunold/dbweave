/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "loomoptionsdialog.h"
#include "language.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

/*-----------------------------------------------------------------*/
LoomOptionsDialog::LoomOptionsDialog(QWidget* _parent)
    : QDialog(_parent)
{
    setWindowTitle(LANG_STR("Loom options", "Websteuerungs-Optionen"));
    setModal(true);

    cbInterface = new QComboBox(this);
    /*  Indices match LOOMINTERFACE enum exactly (0 = dummy … 7 = AVL). */
    cbInterface->addItem(LANG_STR("Dummy (simulation)", "Dummy (Simulation)"));
    cbInterface->addItem(QStringLiteral("ARM Patronic direct"));
    cbInterface->addItem(QStringLiteral("ARM Patronic indirect"));
    cbInterface->addItem(QStringLiteral("ARM Designer electronic"));
    cbInterface->addItem(QStringLiteral("Varpapuu parallel"));
    cbInterface->addItem(QStringLiteral("Generic SLIPS"));
    cbInterface->addItem(QStringLiteral("Generic LIPS"));
    cbInterface->addItem(QStringLiteral("AVL Compu-Dobby III"));

    cbPort = new QComboBox(this);
    for (int i = 1; i <= 8; i++)
        cbPort->addItem(QStringLiteral("COM%1").arg(i));

    spDelay = new QSpinBox(this);
    spDelay->setRange(0, 10);
    spDelay->setSuffix(LANG_STR(" (Varpapuu only)", " (nur Varpapuu)"));

    auto* form = new QFormLayout();
    form->addRow(LANG_STR("&Loom:", "&Webstuhl:"), cbInterface);
    form->addRow(LANG_STR("&Port:", "&Anschluss:"), cbPort);
    form->addRow(LANG_STR("&Delay:", "&Verzögerung:"), spDelay);

#ifndef DBWEAVE_HAVE_LOOM
    auto* warn = new QLabel(
        LANG_STR("<i>Note: the executable was built without "
                 "QtSerialPort support. Only the Dummy loom "
                 "is live; the other entries won't open a "
                 "serial port. Rebuild with "
                 "<tt>-DDBWEAVE_BUILD_LOOM=ON</tt> to enable "
                 "real loom drivers.</i>",
                 "<i>Hinweis: Diese ausführbare Datei wurde ohne "
                 "QtSerialPort-Unterstützung gebaut. Nur die Dummy-"
                 "Steuerung ist aktiv; die übrigen Einträge öffnen "
                 "keinen seriellen Anschluss. Mit "
                 "<tt>-DDBWEAVE_BUILD_LOOM=ON</tt> neu bauen, um "
                 "echte Websteuerungen zu aktivieren.</i>"),
        this);
    warn->setWordWrap(true);
#endif

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
#ifndef DBWEAVE_HAVE_LOOM
    root->addWidget(warn);
#endif
    root->addWidget(btns);

    connect(cbInterface, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LoomOptionsDialog::onInterfaceChanged);
    onInterfaceChanged(cbInterface->currentIndex());
}

/*-----------------------------------------------------------------*/
void LoomOptionsDialog::onInterfaceChanged(int _row)
{
    /*  Varpapuu parallel is the only interface that consumes the
        delay spinbox — keep the UI scoped to it so the user knows
        when the field is meaningful. */
    spDelay->setEnabled(_row == int(intrf_varpapuu_parallel));
    /*  Dummy doesn't need a serial port. */
    cbPort->setEnabled(_row != int(intrf_dummy));
}

/*-----------------------------------------------------------------*/
LOOMINTERFACE LoomOptionsDialog::interf() const
{
    return LOOMINTERFACE(cbInterface->currentIndex());
}

int LoomOptionsDialog::port() const
{
    return cbPort->currentIndex() + 1;
}
int LoomOptionsDialog::delay() const
{
    return spDelay->value();
}

void LoomOptionsDialog::setInterface(LOOMINTERFACE _i)
{
    cbInterface->setCurrentIndex(int(_i));
}
void LoomOptionsDialog::setPort(int _p)
{
    cbPort->setCurrentIndex(_p - 1);
}
void LoomOptionsDialog::setDelay(int _d)
{
    spDelay->setValue(_d);
}
