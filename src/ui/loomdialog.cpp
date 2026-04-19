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

#include "mainwindow.h"
#include "datamodule.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <algorithm>

/*  Matches legacy MAXKLAMMERN. */
static constexpr int kKlammerCount = 9;

/*-----------------------------------------------------------------*/
LoomDialog::LoomDialog (TDBWFRM* _frm, QWidget* _parent)
	: QDialog(_parent)
	, frm(_frm)
{
	setWindowTitle(QStringLiteral("Loom control"));
	setModal(true);
	resize(560, 440);

	/*  Top row: interface picker + wait time. */
	cbInterface = new QComboBox(this);
	cbInterface->addItem(QStringLiteral("Dummy (simulation)"), int(intrf_dummy));
	/*  Real serial-loom controllers (ARM Patronic / Designer,
	    Slips, Varpapuu Parallel, AVL CD-III) are deferred until
	    the QSerialPort path lands; for now the dropdown only
	    offers the dummy. */

	spinWaitMs = new QSpinBox(this);
	spinWaitMs->setRange(0, 5000);
	spinWaitMs->setSingleStep(50);
	spinWaitMs->setSuffix(QStringLiteral(" ms"));
	spinWaitMs->setValue(200);

	auto* top = new QFormLayout();
	top->addRow(QStringLiteral("&Loom:"),         cbInterface);
	top->addRow(QStringLiteral("Wait &per row:"), spinWaitMs);

	/*  Status line + log. */
	labStatus = new QLabel(this);
	log       = new QPlainTextEdit(this);
	log->setReadOnly(true);
	log->setMaximumBlockCount(1000);

	auto* gbStatus = new QGroupBox(QStringLiteral("Status"), this);
	auto* gsL = new QVBoxLayout(gbStatus);
	gsL->addWidget(labStatus);
	gsL->addWidget(log, 1);

	/*  Buttons. */
	bStart = new QPushButton(QStringLiteral("&Start"), this);
	bStop  = new QPushButton(QStringLiteral("S&top"),  this);
	bStep  = new QPushButton(QStringLiteral("Step &1"), this);
	bReset = new QPushButton(QStringLiteral("&Reset"), this);
	bStop->setEnabled(false);
	auto* row = new QHBoxLayout();
	row->addWidget(bStart);
	row->addWidget(bStop);
	row->addWidget(bStep);
	row->addWidget(bReset);
	row->addStretch(1);

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Close, this);
	connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto* root = new QVBoxLayout(this);
	root->addLayout(top);
	root->addLayout(row);
	root->addWidget(gbStatus, 1);
	root->addWidget(btns);

	connect(bStart, &QPushButton::clicked, this, &LoomDialog::onStart);
	connect(bStop,  &QPushButton::clicked, this, &LoomDialog::onStop);
	connect(bStep,  &QPushButton::clicked, this, &LoomDialog::onStep);
	connect(bReset, &QPushButton::clicked, this, &LoomDialog::onReset);
	connect(cbInterface, qOverload<int>(&QComboBox::currentIndexChanged),
	        this, [this](int){ rebuildController(); });

	rebuildController();
	resetCursor();
	refreshStatus();
}

/*-----------------------------------------------------------------*/
void LoomDialog::rebuildController ()
{
	/*  Only dummy is implemented — keep it simple. */
	auto dummy = std::make_unique<StDummyController>();
	dummy->setWaitMs(spinWaitMs->value());
	controller = std::move(dummy);
}

/*-----------------------------------------------------------------*/
void LoomDialog::resetCursor ()
{
	currentKlammer  = -1;
	currentPosition = 0;
	currentRepetition = 1;
	/*  Seek to the first klammer with repetitions > 0. */
	for (int i = 0; i < kKlammerCount; i++) {
		if (frm->klammern[i].repetitions > 0) {
			currentKlammer  = i;
			currentPosition = frm->klammern[i].first;
			break;
		}
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
			/*  Jump to next non-empty klammer; wrap around. */
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

	if (stat == WEAVE_SUCCESS_NEXT || stat == WEAVE_SUCCESS_PREV) {
		advanceCursor();
	}
	refreshStatus();
}

/*-----------------------------------------------------------------*/
void LoomDialog::refreshStatus ()
{
	if (currentKlammer < 0) {
		labStatus->setText(QStringLiteral("No klammer configured — edit klammer ranges first."));
		return;
	}
	const Klammer& k = frm->klammern[currentKlammer];
	labStatus->setText(QStringLiteral("Klammer %1 of 9 — weft row %2 (range %3..%4), repetition %5 of %6")
	    .arg(currentKlammer + 1)
	    .arg(currentPosition)
	    .arg(k.first).arg(k.last)
	    .arg(currentRepetition).arg(k.repetitions));
}

/*-----------------------------------------------------------------*/
void LoomDialog::setUiRunning (bool _running)
{
	running = _running;
	bStart ->setEnabled(!_running);
	bStep  ->setEnabled(!_running);
	bReset ->setEnabled(!_running);
	bStop  ->setEnabled(_running);
	cbInterface->setEnabled(!_running);
}

/*-----------------------------------------------------------------*/
void LoomDialog::onStart ()
{
	if (!controller) return;
	if (currentKlammer < 0) {
		labStatus->setText(QStringLiteral("No klammer configured."));
		return;
	}

	INITDATA init;
	if (!controller->Initialize(init)) {
		labStatus->setText(QStringLiteral("Controller init failed."));
		return;
	}
	static_cast<StDummyController&>(*controller).setWaitMs(spinWaitMs->value());

	setUiRunning(true);
	stopRequested = false;

	/*  Weave continuously; each iteration yields via CheckAbort in
	    the controller so the UI stays responsive. We stop when the
	    cursor wraps back around to the start klammer's first row
	    (after at least one successful schuss) or when Stop is
	    clicked. */
	int guard = 0;
	const int startKl = currentKlammer;
	const int startPos = currentPosition;
	const int startRep = currentRepetition;
	bool firstPass = true;

	while (!stopRequested && controller && !controller->IsAborted()) {
		weaveOne();
		if (stopRequested || controller->IsAborted()) break;
		/*  One-pass loop detection: we've returned to the starting
		    position after at least one weave. */
		if (!firstPass
		 && currentKlammer == startKl
		 && currentPosition == startPos
		 && currentRepetition == startRep) break;
		firstPass = false;
		/*  Safety valve so a mis-configured klammer doesn't spin
		    forever. 10k schüsse is more than any real pattern. */
		if (++guard > 10000) break;
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
	if (!controller || currentKlammer < 0) return;
	INITDATA init;
	controller->Initialize(init);
	static_cast<StDummyController&>(*controller).setWaitMs(spinWaitMs->value());
	weaveOne();
	controller->Terminate();
}

void LoomDialog::onReset ()
{
	resetCursor();
	log->clear();
	refreshStatus();
}

/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::LoomControlClick ()
{
	LoomDialog dlg(this, this);
	dlg.exec();
}
