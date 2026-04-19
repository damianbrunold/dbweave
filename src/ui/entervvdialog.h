/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy/entervv_form.cpp. Thread-ratio entry modal
    used by KettLancierung and SchussLancierung. Six integer
    fields (VV1..VV6) that progressively enable each other as the
    user types (a zero in slot N clears and disables slots N+1..6).
*/

#ifndef DBWEAVE_UI_ENTERVVDIALOG_H
#define DBWEAVE_UI_ENTERVVDIALOG_H

#include <QDialog>

class QLineEdit;

class EnterVVDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnterVVDialog(QWidget* _parent = nullptr);

    /*  Returns VV1..VV6 in slots [0..5]; empty or zero fields read
        back as zero. Caller decides how to interpret maxi.       */
    int value(int _slot) const;

private:
    QLineEdit* vv[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

    void onChange(int _slot);
};

#endif
