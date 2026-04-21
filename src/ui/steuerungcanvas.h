/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Canvas widget inside TSTRGFRM. Legacy TSTRGFRM painted directly
    onto its own form; Qt routes painting through a dedicated child
    widget so the menu/toolbar/status-bar chrome stays out of
    the schlagpatrone + klammern + trittfolge render area.

    paintEvent / mouseEvents / keyEvents dispatch back into the
    owning TSTRGFRM; the methods they call are fleshed out in
    Stage 7b (layout), 7c (drawing), 7d (mouse), 7e (kbd).       */

#ifndef DBWEAVE_UI_STEUERUNG_CANVAS_H
#define DBWEAVE_UI_STEUERUNG_CANVAS_H

#include <QWidget>

class TSTRGFRM;

class SteuerungCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit SteuerungCanvas(TSTRGFRM* _frm);

protected:
    void paintEvent(QPaintEvent* _e) override;
    void resizeEvent(QResizeEvent* _e) override;
    void mousePressEvent(QMouseEvent* _e) override;
    void mouseMoveEvent(QMouseEvent* _e) override;
    void mouseReleaseEvent(QMouseEvent* _e) override;

private:
    TSTRGFRM* frm;
};

#endif
