/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy/einstellverh_form.cpp. Two-spinbox modal that
    sets the faktor_kette / faktor_schuss ratio (weft/warp thread
    density), which stretches the cell geometry so threads with
    different diameters render at a matching visual scale.         */

#ifndef DBWEAVE_UI_VERHAELTNISDIALOG_H
#define DBWEAVE_UI_VERHAELTNISDIALOG_H

#include <QDialog>

class QDoubleSpinBox;

class VerhaeltnisDialog : public QDialog
{
    Q_OBJECT

public:
    VerhaeltnisDialog(QWidget* _parent, double _faktor_kette, double _faktor_schuss);

    double faktorKette() const;
    double faktorSchuss() const;

private:
    QDoubleSpinBox* sbKette = nullptr;
    QDoubleSpinBox* sbSchuss = nullptr;
};

#endif
