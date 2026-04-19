/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy/about.cpp (TAboutBox). Modal About box with
    verbatim legacy prose. The Tech Info side-dialog and the
    Shift+Ctrl+Alt double-click easter egg are deferred until the
    TTechinfoForm port lands. */

#ifndef DBWEAVE_UI_ABOUTDIALOG_H
#define DBWEAVE_UI_ABOUTDIALOG_H

#include <QDialog>

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* _parent = nullptr);
};

#endif
