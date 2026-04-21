/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy farbverlauf_form.cpp (TFarbverlaufForm).
    "Create color blending": interpolate between a start and end
    colour in RGB or HSV space, weighted by a slider, then write the
    resulting ramp into the palette starting at a chosen index.

    Uses QColorDialog for both RGB and HSV (its built-in picker
    exposes both models) and a minimal palette-index prompt for the
    palette option. */

#ifndef DBWEAVE_UI_FARBVERLAUF_DIALOG_H
#define DBWEAVE_UI_FARBVERLAUF_DIALOG_H

#include <QDialog>
#include "colors_compat.h" /* COLORREF */
#include "palette.h"       /* MAX_PAL_ENTRY */

class QCheckBox;
class QLineEdit;
class QRadioButton;
class QSlider;
class QSpinBox;
class QWidget;
class TDBWFRM;

class FarbverlaufDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FarbverlaufDialog(TDBWFRM* _frm, QWidget* _parent = nullptr);

protected:
    void accept() override;

private:
    TDBWFRM* frm = nullptr;

    COLORREF startcolor = 0;
    COLORREF endcolor = 0;
    COLORREF table[MAX_PAL_ENTRY];
    int count = 0;

    QWidget* swatchStart = nullptr;
    QWidget* swatchEnd = nullptr;
    QWidget* swatchBlend = nullptr;

    QSpinBox* edSteps = nullptr;
    QSpinBox* edPosition = nullptr;
    QSlider* slWeight = nullptr;
    QCheckBox* nodividers = nullptr;
    QRadioButton* modelRGB = nullptr;
    QRadioButton* modelHSV = nullptr;

    void createFarbverlauf();
    void farbverlaufRGB(int _abstufungen);
    void farbverlaufHSV(int _abstufungen);

    COLORREF pickColor(COLORREF _col);
    void updateSwatch(QWidget* _w, COLORREF _col);
    void repaintBlend();
};

#endif
