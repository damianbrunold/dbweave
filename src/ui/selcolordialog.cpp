/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "selcolordialog.h"
#include "choosecolordialog.h"
#include "datamodule.h"
#include "palette.h"
#include "mainwindow.h"

#include <QAction>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

#include <cstring>

/*  Legacy layout: 19 columns × 13 rows = 247 cells, but only the
    first MAX_PAL_ENTRY (236) are live. */
static constexpr int kCols = 19;
static constexpr int kRows = 13;

class SelColorCanvas : public QWidget
{
public:
	static constexpr int cell = 18;

	SelColorCanvas (SelColorDialog* _owner, int _initial)
		: QWidget(_owner), owner(_owner), selcolor(_initial)
	{
		setFocusPolicy(Qt::StrongFocus);
		setMinimumSize(kCols*cell + 1, kRows*cell + 1);
	}

	int selcolor = 0;

	int getX () const { return selcolor % kCols; }
	int getY () const { return selcolor / kCols; }
	void setXY (int _x, int _y) {
		const int idx = _y*kCols + _x;
		if (idx < MAX_PAL_ENTRY) selcolor = idx;
	}

protected:
	void paintEvent (QPaintEvent*) override {
		QPainter p(this);
		p.setPen(Qt::black);
		for (int j = 0; j < kRows; j++) {
			for (int i = 0; i < kCols; i++) {
				const int idx = i + j*kCols;
				if (idx >= MAX_PAL_ENTRY) return;
				const COLORREF c = Data->palette->GetColor(idx);
				p.fillRect(i*cell, j*cell, cell, cell,
				           QColor(GetRValue(c), GetGValue(c), GetBValue(c)));
				p.drawRect(i*cell, j*cell, cell, cell);
			}
		}
		/*  Cursor. */
		const int x = getX()*cell;
		const int y = getY()*cell;
		p.setPen(QPen(hasFocus() ? Qt::white : Qt::black, 1));
		p.setBrush(Qt::NoBrush);
		p.drawRect(x, y, cell, cell);
	}

	void mousePressEvent (QMouseEvent* _e) override {
		if (_e->button() != Qt::LeftButton) return;
		const int i = int(_e->position().x()) / cell;
		const int j = int(_e->position().y()) / cell;
		if (i < 0 || i >= kCols || j < 0 || j >= kRows) return;
		if (i + j*kCols >= MAX_PAL_ENTRY) return;
		setXY(i, j);
		setFocus();
		update();
		owner->updateValues();
	}

	void mouseDoubleClickEvent (QMouseEvent* _e) override {
		mousePressEvent(_e);
		owner->editHSV();
	}

	void keyPressEvent (QKeyEvent* _e) override {
		int x = getX();
		int y = getY();
		const int step = _e->modifiers().testFlag(Qt::ControlModifier) ? 5 : 1;
		switch (_e->key()) {
		case Qt::Key_Left:
			x -= step;
			if (x < 0) { if (y > 0) { y--; x = kCols-1; } else x = 0; }
			break;
		case Qt::Key_Right:
			x += step;
			if (x >= kCols) { if (y < kRows-1) { y++; x = 0; } else x = kCols-1; }
			break;
		case Qt::Key_Up:
			y -= step;
			if (y < 0) { if (x > 0) { x--; y = kRows-1; if (x+y*kCols >= MAX_PAL_ENTRY) y--; } else y = 0; }
			break;
		case Qt::Key_Down:
			y += step;
			if (y >= kRows) { if (x < kCols) { x++; y = 0; } else y = kRows-1; }
			else if (x + y*kCols >= MAX_PAL_ENTRY && x < kCols) { x++; y = 0; }
			break;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			if (_e->modifiers().testFlag(Qt::ControlModifier)) owner->editRGB();
			else owner->editHSV();
			return;
		case Qt::Key_R: owner->editRGB(); return;
		case Qt::Key_H: owner->editHSV(); return;
		default:
			QWidget::keyPressEvent(_e);
			return;
		}
		setXY(x, y);
		update();
		owner->updateValues();
	}

private:
	SelColorDialog* owner;
};

/*-----------------------------------------------------------------*/
SelColorDialog::SelColorDialog (int _initialIndex, QWidget* _parent)
	: QDialog(_parent)
{
	setWindowTitle(QStringLiteral("Color definition"));
	setModal(true);

	oldpal = new COLORREF[MAX_PAL_ENTRY];
	std::memcpy(oldpal, Data->palette->data, sizeof(COLORREF)*MAX_PAL_ENTRY);

	canvas = new SelColorCanvas(this, _initialIndex);

	/*  Menu bar. */
	auto* menu = new QMenuBar(this);
	QMenu* m = menu->addMenu(QStringLiteral("&Colors"));
	QAction* aRGB    = m->addAction(QStringLiteral("Edit &RGB..."));
	QAction* aHSV    = m->addAction(QStringLiteral("Edit &HSV..."));
	m->addSeparator();
	QAction* aRevert = m->addAction(QStringLiteral("Re&vert changes"));
	QAction* aClose  = m->addAction(QStringLiteral("&Close"));
	connect(aRGB,    &QAction::triggered, this, &SelColorDialog::editRGB);
	connect(aHSV,    &QAction::triggered, this, &SelColorDialog::editHSV);
	connect(aRevert, &QAction::triggered, this, &SelColorDialog::revertChanges);
	connect(aClose,  &QAction::triggered, this, &QDialog::accept);

	/*  Readout side panel. */
	labIdx = new QLabel(this);
	labH = new QLabel(this);
	labS = new QLabel(this);
	labV = new QLabel(this);
	labR = new QLabel(this);
	labG = new QLabel(this);
	labB = new QLabel(this);

	auto* gbHSV = new QGroupBox(QStringLiteral("HSV"), this);
	auto* fHSV = new QFormLayout(gbHSV);
	fHSV->addRow(QStringLiteral("Hue:"),        labH);
	fHSV->addRow(QStringLiteral("Saturation:"), labS);
	fHSV->addRow(QStringLiteral("Value:"),      labV);

	auto* gbRGB = new QGroupBox(QStringLiteral("RGB"), this);
	auto* fRGB = new QFormLayout(gbRGB);
	fRGB->addRow(QStringLiteral("Red:"),   labR);
	fRGB->addRow(QStringLiteral("Green:"), labG);
	fRGB->addRow(QStringLiteral("Blue:"),  labB);

	auto* gbIdx = new QGroupBox(this);
	auto* fIdx = new QFormLayout(gbIdx);
	fIdx->addRow(QStringLiteral("Index:"), labIdx);

	auto* side = new QVBoxLayout();
	side->addWidget(gbIdx);
	side->addWidget(gbHSV);
	side->addWidget(gbRGB);
	side->addStretch(1);

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto* row = new QHBoxLayout();
	row->addWidget(canvas, 1);
	row->addLayout(side);

	auto* root = new QVBoxLayout(this);
	root->setMenuBar(menu);
	root->addLayout(row, 1);
	root->addWidget(btns);

	canvas->setFocus();
	updateValues();
}

SelColorDialog::~SelColorDialog ()
{
	delete[] oldpal;
}

int SelColorDialog::selectedIndex () const { return canvas->selcolor; }

void SelColorDialog::updateValues ()
{
	const int idx = canvas->selcolor;
	const COLORREF clr = Data->palette->GetColor(idx);
	const int r = GetRValue(clr);
	const int g = GetGValue(clr);
	const int b = GetBValue(clr);

	QColor qc(r, g, b);
	int h, s, v;
	qc.getHsv(&h, &s, &v);
	if (h < 0) {
		labH->setText(QStringLiteral("0"));
		labS->setText(QStringLiteral("0"));
		labV->setText(QString::number(v/255.0, 'f', 3));
	} else {
		labH->setText(QString::number(h) + QStringLiteral("°"));
		labS->setText(QString::number(s/255.0, 'f', 3));
		labV->setText(QString::number(v/255.0, 'f', 3));
	}

	labR->setText(QString::number(r));
	labG->setText(QString::number(g));
	labB->setText(QString::number(b));
	labIdx->setText(QString::number(idx + 1));
}

void SelColorDialog::editRGB ()
{
	ChooseRGBDialog dlg(this);
	dlg.SelectColor(Data->palette->GetColor(canvas->selcolor));
	if (dlg.exec() == QDialog::Accepted) {
		Data->palette->SetColor(canvas->selcolor, dlg.GetSelectedColor());
		updateValues();
		canvas->update();
	}
}

void SelColorDialog::editHSV ()
{
	ChooseHSVDialog dlg(this);
	dlg.SelectColor(Data->palette->GetColor(canvas->selcolor));
	if (dlg.exec() == QDialog::Accepted) {
		Data->palette->SetColor(canvas->selcolor, dlg.GetSelectedColor());
		updateValues();
		canvas->update();
	}
}

void SelColorDialog::revertChanges ()
{
	for (int i = 0; i < MAX_PAL_ENTRY; i++)
		Data->palette->SetColor(i, oldpal[i]);
	updateValues();
	canvas->update();
}

void SelColorDialog::reject ()
{
	/*  Legacy Cancel/Escape doesn't restore; legacy only has
	    an explicit "Revert changes" menu action. Preserve that. */
	QDialog::reject();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::DefineColorsClick ()
{
	SelColorDialog dlg(Data->color, this);
	dlg.exec();
	refresh();
	UpdateStatusBar();
}
