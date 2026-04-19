/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy importbmp_form.cpp (TImportBmpForm).
    "Import picture" dialog: shows the bitmap on the left, a
    downsampled preview on the right with a Resolution (cell-pixel
    size) spinner. On OK builds a range-index array (data[x*y]).

    Colour mapping: iterates the source pixels, records up to 9
    non-background colours encountered in scan order and maps each
    cell to a range (1..9) based on which slot the centre pixel
    matched; all other colours → 0 (empty). Verbatim port.         */

#ifndef DBWEAVE_UI_IMPORTBMP_DIALOG_H
#define DBWEAVE_UI_IMPORTBMP_DIALOG_H

#include <QDialog>
#include <QImage>

class QSpinBox;
class PreviewCanvas;

class ImportBmpDialog : public QDialog
{
	Q_OBJECT
public:
	explicit ImportBmpDialog (const QString& _filename, QWidget* _parent = nullptr);

	/*  Populated on accept(); 0 × 0 on cancel. Row-major, index
	    (i, j) lives at data[i + j*x]. */
	int        x = 0;
	int        y = 0;
	QByteArray data;

protected:
	void accept () override;

private:
	QImage         bitmap;
	QSpinBox*      spinRes = nullptr;
	PreviewCanvas* preview = nullptr;

	QRgb  bkground = 0;
	QRgb  colors[9] = { };
	int   used     = 0;

	void buildColorTable ();
	int  rangeOf (QRgb _col) const;
};

#endif
