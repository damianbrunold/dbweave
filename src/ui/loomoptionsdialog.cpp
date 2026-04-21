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
#include "loominfodialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

/*-----------------------------------------------------------------*/
LoomOptionsDialog::LoomOptionsDialog(QWidget* _parent)
    : QDialog(_parent)
{
    setWindowTitle(LANG_STR("Loom options", "Websteuerungs-Optionen"));
    setModal(true);

    cbInterface = new QComboBox(this);
    /*  Each entry carries its LOOMINTERFACE value via userData; the
        enum itself is sparse (slots 4 and 6 reserved for the two
        parallel-port interfaces that were never implemented). */
    cbInterface->addItem(LANG_STR("Dummy (simulation)", "Dummy (Simulation)"),
                         int(intrf_dummy));
    cbInterface->addItem(QStringLiteral("ARM Patronic direct"), int(intrf_arm_patronic));
    cbInterface->addItem(QStringLiteral("ARM Patronic indirect"),
                         int(intrf_arm_patronic_indirect));
    cbInterface->addItem(QStringLiteral("ARM Designer electronic"), int(intrf_arm_designer));
    cbInterface->addItem(QStringLiteral("Generic SLIPS"), int(intrf_slips));
    cbInterface->addItem(QStringLiteral("AVL Compu-Dobby III"), int(intrf_avl_cd_iii));

    cbPort = new QComboBox(this);
    for (int i = 1; i <= 8; i++)
        cbPort->addItem(QStringLiteral("COM%1").arg(i));

    auto* form = new QFormLayout();
    form->addRow(LANG_STR("&Loom:", "&Webstuhl:"), cbInterface);
    form->addRow(LANG_STR("&Port:", "&Anschluss:"), cbPort);

#ifndef DBWEAVE_HAVE_LOOM
    auto* warn = new QLabel(
        LANG_STR("<i>Note: this build of DB-WEAVE was configured with "
                 "<tt>-DDBWEAVE_NO_LOOM=ON</tt>, so only the Dummy "
                 "loom is live; the other entries won't open a serial "
                 "port. Rebuild without that option to enable real "
                 "loom drivers.</i>",
                 "<i>Hinweis: Diese DB-WEAVE-Version wurde mit "
                 "<tt>-DDBWEAVE_NO_LOOM=ON</tt> gebaut; nur die "
                 "Dummy-Steuerung ist aktiv, die übrigen Einträge "
                 "öffnen keinen seriellen Anschluss. Ohne diese "
                 "Option neu bauen, um echte Websteuerungen zu "
                 "aktivieren.</i>"),
        this);
    warn->setWordWrap(true);
#endif

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    QPushButton* bInfo = btns->addButton(LANG_STR("&Info", "&Info"), QDialogButtonBox::ActionRole);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(bInfo, &QPushButton::clicked, this, [this] {
        LoomInfoDialog dlg(this, LOOMINTERFACE(cbInterface->currentData().toInt()));
        dlg.exec();
    });

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
    const int val = cbInterface->itemData(_row).toInt();
    /*  Dummy doesn't need a serial port. */
    cbPort->setEnabled(val != int(intrf_dummy));
}

/*-----------------------------------------------------------------*/
LOOMINTERFACE LoomOptionsDialog::interf() const
{
    return LOOMINTERFACE(cbInterface->currentData().toInt());
}

int LoomOptionsDialog::port() const
{
    return cbPort->currentIndex() + 1;
}

void LoomOptionsDialog::setInterface(LOOMINTERFACE _i)
{
    const int v = int(_i);
    for (int r = 0; r < cbInterface->count(); r++) {
        if (cbInterface->itemData(r).toInt() == v) {
            cbInterface->setCurrentIndex(r);
            return;
        }
    }
    /*  Unknown / deprecated interface value (e.g. a stale QSettings
        entry pointing at Varpapuu or LIPS); fall back to dummy.    */
    cbInterface->setCurrentIndex(0);
}
void LoomOptionsDialog::setPort(int _p)
{
    cbPort->setCurrentIndex(_p - 1);
}
