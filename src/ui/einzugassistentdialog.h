/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy einzugassistent_form.cpp (TEinzugassistentForm).
    "Threading wizard": generates two kinds of threading drafts —
    Geradedurch (straight-through, Z or S orientation) and Abgesetzt
    (stepped/broken draft with gratlen + versatz). On OK: writes
    einzug.feld then runs RecalcGewebe, CalcRangeKette,
    CalcRangeSchuesse, UpdateRapport.                              */

#ifndef DBWEAVE_UI_EINZUGASSISTENT_DIALOG_H
#define DBWEAVE_UI_EINZUGASSISTENT_DIALOG_H

#include <QDialog>

class QRadioButton;
class QSpinBox;
class QTabWidget;
class TDBWFRM;

class EinzugassistentDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EinzugassistentDialog(TDBWFRM* _frm, QWidget* _parent = nullptr);

protected:
    void accept() override;

private:
    TDBWFRM* frm = nullptr;

    QTabWidget* tabs = nullptr;

    /*  Geradedurch (straight-through) page. */
    QSpinBox* gdFirstKettfaden = nullptr;
    QSpinBox* gdFirstSchaft = nullptr;
    QSpinBox* gdSchaefte = nullptr;
    QRadioButton* gdZ = nullptr;
    QRadioButton* gdS = nullptr;

    /*  Abgesetzt (stepped) page. */
    QSpinBox* abFirstKettfaden = nullptr;
    QSpinBox* abFirstSchaft = nullptr;
    QSpinBox* abSchaefte = nullptr;
    QSpinBox* abGratlen = nullptr;
    QSpinBox* abVersatz = nullptr;

    void recalc();
    void createGerade(int _firstKettfaden, int _firstSchaft, int _schaefte, bool _steigend);
    void createAbgesetzt(int _firstKettfaden, int _firstSchaft, int _schaefte, int _gratlen,
                         int _versatz);
};

#endif
