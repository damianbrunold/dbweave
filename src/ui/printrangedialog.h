/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy/printselection_form.cpp. Four range pickers
    (warp / weft / shaft / treadle) for File > Print part... The
    user-visible indices are 1-based; the SZ accessors return
    0-based [a, b] ranges for feeding into PrPrinterPrint::PrintRange.

    A range of {-1, -1} means "skip this axis" -- set automatically
    when the matching View toggle is off for shafts/treadles.      */

#ifndef DBWEAVE_UI_PRINTRANGEDIALOG_H
#define DBWEAVE_UI_PRINTRANGEDIALOG_H

#include "dbw3_base.h"

#include <QDialog>

class QSpinBox;

class PrintRangeDialog : public QDialog
{
    Q_OBJECT

public:
    PrintRangeDialog(QWidget* _parent, const SZ& _kette, int _maxKette, const SZ& _schuesse,
                     int _maxSchuesse, const SZ& _schaefte, int _maxSchaefte, const SZ& _tritte,
                     int _maxTritte);

    SZ kette() const;
    SZ schuesse() const;
    SZ schaefte() const;
    SZ tritte() const;

private:
    QSpinBox* ketteVon;
    QSpinBox* ketteBis;
    QSpinBox* schussVon;
    QSpinBox* schussBis;
    QSpinBox* schaftVon;
    QSpinBox* schaftBis;
    QSpinBox* trittVon;
    QSpinBox* trittBis;

    /*  The "skip this axis" sentinel: when the caller passes (-1,-1),
        the matching group is disabled so the user can't turn it back
        on, and the pair returns {-1,-1}. */
    bool skipSchaefte = false;
    bool skipTritte = false;
};

#endif
