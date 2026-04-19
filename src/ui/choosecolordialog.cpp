/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "choosecolordialog.h"

#include "datamodule.h"
#include "palette.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QSlider>
#include <QVBoxLayout>

#include <cmath>

/*=================================================================*/
/*  ChooseRGBDialog — three sliders, coloured background, preview.  */
/*=================================================================*/

/*  Gradient swatch widget: paints a vertical pure-R/G/B gradient
    behind the slider so the slider reads like the legacy pbRed/
    pbGreen/pbBlue paint boxes. */
class ChannelSwatch : public QWidget
{
public:
	enum Chan { R, G, B };
	ChannelSwatch (Chan _c, QWidget* _p) : QWidget(_p), chan(_c) {
		setFixedWidth(18);
	}
protected:
	void paintEvent (QPaintEvent*) override {
		QPainter p(this);
		const int h = height();
		const int w = width();
		for (int y = 0; y < h; y++) {
			const int v = 255*y/h;
			QColor c;
			switch (chan) {
			case R: c = QColor(v, 0, 0); break;
			case G: c = QColor(0, v, 0); break;
			case B: c = QColor(0, 0, v); break;
			}
			p.fillRect(0, y, w, 1, c);
		}
	}
private:
	Chan chan;
};

ChooseRGBDialog::ChooseRGBDialog (QWidget* _parent)
	: QDialog(_parent)
{
	setWindowTitle(QStringLiteral("Colors RGB"));
	setModal(true);

	slRed   = new QSlider(Qt::Vertical, this);
	slGreen = new QSlider(Qt::Vertical, this);
	slBlue  = new QSlider(Qt::Vertical, this);
	for (QSlider* s : { slRed, slGreen, slBlue }) {
		s->setRange(0, 255);
		s->setInvertedAppearance(true);
		connect(s, &QSlider::valueChanged, this, [this](int){ updateAll(); });
	}

	valR = new QLabel(this);
	valG = new QLabel(this);
	valB = new QLabel(this);

	preview = new QWidget(this);
	preview->setAutoFillBackground(true);
	preview->setMinimumSize(120, 120);

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto makeCol = [this](const QString& lab, QSlider* sl, QLabel* val,
	                      ChannelSwatch::Chan chan) {
		auto* col = new QVBoxLayout();
		col->addWidget(new QLabel(lab, this), 0, Qt::AlignHCenter);
		auto* row = new QHBoxLayout();
		row->addWidget(new ChannelSwatch(chan, this));
		row->addWidget(sl);
		col->addLayout(row, 1);
		col->addWidget(val, 0, Qt::AlignHCenter);
		return col;
	};

	auto* row = new QHBoxLayout();
	row->addLayout(makeCol(QStringLiteral("&Red"),   slRed,   valR, ChannelSwatch::R));
	row->addLayout(makeCol(QStringLiteral("&Green"), slGreen, valG, ChannelSwatch::G));
	row->addLayout(makeCol(QStringLiteral("&Blue"),  slBlue,  valB, ChannelSwatch::B));
	row->addSpacing(12);
	{
		auto* col = new QVBoxLayout();
		col->addWidget(new QLabel(QStringLiteral("Color sample"), this), 0, Qt::AlignHCenter);
		col->addWidget(preview, 1);
		row->addLayout(col);
	}

	auto* root = new QVBoxLayout(this);
	root->addLayout(row, 1);
	root->addWidget(btns);

	resize(360, 320);
	SelectColor(RGB(0, 0, 0));
}

void ChooseRGBDialog::SelectColor (COLORREF _c)
{
	slRed  ->setValue(GetRValue(_c));
	slGreen->setValue(GetGValue(_c));
	slBlue ->setValue(GetBValue(_c));
	updateAll();
}

COLORREF ChooseRGBDialog::GetSelectedColor () const
{
	return RGB(slRed->value(), slGreen->value(), slBlue->value());
}

void ChooseRGBDialog::updateAll ()
{
	valR->setText(QString::number(slRed  ->value()));
	valG->setText(QString::number(slGreen->value()));
	valB->setText(QString::number(slBlue ->value()));
	QPalette pal = preview->palette();
	pal.setColor(QPalette::Window, QColor(slRed->value(), slGreen->value(), slBlue->value()));
	preview->setPalette(pal);
}

/*=================================================================*/
/*  ChooseHSVDialog — hue wheel + sat/val patch + 3 sliders.        */
/*=================================================================*/

class HueWheel : public QWidget
{
public:
	HueWheel (ChooseHSVDialog* _owner) : QWidget(_owner), owner(_owner) {
		setMinimumSize(160, 160);
	}
protected:
	void paintEvent (QPaintEvent*) override {
		QPainter p(this);
		p.setRenderHint(QPainter::Antialiasing);
		const int w = width();
		const int h = height();
		const int r = std::min(w, h) - 2;
		const QRect box((w-r)/2, (h-r)/2, r, r);

		/*  Pie slices 360 times — simpler than per-pixel. */
		for (int i = 0; i < 360; i++) {
			QColor c = QColor::fromHsv(i, 255, 255);
			p.setPen(Qt::NoPen);
			p.setBrush(c);
			/*  Qt angles are 1/16th degree, 0 at 3 o'clock, CCW. */
			p.drawPie(box, i*16, 16);
		}

		/*  Hue indicator. */
		const int hue = owner->hue();
		const double phi = -hue * M_PI / 180.0;
		const int cx = box.center().x();
		const int cy = box.center().y();
		const int rout = r/2;
		const int x1 = cx + int(std::cos(phi) * rout);
		const int y1 = cy + int(std::sin(phi) * rout);
		const int x2 = cx + int(std::cos(phi) * (rout - 12));
		const int y2 = cy + int(std::sin(phi) * (rout - 12));
		p.setPen(QPen(owner->val() > 120 ? Qt::black : Qt::white, 2));
		p.drawLine(x1, y1, x2, y2);
	}

	void mouseReleaseEvent (QMouseEvent* _e) override {
		const int cx = width()/2;
		const int cy = height()/2;
		int dx = int(_e->position().x()) - cx;
		int dy = int(_e->position().y()) - cy;
		if (dx == 0 && dy == 0) return;
		double phi = std::atan2(double(-dy), double(dx));
		if (phi < 0) phi += 2.0 * M_PI;
		int h = int(360.0 * phi / (2.0 * M_PI)) % 360;
		owner->setHSV(h, owner->sat(), owner->val());
	}
private:
	ChooseHSVDialog* owner;
};

class SatValPatch : public QWidget
{
public:
	SatValPatch (ChooseHSVDialog* _owner) : QWidget(_owner), owner(_owner) {
		setMinimumSize(140, 140);
	}
protected:
	void paintEvent (QPaintEvent*) override {
		QPainter p(this);
		const int w = width();
		const int h = height();
		const int hue = owner->hue();
		/*  Simple axis-aligned fill: x=sat, y=inverse val. */
		for (int y = 0; y < h; y += 2) {
			const int v = 255 - 255*y/h;
			for (int x = 0; x < w; x += 2) {
				const int s = 255*x/w;
				p.fillRect(x, y, 2, 2, QColor::fromHsv(hue, s, v));
			}
		}
		/*  Marker. */
		const int mx = w * owner->sat() / 255;
		const int my = h * (255 - owner->val()) / 255;
		p.setPen(QPen(owner->val() > 120 ? Qt::black : Qt::white, 2));
		p.setBrush(Qt::NoBrush);
		p.drawEllipse(QPoint(mx, my), 4, 4);
	}

	void mouseReleaseEvent (QMouseEvent* _e) override {
		const int s = 255 * int(_e->position().x()) / width();
		const int v = 255 * (height() - int(_e->position().y())) / height();
		owner->setHSV(owner->hue(),
		              std::clamp(s, 0, 255),
		              std::clamp(v, 0, 255));
	}
private:
	ChooseHSVDialog* owner;
};

ChooseHSVDialog::ChooseHSVDialog (QWidget* _parent)
	: QDialog(_parent)
{
	setWindowTitle(QStringLiteral("Colors HSV"));
	setModal(true);

	slHue = new QSlider(Qt::Horizontal, this);
	slSat = new QSlider(Qt::Horizontal, this);
	slVal = new QSlider(Qt::Horizontal, this);
	slHue->setRange(0, 360);
	slSat->setRange(0, 255);
	slVal->setRange(0, 255);

	valH = new QLabel(this);
	valS = new QLabel(this);
	valV = new QLabel(this);

	wheel = new HueWheel(this);
	patch = new SatValPatch(this);

	preview = new QWidget(this);
	preview->setAutoFillBackground(true);
	preview->setMinimumSize(80, 80);

	connect(slHue, &QSlider::valueChanged, this, [this]{ sync(); });
	connect(slSat, &QSlider::valueChanged, this, [this]{ sync(); });
	connect(slVal, &QSlider::valueChanged, this, [this]{ sync(); });

	auto* form = new QFormLayout();
	form->addRow(QStringLiteral("&Hue:"),        slHue);
	form->addRow(QStringLiteral("&Saturation:"), slSat);
	form->addRow(QStringLiteral("&Value:"),      slVal);

	auto* vals = new QFormLayout();
	vals->addRow(QStringLiteral("Hue:"),        valH);
	vals->addRow(QStringLiteral("Saturation:"), valS);
	vals->addRow(QStringLiteral("Value:"),      valV);

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto* grid = new QGridLayout();
	grid->addWidget(new QLabel(QStringLiteral("Color hues"), this),            0, 0, Qt::AlignHCenter);
	grid->addWidget(new QLabel(QStringLiteral("Saturation and Value"), this), 0, 1, Qt::AlignHCenter);
	grid->addWidget(new QLabel(QStringLiteral("Color sample"), this),          0, 2, Qt::AlignHCenter);
	grid->addWidget(wheel,   1, 0);
	grid->addWidget(patch,   1, 1);
	grid->addWidget(preview, 1, 2);
	grid->addLayout(form,    2, 0, 1, 2);
	grid->addLayout(vals,    2, 2);

	auto* root = new QVBoxLayout(this);
	root->addLayout(grid, 1);
	root->addWidget(btns);

	resize(560, 420);
	SelectColor(RGB(255, 0, 0));
}

int ChooseHSVDialog::hue () const { return slHue->value(); }
int ChooseHSVDialog::sat () const { return slSat->value(); }
int ChooseHSVDialog::val () const { return slVal->value(); }

void ChooseHSVDialog::setHSV (int _h, int _s, int _v)
{
	QSignalBlocker b1(slHue), b2(slSat), b3(slVal);
	slHue->setValue(_h);
	slSat->setValue(_s);
	slVal->setValue(_v);
	updateAll();
}

void ChooseHSVDialog::SelectColor (COLORREF _c)
{
	QColor q(GetRValue(_c), GetGValue(_c), GetBValue(_c));
	int h, s, v;
	q.getHsv(&h, &s, &v);
	if (h < 0) h = 0;
	setHSV(h, s, v);
}

COLORREF ChooseHSVDialog::GetSelectedColor () const
{
	QColor q = QColor::fromHsv(hue(), sat(), val());
	return RGB(q.red(), q.green(), q.blue());
}

void ChooseHSVDialog::sync () { updateAll(); }

void ChooseHSVDialog::updateAll ()
{
	valH->setText(QString::number(slHue->value()) + QStringLiteral("°"));
	valS->setText(QString::number(slSat->value()/255.0, 'f', 3));
	valV->setText(QString::number(slVal->value()/255.0, 'f', 3));
	QPalette pal = preview->palette();
	pal.setColor(QPalette::Window,
	             QColor::fromHsv(hue(), sat(), val()));
	preview->setPalette(pal);
	preview->update();
	wheel->update();
	patch->update();
}

/*=================================================================*/
/*  ChoosePaletteDialog — pick an entry from the 236-colour table.  */
/*=================================================================*/

class PaletteCanvas : public QWidget
{
public:
	static constexpr int gridSize = 16;
	PaletteCanvas (ChoosePaletteDialog* _owner)
		: QWidget(_owner), owner(_owner)
	{
		setFocusPolicy(Qt::StrongFocus);
		setMinimumSize(gridSize*cell + 1, gridSize*cell + 1);
	}

	int index = 0;
	static constexpr int cell = 16;

protected:
	void paintEvent (QPaintEvent*) override {
		QPainter p(this);
		p.setPen(Qt::black);
		for (int i = 0; i < gridSize; i++) {
			for (int j = 0; j < gridSize; j++) {
				const int idx = i*gridSize + j;
				if (idx >= MAX_PAL_ENTRY) break;
				const COLORREF c = Data->palette->GetColor(idx);
				p.fillRect(i*cell, j*cell, cell, cell,
				           QColor(GetRValue(c), GetGValue(c), GetBValue(c)));
				p.drawRect(i*cell, j*cell, cell, cell);
			}
		}
		/*  Cursor. */
		const int x = (index/gridSize)*cell;
		const int y = (index%gridSize)*cell;
		p.setPen(QPen(hasFocus() ? Qt::white : Qt::black, 1));
		p.setBrush(Qt::NoBrush);
		p.drawRect(x, y, cell, cell);
	}

	void mousePressEvent (QMouseEvent* _e) override {
		if (_e->button() != Qt::LeftButton) return;
		const int i = int(_e->position().x())/cell;
		const int j = int(_e->position().y())/cell;
		if (i < 0 || i >= gridSize || j < 0 || j >= gridSize) return;
		const int newIdx = i*gridSize + j;
		if (newIdx >= MAX_PAL_ENTRY) return;
		index = newIdx;
		setFocus();
		update();
	}

	void mouseDoubleClickEvent (QMouseEvent* _e) override {
		mousePressEvent(_e);
		if (auto* d = qobject_cast<QDialog*>(window())) d->accept();
	}

	void keyPressEvent (QKeyEvent* _e) override {
		int x = index / gridSize;
		int y = index % gridSize;
		switch (_e->key()) {
		case Qt::Key_Left:  if (x > 0) --x; break;
		case Qt::Key_Right: if (x < gridSize-1) ++x; break;
		case Qt::Key_Up:
			if (y > 0) --y;
			else if (x > 0) { y = gridSize-1; --x; }
			break;
		case Qt::Key_Down:
			if (y < gridSize-1) ++y;
			else if (x < gridSize-1) { y = 0; ++x; }
			break;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			if (auto* d = qobject_cast<QDialog*>(window())) d->accept();
			return;
		default:
			QWidget::keyPressEvent(_e);
			return;
		}
		const int newIdx = x*gridSize + y;
		if (newIdx < MAX_PAL_ENTRY) { index = newIdx; update(); }
	}

private:
	ChoosePaletteDialog* owner;
};

ChoosePaletteDialog::ChoosePaletteDialog (QWidget* _parent)
	: QDialog(_parent)
{
	setWindowTitle(QStringLiteral("Colors Palette"));
	setModal(true);

	canvas = new PaletteCanvas(this);

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto* root = new QVBoxLayout(this);
	root->addWidget(canvas, 1);
	root->addWidget(btns);

	canvas->setFocus();
}

int ChoosePaletteDialog::GetSelectedIndex () const { return canvas->index; }

COLORREF ChoosePaletteDialog::GetSelectedColor () const
{
	return Data->palette->GetColor(canvas->index);
}

void ChoosePaletteDialog::SelectColor (COLORREF _c)
{
	for (int i = 0; i < MAX_PAL_ENTRY; i++) {
		if (Data->palette->GetColor(i) == _c) {
			canvas->index = i;
			canvas->update();
			return;
		}
	}
	canvas->index = 0;
	canvas->update();
}
