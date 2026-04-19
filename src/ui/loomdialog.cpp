/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "loomdialog.h"
#include "loomoptionsdialog.h"

#include "mainwindow.h"
#include "datamodule.h"

#if defined(DBWEAVE_HAVE_LOOM)
#  include "serialcontrollers.h"
#endif

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

#include <algorithm>

static constexpr int kKlammerCount = 9;

/*-----------------------------------------------------------------*/
static std::unique_ptr<StWeaveController> buildController (LOOMINTERFACE _i)
{
#if defined(DBWEAVE_HAVE_LOOM)
	return makeLoomController(_i);
#else
	(void)_i;
	return std::make_unique<StDummyController>();
#endif
}

/*-----------------------------------------------------------------*/
LoomDialog::LoomDialog (TDBWFRM* _frm, QWidget* _parent)
	: QDialog(_parent)
	, frm(_frm)
{
	setWindowTitle(QStringLiteral("Loom control"));
	setModal(true);
	resize(720, 560);

	loadSettings();

	spinWaitMs = new QSpinBox(this);
	spinWaitMs->setRange(0, 5000);
	spinWaitMs->setSingleStep(50);
	spinWaitMs->setSuffix(QStringLiteral(" ms"));
	spinWaitMs->setValue(200);

	bOptions = new QPushButton(QStringLiteral("&Options..."), this);
	connect(bOptions, &QPushButton::clicked, this, &LoomDialog::onOptions);

	cbLoop      = new QCheckBox(QStringLiteral("&Loop forever"), this);
	cbBackwards = new QCheckBox(QStringLiteral("Weave &backwards"), this);
	cbReverse   = new QCheckBox(QStringLiteral("Re&verse shaft order"), this);

	/*  Top row: current loom label + options button + toggles. */
	auto* gbLoom = new QGroupBox(QStringLiteral("Loom"), this);
	auto* gbl = new QFormLayout(gbLoom);
	auto* row = new QHBoxLayout();
	row->addWidget(bOptions);
	row->addWidget(cbLoop);
	row->addWidget(cbBackwards);
	row->addWidget(cbReverse);
	row->addStretch(1);
	gbl->addRow(QStringLiteral("&Wait per row:"), spinWaitMs);
	gbl->addRow(row);

	/*  Klammer editor: nine rows. */
	auto* gbKlammer = new QGroupBox(QStringLiteral("Klammer ranges"), this);
	auto* kl = new QGridLayout(gbKlammer);
	kl->addWidget(new QLabel(QStringLiteral("#"),         this), 0, 0, Qt::AlignCenter);
	kl->addWidget(new QLabel(QStringLiteral("first"),     this), 0, 1, Qt::AlignCenter);
	kl->addWidget(new QLabel(QStringLiteral("last"),      this), 0, 2, Qt::AlignCenter);
	kl->addWidget(new QLabel(QStringLiteral("reps"),      this), 0, 3, Qt::AlignCenter);
	kl->addWidget(new QLabel(QString(),                   this), 0, 4);
	for (int i = 0; i < kKlammerCount; i++) {
		kl->addWidget(new QLabel(QString::number(i + 1), this), i + 1, 0, Qt::AlignCenter);
		klFirst[i] = new QSpinBox(this);
		klLast [i] = new QSpinBox(this);
		klReps [i] = new QSpinBox(this);
		klFirst[i]->setRange(0, 4096);
		klLast [i]->setRange(0, 4096);
		klReps [i]->setRange(0, 9999);
		klFirst[i]->setMaximumWidth(80);
		klLast [i]->setMaximumWidth(80);
		klReps [i]->setMaximumWidth(80);
		kl->addWidget(klFirst[i], i + 1, 1);
		kl->addWidget(klLast [i], i + 1, 2);
		kl->addWidget(klReps [i], i + 1, 3);
		klGoto[i] = new QPushButton(QStringLiteral("Go"), this);
		klGoto[i]->setMaximumWidth(48);
		kl->addWidget(klGoto[i], i + 1, 4);
		connect(klGoto[i], &QPushButton::clicked, this, [this, i]{ onGotoKlammer(i); });
	}

	/*  Status line + weaving log. */
	labStatus = new QLabel(this);
	log       = new QPlainTextEdit(this);
	log->setReadOnly(true);
	log->setMaximumBlockCount(2000);

	auto* gbStatus = new QGroupBox(QStringLiteral("Status"), this);
	auto* gsL = new QVBoxLayout(gbStatus);
	gsL->addWidget(labStatus);
	gsL->addWidget(log, 1);

	/*  Button row. */
	bStart    = new QPushButton(QStringLiteral("&Start"),     this);
	bStop     = new QPushButton(QStringLiteral("S&top"),      this);
	bStep     = new QPushButton(QStringLiteral("Step &1"),    this);
	bReset    = new QPushButton(QStringLiteral("&Reset"),     this);
	bGotoLast = new QPushButton(QStringLiteral("Goto &last"), this);
	bStop->setEnabled(false);
	auto* btnRow = new QHBoxLayout();
	btnRow->addWidget(bStart);
	btnRow->addWidget(bStop);
	btnRow->addWidget(bStep);
	btnRow->addWidget(bReset);
	btnRow->addWidget(bGotoLast);
	btnRow->addStretch(1);

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Close, this);
	connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto* root = new QVBoxLayout(this);
	root->addWidget(gbLoom);
	root->addWidget(gbKlammer);
	root->addLayout(btnRow);
	root->addWidget(gbStatus, 1);
	root->addWidget(btns);

	connect(bStart,    &QPushButton::clicked, this, &LoomDialog::onStart);
	connect(bStop,     &QPushButton::clicked, this, &LoomDialog::onStop);
	connect(bStep,     &QPushButton::clicked, this, &LoomDialog::onStep);
	connect(bReset,    &QPushButton::clicked, this, &LoomDialog::onReset);
	connect(bGotoLast, &QPushButton::clicked, this, &LoomDialog::onGotoLast);

	pullKlammersFromFrm();
	updateGotoLabels();
	rebuildController();
	resetCursor();
	refreshStatus();
}

/*-----------------------------------------------------------------*/
void LoomDialog::rebuildController ()
{
	controller = buildController(intrf);
	if (auto* dummy = dynamic_cast<StDummyController*>(controller.get()))
		dummy->setWaitMs(spinWaitMs->value());
}

/*-----------------------------------------------------------------*/
void LoomDialog::resetCursor ()
{
	currentKlammer    = -1;
	currentPosition   = 0;
	currentRepetition = 1;
	lastKlammer    = -1;
	lastPosition   = 0;
	lastRepetition = 1;
	pushKlammersToFrm();
	for (int i = 0; i < kKlammerCount; i++) {
		if (frm->klammern[i].repetitions > 0) {
			currentKlammer  = i;
			currentPosition = frm->klammern[i].first;
			break;
		}
	}
}

/*-----------------------------------------------------------------*/
void LoomDialog::pullKlammersFromFrm ()
{
	for (int i = 0; i < kKlammerCount; i++) {
		klFirst[i]->setValue(frm->klammern[i].first);
		klLast [i]->setValue(frm->klammern[i].last);
		klReps [i]->setValue(frm->klammern[i].repetitions);
	}
}

void LoomDialog::pushKlammersToFrm () const
{
	for (int i = 0; i < kKlammerCount; i++) {
		frm->klammern[i].first       = klFirst[i]->value();
		frm->klammern[i].last        = klLast [i]->value();
		frm->klammern[i].repetitions = klReps [i]->value();
	}
}

/*-----------------------------------------------------------------*/
void LoomDialog::updateGotoLabels ()
{
	for (int i = 0; i < kKlammerCount; i++) {
		const bool active = klReps[i]->value() > 0;
		klGoto[i]->setEnabled(active);
	}
}

/*-----------------------------------------------------------------*/
std::uint32_t LoomDialog::computeShafts () const
{
	std::uint32_t data = 0;
	if (!frm) return data;
	const bool schlag = frm->ViewSchlagpatrone && frm->ViewSchlagpatrone->isChecked();
	const int maxShaft = std::min(int(Data->MAXY1), 32);

	if (currentPosition < 0 || currentPosition >= Data->MAXY2) return 0;

	if (schlag) {
		for (int i = 0; i < maxShaft; i++)
			if (i < Data->MAXX2
			 && frm->trittfolge.feld.Get(i, currentPosition) > 0)
				data |= (std::uint32_t(1) << i);
	} else {
		const int maxTritt = std::min(int(Data->MAXX2), 32);
		for (int i = 0; i < maxTritt; i++) {
			if (frm->trittfolge.feld.Get(i, currentPosition) > 0) {
				for (int k = 0; k < maxShaft; k++)
					if (frm->aufknuepfung.feld.Get(i, k) > 0)
						data |= (std::uint32_t(1) << k);
			}
		}
	}

	/*  "Reverse shaft order": some looms expect shaft 1 on the
	    opposite end. Matches legacy `reverse` flag. */
	if (cbReverse && cbReverse->isChecked()) {
		std::uint32_t flipped = 0;
		for (int i = 0; i < maxShaft; i++)
			if (data & (std::uint32_t(1) << i))
				flipped |= (std::uint32_t(1) << (maxShaft - 1 - i));
		data = flipped;
	}
	return data;
}

/*-----------------------------------------------------------------*/
bool LoomDialog::advanceCursor ()
{
	if (currentKlammer < 0) return true;
	currentPosition++;
	if (currentPosition > frm->klammern[currentKlammer].last) {
		if (currentRepetition < frm->klammern[currentKlammer].repetitions) {
			currentRepetition++;
			currentPosition = frm->klammern[currentKlammer].first;
		} else {
			int k = (currentKlammer + 1) % kKlammerCount;
			int wrapped = 0;
			while (frm->klammern[k].repetitions == 0 && wrapped < kKlammerCount) {
				k = (k + 1) % kKlammerCount;
				wrapped++;
			}
			currentKlammer    = k;
			currentRepetition = 1;
			currentPosition   = frm->klammern[k].first;
			return true;
		}
	}
	return false;
}

bool LoomDialog::retreatCursor ()
{
	if (currentKlammer < 0) return true;
	currentPosition--;
	if (currentPosition < frm->klammern[currentKlammer].first) {
		if (currentRepetition > 1) {
			currentRepetition--;
			currentPosition = frm->klammern[currentKlammer].last;
		} else {
			int k = (currentKlammer - 1 + kKlammerCount) % kKlammerCount;
			int wrapped = 0;
			while (frm->klammern[k].repetitions == 0 && wrapped < kKlammerCount) {
				k = (k - 1 + kKlammerCount) % kKlammerCount;
				wrapped++;
			}
			currentKlammer    = k;
			currentRepetition = frm->klammern[k].repetitions;
			currentPosition   = frm->klammern[k].last;
			return true;
		}
	}
	return false;
}

/*-----------------------------------------------------------------*/
void LoomDialog::weaveOne ()
{
	if (!controller || currentKlammer < 0) return;

	const std::uint32_t shafts = computeShafts();
	log->appendPlainText(QStringLiteral("Schuss %1, rep %2 → 0x%3 (shafts %4)")
	    .arg(currentPosition)
	    .arg(currentRepetition)
	    .arg(quint32(shafts), 8, 16, QChar('0'))
	    .arg([shafts](){
	        QStringList out;
	        for (int i = 0; i < 32; i++)
	            if (shafts & (std::uint32_t(1) << i)) out << QString::number(i + 1);
	        return out.join(QStringLiteral(","));
	    }()));

	WEAVE_STATUS stat = WEAVE_REPEAT;
	try {
		stat = controller->WeaveSchuss(shafts);
	} catch (...) {
		log->appendPlainText(QStringLiteral("— aborted —"));
		return;
	}

	const bool forward = (stat == WEAVE_SUCCESS_NEXT);
	const bool reverse = (stat == WEAVE_SUCCESS_PREV);
	if (forward || reverse) {
		/*  Record prior position for "Goto last". */
		lastKlammer    = currentKlammer;
		lastPosition   = currentPosition;
		lastRepetition = currentRepetition;

		const bool backToStart = (cbBackwards && cbBackwards->isChecked())
		                           ? retreatCursor()
		                           : (forward ? advanceCursor() : retreatCursor());
		(void)backToStart;
	}
	refreshStatus();
}

/*-----------------------------------------------------------------*/
void LoomDialog::refreshStatus ()
{
	if (currentKlammer < 0) {
		labStatus->setText(QStringLiteral("No klammer configured — edit ranges above first."));
		return;
	}
	const Klammer& k = frm->klammern[currentKlammer];
	labStatus->setText(QStringLiteral("Klammer %1/9  —  weft row %2 (range %3..%4), repetition %5/%6")
	    .arg(currentKlammer + 1)
	    .arg(currentPosition)
	    .arg(k.first).arg(k.last)
	    .arg(currentRepetition).arg(k.repetitions));
}

/*-----------------------------------------------------------------*/
void LoomDialog::setUiRunning (bool _running)
{
	running = _running;
	bStart   ->setEnabled(!_running);
	bStep    ->setEnabled(!_running);
	bReset   ->setEnabled(!_running);
	bOptions ->setEnabled(!_running);
	bGotoLast->setEnabled(!_running && lastKlammer >= 0);
	bStop    ->setEnabled(_running);
	/*  Klammer editor is immutable during weaving so the cursor
	    indices stay meaningful. */
	for (int i = 0; i < kKlammerCount; i++) {
		klFirst[i]->setEnabled(!_running);
		klLast [i]->setEnabled(!_running);
		klReps [i]->setEnabled(!_running);
		klGoto[i] ->setEnabled(!_running && klReps[i]->value() > 0);
	}
}

/*-----------------------------------------------------------------*/
void LoomDialog::onStart ()
{
	if (!controller) return;
	pushKlammersToFrm();
	if (currentKlammer < 0) resetCursor();
	if (currentKlammer < 0) {
		labStatus->setText(QStringLiteral("No klammer configured."));
		return;
	}

	INITDATA init;
	init.port  = optPort;
	init.delay = optDelay;
	if (!controller->Initialize(init)) {
		QMessageBox::warning(this, QStringLiteral("Loom control"),
		    QStringLiteral("Could not initialize the loom (check port / cabling)."));
		return;
	}
	if (auto* dummy = dynamic_cast<StDummyController*>(controller.get()))
		dummy->setWaitMs(spinWaitMs->value());

	setUiRunning(true);
	stopRequested = false;

	int guard = 0;
	const int startKl  = currentKlammer;
	const int startPos = currentPosition;
	const int startRep = currentRepetition;
	bool firstPass = true;
	const bool loop = cbLoop && cbLoop->isChecked();

	while (!stopRequested && controller && !controller->IsAborted()) {
		weaveOne();
		if (stopRequested || controller->IsAborted()) break;
		if (!loop && !firstPass
		 && currentKlammer == startKl
		 && currentPosition == startPos
		 && currentRepetition == startRep) break;
		firstPass = false;
		if (++guard > 100000) break;   /*  safety valve */
		QCoreApplication::processEvents();
	}

	controller->Terminate();
	setUiRunning(false);
}

void LoomDialog::onStop ()
{
	if (!controller) return;
	stopRequested = true;
	controller->Abort();
}

void LoomDialog::onStep ()
{
	if (!controller || currentKlammer < 0) {
		pushKlammersToFrm();
		resetCursor();
		if (currentKlammer < 0) return;
	}
	pushKlammersToFrm();
	INITDATA init;
	init.port  = optPort;
	init.delay = optDelay;
	controller->Initialize(init);
	if (auto* dummy = dynamic_cast<StDummyController*>(controller.get()))
		dummy->setWaitMs(spinWaitMs->value());
	weaveOne();
	controller->Terminate();
	if (bGotoLast) bGotoLast->setEnabled(lastKlammer >= 0);
}

void LoomDialog::onReset ()
{
	pushKlammersToFrm();
	resetCursor();
	log->clear();
	refreshStatus();
	if (bGotoLast) bGotoLast->setEnabled(false);
}

void LoomDialog::onOptions ()
{
	LoomOptionsDialog dlg(this);
	dlg.setInterface(intrf);
	dlg.setPort     (optPort);
	dlg.setDelay    (optDelay);
	if (dlg.exec() != QDialog::Accepted) return;
	intrf    = dlg.interf();
	optPort  = dlg.port();
	optDelay = dlg.delay();
	saveSettings();
	rebuildController();
}

void LoomDialog::onGotoLast ()
{
	if (lastKlammer < 0) return;
	currentKlammer    = lastKlammer;
	currentPosition   = lastPosition;
	currentRepetition = lastRepetition;
	refreshStatus();
}

void LoomDialog::onGotoKlammer (int _index)
{
	pushKlammersToFrm();
	if (_index < 0 || _index >= kKlammerCount) return;
	if (frm->klammern[_index].repetitions == 0) return;
	currentKlammer    = _index;
	currentPosition   = frm->klammern[_index].first;
	currentRepetition = 1;
	refreshStatus();
}

/*-----------------------------------------------------------------*/
void LoomDialog::loadSettings ()
{
	QSettings s;
	s.beginGroup(QStringLiteral("Loom"));
	intrf     = LOOMINTERFACE(s.value(QStringLiteral("Interface"), int(intrf_dummy)).toInt());
	optPort   = s.value(QStringLiteral("Port"),  1).toInt();
	optDelay  = s.value(QStringLiteral("Delay"), 0).toInt();
	s.endGroup();
}

void LoomDialog::saveSettings () const
{
	QSettings s;
	s.beginGroup(QStringLiteral("Loom"));
	s.setValue(QStringLiteral("Interface"), int(intrf));
	s.setValue(QStringLiteral("Port"),      optPort);
	s.setValue(QStringLiteral("Delay"),     optDelay);
	s.endGroup();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::LoomControlClick ()
{
	LoomDialog dlg(this, this);
	dlg.exec();
}
