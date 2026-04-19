/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy selcolor_form.cpp (TSelColorForm). Shows the
    19x13 = 247-slot (clamped at MAX_PAL_ENTRY=236) palette grid
    with keyboard/mouse cursor, HSV/RGB readout for the selected
    entry, and menu entries to edit the entry via the RGB or HSV
    picker. Cancel reverts the palette.                            */

#ifndef DBWEAVE_UI_SELCOLOR_DIALOG_H
#define DBWEAVE_UI_SELCOLOR_DIALOG_H

#include <QDialog>
#include "colors_compat.h"

class QLabel;
class SelColorCanvas;

class SelColorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelColorDialog(int _initialIndex, QWidget* _parent = nullptr);
    ~SelColorDialog() override;

    int selectedIndex() const;

protected:
    void reject() override;

private slots:
    void editRGB();
    void editHSV();
    void revertChanges();

private:
    SelColorCanvas* canvas = nullptr;
    QLabel* labIdx = nullptr;
    QLabel* labH = nullptr;
    QLabel* labS = nullptr;
    QLabel* labV = nullptr;
    QLabel* labR = nullptr;
    QLabel* labG = nullptr;
    QLabel* labB = nullptr;

    COLORREF* oldpal = nullptr;

    void updateValues();

    friend class SelColorCanvas;
};

#endif
