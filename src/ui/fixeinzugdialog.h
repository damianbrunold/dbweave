/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy fixeinzug_form.cpp (TFixeinzugForm). "User-
    defined threading" editor. The legacy form also implemented a
    keyboard cursor overlay; this port keeps the equivalent mouse-
    click + menu workflow (Grab / Delete / Revert / Close) which
    cover the full edit loop. Keyboard cursor navigation is deferred
    -- mouse editing is authoritative in the legacy UI as well.    */

#ifndef DBWEAVE_UI_FIXEINZUG_DIALOG_H
#define DBWEAVE_UI_FIXEINZUG_DIALOG_H

#include <QDialog>

class QScrollBar;
class TDBWFRM;
class FixeinzugCanvas;

class FixeinzugDialog : public QDialog
{
	Q_OBJECT
public:
	explicit FixeinzugDialog (TDBWFRM* _frm, QWidget* _parent = nullptr);
	~FixeinzugDialog() override;

	/*  Scratch-buffer the user is editing, and the resulting size
	    (max non-zero index + 1). Accept() writes both back into
	    TDBWFRM::fixeinzug / fixsize / firstfree and rapports the
	    scratch across the rest of MAXX1. */
	short* scratch   = nullptr;
	int    size      = 0;
	short  firstfree = 0;

private slots:
	void onGrab   ();
	void onDelete ();
	void onRevert ();
	void onClose  ();

protected:
	void accept () override;

private:
	TDBWFRM*         frm    = nullptr;
	FixeinzugCanvas* canvas = nullptr;
	QScrollBar*      sbH    = nullptr;

	void calcRange ();
	short calcFirstFree () const;
};

#endif
