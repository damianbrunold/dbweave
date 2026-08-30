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

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#ifdef DBWEAVE_HAVE_LOOM
#include <QSerialPortInfo>

#include "loomlog.h"
#endif

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
    cbInterface->addItem(LANG_STR("Dummy (simulation)", "Dummy (Simulation)"), int(intrf_dummy));
    cbInterface->addItem(QStringLiteral("ARM Patronic direct"), int(intrf_arm_patronic));
    cbInterface->addItem(QStringLiteral("ARM Patronic indirect"), int(intrf_arm_patronic_indirect));
    cbInterface->addItem(QStringLiteral("ARM Designer electronic"), int(intrf_arm_designer));
    cbInterface->addItem(QStringLiteral("Generic SLIPS"), int(intrf_slips));
    cbInterface->addItem(QStringLiteral("AVL Compu-Dobby III"), int(intrf_avl_cd_iii));

    /*  Legacy hard-coded COM1..COM8. Enumerate what the machine
        actually has instead, and stay editable so a device the
        enumeration misses (or a name the user knows) can still be
        typed in. */
    cbPort = new QComboBox(this);
    cbPort->setEditable(true);
    cbPort->setInsertPolicy(QComboBox::NoInsert);
#ifdef DBWEAVE_HAVE_LOOM
    for (const QSerialPortInfo& pi : QSerialPortInfo::availablePorts()) {
        const QString label = pi.description().isEmpty()
                                  ? pi.portName()
                                  : pi.portName() + QStringLiteral(" — ") + pi.description();
        cbPort->addItem(label, pi.portName());
    }
#endif
    if (cbPort->count() == 0) {
        cbPort->addItem(
            LANG_STR("(no serial ports found)", "(keine seriellen Anschlüsse gefunden)"),
            QString());
    }

    /*  Qt's Windows backend forces RTS low whenever flow control is
        not hardware handshaking; the legacy Winsoft component left
        both lines asserted. Default to the legacy behaviour, but
        let the user drop it for a box that wants them quiet. */
    chkAssertLines = new QCheckBox(LANG_STR("Assert &DTR/RTS after opening the port",
                                            "&DTR/RTS nach dem Öffnen des Anschlusses setzen"),
                                   this);
    chkAssertLines->setChecked(true);
    chkAssertLines->setToolTip(
        LANG_STR("Most loom interfaces need these lines high. Turn this off only "
                 "if your interface requires them low.",
                 "Die meisten Websteuerungen benötigen diese Leitungen auf High. "
                 "Nur abschalten, wenn Ihre Steuerung sie auf Low erwartet."));

    chkTrace = new QCheckBox(
        LANG_STR("Write a &communication log", "&Kommunikationsprotokoll schreiben"), this);
    labTracePath = new QLabel(this);
    labTracePath->setWordWrap(true);
    labTracePath->setTextInteractionFlags(Qt::TextSelectableByMouse);
#ifdef DBWEAVE_HAVE_LOOM
    labTracePath->setText(QStringLiteral("<small>%1<br><tt>%2</tt></small>")
                              .arg(LANG_STR("Records every byte exchanged with the loom to:",
                                            "Protokolliert jedes mit dem Webstuhl "
                                            "ausgetauschte Byte nach:"),
                                   LoomLog::FilePath().toHtmlEscaped()));
#else
    labTracePath->hide();
#endif

    auto* form = new QFormLayout();
    form->addRow(LANG_STR("&Loom:", "&Webstuhl:"), cbInterface);
    form->addRow(LANG_STR("&Port:", "&Anschluss:"), cbPort);

#ifndef DBWEAVE_HAVE_LOOM
    auto* warn = new QLabel(LANG_STR("<i>Note: this build of DB-WEAVE was configured with "
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
    root->addWidget(chkAssertLines);
    root->addWidget(chkTrace);
    root->addWidget(labTracePath);
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
    const bool serial = val != int(intrf_dummy);
    cbPort->setEnabled(serial);
    chkAssertLines->setEnabled(serial);
}

/*-----------------------------------------------------------------*/
LOOMINTERFACE LoomOptionsDialog::interf() const
{
    return LOOMINTERFACE(cbInterface->currentData().toInt());
}

QString LoomOptionsDialog::port() const
{
    /*  A typed-in name never becomes an item, so prefer the edit
        text whenever it differs from the current item's label. */
    const QString typed = cbPort->currentText().trimmed();
    const int row = cbPort->currentIndex();
    if (row >= 0 && cbPort->itemText(row) == typed)
        return cbPort->itemData(row).toString();
    return typed;
}

bool LoomOptionsDialog::assertDtrRts() const
{
    return chkAssertLines->isChecked();
}

bool LoomOptionsDialog::trace() const
{
    return chkTrace->isChecked();
}

/*-----------------------------------------------------------------*/
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

void LoomOptionsDialog::setPort(const QString& _p)
{
    for (int r = 0; r < cbPort->count(); r++) {
        if (cbPort->itemData(r).toString() == _p) {
            cbPort->setCurrentIndex(r);
            return;
        }
    }
    /*  Configured port isn't plugged in right now (or this is a
        NO_LOOM build): keep showing the saved name rather than
        silently switching the user to a different device. */
    if (!_p.isEmpty())
        cbPort->setEditText(_p);
}

void LoomOptionsDialog::setAssertDtrRts(bool _on)
{
    chkAssertLines->setChecked(_on);
}

void LoomOptionsDialog::setTrace(bool _on)
{
    chkTrace->setChecked(_on);
}
