/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy strggoto_form.cpp. Modal dialog opened from
    TSTRGFRM's Position > Set current position entry: lets the user
    set the weaving cursor (schuss / klammer / repetition) to an
    arbitrary valid position.                                       */

#ifndef DBWEAVE_UI_STRGGOTO_DIALOG_H
#define DBWEAVE_UI_STRGGOTO_DIALOG_H

#include <QDialog>

class QComboBox;
class QSpinBox;
class TSTRGFRM;

class StrgGotoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StrgGotoDialog(TSTRGFRM* _frm);

    int schuss() const;      /* 0-based */
    int klammer() const;     /* 0-based, matches TSTRGFRM::klammern */
    int repetition() const;  /* 1-based */

private:
    TSTRGFRM* frm = nullptr;
    QSpinBox* sbSchuss = nullptr;
    QComboBox* cbKlammer = nullptr;
    QComboBox* cbRepeat = nullptr;

    void refreshRepeatOptions();
};

#endif
