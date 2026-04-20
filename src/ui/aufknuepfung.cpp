/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope: MinimizeAufknuepfung plus the nine Auf*Click menu
    handlers (ClearAufknuepfungClick, AufInvertClick,
    AufZentralsymmClick, four AufRoll*Click, AufSteigung{Inc,Dec}Click).

    None of these are gated by GewebeLocked: the user edits the
    aufknuepfung (or clears it, together with gewebe in the Clear
    case) and the fabric preview is refreshed via RecalcGewebe --
    there is no RecalcAll here, so einzug / trittfolge / aufknuepfung
    are never regenerated from gewebe.
*/

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "einzug.h"
#include "dbw3_strings.h"
#include "zentralsymm.h"
#include "undoredo.h"
#include <QAction>
#include <QMessageBox>
/*-----------------------------------------------------------------*/
void TDBWFRM::MinimizeAufknuepfung()
{
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
        return;

    // Leere Schaefte killen
    for (int j = 0; j < Data->MAXY1; j++)
        if (einzughandler && einzughandler->IsEmptySchaft(j)) {
            for (int i = 0; i < Data->MAXX2; i++)
                aufknuepfung.feld.Set(i, j, 0);
            einzughandler->RedrawAufknuepfungSchaft(j);
        }

    // Leere Tritte killen
    for (int i = 0; i < Data->MAXX2; i++)
        if (IsEmptyTritt(i)) {
            for (int j = 0; j < Data->MAXY1; j++)
                aufknuepfung.feld.Set(i, j, 0);
            RedrawAufknuepfungTritt(i);
        }
}

/*-----------------------------------------------------------------*/
void TDBWFRM::ClearAufknuepfungClick()
{
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
        return;

    for (int i = 0; i < Data->MAXX2; i++)
        for (int j = 0; j < Data->MAXY1; j++)
            aufknuepfung.feld.Set(i, j, 0);

    for (int i = 0; i < Data->MAXX1; i++)
        for (int j = 0; j < Data->MAXY2; j++)
            gewebe.feld.Set(i, j, 0);

    CalcRangeKette();
    CalcRangeSchuesse();
    UpdateRapport();
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::AufInvertClick()
{
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
        return;

    const int s1 = GetFirstSchaft();
    const int s2 = GetLastSchaft();
    if (s2 < s1)
        return;
    const int t1 = GetFirstTritt();
    const int t2 = GetLastTritt();
    if (t2 < t1)
        return;

    for (int i = t1; i <= t2; i++)
        for (int j = s1; j <= s2; j++) {
            const char s = aufknuepfung.feld.Get(i, j);
            aufknuepfung.feld.Set(i, j, char(s == 0 ? currentrange : -s));
        }

    if (kette.a != -1 && schuesse.a != -1) {
        for (int i = kette.a; i <= kette.b; i++)
            for (int j = schuesse.a; j <= schuesse.b; j++) {
                const char s = gewebe.feld.Get(i, j);
                gewebe.feld.Set(i, j, char(s == 0 ? currentrange : -s));
            }
    }

    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::AufZentralsymmClick()
{
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
        return;

    const int s1 = GetFirstSchaft();
    const int s2 = GetLastSchaft();
    if (s2 < s1)
        return;
    const int t1 = GetFirstTritt();
    const int t2 = GetLastTritt();
    if (t2 < t1)
        return;
    /*  The central-symmetry search requires a square tie-up region. */
    if ((t2 - t1) != (s2 - s1))
        return;

    ZentralSymmChecker symchecker(t2 - t1 + 1, s2 - s1 + 1);
    for (int i = t1; i <= t2; i++)
        for (int j = s1; j <= s2; j++)
            symchecker.SetData(i - t1, j - s1, aufknuepfung.feld.Get(i, j));

    if (symchecker.IsAlreadySymmetric())
        return;

    if (symchecker.SearchSymmetry()) {
        for (int i = t1; i <= t2; i++)
            for (int j = s1; j <= s2; j++)
                aufknuepfung.feld.Set(i, j, symchecker.GetData(i - t1, j - s1));
    } else {
        QMessageBox::information(this, QStringLiteral(APP_TITLE), NOCENTRALSYMMFOUND);
        return;
    }

    RecalcGewebe();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();

    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
/*  AufRoll{Up,Down,Left,Right}Click: rotate the tie-up rectangle one
    row or column and let RecalcGewebe repaint the fabric preview. */
void TDBWFRM::AufRollUpClick()
{
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
        return;
    const int s1 = GetFirstSchaft();
    const int s2 = GetLastSchaft();
    if (s2 < s1)
        return;
    const int t1 = GetFirstTritt();
    const int t2 = GetLastTritt();
    if (t2 < t1)
        return;

    QByteArray data(t2 - t1 + 1, 0);
    for (int i = t1; i <= t2; i++)
        data[i - t1] = aufknuepfung.feld.Get(i, s2);
    for (int j = s2; j > s1; j--)
        for (int i = t1; i <= t2; i++)
            aufknuepfung.feld.Set(i, j, aufknuepfung.feld.Get(i, j - 1));
    for (int i = t1; i <= t2; i++)
        aufknuepfung.feld.Set(i, s1, data.at(i - t1));

    RecalcGewebe();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

void TDBWFRM::AufRollDownClick()
{
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
        return;
    const int s1 = GetFirstSchaft();
    const int s2 = GetLastSchaft();
    if (s2 < s1)
        return;
    const int t1 = GetFirstTritt();
    const int t2 = GetLastTritt();
    if (t2 < t1)
        return;

    QByteArray data(t2 - t1 + 1, 0);
    for (int i = t1; i <= t2; i++)
        data[i - t1] = aufknuepfung.feld.Get(i, s1);
    for (int j = s1; j < s2; j++)
        for (int i = t1; i <= t2; i++)
            aufknuepfung.feld.Set(i, j, aufknuepfung.feld.Get(i, j + 1));
    for (int i = t1; i <= t2; i++)
        aufknuepfung.feld.Set(i, s2, data.at(i - t1));

    RecalcGewebe();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

void TDBWFRM::AufRollLeftClick()
{
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
        return;
    const int s1 = GetFirstSchaft();
    const int s2 = GetLastSchaft();
    if (s2 < s1)
        return;
    const int t1 = GetFirstTritt();
    const int t2 = GetLastTritt();
    if (t2 < t1)
        return;

    QByteArray data(s2 - s1 + 1, 0);
    for (int j = s1; j <= s2; j++)
        data[j - s1] = aufknuepfung.feld.Get(t1, j);
    for (int i = t1; i < t2; i++)
        for (int j = s1; j <= s2; j++)
            aufknuepfung.feld.Set(i, j, aufknuepfung.feld.Get(i + 1, j));
    for (int j = s1; j <= s2; j++)
        aufknuepfung.feld.Set(t2, j, data.at(j - s1));

    RecalcGewebe();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

void TDBWFRM::AufRollRightClick()
{
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
        return;
    const int s1 = GetFirstSchaft();
    const int s2 = GetLastSchaft();
    if (s2 < s1)
        return;
    const int t1 = GetFirstTritt();
    const int t2 = GetLastTritt();
    if (t2 < t1)
        return;

    QByteArray data(s2 - s1 + 1, 0);
    for (int j = s1; j <= s2; j++)
        data[j - s1] = aufknuepfung.feld.Get(t2, j);
    for (int i = t2; i > t1; i--)
        for (int j = s1; j <= s2; j++)
            aufknuepfung.feld.Set(i, j, aufknuepfung.feld.Get(i - 1, j));
    for (int j = s1; j <= s2; j++)
        aufknuepfung.feld.Set(t1, j, data.at(j - s1));

    RecalcGewebe();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
/*  AufSteigung{Inc,Dec}Click: shift the tie-up rectangle's rows
    relative to their columns (diagonal slope), using the shared
    Increment/DecrementSteigung helpers from steigung.cpp.       */
void TDBWFRM::AufSteigungIncClick()
{
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
        return;
    const int s1 = GetFirstSchaft();
    const int s2 = GetLastSchaft();
    if (s2 < s1)
        return;
    const int t1 = GetFirstTritt();
    const int t2 = GetLastTritt();
    if (t2 < t1)
        return;

    IncrementSteigung(t1, s1, t2, s2, AUFKNUEPFUNG);

    RecalcGewebe();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}

void TDBWFRM::AufSteigungDecClick()
{
    if (ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
        return;
    const int s1 = GetFirstSchaft();
    const int s2 = GetLastSchaft();
    if (s2 < s1)
        return;
    const int t1 = GetFirstTritt();
    const int t2 = GetLastTritt();
    if (t2 < t1)
        return;

    DecrementSteigung(t1, s1, t2, s2, AUFKNUEPFUNG);

    RecalcGewebe();
    CalcRangeSchuesse();
    CalcRangeKette();
    UpdateRapport();
    refresh();
    SetModified();
    if (undo)
        undo->Snapshot();
}
/*-----------------------------------------------------------------*/
