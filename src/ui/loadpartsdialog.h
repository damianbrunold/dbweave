/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy/loadparts_form.cpp. Partial-load selector:
    eleven per-section checkboxes plus an "All" master that
    toggles every other box on and also disables them. Used by
    File -> Load partial to import selected sections from another
    .dbw without overwriting the current document wholesale. */

#ifndef DBWEAVE_UI_LOADPARTSDIALOG_H
#define DBWEAVE_UI_LOADPARTSDIALOG_H

#include "loadoptions.h"

#include <QDialog>

class QCheckBox;

class LoadPartsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadPartsDialog(QWidget* _parent = nullptr);

    LOADPARTS getLoadParts() const;

private:
    QCheckBox* alle = nullptr;
    QCheckBox* einzug = nullptr;
    QCheckBox* trittfolge = nullptr;
    QCheckBox* aufknuepfung = nullptr;
    QCheckBox* kettfarben = nullptr;
    QCheckBox* schussfarben = nullptr;
    QCheckBox* blatteinzug = nullptr;
    QCheckBox* hilfslinien = nullptr;
    QCheckBox* farbpalette = nullptr;
    QCheckBox* benedefeinzug = nullptr;
    QCheckBox* blockmuster = nullptr;
    QCheckBox* bereichmuster = nullptr;
};

#endif
