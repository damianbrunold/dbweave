/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy strginfo_form.cpp. Read-only driver info
    dialog (manufacturer / information / description) shown from
    LoomOptionsDialog's "Info..." button. Legacy shipped the
    content as RTF blobs; the port uses HTML via QTextBrowser.   */

#ifndef DBWEAVE_UI_LOOMINFO_DIALOG_H
#define DBWEAVE_UI_LOOMINFO_DIALOG_H

#include <QDialog>

#include "loom.h" /* LOOMINTERFACE */

class QTextBrowser;

class LoomInfoDialog : public QDialog
{
    Q_OBJECT

public:
    LoomInfoDialog(QWidget* _parent, LOOMINTERFACE _loom);

private:
    QTextBrowser* text = nullptr;
};

#endif
