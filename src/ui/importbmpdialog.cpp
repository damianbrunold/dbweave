/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "importbmpdialog.h"

#include "mainwindow.h"
#include "datamodule.h"
#include "undoredo.h"
#include "rangecolors.h"
#include "colors_compat.h"

#include <QApplication>
#include <QCursor>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QSpinBox>
#include <QVBoxLayout>

#include <algorithm>

/*-----------------------------------------------------------------*/
static QColor qcolorFromTColor (TColor _c)
{
	return QColor(_c & 0xFF, (_c >> 8) & 0xFF, (_c >> 16) & 0xFF);
}

/*-----------------------------------------------------------------*/
/*  Preview canvas: samples the source bitmap at the centre of each
    "cell" and paints the mapped range colour at `res x res` pixels.
    Mirrors legacy pbPreviewPaint. */
class PreviewCanvas : public QWidget
{
public:
	PreviewCanvas (const QImage& _bmp, QSpinBox* _res,
	               int _used, const QRgb* _colors,
	               QWidget* _parent = nullptr)
		: QWidget(_parent), bmp(_bmp), spin(_res), used(_used), colors(_colors)
	{
		setMinimumSize(300, 300);
	}

protected:
	void paintEvent (QPaintEvent*) override {
		QPainter p(this);
		p.fillRect(rect(), Qt::white);
		const int res = spin ? spin->value() : 4;
		if (res <= 0 || bmp.isNull()) return;
		const int width  = bmp.width();
		const int height = bmp.height();
		const int dx = width  / res;
		const int dy = height / res;
		for (int i = 0; i < dx; i++) {
			for (int j = 0; j < dy; j++) {
				const int xx = (i*2+1)*res/2;
				const int yy = (j*2+1)*res/2;
				if (xx >= width || yy >= height) continue;
				const QRgb src = bmp.pixel(xx, yy);
				int range = 0;
				for (int k = 0; k < used; k++)
					if (colors[k] == src) { range = k + 1; break; }
				QColor col = range < 1 ? QColor(Qt::white)
				           : range < 10 ? qcolorFromTColor(GetRangeColor(range))
				                         : QColor(Qt::white);
				p.setPen(res < 3 ? col : QColor(105, 105, 105));
				p.setBrush(col);
				p.drawRect(i*res, j*res, res, res);
			}
		}
	}

private:
	const QImage& bmp;
	QSpinBox*     spin;
	int           used;
	const QRgb*   colors;
};

/*-----------------------------------------------------------------*/
ImportBmpDialog::ImportBmpDialog (const QString& _filename, QWidget* _parent)
	: QDialog(_parent)
{
	setWindowTitle(QStringLiteral("Import picture"));
	setModal(true);

	bitmap.load(_filename);

	buildColorTable();

	spinRes = new QSpinBox(this);
	spinRes->setRange(1, 64);
	spinRes->setValue(4);

	auto* bmpLabel = new QLabel(this);
	if (!bitmap.isNull()) {
		QPixmap pix = QPixmap::fromImage(bitmap);
		bmpLabel->setPixmap(pix.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}

	preview = new PreviewCanvas(bitmap, spinRes, used, colors, this);

	connect(spinRes, qOverload<int>(&QSpinBox::valueChanged),
	        preview, QOverload<>::of(&QWidget::update));

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(btns, &QDialogButtonBox::accepted, this, &ImportBmpDialog::accept);
	connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto* form = new QFormLayout();
	form->addRow(QStringLiteral("&Resolution:"), spinRes);

	auto* row = new QHBoxLayout();
	auto* lcol = new QVBoxLayout();
	lcol->addWidget(new QLabel(QStringLiteral("Picture template:"), this));
	lcol->addWidget(bmpLabel);
	auto* rcol = new QVBoxLayout();
	rcol->addWidget(new QLabel(QStringLiteral("Preview:"), this));
	rcol->addWidget(preview);
	row->addLayout(lcol);
	row->addLayout(rcol);

	auto* root = new QVBoxLayout(this);
	root->addLayout(row, 1);
	root->addLayout(form);
	root->addWidget(btns);

	resize(700, 420);
}

/*-----------------------------------------------------------------*/
void ImportBmpDialog::buildColorTable ()
{
	if (bitmap.isNull()) return;
	used     = 0;
	bkground = bitmap.pixel(0, 0);
	for (int i = 0; i < bitmap.width(); i++) {
		for (int j = 0; j < bitmap.height(); j++) {
			const QRgb col = bitmap.pixel(i, j);
			if (col == bkground) continue;
			bool found = false;
			for (int k = 0; k < used; k++)
				if (colors[k] == col) { found = true; break; }
			if (!found && used < 9) colors[used++] = col;
			if (used > 9) return;
		}
	}
}

int ImportBmpDialog::rangeOf (QRgb _col) const
{
	for (int k = 0; k < used; k++)
		if (colors[k] == _col) return k + 1;
	return 0;
}

/*-----------------------------------------------------------------*/
void ImportBmpDialog::accept ()
{
	const int res = spinRes->value();
	if (res == 0 || bitmap.isNull()) { QDialog::reject(); return; }
	const int width  = bitmap.width();
	const int height = bitmap.height();
	x = width  / res;
	y = height / res;
	data.resize(x * y);
	for (int i = 0; i < x; i++) {
		for (int j = y-1; j >= 0; j--) {
			const int xx = (i*res + (i+1)*res) / 2;
			const int yy = (j*res + (j+1)*res) / 2;
			if (xx >= width || yy >= height) { data[i + (y-1-j)*x] = 0; continue; }
			data[i + (y-1-j)*x] = char(rangeOf(bitmap.pixel(xx, yy)));
		}
	}
	QDialog::accept();
}

/*-----------------------------------------------------------------*/
/*  TDBWFRM::ImportBitmapClick — prompt for a bitmap, run the
    import dialog, wipe the gewebe, drop the returned data in at
    the origin. Matches legacy importbmp.cpp one-to-one.          */
void __fastcall TDBWFRM::ImportBitmapClick ()
{
	const QString fn = QFileDialog::getOpenFileName(
	    this,
	    QStringLiteral("Import picture"),
	    QString(),
	    QStringLiteral("Images (*.bmp *.png *.jpg *.jpeg *.gif);;All files (*)"));
	if (fn.isEmpty()) return;

	ImportBmpDialog dlg(fn, this);
	if (dlg.exec() != QDialog::Accepted) return;

	/*  Switch to Schlagpatrone (pegplan) mode, matching legacy. */
	if (ViewSchlagpatrone) ViewSchlagpatrone->setChecked(true);
	trittfolge.einzeltritt = false;

	/*  Wipe the gewebe. */
	for (int i = 0; i < Data->MAXX1; i++)
		for (int j = 0; j < Data->MAXY2; j++)
			gewebe.feld.Set(i, j, 0);

	/*  Drop the imported range data. */
	const int nx = std::min(int(Data->MAXX1), dlg.x);
	const int ny = std::min(int(Data->MAXY2), dlg.y);
	for (int i = 0; i < nx; i++)
		for (int j = 0; j < ny; j++)
			gewebe.feld.Set(i, j, dlg.data[i + j*dlg.x]);

	QApplication::setOverrideCursor(Qt::WaitCursor);
	CalcRange();
	CalcRapport();
	RecalcAll();
	QApplication::restoreOverrideCursor();

	refresh();
	if (undo) undo->Snapshot();
	SetModified();
}
