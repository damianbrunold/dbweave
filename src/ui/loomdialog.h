/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Loom-control dialog — drives a StWeaveController (Dummy or one
    of the real serial controllers from src/loom/) through the
    current pattern's klammer ranges. Combines the roles of legacy
    TSTRGFRM (weaving loop + klammer editor + position selector +
    goto buttons) and TStrgOptLoomForm (loom options) into one
    modal dialog.                                                 */

#ifndef DBWEAVE_UI_LOOM_DIALOG_H
#define DBWEAVE_UI_LOOM_DIALOG_H

#include <QDialog>
#include <memory>

#include "loom.h"

class QCheckBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class TDBWFRM;

class LoomDialog : public QDialog
{
	Q_OBJECT
public:
	explicit LoomDialog (TDBWFRM* _frm, QWidget* _parent = nullptr);

private slots:
	void onStart   ();
	void onStop    ();
	void onStep    ();
	void onReset   ();
	void onOptions ();
	void onGotoLast();
	void onGotoKlammer (int _index);

private:
	TDBWFRM*                             frm = nullptr;
	std::unique_ptr<StWeaveController>   controller;

	/*  Loom options — persisted to QSettings under "Loom/*"
	    so subsequent runs pick up the user's last setup. */
	LOOMINTERFACE intrf = intrf_dummy;
	int  optPort    = 1;
	int  optDelay   = 0;
	int  slipsBytes = 4;

	/*  Klammer editor widgets: 9 rows × (first, last, repetitions)
	    plus a "goto" button per row. */
	QSpinBox*    klFirst[9] = { };
	QSpinBox*    klLast [9] = { };
	QSpinBox*    klReps [9] = { };
	QPushButton* klGoto [9] = { };

	/*  Weaving state. */
	int  currentKlammer    = -1;
	int  currentPosition   = 0;
	int  currentRepetition = 1;
	bool stopRequested     = false;
	bool running           = false;

	/*  Previous successful position (for "Goto last" button). */
	int lastKlammer    = -1;
	int lastPosition   = 0;
	int lastRepetition = 1;

	/*  UI widgets. */
	QLabel*         labStatus   = nullptr;
	QPlainTextEdit* log         = nullptr;
	QPushButton*    bStart      = nullptr;
	QPushButton*    bStop       = nullptr;
	QPushButton*    bStep       = nullptr;
	QPushButton*    bReset      = nullptr;
	QPushButton*    bOptions    = nullptr;
	QPushButton*    bGotoLast   = nullptr;
	QCheckBox*      cbLoop      = nullptr;
	QCheckBox*      cbBackwards = nullptr;
	QCheckBox*      cbReverse   = nullptr;
	QSpinBox*       spinWaitMs  = nullptr;

	void rebuildController ();
	void resetCursor ();
	bool advanceCursor ();
	bool retreatCursor ();
	std::uint32_t computeShafts () const;

	void weaveOne ();
	void refreshStatus ();
	void setUiRunning (bool _running);
	void updateGotoLabels ();

	/*  Klammer editor ↔ TDBWFRM::klammern[]. */
	void pullKlammersFromFrm ();
	void pushKlammersToFrm   () const;

	/*  Loom/* settings load/save. */
	void loadSettings ();
	void saveSettings () const;
};

#endif
