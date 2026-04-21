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
    defined threading" editor. Supports mouse click-to-edit, the
    Grab / Delete / Revert / Close menu actions, and arrow-key /
    Space keyboard navigation over the cursor overlay.            */

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
    explicit FixeinzugDialog(TDBWFRM* _frm, QWidget* _parent = nullptr);
    ~FixeinzugDialog() override;

    /*  Scratch-buffer the user is editing, and the resulting size
        (max non-zero index + 1). Accept() writes both back into
        TDBWFRM::fixeinzug / fixsize / firstfree and rapports the
        scratch across the rest of MAXX1. */
    short* scratch = nullptr;
    int size = 0;
    short firstfree = 0;

private slots:
    void onGrab();
    void onDelete();
    void onRevert();
    void onClose();

protected:
    void accept() override;

private:
    TDBWFRM* frm = nullptr;
    FixeinzugCanvas* canvas = nullptr;
    QScrollBar* sbH = nullptr;

    void calcRange();
    short calcFirstFree() const;
};

#endif
