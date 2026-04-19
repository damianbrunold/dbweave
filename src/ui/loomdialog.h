/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Minimal loom-control dialog (Phase 11). Drives a
    StDummyController through the current pattern's trittfolge +
    aufknuepfung (or, in schlagpatrone mode, trittfolge directly)
    and logs each sent shaft bitmap. Enough to simulate weaving
    end-to-end without a physical loom attached.

    Legacy TSTRGFRM was a full 3600-line editor window
    (steuerung_form + 6 split units). This port replaces it with a
    compact modal dialog; the mini-editor features (position
    selector grid, klammer editor, loom-specific options) are
    deferred until there's real hardware to exercise them.       */

#ifndef DBWEAVE_UI_LOOM_DIALOG_H
#define DBWEAVE_UI_LOOM_DIALOG_H

#include <QDialog>
#include <memory>

#include "loom.h"

class QComboBox;
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
	void onStart ();
	void onStop  ();
	void onStep  ();
	void onReset ();

private:
	TDBWFRM*                             frm = nullptr;
	std::unique_ptr<StWeaveController>   controller;

	/*  UI widgets. */
	QComboBox*      cbInterface = nullptr;
	QSpinBox*       spinWaitMs  = nullptr;
	QLabel*         labStatus   = nullptr;
	QPlainTextEdit* log         = nullptr;
	QPushButton*    bStart      = nullptr;
	QPushButton*    bStop       = nullptr;
	QPushButton*    bStep       = nullptr;
	QPushButton*    bReset      = nullptr;

	/*  Weaving cursor — matches legacy weave_klammer /
	    weave_position / weave_repetition. */
	int  currentKlammer    = -1;
	int  currentPosition   = 0;
	int  currentRepetition = 1;
	bool stopRequested     = false;
	bool running           = false;

	/*  Make the controller for the combo-box's current selection.
	    Only intrf_dummy is live; other entries are shown disabled. */
	void rebuildController ();

	/*  Seek to the first non-empty klammer / first weft thread
	    that's within its range. Resets position state used by
	    advanceCursor() for a subsequent loom-drive pass. */
	void resetCursor ();

	/*  Compute the shaft-lift bitmap for the current weft row
	    (weave_position). In schlagpatrone mode, trittfolge rows
	    directly encode the lifts; otherwise each set treadle cell
	    activates every shaft tied to it in aufknuepfung. */
	std::uint32_t computeShafts () const;

	/*  Move the cursor one weft row forward, honouring klammer
	    boundaries × repetitions. Returns true if we're back at
	    the start after at least one full pass. */
	bool advanceCursor ();

	void weaveOne ();
	void refreshStatus ();

	void setUiRunning (bool _running);
};

#endif
