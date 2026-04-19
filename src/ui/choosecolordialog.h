/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt ports of the three custom colour pickers:
      ChooseRGBDialog      — legacy farbauswahl1_form (RGB sliders)
      ChooseHSVDialog      — legacy farbauswahl_form (hue wheel + S/V patch)
      ChoosePaletteDialog  — legacy farbauswahl2_form (palette grid)

    Each exposes SelectColor(COLORREF) / GetSelectedColor() so the
    FarbverlaufDialog popup-menu entries can replace their
    QColorDialog stubs with the real pickers.                      */

#ifndef DBWEAVE_UI_CHOOSECOLOR_DIALOG_H
#define DBWEAVE_UI_CHOOSECOLOR_DIALOG_H

#include <QDialog>
#include "colors_compat.h"

class QLabel;
class QSlider;
class QWidget;

/*-----------------------------------------------------------------*/
class ChooseRGBDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ChooseRGBDialog(QWidget* _parent = nullptr);

    void SelectColor(COLORREF _c);
    COLORREF GetSelectedColor() const;

private:
    QSlider* slRed = nullptr;
    QSlider* slGreen = nullptr;
    QSlider* slBlue = nullptr;
    QLabel* valR = nullptr;
    QLabel* valG = nullptr;
    QLabel* valB = nullptr;
    QWidget* preview = nullptr;

    void updateAll();
};

/*-----------------------------------------------------------------*/
class HueWheel;
class SatValPatch;

class ChooseHSVDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ChooseHSVDialog(QWidget* _parent = nullptr);

    void SelectColor(COLORREF _c);
    COLORREF GetSelectedColor() const;

    /*  0..360, 0..255, 0..255 */
    int hue() const;
    int sat() const;
    int val() const;

    void setHSV(int _h, int _s, int _v);

private:
    QSlider* slHue = nullptr;
    QSlider* slSat = nullptr;
    QSlider* slVal = nullptr;
    QLabel* valH = nullptr;
    QLabel* valS = nullptr;
    QLabel* valV = nullptr;
    QWidget* preview = nullptr;
    HueWheel* wheel = nullptr;
    SatValPatch* patch = nullptr;

    void updateAll();
    void sync();
};

/*-----------------------------------------------------------------*/
class PaletteCanvas;

class ChoosePaletteDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ChoosePaletteDialog(QWidget* _parent = nullptr);

    void SelectColor(COLORREF _c);
    COLORREF GetSelectedColor() const;
    int GetSelectedIndex() const;

private:
    PaletteCanvas* canvas = nullptr;
};

#endif
