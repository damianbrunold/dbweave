/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "blockmusterdialog.h"

#include "mainwindow.h"
#include "datamodule.h"
#include "cursor.h"
#include "undoredo.h"
#include "einzug.h"
#include "rangecolors.h"
#include "zentralsymm.h"
#include "colors_compat.h"

#include <QAction>
#include <QActionGroup>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>

/*-----------------------------------------------------------------*/
static QColor colorFromTColor (TColor _c)
{
	const int r = _c & 0xFF;
	const int g = (_c >> 8) & 0xFF;
	const int b = (_c >> 16) & 0xFF;
	return QColor(r, g, b);
}

static QColor bindungColor (int _s)
{
	/*  Index 0 (Grundmuster) shows in grey; the numbered patterns
	    borrow the shared range palette. */
	if (_s == 0) return QColor(128, 128, 128);
	return colorFromTColor(GetRangeColor(_s));
}

/*-----------------------------------------------------------------*/
/*  Inner widget: paints the 12x12 editor grid and converts mouse
    clicks into toggles on the current Muster.                     */
class BlockmusterCanvas : public QWidget
{
public:
	BlockmusterCanvas (BlockmusterDialog* _owner)
		: QWidget(_owner), owner(_owner)
	{
		setMinimumSize(Muster::maxx*cellSize + 2, Muster::maxy*cellSize + 2);
		QPalette pal = palette();
		pal.setColor(QPalette::Window, QColor(192, 192, 192));
		setPalette(pal);
		setAutoFillBackground(true);
		setFocusPolicy(Qt::StrongFocus);
	}

	int cx = 0;
	int cy = 0;
	CURSORDIRECTION cursordirection = CD_UP;

	static constexpr int cellSize = 18;

protected:
	void paintEvent (QPaintEvent*) override {
		QPainter p(this);
		const int dx = cellSize;
		const int dy = cellSize;
		const int mxG = Muster::maxx;
		const int myG = Muster::maxy;

		p.setPen(QColor(105, 105, 105));
		for (int i = 0; i <= mxG; i++) p.drawLine(i*dx, 0, i*dx, myG*dy);
		for (int j = 0; j <= myG; j++) p.drawLine(0, j*dy, mxG*dx, j*dy);

		Muster& m = (*owner->bindungen)[owner->current];
		for (int i = 0; i < mxG; i++) {
			for (int j = 0; j < myG; j++) {
				const char c = m.Get(i, j);
				if (c == 0) continue;
				const QColor col = owner->current == 0
				    ? QColor(128, 128, 128)
				    : bindungColor(c);
				p.fillRect(i*dx+2, (myG-j-1)*dy+2, dx-3, dy-3, col);
			}
		}

		if (owner->withRange && owner->mx >= 0 && owner->my >= 0) {
			p.setPen(QColor(Qt::red));
			p.drawLine(1, (myG-1-owner->my)*dy, (owner->mx+1)*dx, (myG-1-owner->my)*dy);
			p.drawLine((owner->mx+1)*dx, (myG-1-owner->my)*dy, (owner->mx+1)*dx, myG*dy);
		}

		if (hasFocus()) {
			p.setPen(QColor(Qt::white));
			p.setBrush(Qt::NoBrush);
			p.drawRect(cx*dx, (myG-1-cy)*dy, dx, dy);
		}
	}

	void keyPressEvent (QKeyEvent* _e) override {
		const int mxG = Muster::maxx;
		const int myG = Muster::maxy;
		const bool ctrl  = _e->modifiers().testFlag(Qt::ControlModifier);
		const bool shift = _e->modifiers().testFlag(Qt::ShiftModifier);
		const int step = ctrl ? 4 : 1;

		switch (_e->key()) {
		case Qt::Key_Left:
			if (shift) { owner->rollLeft(); return; }
			cx -= step; if (cx < 0) cx = 0;
			update();
			break;
		case Qt::Key_Right:
			if (shift) { owner->rollRight(); return; }
			cx += step; if (cx > mxG-1) cx = mxG-1;
			update();
			break;
		case Qt::Key_Up:
			if (shift) { owner->rollUp(); return; }
			cy += step; if (cy > myG-1) cy = myG-1;
			update();
			break;
		case Qt::Key_Down:
			if (shift) { owner->rollDown(); return; }
			cy -= step; if (cy < 0) cy = 0;
			update();
			break;
		case Qt::Key_Space: {
			Muster& m = (*owner->bindungen)[owner->current];
			const char old = m.Get(cx, cy);
			m.Set(cx, cy, BlockmusterDialog::toggle(old, owner->current));
			owner->calcRange();
			owner->refreshUsed();
			owner->undo.Snapshot();
			/*  Advance per cursor direction. */
			if ((cursordirection & CD_UP)    && cy < myG-1) cy++;
			else if ((cursordirection & CD_DOWN)  && cy > 0) cy--;
			if ((cursordirection & CD_LEFT)  && cx > 0) cx--;
			else if ((cursordirection & CD_RIGHT) && cx < mxG-1) cx++;
			update();
			break;
		}
		case Qt::Key_Return:
		case Qt::Key_Enter:
			owner->selectBindung(shift
			    ? (owner->current + 9) % 10
			    : (owner->current + 1) % 10);
			break;
		case Qt::Key_0: owner->selectBindung(0); break;
		case Qt::Key_1: owner->selectBindung(1); break;
		case Qt::Key_2: owner->selectBindung(2); break;
		case Qt::Key_3: owner->selectBindung(3); break;
		case Qt::Key_4: owner->selectBindung(4); break;
		case Qt::Key_5: owner->selectBindung(5); break;
		case Qt::Key_6: owner->selectBindung(6); break;
		case Qt::Key_7: owner->selectBindung(7); break;
		case Qt::Key_8: owner->selectBindung(8); break;
		case Qt::Key_9: owner->selectBindung(9); break;
		default:
			QWidget::keyPressEvent(_e);
			return;
		}
	}

	void mouseReleaseEvent (QMouseEvent* _e) override {
		if (_e->button() != Qt::LeftButton) return;
		const int dx = cellSize;
		const int dy = cellSize;
		const int myG = Muster::maxy;
		const int i = int(_e->position().x()) / dx;
		const int j = myG-1 - int(_e->position().y()) / dy;
		if (i < 0 || j < 0) return;
		if (i >= Muster::maxx || j >= Muster::maxy) return;
		Muster& m = (*owner->bindungen)[owner->current];
		const char old = m.Get(i, j);
		m.Set(i, j, BlockmusterDialog::toggle(old, owner->current));
		owner->calcRange();
		owner->refreshUsed();
		owner->undo.Snapshot();
		cx = i; cy = j;
		setFocus();
		update();
	}

private:
	BlockmusterDialog* owner;
};

/*-----------------------------------------------------------------*/
BlockmusterDialog::BlockmusterDialog (TDBWFRM* _frm, BlockUndo& _undo,
                                      PMUSTERARRAY _bindungen, int& _current,
                                      const QString& _caption, bool _withRange)
	: QDialog(_frm)
	, frm(_frm)
	, undo(_undo)
	, bindungen(_bindungen)
	, current(_current)
	, withRange(_withRange)
{
	setWindowTitle(_caption);
	setModal(true);

	einzugZ     = frm->einzugZ;
	trittfolgeZ = frm->trittfolgeZ;

	auto* menubar = new QMenuBar(this);

	/*  Pattern menu: Apply / Close. */
	QMenu* mMuster = menubar->addMenu(QStringLiteral("&Pattern"));
	QAction* aApply = mMuster->addAction(QStringLiteral("&Apply"));
	QAction* aClose = mMuster->addAction(QStringLiteral("&Close"));
	connect(aApply, &QAction::triggered, this, &BlockmusterDialog::accept);
	connect(aClose, &QAction::triggered, this, &QDialog::reject);

	/*  Threading / treadling direction sub-menus; hidden unless
	    called with withRange=true (block-substitution mode). */
	if (withRange) {
		QMenu* mEz = menubar->addMenu(QStringLiteral("&Threading"));
		auto* gEz = new QActionGroup(this);
		einzugZAct = mEz->addAction(QStringLiteral("Straight &rising"));
		einzugSAct = mEz->addAction(QStringLiteral("Straight &falling"));
		einzugZAct->setCheckable(true);
		einzugSAct->setCheckable(true);
		gEz->addAction(einzugZAct);
		gEz->addAction(einzugSAct);
		(einzugZ ? einzugZAct : einzugSAct)->setChecked(true);
		connect(einzugZAct, &QAction::triggered, this, [this]{ einzugZ = true;  });
		connect(einzugSAct, &QAction::triggered, this, [this]{ einzugZ = false; });

		QMenu* mTf = menubar->addMenu(QStringLiteral("Tre&adling"));
		auto* gTf = new QActionGroup(this);
		trittfolgeZAct = mTf->addAction(QStringLiteral("Straight &rising"));
		trittfolgeSAct = mTf->addAction(QStringLiteral("Straight &falling"));
		trittfolgeZAct->setCheckable(true);
		trittfolgeSAct->setCheckable(true);
		gTf->addAction(trittfolgeZAct);
		gTf->addAction(trittfolgeSAct);
		(trittfolgeZ ? trittfolgeZAct : trittfolgeSAct)->setChecked(true);
		connect(trittfolgeZAct, &QAction::triggered, this, [this]{ trittfolgeZ = true;  });
		connect(trittfolgeSAct, &QAction::triggered, this, [this]{ trittfolgeZ = false; });
	}

	/*  Edit menu (transforms). */
	QMenu* mEdit = menubar->addMenu(QStringLiteral("&Edit"));
	QAction* aDel  = mEdit->addAction(QStringLiteral("&Delete"));
	QAction* aMh   = mEdit->addAction(QStringLiteral("Mirror &horizontal"));
	QAction* aMv   = mEdit->addAction(QStringLiteral("Mirror &vertical"));
	QAction* aRot  = mEdit->addAction(QStringLiteral("&Rotate"));
	QAction* aInv  = mEdit->addAction(QStringLiteral("&Invert"));
	QAction* aSym  = mEdit->addAction(QStringLiteral("&Make central symmetric"));
	mEdit->addSeparator();
	QMenu*   mRoll = mEdit->addMenu(QStringLiteral("&Roll"));
	QAction* aRu   = mRoll->addAction(QStringLiteral("&Up"));
	QAction* aRd   = mRoll->addAction(QStringLiteral("&Down"));
	QAction* aRl   = mRoll->addAction(QStringLiteral("&Left"));
	QAction* aRr   = mRoll->addAction(QStringLiteral("&Right"));
	mEdit->addSeparator();
	undoAct = mEdit->addAction(QStringLiteral("&Undo"));
	redoAct = mEdit->addAction(QStringLiteral("Re&do"));
	connect(aDel, &QAction::triggered, this, &BlockmusterDialog::editDelete);
	connect(aMh,  &QAction::triggered, this, &BlockmusterDialog::editMirrorH);
	connect(aMv,  &QAction::triggered, this, &BlockmusterDialog::editMirrorV);
	connect(aRot, &QAction::triggered, this, &BlockmusterDialog::editRotate);
	connect(aInv, &QAction::triggered, this, &BlockmusterDialog::editInvert);
	connect(aSym, &QAction::triggered, this, &BlockmusterDialog::editCentralsym);
	connect(aRu,  &QAction::triggered, this, &BlockmusterDialog::rollUp);
	connect(aRd,  &QAction::triggered, this, &BlockmusterDialog::rollDown);
	connect(aRl,  &QAction::triggered, this, &BlockmusterDialog::rollLeft);
	connect(aRr,  &QAction::triggered, this, &BlockmusterDialog::rollRight);
	connect(undoAct, &QAction::triggered, this, &BlockmusterDialog::doUndo);
	connect(redoAct, &QAction::triggered, this, &BlockmusterDialog::doRedo);

	/*  Pattern-preset menu (Leinwand / Koeper / Atlas / Panama). */
	QMenu* mPreset = menubar->addMenu(QStringLiteral("&Binding"));
	auto addPreset = [&](const QString& label, auto slot) {
		QAction* a = mPreset->addAction(label);
		connect(a, &QAction::triggered, this, slot);
	};
	addPreset(QStringLiteral("Tabby (Leinwand)"), [this]{ musterKoeper(1,1); });
	addPreset(QStringLiteral("Twill 2/2"),        [this]{ musterKoeper(2,2); });
	addPreset(QStringLiteral("Twill 3/3"),        [this]{ musterKoeper(3,3); });
	addPreset(QStringLiteral("Twill 4/4"),        [this]{ musterKoeper(4,4); });
	mPreset->addSeparator();
	addPreset(QStringLiteral("Twill 2/1"), [this]{ musterKoeper(2,1); });
	addPreset(QStringLiteral("Twill 3/1"), [this]{ musterKoeper(3,1); });
	addPreset(QStringLiteral("Twill 3/2"), [this]{ musterKoeper(3,2); });
	addPreset(QStringLiteral("Twill 4/1"), [this]{ musterKoeper(4,1); });
	addPreset(QStringLiteral("Twill 4/2"), [this]{ musterKoeper(4,2); });
	addPreset(QStringLiteral("Twill 4/3"), [this]{ musterKoeper(4,3); });
	addPreset(QStringLiteral("Twill 5/1"), [this]{ musterKoeper(5,1); });
	addPreset(QStringLiteral("Twill 5/2"), [this]{ musterKoeper(5,2); });
	addPreset(QStringLiteral("Twill 5/3"), [this]{ musterKoeper(5,3); });
	mPreset->addSeparator();
	addPreset(QStringLiteral("Satin 5x5"),  [this]{ musterAtlas(5); });
	addPreset(QStringLiteral("Satin 7x7"),  [this]{ musterAtlas(7); });
	addPreset(QStringLiteral("Satin 9x9"),  [this]{ musterAtlas(9); });
	mPreset->addSeparator();
	addPreset(QStringLiteral("Extended tabby 2/1"), [this]{ musterPanama(2,1); });
	addPreset(QStringLiteral("Extended tabby 2/2"), [this]{ musterPanama(2,2); });

	/*  Bindung selector menu — 10 entries Grundmuster + 1..9. */
	QMenu* mBind = menubar->addMenu(QStringLiteral("B&indung"));
	auto* gBind = new QActionGroup(this);
	const QString labels[10] = {
	    QStringLiteral("&Base pattern"),
	    QStringLiteral("Pattern &1"), QStringLiteral("Pattern &2"),
	    QStringLiteral("Pattern &3"), QStringLiteral("Pattern &4"),
	    QStringLiteral("Pattern &5"), QStringLiteral("Pattern &6"),
	    QStringLiteral("Pattern &7"), QStringLiteral("Pattern &8"),
	    QStringLiteral("Pattern &9")
	};
	for (int i = 0; i < 10; i++) {
		bindungActs[i] = mBind->addAction(labels[i]);
		bindungActs[i]->setCheckable(true);
		gBind->addAction(bindungActs[i]);
		connect(bindungActs[i], &QAction::triggered, this, [this, i]{ selectBindung(i); });
	}

	/*  Copy-from menu: 10 entries, copy the current layout from
	    another Bindung slot (double-toggled to re-colour to the
	    destination slot's shade). */
	QMenu* mCopy = menubar->addMenu(QStringLiteral("&Copy from"));
	for (int i = 0; i < 10; i++) {
		QAction* a = mCopy->addAction(labels[i]);
		connect(a, &QAction::triggered, this, [this, i]{ grabFrom(i); });
	}

	/*  Layout. */
	canvas      = new BlockmusterCanvas(this);
	description = new QLabel(this);
	usedLabel   = new QLabel(this);

	auto* header = new QHBoxLayout();
	header->addWidget(description);
	header->addStretch(1);
	header->addWidget(new QLabel(QStringLiteral("Used patterns:"), this));
	header->addWidget(usedLabel);

	auto* btns = new QDialogButtonBox(this);
	auto* pbOk = btns->addButton(QStringLiteral("&Apply"), QDialogButtonBox::AcceptRole);
	auto* pbCa = btns->addButton(QStringLiteral("Cancel"), QDialogButtonBox::RejectRole);
	connect(pbOk, &QPushButton::clicked, this, &BlockmusterDialog::accept);
	connect(pbCa, &QPushButton::clicked, this, &QDialog::reject);

	auto* root = new QVBoxLayout(this);
	root->setMenuBar(menubar);
	root->addLayout(header);
	root->addWidget(canvas, 1);
	root->addWidget(btns);

	if (frm->cursorhandler)
		canvas->cursordirection = frm->cursorhandler->GetCursorDirection();
	canvas->setFocus();

	selectBindung(0);
	calcRange();
	refreshUsed();

	/*  Poll undo/redo availability (mirror of legacy updatetimer). */
	auto* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [this]{
		if (undoAct) undoAct->setEnabled(undo.CanUndo());
		if (redoAct) redoAct->setEnabled(undo.CanRedo());
	});
	timer->start(200);

	resize(560, 400);
}

/*-----------------------------------------------------------------*/
void BlockmusterDialog::selectBindung (int _b)
{
	current = _b;
	if (bindungActs[_b]) bindungActs[_b]->setChecked(true);
	refreshDescription();
	canvas->update();
	refreshUsed();
}

void BlockmusterDialog::refreshDescription ()
{
	if (current == 0)
		description->setText(QStringLiteral("Base pattern"));
	else
		description->setText(QStringLiteral("Pattern %1").arg(current));
}

void BlockmusterDialog::refreshUsed ()
{
	QStringList parts;
	parts << (!(*bindungen)[0].IsEmpty()
	        ? QStringLiteral("<span style='color:red'>B</span>")
	        : QStringLiteral("<span style='color:grey'>B</span>"));
	for (int i = 1; i < 10; i++) {
		QString tag = !(*bindungen)[i].IsEmpty()
		    ? QStringLiteral("<span style='color:red'>%1</span>")
		    : QStringLiteral("<span style='color:grey'>%1</span>");
		parts << tag.arg(i);
	}
	usedLabel->setText(parts.join(QString()));
	usedLabel->setTextFormat(Qt::RichText);
}

void BlockmusterDialog::calcRange ()
{
	mx = -1;
	my = -1;
	bool nonempty = false;
	for (int i = 0; i < Muster::maxx; i++)
		for (int j = 0; j < Muster::maxy; j++)
			for (int k = 0; k < 10; k++)
				if ((*bindungen)[k].Get(i,j) != 0) {
					if (mx < i) mx = i;
					if (my < j) my = j;
					nonempty = true;
				}
	if (!nonempty) mx = my = -1;
}

/*-----------------------------------------------------------------*/
char BlockmusterDialog::toggle (char _s, int _current)
{
	if (_s == 0) return char(_current != 0 ? _current : 1);
	return 0;
}

/*-----------------------------------------------------------------*/
void BlockmusterDialog::editDelete ()
{
	if (mx == -1 || my == -1) return;
	(*bindungen)[current].Clear();
	calcRange();
	refreshUsed();
	canvas->update();
	undo.Snapshot();
}

void BlockmusterDialog::editMirrorH ()
{
	if (mx == -1 || my == -1) return;
	for (int i = 0; i <= mx/2; i++)
		for (int j = 0; j <= my; j++) {
			char s = (*bindungen)[current].Get(i, j);
			(*bindungen)[current].Set(i,    j, (*bindungen)[current].Get(mx-i, j));
			(*bindungen)[current].Set(mx-i, j, s);
		}
	canvas->update();
	undo.Snapshot();
}

void BlockmusterDialog::editMirrorV ()
{
	if (mx == -1 || my == -1) return;
	for (int i = 0; i <= mx; i++)
		for (int j = 0; j <= my/2; j++) {
			char s = (*bindungen)[current].Get(i, j);
			(*bindungen)[current].Set(i, j,    (*bindungen)[current].Get(i, my-j));
			(*bindungen)[current].Set(i, my-j, s);
		}
	canvas->update();
	undo.Snapshot();
}

void BlockmusterDialog::editRotate ()
{
	if (mx == -1 || my == -1) return;
	if (mx != my) return;
	char* data = new char[(mx+1)*(my+1)];
	for (int i = 0; i <= mx; i++)
		for (int j = 0; j <= my; j++) {
			data[j*(mx+1)+i] = (*bindungen)[current].Get(i, j);
			(*bindungen)[current].Set(i, j, 0);
		}
	for (int i = 0; i <= mx; i++)
		for (int j = 0; j <= my; j++)
			(*bindungen)[current].Set(j, my-i, data[j*(mx+1)+i]);
	delete[] data;
	canvas->update();
	undo.Snapshot();
}

void BlockmusterDialog::editInvert ()
{
	if (mx == -1 || my == -1) return;
	for (int i = 0; i <= mx; i++)
		for (int j = 0; j <= my; j++) {
			const char c = toggle((*bindungen)[current].Get(i, j), current);
			(*bindungen)[current].Set(i, j, c);
		}
	canvas->update();
	refreshUsed();
	undo.Snapshot();
}

void BlockmusterDialog::editCentralsym ()
{
	if (mx == -1 || my == -1 || mx != my) return;
	ZentralSymmChecker sym(mx+1, my+1);
	for (int i = 0; i <= mx; i++)
		for (int j = 0; j <= my; j++)
			sym.SetData(i, j, (*bindungen)[current].Get(i, j));
	if (sym.IsAlreadySymmetric()) return;
	if (sym.SearchSymmetry()) {
		for (int i = 0; i <= mx; i++)
			for (int j = 0; j <= my; j++)
				(*bindungen)[current].Set(i, j, sym.GetData(i, j));
		canvas->update();
		undo.Snapshot();
	} else {
		QMessageBox::information(this, QStringLiteral("DB-WEAVE"),
		    QStringLiteral("No central symmetry found."));
	}
}

/*-----------------------------------------------------------------*/
void BlockmusterDialog::rollUp ()
{
	if (mx == -1 || my == -1) return;
	Muster& b = (*bindungen)[current];
	char* data = new char[mx+1];
	for (int i = 0; i <= mx; i++) data[i] = b.Get(i, my);
	for (int j = my; j > 0; j--)
		for (int i = 0; i <= mx; i++) b.Set(i, j, b.Get(i, j-1));
	for (int i = 0; i <= mx; i++) b.Set(i, 0, data[i]);
	delete[] data;
	calcRange();
	canvas->update();
	undo.Snapshot();
}

void BlockmusterDialog::rollDown ()
{
	if (mx == -1 || my == -1) return;
	Muster& b = (*bindungen)[current];
	char* data = new char[mx+1];
	for (int i = 0; i <= mx; i++) data[i] = b.Get(i, 0);
	for (int j = 0; j < my; j++)
		for (int i = 0; i <= mx; i++) b.Set(i, j, b.Get(i, j+1));
	for (int i = 0; i <= mx; i++) b.Set(i, my, data[i]);
	delete[] data;
	calcRange();
	canvas->update();
	undo.Snapshot();
}

void BlockmusterDialog::rollLeft ()
{
	if (mx == -1 || my == -1) return;
	Muster& b = (*bindungen)[current];
	char* data = new char[my+1];
	for (int j = 0; j <= my; j++) data[j] = b.Get(0, j);
	for (int i = 0; i < mx; i++)
		for (int j = 0; j <= my; j++) b.Set(i, j, b.Get(i+1, j));
	for (int j = 0; j <= my; j++) b.Set(mx, j, data[j]);
	delete[] data;
	calcRange();
	canvas->update();
	undo.Snapshot();
}

void BlockmusterDialog::rollRight ()
{
	if (mx == -1 || my == -1) return;
	Muster& b = (*bindungen)[current];
	char* data = new char[my+1];
	for (int j = 0; j <= my; j++) data[j] = b.Get(mx, j);
	for (int i = mx; i > 0; i--)
		for (int j = 0; j <= my; j++) b.Set(i, j, b.Get(i-1, j));
	for (int j = 0; j <= my; j++) b.Set(0, j, data[j]);
	delete[] data;
	calcRange();
	canvas->update();
	undo.Snapshot();
}

/*-----------------------------------------------------------------*/
void BlockmusterDialog::musterKoeper (int _h, int _s)
{
	Muster& m = (*bindungen)[current];
	m.Clear();
	const int n = _h + _s;
	const char s = char(current != 0 ? current : 1);
	if (_h <= _s) {
		for (int i = 0; i < n; i++)
			for (int j = i; j < i+_h; j++)
				m.Set(i, j%n, s);
	} else {
		for (int i = 0; i < n; i++)
			for (int j = i+_s; j < i+n; j++)
				m.Set(i, j%n, s);
	}
	calcRange();
	refreshUsed();
	canvas->update();
	undo.Snapshot();
}

void BlockmusterDialog::musterAtlas (int _n)
{
	Muster& m = (*bindungen)[current];
	m.Clear();
	const char s = char(current != 0 ? current : 1);
	switch (_n) {
	case 5:
		m.Set(0,0,s); m.Set(1,2,s); m.Set(2,4,s); m.Set(3,1,s); m.Set(4,3,s);
		break;
	case 7:
		m.Set(0,2,s); m.Set(1,6,s); m.Set(2,3,s); m.Set(3,0,s);
		m.Set(4,4,s); m.Set(5,1,s); m.Set(6,5,s);
		break;
	case 9:
		m.Set(0,0,s); m.Set(1,2,s); m.Set(2,4,s); m.Set(3,6,s);
		m.Set(4,8,s); m.Set(5,1,s); m.Set(6,3,s); m.Set(7,5,s); m.Set(8,7,s);
		break;
	}
	calcRange();
	refreshUsed();
	canvas->update();
	undo.Snapshot();
}

void BlockmusterDialog::musterPanama (int _h, int _s)
{
	Muster& m = (*bindungen)[current];
	m.Clear();
	const char s = char(current != 0 ? current : 1);
	for (int i = 0; i < _h; i++)
		for (int j = 0; j < _h; j++) m.Set(i, j, s);
	for (int i = _h; i < _h+_s; i++)
		for (int j = _h; j < _h+_s; j++) m.Set(i, j, s);
	calcRange();
	refreshUsed();
	canvas->update();
	undo.Snapshot();
}

/*-----------------------------------------------------------------*/
void BlockmusterDialog::grabFrom (int _from)
{
	if (_from < 0 || _from >= 10) return;
	if (_from == current) return;
	if (mx == -1 || my == -1) return;
	for (int i = 0; i <= mx; i++)
		for (int j = 0; j <= my; j++) {
			char c = (*bindungen)[_from].Get(i, j);
			/*  Double-toggle re-colours the cell from `_from`'s
			    palette to `current`'s palette. */
			c = toggle(toggle(c, current), current);
			(*bindungen)[current].Set(i, j, c);
		}
	canvas->update();
	refreshUsed();
	undo.Snapshot();
}

/*-----------------------------------------------------------------*/
void BlockmusterDialog::doUndo ()
{
	if (!undo.CanUndo()) return;
	undo.Undo();
	calcRange();
	selectBindung(current);
}

void BlockmusterDialog::doRedo ()
{
	if (!undo.CanRedo()) return;
	undo.Redo();
	calcRange();
	selectBindung(current);
}

/*-----------------------------------------------------------------*/
void BlockmusterDialog::accept ()
{
	calcRange();
	QDialog::accept();
}


/*-----------------------------------------------------------------*/
/*  TDBWFRM::BlockExpandEinzug — verbatim port of legacy
    blockmuster.cpp:BlockExpandEinzug. Expands each existing einzug
    thread into `_count` consecutive threads on `_count` consecutive
    shafts; direction is controlled by einzugZ.                    */
void TDBWFRM::BlockExpandEinzug (int _count)
{
	if (kette.a == -1 || kette.b == -1) return;

	short* pData = new short[kette.count()];
	for (int i = kette.a; i <= kette.b; i++) {
		pData[i-kette.a] = einzug.feld.Get(i);
		einzug.feld.Set(i, 0);
	}
	for (int i = kette.a; i <= kette.b; i++) {
		if (pData[i-kette.a] > 0) {
			if (einzugZ) {
				for (int k = 0; k < _count; k++) {
					if (i*_count+k >= Data->MAXX1) break;
					if ((pData[i-kette.a]-1)*_count+k+1 >= Data->MAXY1) ExtendSchaefte();
					einzug.feld.Set(i*_count+k, short((pData[i-kette.a]-1)*_count+k+1));
				}
			} else {
				for (int k = 0; k < _count; k++) {
					if ((i+1)*_count-1-k >= Data->MAXX1) break;
					if ((pData[i-kette.a]-1)*_count+k+1 >= Data->MAXY1) ExtendSchaefte();
					einzug.feld.Set((i+1)*_count-1-k, short((pData[i-kette.a]-1)*_count+k+1));
				}
			}
		}
	}
	delete[] pData;
}

/*-----------------------------------------------------------------*/
void TDBWFRM::BlockExpandTrittfolge (int _count)
{
	if (schuesse.a == -1 || schuesse.b == -1) return;

	char* pData = new char[schuesse.count()*Data->MAXX2];
	for (int j = schuesse.a; j <= schuesse.b; j++) {
		for (int i = 0; i < Data->MAXX2; i++) {
			pData[j*Data->MAXX2+i] = trittfolge.feld.Get(i, j);
			trittfolge.feld.Set(i, j, 0);
		}
		trittfolge.isempty.Set(j, true);
	}
	for (int j = schuesse.a; j <= schuesse.b; j++) {
		for (int i = 0; i < Data->MAXX2; i++) {
			if (pData[j*Data->MAXX2+i] != 0) {
				if (trittfolgeZ) {
					for (int k = 0; k < _count; k++) {
						if (j*_count+k >= Data->MAXY2) break;
						if (i*_count+k >= Data->MAXX2) ExtendTritte();
						trittfolge.feld.Set(i*_count+k, j*_count+k, 1);
						trittfolge.isempty.Set(j*_count+k, false);
					}
				} else {
					for (int k = 0; k < _count; k++) {
						if (j*_count+k >= Data->MAXY2) break;
						if ((i+1)*_count-1-k >= Data->MAXX2) ExtendTritte();
						trittfolge.feld.Set((i+1)*_count-1-k, j*_count+k, 1);
						trittfolge.isempty.Set(j*_count+k, false);
					}
				}
			}
		}
	}
	delete[] pData;
}

/*-----------------------------------------------------------------*/
void TDBWFRM::BlockExpandAufknuepfung (int _x, int _y)
{
	char* pData = new char[Data->MAXY1*Data->MAXX2];
	for (int j = 0; j < Data->MAXY1; j++)
		for (int i = 0; i < Data->MAXX2; i++) {
			pData[j*Data->MAXX2+i] = aufknuepfung.feld.Get(i, j);
			aufknuepfung.feld.Set(i, j, 0);
		}

	int i1 = Data->MAXX2-1, i2 = 0;
	int j1 = Data->MAXY1-1, j2 = 0;
	bool nonempty = false;
	for (int i = 0; i < Data->MAXX2; i++)
		for (int j = 0; j < Data->MAXY1; j++)
			if (pData[j*Data->MAXX2+i] != 0) {
				if (i1 > i) i1 = i;
				if (i2 < i) i2 = i;
				if (j1 > j) j1 = j;
				if (j2 < j) j2 = j;
				nonempty = true;
			}
	if (!nonempty) { i1 = j1 = 0; i2 = j2 = -1; }

	for (int i = i1; i <= i2; i++) {
		for (int j = j1; j <= j2; j++) {
			const char bindung = pData[j*Data->MAXX2+i];
			if (bindung < 0 || bindung >= 10) continue;
			for (int ii = _x*i; ii < _x*(i+1); ii++)
				for (int jj = _y*j; jj < _y*(j+1); jj++)
					aufknuepfung.feld.Set(ii, jj, blockmuster[int(bindung)].Get(ii-_x*i, jj-_y*j));
		}
	}
	delete[] pData;
}

/*-----------------------------------------------------------------*/
void TDBWFRM::EditBlockmusterClick ()
{
	if (cursorhandler) cursorhandler->DisableCursor();
	if (!blockundo) return;

	BlockmusterDialog dlg(this, *blockundo, &blockmuster, currentbm,
	                      QStringLiteral("Substitute with block patterns"),
	                      /*withRange=*/true);
	if (dlg.exec() == QDialog::Accepted) {
		einzugZ     = dlg.einzugZ;
		trittfolgeZ = dlg.trittfolgeZ;

		const int x = dlg.mx + 1;
		const int y = dlg.my + 1;

		/*  Legacy temporarily disables Schlagpatrone to make the
		    substitution use single-tread semantics, then restores.  */
		bool wasSchlagpatrone = false;
		if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked()) {
			wasSchlagpatrone = true;
			ViewSchlagpatrone->setChecked(false);
			trittfolge.einzeltritt = true;
			RecalcAll();
		}

		BlockExpandEinzug(y);
		BlockExpandTrittfolge(x);
		BlockExpandAufknuepfung(x, y);

		if (EzBelassen) EzBelassen->setChecked(true);
		if (TfBelassen) TfBelassen->setChecked(true);

		CalcRange();
		CalcRapport();
		RecalcGewebe();

		if (wasSchlagpatrone) {
			ViewSchlagpatrone->setChecked(true);
			trittfolge.einzeltritt = !ViewSchlagpatrone->isChecked();
			RecalcSchlagpatrone();
		}

		refresh();
		if (undo) undo->Snapshot();
	}
	SetModified();
	if (cursorhandler) cursorhandler->EnableCursor();
}
