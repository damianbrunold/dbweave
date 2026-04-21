/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Environment-options dialog. App-level preferences: UI language
    (English / German). Values persist via Settings under the
    "Environment" category. */

#ifndef DBWEAVE_UI_ENVOPTIONSDIALOG_H
#define DBWEAVE_UI_ENVOPTIONSDIALOG_H

#include <QDialog>

class QComboBox;

class EnvOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnvOptionsDialog(QWidget* _parent = nullptr);

protected:
    void accept() override;

private:
    QComboBox* cbLanguage = nullptr;
};

#endif
