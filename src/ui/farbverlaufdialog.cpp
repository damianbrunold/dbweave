/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "farbverlaufdialog.h"

#include "mainwindow.h"
#include "datamodule.h"
#include "palette.h"

#include <QCheckBox>
#include <QMenu>
#include "choosecolordialog.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>

/*  Last-used endpoints survive across dialog invocations like the
    legacy g_startfarbe / g_endfarbe globals. */
static COLORREF g_startfarbe = RGB(0, 0, 255);
static COLORREF g_endfarbe   = RGB(255, 255, 0);

/*-----------------------------------------------------------------*/
static QColor qcol (COLORREF _c)
{
	return QColor(GetRValue(_c), GetGValue(_c), GetBValue(_c));
}

/*-----------------------------------------------------------------*/
/*  Swatch is a plain QWidget filled with a solid colour via
    autoFillBackground + palette. Click opens QColorDialog. */
class Swatch : public QWidget
{
public:
	Swatch (QWidget* _parent, std::function<void()> _onClick)
		: QWidget(_parent), onClick(std::move(_onClick))
	{
		setAutoFillBackground(true);
		setFixedSize(60, 24);
		setCursor(Qt::PointingHandCursor);
	}
	void setColor (COLORREF _c) {
		QPalette p = palette();
		p.setColor(QPalette::Window, qcol(_c));
		setPalette(p);
		update();
	}
protected:
	void mouseReleaseEvent (QMouseEvent* _e) override {
		if (_e->button() == Qt::LeftButton) onClick();
	}
private:
	std::function<void()> onClick;
};

/*-----------------------------------------------------------------*/
class BlendStrip : public QWidget
{
public:
	BlendStrip (FarbverlaufDialog* _parent,
	            const COLORREF* _table, const int* _count,
	            const COLORREF* _start, const COLORREF* _end,
	            const bool* _noDividers)
		: QWidget(_parent), table(_table), countPtr(_count),
		  startPtr(_start), endPtr(_end), noDiv(_noDividers)
	{
		setMinimumHeight(32);
	}
protected:
	void paintEvent (QPaintEvent*) override {
		QPainter p(this);
		const int n  = *countPtr;
		const int w  = width();
		const int dx = w / (n+2);
		const QColor border = *noDiv ? qcol(*startPtr) : QColor(Qt::black);
		p.setPen(border);
		p.setBrush(qcol(*startPtr));
		p.drawRect(0, 0, dx, height()-1);
		for (int i = 0; i < n; i++) {
			const QColor c = qcol(table[i]);
			p.setPen(*noDiv ? c : QColor(Qt::black));
			p.setBrush(c);
			p.drawRect((i+1)*dx, 0, dx, height()-1);
		}
		p.setPen(*noDiv ? qcol(*endPtr) : QColor(Qt::black));
		p.setBrush(qcol(*endPtr));
		p.drawRect((n+1)*dx, 0, dx, height()-1);
	}
private:
	const COLORREF* table;
	const int*      countPtr;
	const COLORREF* startPtr;
	const COLORREF* endPtr;
	const bool*     noDiv;
};

/*-----------------------------------------------------------------*/
FarbverlaufDialog::FarbverlaufDialog (TDBWFRM* _frm, QWidget* _parent)
	: QDialog(_parent), frm(_frm)
{
	setWindowTitle(QStringLiteral("Create color blending"));
	setModal(true);

	startcolor = g_startfarbe;
	endcolor   = g_endfarbe;
	for (int i = 0; i < MAX_PAL_ENTRY; i++) table[i] = RGB(255, 255, 255);
	count = 0;

	auto* swStart = new Swatch(this, [this]{
		COLORREF c = pickColor(startcolor);
		if (c != startcolor) { startcolor = c; updateSwatch(swatchStart, startcolor); createFarbverlauf(); repaintBlend(); }
	});
	auto* swEnd = new Swatch(this, [this]{
		COLORREF c = pickColor(endcolor);
		if (c != endcolor) { endcolor = c; updateSwatch(swatchEnd, endcolor); createFarbverlauf(); repaintBlend(); }
	});
	swatchStart = swStart;
	swatchEnd   = swEnd;
	updateSwatch(swatchStart, startcolor);
	updateSwatch(swatchEnd,   endcolor);

	edSteps = new QSpinBox(this);
	edSteps->setRange(1, 150);
	edSteps->setValue(20);

	edPosition = new QSpinBox(this);
	edPosition->setRange(1, MAX_PAL_ENTRY-1);
	edPosition->setValue(1);

	slWeight = new QSlider(Qt::Horizontal, this);
	slWeight->setRange(1, 100);
	slWeight->setValue(50);

	nodividers = new QCheckBox(QStringLiteral("N&o dividers"), this);
	nodividers->setChecked(true);

	modelRGB = new QRadioButton(QStringLiteral("&RGB"), this);
	modelHSV = new QRadioButton(QStringLiteral("&HSV"), this);
	modelRGB->setChecked(true);
	auto* gb = new QGroupBox(QStringLiteral("Color model for interpolation"), this);
	{
		auto* l = new QHBoxLayout(gb);
		l->addWidget(modelRGB);
		l->addWidget(modelHSV);
	}

	static bool noDivFlag = true;
	auto* blend = new BlendStrip(this, table, &count, &startcolor, &endcolor, &noDivFlag);
	swatchBlend = blend;

	connect(edSteps, qOverload<int>(&QSpinBox::valueChanged), this, [this]{ createFarbverlauf(); repaintBlend(); });
	connect(slWeight, &QSlider::valueChanged, this, [this]{ createFarbverlauf(); repaintBlend(); });
	connect(nodividers, &QCheckBox::toggled, this, [this](bool _v){
		/*  NoDivFlag is static to let BlendStrip reference it; keep in
		    sync with the checkbox. */
		static_cast<void>(_v);
		repaintBlend();
	});
	connect(modelRGB, &QRadioButton::toggled, this, [this]{ createFarbverlauf(); repaintBlend(); });
	connect(modelHSV, &QRadioButton::toggled, this, [this]{ createFarbverlauf(); repaintBlend(); });
	connect(nodividers, &QCheckBox::toggled, this, [](bool _v){ noDivFlag = _v; });

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(btns, &QDialogButtonBox::accepted, this, &FarbverlaufDialog::accept);
	connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

	/*  Layout. */
	auto* top = new QGridLayout();
	top->addWidget(new QLabel(QStringLiteral("Start color:"), this), 0, 0);
	top->addWidget(swatchStart,                                       0, 1);
	top->addWidget(new QLabel(QStringLiteral("End color:"), this),   0, 2);
	top->addWidget(swatchEnd,                                         0, 3);

	auto* form = new QFormLayout();
	form->addRow(QStringLiteral("S&teps:"),         edSteps);
	form->addRow(QStringLiteral("&Weighting:"),     slWeight);
	form->addRow(QStringLiteral("&Palette index:"), edPosition);

	auto* root = new QVBoxLayout(this);
	root->addLayout(top);
	root->addWidget(gb);
	root->addLayout(form);
	root->addWidget(nodividers);
	root->addWidget(blend);
	root->addWidget(btns);

	createFarbverlauf();
	repaintBlend();
}

/*-----------------------------------------------------------------*/
/*  Popup-menu matching legacy PopupMenuFarbwahl: choose the colour
    via HSV picker, RGB picker, palette picker, or copy the other
    endpoint. Returns _col unchanged if the user cancels. `other`
    is the peer endpoint for the Copy-from action. */
static COLORREF pickColorMenu (QWidget* _parent, COLORREF _col, COLORREF _other)
{
	QMenu menu(_parent);
	QAction* aHSV = menu.addAction(QStringLiteral("Choose color with &HSV model"));
	QAction* aRGB = menu.addAction(QStringLiteral("Choose color with &RGB model"));
	QAction* aPal = menu.addAction(QStringLiteral("Choose color from &palette"));
	menu.addSeparator();
	QAction* aCopy = menu.addAction(QStringLiteral("&Copy other color"));

	QAction* chosen = menu.exec(QCursor::pos());
	if (!chosen) return _col;

	if (chosen == aHSV) {
		ChooseHSVDialog dlg(_parent);
		dlg.SelectColor(_col);
		if (dlg.exec() == QDialog::Accepted) return dlg.GetSelectedColor();
	} else if (chosen == aRGB) {
		ChooseRGBDialog dlg(_parent);
		dlg.SelectColor(_col);
		if (dlg.exec() == QDialog::Accepted) return dlg.GetSelectedColor();
	} else if (chosen == aPal) {
		ChoosePaletteDialog dlg(_parent);
		dlg.SelectColor(_col);
		if (dlg.exec() == QDialog::Accepted) return dlg.GetSelectedColor();
	} else if (chosen == aCopy) {
		return _other;
	}
	return _col;
}

COLORREF FarbverlaufDialog::pickColor (COLORREF _col)
{
	/*  The "other" endpoint for the Copy action: whichever of the
	    two isn't _col. */
	COLORREF other = (_col == startcolor) ? endcolor : startcolor;
	return pickColorMenu(this, _col, other);
}

void FarbverlaufDialog::updateSwatch (QWidget* _w, COLORREF _col)
{
	if (auto* s = static_cast<Swatch*>(_w)) s->setColor(_col);
}

void FarbverlaufDialog::repaintBlend ()
{
	if (swatchBlend) swatchBlend->update();
}

/*-----------------------------------------------------------------*/
void FarbverlaufDialog::createFarbverlauf ()
{
	const int abstufungen = edSteps->value();
	if (modelRGB->isChecked()) farbverlaufRGB(abstufungen);
	else                        farbverlaufHSV(abstufungen);
}

/*  Legacy weighting formula verbatim -- see farbverlauf_form.cpp.
    d=1 is fixed; m comes from the weighting slider normalised to
    [0,1]. The three branches cover m in [0.125, 1]; we clamp
    silently outside that range because Qt's slider can dip lower. */
static double weightT (double _x, double _m)
{
	const double d = 1.0;
	double m = _m;
	if (m < 0.125) m = 0.125;
	if (m > 1.0)   m = 1.0;
	if (m > 0.5)
		return (2*d - 4*m)*_x*_x + (4*m - d)*_x;
	if (m >= 0.25)
		return (2*d - 4*m)*_x*_x + (4*m - d)*_x;
	return (2*d - 8*m)*_x*_x*_x + (8*m - d)*_x*_x;
}

void FarbverlaufDialog::farbverlaufRGB (int _abstufungen)
{
	const double m = double(slWeight->value()) / (slWeight->minimum() + slWeight->maximum());
	count = 0;
	for (int i = 0; i < _abstufungen; i++) {
		const double x = double(i+1)/(_abstufungen+1);
		const double t = weightT(x, m);
		const int r = GetRValue(startcolor) + int((GetRValue(endcolor) - GetRValue(startcolor)) * t);
		const int g = GetGValue(startcolor) + int((GetGValue(endcolor) - GetGValue(startcolor)) * t);
		const int b = GetBValue(startcolor) + int((GetBValue(endcolor) - GetBValue(startcolor)) * t);
		table[count++] = RGB(r, g, b);
	}
}

void FarbverlaufDialog::farbverlaufHSV (int _abstufungen)
{
	const double m = double(slWeight->value()) / (slWeight->minimum() + slWeight->maximum());
	const QColor cs = qcol(startcolor);
	const QColor ce = qcol(endcolor);
	float h1, s1, v1, h2, s2, v2;
	cs.getHsvF(&h1, &s1, &v1);
	ce.getHsvF(&h2, &s2, &v2);
	if (h1 < 0) h1 = 0; /*  achromatic — Qt returns -1 */
	if (h2 < 0) h2 = 0;
	count = 0;
	for (int i = 0; i < _abstufungen; i++) {
		const double x = double(i+1)/(_abstufungen+1);
		const double t = weightT(x, m);
		const float h = float(h1 + (h2-h1)*t);
		const float s = float(s1 + (s2-s1)*t);
		const float v = float(v1 + (v2-v1)*t);
		QColor c = QColor::fromHsvF(std::max(0.f, std::min(1.f, h)),
		                            std::max(0.f, std::min(1.f, s)),
		                            std::max(0.f, std::min(1.f, v)));
		table[count++] = RGB(c.red(), c.green(), c.blue());
	}
}

/*-----------------------------------------------------------------*/
void FarbverlaufDialog::accept ()
{
	if (count != 0) {
		const int index = edPosition->value();
		if (index < 1 || index >= MAX_PAL_ENTRY) {
			QMessageBox::information(this, QStringLiteral("DB-WEAVE"),
			    QStringLiteral("Palette index out of range."));
			return;
		}
		Data->palette->SetColor(index-1,         startcolor);
		Data->palette->SetColor(index-1+count+1, endcolor);
		for (int i = index; i < std::min(index+count, MAX_PAL_ENTRY); i++)
			Data->palette->SetColor(i, table[i-index]);
		frm->update();
		g_startfarbe = startcolor;
		g_endfarbe   = endcolor;
	}
	QDialog::accept();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::FarbverlaufClick ()
{
	FarbverlaufDialog dlg(this, this);
	dlg.exec();
}
