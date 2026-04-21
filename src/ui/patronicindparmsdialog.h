/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy strgpatronicindparms_form.cpp. Tiny two-field
    modal shown by WeaveStartClick when the selected interface is
    ARM Patronic indirect: starting position (1..220) and max
    number of picks (1..220).                                    */

#ifndef DBWEAVE_UI_PATRONIC_INDPARMS_DIALOG_H
#define DBWEAVE_UI_PATRONIC_INDPARMS_DIALOG_H

#include <QDialog>

class QSpinBox;

class PatronicIndParmsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PatronicIndParmsDialog(QWidget* _parent, int _initialCount);

    int startingPosition() const; /* 1..220 */
    int maxPicks() const;         /* 1..220 */

private:
    QSpinBox* sbPos = nullptr;
    QSpinBox* sbCount = nullptr;
};

#endif
