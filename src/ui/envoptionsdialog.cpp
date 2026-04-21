/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "envoptionsdialog.h"
#include "language.h"
#include "settings.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLocale>
#include <QVBoxLayout>

EnvOptionsDialog::EnvOptionsDialog(QWidget* _parent)
    : QDialog(_parent)
{
    setWindowTitle(LANG_STR("Environment options", "Umgebungsoptionen"));
    setModal(true);

    cbLanguage = new QComboBox(this);
    cbLanguage->addItem(LANG_STR("English", "Englisch"));
    cbLanguage->addItem(LANG_STR("German", "Deutsch"));

    auto* form = new QFormLayout();
    form->addRow(LANG_STR("&Language:", "&Sprache:"), cbLanguage);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(btns);

    /*  Legacy: read from the "Environment" settings category; if
        Language is -1 / absent, derive from the OS locale (GE if
        the abbreviated language name starts with "de"). */
    Settings settings;
    settings.SetCategory(AnsiString("Environment"));
    int lang = settings.Load(AnsiString("Language"), -1);
    if (lang == -1) {
        const QString tag = QLocale::system().name().toLower();
        lang = tag.startsWith(QStringLiteral("de")) ? 1 : 0;
    }
    cbLanguage->setCurrentIndex(lang == 1 ? 1 : 0);
}

void EnvOptionsDialog::accept()
{
    LANGUAGES language = EN;
    switch (cbLanguage->currentIndex()) {
    case 0:
        language = EN;
        break;
    case 1:
        language = GE;
        break;
    }
    SwitchLanguage(language);

    Settings settings;
    settings.SetCategory(AnsiString("Environment"));
    settings.Save(AnsiString("Language"), cbLanguage->currentIndex());

    QDialog::accept();
}
