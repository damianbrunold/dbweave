/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy pagesetup_form.cpp (TPageSetupForm). Four
    margin spinboxes in centimetres plus two text edits for the
    header/footer substitution templates.                          */

#ifndef DBWEAVE_UI_PAGESETUP_DIALOG_H
#define DBWEAVE_UI_PAGESETUP_DIALOG_H

#include <QDialog>

class QDoubleSpinBox;
class QLineEdit;

class PageSetupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PageSetupDialog(QWidget* _parent = nullptr);

    /*  Margins in millimetre-tenths (matching TDBWFRM::Borders). */
    void setMargins(int _leftTenths, int _rightTenths, int _topTenths, int _bottomTenths);
    int leftTenths() const;
    int rightTenths() const;
    int topTenths() const;
    int bottomTenths() const;

    void setHeader(const QString& _s);
    void setFooter(const QString& _s);
    QString headerText() const;
    QString footerText() const;

private:
    QDoubleSpinBox* left = nullptr;
    QDoubleSpinBox* right = nullptr;
    QDoubleSpinBox* top = nullptr;
    QDoubleSpinBox* bottom = nullptr;
    QLineEdit* header = nullptr;
    QLineEdit* footer = nullptr;
};

#endif
