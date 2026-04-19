/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "propertiesdialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

PropertiesDialog::PropertiesDialog(QWidget* _parent)
    : QDialog(_parent)
{
    setWindowTitle(QStringLiteral("Properties"));
    setModal(true);

    edAuthor = new QLineEdit(this);
    edOrganization = new QLineEdit(this);
    edRemarks = new QPlainTextEdit(this);

    auto* form = new QFormLayout();
    form->addRow(QStringLiteral("&Author:"), edAuthor);
    form->addRow(QStringLiteral("&Organization:"), edOrganization);
    form->addRow(QStringLiteral("&Notes:"), edRemarks);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(btns);

    resize(420, 320);
}

QString PropertiesDialog::author() const
{
    return edAuthor->text();
}
QString PropertiesDialog::organization() const
{
    return edOrganization->text();
}
QString PropertiesDialog::remarks() const
{
    return edRemarks->toPlainText();
}

void PropertiesDialog::setAuthor(const QString& _s)
{
    edAuthor->setText(_s);
}
void PropertiesDialog::setOrganization(const QString& _s)
{
    edOrganization->setText(_s);
}
void PropertiesDialog::setRemarks(const QString& _s)
{
    edRemarks->setPlainText(_s);
}

/*-----------------------------------------------------------------*/
/*  TDBWFRM::FilePropsClick — verbatim port of legacy dbw3_form.cpp
    FilePropsClick. Open PropertiesDialog prefilled from the current
    FileProperties; on OK push the three fields back.               */
#include "mainwindow.h"
#include "datamodule.h"
#include "properties.h"
#include "cursor.h"

void TDBWFRM::FilePropsClick()
{
    if (!Data->properties)
        return;
    if (cursorhandler)
        cursorhandler->DisableCursor();
    PropertiesDialog dlg(this);
    dlg.setAuthor(QString::fromUtf8(Data->properties->Author()));
    dlg.setOrganization(QString::fromUtf8(Data->properties->Organization()));
    dlg.setRemarks(QString::fromUtf8(Data->properties->Remarks()));
    if (dlg.exec() == QDialog::Accepted) {
        Data->properties->SetAuthor(dlg.author().toUtf8().constData());
        Data->properties->SetOrganization(dlg.organization().toUtf8().constData());
        Data->properties->SetRemarks(dlg.remarks().toUtf8().constData());
        SetModified();
    }
    if (cursorhandler)
        cursorhandler->EnableCursor();
}
