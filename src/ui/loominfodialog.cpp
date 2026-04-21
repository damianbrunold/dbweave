/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "loominfodialog.h"

#include "language.h"

#include <QDialogButtonBox>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace
{
/*  HTML equivalents of legacy strginfo_form.cpp's RTF blobs. One
    pair per LOOMINTERFACE; English first, then German. Bold / h2
    replace legacy's \b / \fs26.                                   */
struct LoomInfo {
    QString en;
    QString ge;
};

static LoomInfo infoFor(LOOMINTERFACE _loom)
{
    switch (_loom) {
    case intrf_dummy:
        return {
            QStringLiteral(
                "<h2>No loom</h2>"
                "<p><b>Description</b><br/>"
                "This is a driver for testing purposes. No loom is controlled. The "
                "weaving is simulated.</p>"),
            QStringLiteral(
                "<h2>Kein Webstuhl</h2>"
                "<p><b>Beschreibung</b><br/>"
                "Dies ist ein Treiber, der für Testzwecke eingesetzt werden kann. "
                "Es wird kein Webstuhl angesteuert. Das Weben wird simuliert.</p>")
        };
    case intrf_arm_patronic:
        return {
            QStringLiteral(
                "<h2>ARM Patronic</h2>"
                "<p><b>Manufacturer</b><br/>ARM AG, Biglen, Schweiz</p>"
                "<p><b>Information</b><br/>"
                "Directly from the manufacturer (armloom@hotmail.com)</p>"
                "<p><b>Description</b><br/>"
                "The ARM Patronic is a pattern loom with 24 harnesses, equipped with the "
                "Selecontrol unit. This driver supports the Selecontrol unit in direct "
                "mode: every pick transferred from the computer to the loom is woven "
                "immediately.</p>"),
            QStringLiteral(
                "<h2>ARM Patronic</h2>"
                "<p><b>Hersteller</b><br/>ARM AG, Biglen, Schweiz</p>"
                "<p><b>Informationen</b><br/>"
                "Direkt beim Hersteller (armloom@hotmail.com)</p>"
                "<p><b>Beschreibung</b><br/>"
                "Der ARM Patronic ist ein Musterwebstuhl mit 24 Schäften, der mit der "
                "Selecontrol-Steuerung ausgestattet ist. Dieser Treiber unterstützt die "
                "Selecontrol-Steuerung im direkten Modus: jeder Schuss, den der Computer "
                "auf den Webstuhl überträgt, wird gleich gewoben.</p>")
        };
    case intrf_arm_patronic_indirect:
        return {
            QStringLiteral(
                "<h2>ARM Patronic indirect</h2>"
                "<p><b>Manufacturer</b><br/>ARM AG, Biglen, Schweiz</p>"
                "<p><b>Information</b><br/>"
                "Directly from the manufacturer (armloom@hotmail.com)</p>"
                "<p><b>Description</b><br/>"
                "Indirect mode: the desired picks are transmitted all at once to the "
                "Selecontrol unit; afterwards the loom weaves the pattern autonomously "
                "(without the computer).</p>"),
            QStringLiteral(
                "<h2>ARM Patronic Indirekt</h2>"
                "<p><b>Hersteller</b><br/>ARM AG, Biglen, Schweiz</p>"
                "<p><b>Informationen</b><br/>"
                "Direkt beim Hersteller (armloom@hotmail.com)</p>"
                "<p><b>Beschreibung</b><br/>"
                "Indirekter Modus: die gewünschten Schäfte werden auf einmal in die "
                "Selecontrol-Steuerung übertragen; danach kann der Webstuhl autonom "
                "(ohne Computer) das Muster weben.</p>")
        };
    case intrf_arm_designer:
        return {
            QStringLiteral(
                "<h2>ARM Designer Electronic</h2>"
                "<p><b>Manufacturer</b><br/>ARM AG, Biglen, Schweiz</p>"
                "<p><b>Information</b><br/>"
                "Directly from the manufacturer (armloom@hotmail.com)</p>"
                "<p><b>Description</b><br/>"
                "The ARM Designer Electronic is a hand loom with computer control. "
                "The controller is available in two variants: a direct Designer "
                "controller or a Selecontrol module like the Patronic pattern loom. If "
                "your Designer has a Selecontrol unit, use the ARM Patronic direct / "
                "indirect driver.</p>"),
            QStringLiteral(
                "<h2>ARM Designer Electronic</h2>"
                "<p><b>Hersteller</b><br/>ARM AG, Biglen, Schweiz</p>"
                "<p><b>Informationen</b><br/>"
                "Direkt beim Hersteller (armloom@hotmail.com)</p>"
                "<p><b>Beschreibung</b><br/>"
                "Der ARM Designer Electronic ist ein Handwebstuhl mit Computersteuerung. "
                "Die Steuerung ist in zwei Varianten erhältlich: eine direkte Designer-"
                "Steuerung oder ein Selecontrol-Modul wie beim Patronic. Falls Sie einen "
                "Designer mit Selecontrol haben, nutzen Sie den ARM Patronic Direkt/"
                "Indirekt Treiber.</p>")
        };
    case intrf_varpapuu_parallel:
        return {
            QStringLiteral(
                "<h2>Varpapuu Parallel</h2>"
                "<p><b>Manufacturer</b><br/>Varpa-Set Oy, Pieksämäki, Finland</p>"
                "<p><b>Information</b><br/>"
                "Directly from the manufacturer (varpa-set@co.inet.fi)</p>"
                "<p><b>Description</b><br/>"
                "The Varpapuu controller mounts onto any normal (non-computerised) loom. "
                "This driver targets the parallel-port variant. (An older version used "
                "an ISA card.)</p>"),
            QStringLiteral(
                "<h2>Varpapuu Parallel</h2>"
                "<p><b>Hersteller</b><br/>Varpa-Set Oy, Pieksämäki, Finnland</p>"
                "<p><b>Informationen</b><br/>"
                "Direkt beim Hersteller (varpa-set@co.inet.fi)</p>"
                "<p><b>Beschreibung</b><br/>"
                "Die Varpapuu-Steuerung lässt sich auf jeden normalen (nicht "
                "computergesteuerten) Webstuhl montieren. Dieser Treiber ist für die "
                "Druckerport-Variante. (Eine ältere Version arbeitete über eine "
                "ISA-Karte.)</p>")
        };
    case intrf_slips:
        return {
            QStringLiteral(
                "<h2>Generic SLIPS</h2>"
                "<p><b>Information</b><br/>Ask your loom manufacturer whether it supports "
                "SLIPS.</p>"
                "<p><b>Description</b><br/>"
                "Generic SLIPS is an open standard for controlling looms via the serial "
                "port. Developed jointly by Bob and Jacquie Kelly (Citadel Enterprises) "
                "and Ravi Nielsen (Maple Hill Software). Schacht Spindle and Cyrefco sell "
                "compatible looms.</p>"),
            QStringLiteral(
                "<h2>Allgemeines SLIPS</h2>"
                "<p><b>Information</b><br/>Fragen Sie Ihren Webstuhlhersteller ob er "
                "SLIPS unterstützt.</p>"
                "<p><b>Beschreibung</b><br/>"
                "Allgemeines SLIPS ist ein offener Standard um Webstühle über die "
                "serielle Schnittstelle zu steuern. Entwickelt von Bob und Jacquie Kelly "
                "(Citadel Enterprises) und Ravi Nielsen (Maple Hill Software). Z.B. "
                "Schacht Spindle und Cyrefco bieten kompatible Webstühle an.</p>")
        };
    case intrf_lips:
        return {
            QStringLiteral(
                "<h2>Generic LIPS</h2>"
                "<p><b>Information</b><br/>Ask your loom manufacturer whether it supports "
                "LIPS.</p>"
                "<p><b>Description</b><br/>"
                "Generic LIPS is the parallel-port equivalent of SLIPS. Same authors: "
                "Bob and Jacquie Kelly (Citadel Enterprises) and Ravi Nielsen (Maple Hill "
                "Software).</p>"),
            QStringLiteral(
                "<h2>Allgemeines LIPS</h2>"
                "<p><b>Information</b><br/>Fragen Sie Ihren Webstuhlhersteller ob er "
                "LIPS unterstützt.</p>"
                "<p><b>Beschreibung</b><br/>"
                "Allgemeines LIPS ist das Parallelport-Gegenstück zu SLIPS. Gleiche "
                "Entwickler: Bob und Jacquie Kelly (Citadel Enterprises) und Ravi Nielsen "
                "(Maple Hill Software).</p>")
        };
    case intrf_avl_cd_iii:
        return {
            QStringLiteral(
                "<h2>AVL Compu-Dobby III</h2>"
                "<p><b>Manufacturer</b><br/>AVL, USA</p>"
                "<p><b>Information</b><br/>"
                "Directly from the manufacturer (www.avlusa.com)</p>"),
            QStringLiteral(
                "<h2>AVL Compu-Dobby III</h2>"
                "<p><b>Hersteller</b><br/>AVL, USA</p>"
                "<p><b>Informationen</b><br/>"
                "Direkt beim Hersteller (www.avlusa.com)</p>")
        };
    }
    return {};
}
} // namespace

LoomInfoDialog::LoomInfoDialog(QWidget* _parent, LOOMINTERFACE _loom)
    : QDialog(_parent)
{
    setWindowTitle(LANG_STR("Loom information", "Webstuhl Informationen"));
    setModal(true);
    resize(500, 380);

    text = new QTextBrowser(this);
    text->setOpenExternalLinks(true);
    const LoomInfo info = infoFor(_loom);
    text->setHtml(active_language == GE ? info.ge : info.en);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto* root = new QVBoxLayout(this);
    root->addWidget(text, 1);
    root->addWidget(btns);
}
