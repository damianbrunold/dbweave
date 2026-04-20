/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy/overview_form.cpp (TOverviewForm).

    Opens a modal "pattern overview" window that renders the entire
    gewebe at a small zoom so the user can see much more of the
    pattern than the main editor viewport shows. Esc or F4 closes it;
    Ctrl+I / Ctrl+U adjust the zoom; Ctrl+G toggles the grid overlay.

    The legacy form also had a Print button; that's deferred -- the
    user can still print the full pattern from File > Print.
*/

#ifndef DBWEAVE_UI_OVERVIEWDIALOG_H
#define DBWEAVE_UI_OVERVIEWDIALOG_H

#include <QDialog>

class QAction;
class TDBWFRM;
class OverviewCanvas;

class OverviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OverviewDialog(TDBWFRM* _frm, QWidget* _parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* _e) override;
    void showEvent(QShowEvent* _e) override;

private:
    TDBWFRM* frm;
    OverviewCanvas* canvas;
    QAction* actZoomIn;
    QAction* actZoomOut;
    QAction* actGrid;

    void updateZoomActions();
};

#endif
