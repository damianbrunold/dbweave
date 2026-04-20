/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope: the six non-click replication utilities plus the
    three click handlers from legacy/rapportieren.cpp. The
    RappRapportierenClick dispatches to a Qt port of the legacy
    TRapportForm (RapportDialog) for the repeat-count entry.
*/

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "rapport.h"
#include "rapportdialog.h"
#include "undoredo.h"
/*-----------------------------------------------------------------*/
void TDBWFRM::RapportSchuss(int _ry, bool _withcolors)
{
    int j, j1, j2, sj1, sj2;

    // Schussbereich bestimmen
    CalcRangeSchuesse();
    j1 = schuesse.a;
    j2 = schuesse.b;
    if (j1 == -1)
        return;
    if (j2 == -1)
        return;

    // Schussrapport bestimmen
    CalcRapport();
    sj1 = rapport.sr.a;
    sj2 = rapport.sr.b;
    if (sj1 == 0 && sj2 == -1) {
        sj1 = j1;
        sj2 = j2;
    }

    // Alles ausserhalb Rapport loeschen
    for (j = j1; j < sj1; j++)
        ClearSchussfaden(j);
    for (j = sj2 + 1; j <= j2; j++)
        ClearSchussfaden(j);

    // Rapportieren
    // xxxx momentan rapportiere ich nur nach oben...
    int maxj;
    if (_ry != -1)
        maxj = sj1 + (sj2 - sj1 + 1) * _ry;
    else
        maxj = Data->MAXY2;
    if (maxj >= Data->MAXY2)
        maxj = Data->MAXY2;
    for (j = sj2 + 1; j < maxj; j++)
        CopySchussfaden(sj1 + (j - sj2 - 1) % (sj2 - sj1 + 1), j, _withcolors);

    // Schussbereich nachfuehren
    // xxx koennte optimaler sein...
    CalcRangeSchuesse();
}
/*-----------------------------------------------------------------*/
void TDBWFRM::RapportKette(int _rx, bool _withcolors)
{
    int i, i1, i2, ki1, ki2;

    // Kettbereich bestimmen
    CalcRangeKette();
    i1 = kette.a;
    i2 = kette.b;
    if (i1 == -1)
        return;
    if (i2 == -1)
        return;

    // Kettrapport bestimmen
    CalcRapport();
    ki1 = rapport.kr.a;
    ki2 = rapport.kr.b;
    if (ki1 == 0 && ki2 == -1) {
        ki1 = i1;
        ki2 = i2;
    }

    // Alles ausserhalb Rapport loeschen
    for (i = i1; i < ki1; i++)
        ClearKettfaden(i);
    for (i = ki2 + 1; i <= i2; i++)
        ClearKettfaden(i);

    // Rapportieren
    // xxxx momentan rapportiere ich nur nach rechts...
    int maxi;
    if (_rx != -1)
        maxi = ki1 + (ki2 - ki1 + 1) * _rx;
    else
        maxi = Data->MAXX1;
    if (maxi >= Data->MAXX1)
        maxi = Data->MAXX1;
    for (i = ki2 + 1; i < maxi; i++)
        CopyKettfaden(ki1 + (i - ki2 - 1) % (ki2 - ki1 + 1), i, _withcolors);

    // Kettbereich nachfuehren
    // xxx koennte optimaler sein...
    CalcRangeKette();
}
/*-----------------------------------------------------------------*/
void TDBWFRM::CopyKettfaden(int _von, int _nach, bool _withcolors)
{
    // Blatteinzug kopieren
    // xxxx ?? muss ich das?!
    //    blatteinzug.Set (_nach, blatteinzug.Get (_von));

    // Farbe
    if (_withcolors)
        kettfarben.feld.Set(_nach, kettfarben.feld.Get(_von));

    // Einzug kopieren
    einzug.feld.Set(_nach, einzug.feld.Get(_von));

    // Gewebe kopieren
    if (schuesse.a != -1 && schuesse.b != -1) {
        for (int j = schuesse.a; j <= schuesse.b; j++)
            gewebe.feld.Set(_nach, j, gewebe.feld.Get(_von, j));
    }
}
/*-----------------------------------------------------------------*/
void TDBWFRM::CopySchussfaden(int _von, int _nach, bool _withcolors)
{
    // Tritt kopieren
    for (int i = 0; i < Data->MAXX2; i++)
        trittfolge.feld.Set(i, _nach, trittfolge.feld.Get(i, _von));

    // Farbe kopieren
    if (_withcolors)
        schussfarben.feld.Set(_nach, schussfarben.feld.Get(_von));

    // Gewebe kopieren
    if (kette.a != -1 && kette.b != -1) {
        for (int i = kette.a; i <= kette.b; i++)
            gewebe.feld.Set(i, _nach, gewebe.feld.Get(i, _von));
    }
}
/*-----------------------------------------------------------------*/
void TDBWFRM::ClearKettfaden(int _i)
{
    einzug.feld.Set(_i, 0);
    if (schuesse.b != -1)
        for (int j = schuesse.a; j <= schuesse.b; j++)
            gewebe.feld.Set(_i, j, 0);
}
/*-----------------------------------------------------------------*/
void TDBWFRM::ClearSchussfaden(int _j)
{
    for (int i = 0; i < Data->MAXX2; i++)
        trittfolge.feld.Set(i, _j, 0);
    if (kette.b != -1)
        for (int i = kette.a; i <= kette.b; i++)
            gewebe.feld.Set(i, _j, 0);
}
/*-----------------------------------------------------------------*/
void TDBWFRM::RappRapportierenClick()
{
    /*  Not gated by GewebeLocked: replicates the current rapport
        across gewebe / einzug / trittfolge / colours in lockstep and
        does not trigger a recalc-from-gewebe.                     */
    RapportDialog dlg(this);
    dlg.setRepeatAll(false);
    const int kx = kette.b - kette.a + 1;
    const int rx = rapport.kr.b - rapport.kr.a + 1;
    if (kx != 0 && rx != 0)
        dlg.setHorz(kx / rx);
    const int ky = schuesse.b - schuesse.a + 1;
    const int ry = rapport.sr.b - rapport.sr.a + 1;
    if (ky != 0 && ry != 0)
        dlg.setVert(ky / ry);

    if (dlg.exec() != QDialog::Accepted)
        return;

    int rxn = dlg.horz();
    int ryn = dlg.vert();
    if (dlg.repeatAll())
        rxn = ryn = -1;
    const bool rappcolors = dlg.repeatColors();
    RapportKette(rxn == 0 ? 1 : rxn, rappcolors);
    RapportSchuss(ryn == 0 ? 1 : ryn, rappcolors);

    CalcRangeSchuesse();
    CalcRangeKette();
    CalcRapport();

    UpdateStatusBar();
    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void TDBWFRM::RappReduzierenClick()
{
    /*  Not gated by GewebeLocked: reducing the pattern to a single
        rapport is a deliberate user action on structural data and
        is undoable. Under EzFixiert it triggers a RecalcAll below,
        which the user can back out of via undo if unwanted.     */
    RapportKette(1, false);
    RapportSchuss(1, false);
    if (EzFixiert && EzFixiert->isChecked())
        RecalcAll();

    CalcRangeSchuesse();
    CalcRangeKette();
    CalcRapport();

    UpdateStatusBar();
    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}
/*-----------------------------------------------------------------*/
void TDBWFRM::RappOverrideClick()
{
    if (!rapport.overridden) {
        if (selection.Valid() && selection.feld == GEWEBE) {
            RANGE save = selection;
            selection.Normalize();
            if (RappViewRapport && RappViewRapport->isChecked())
                ClearRapport();
            rapport.overridden = true;
            RAPPORT old = rapport;
            rapport.kr.a = selection.begin.i;
            rapport.kr.b = selection.end.i;
            rapport.sr.a = selection.begin.j;
            rapport.sr.b = selection.end.j;
            if (RappViewRapport && RappViewRapport->isChecked()) {
                DrawRapport();
                if (rapporthandler)
                    rapporthandler->DrawDifferences(old, rapport);
            }
            ClearSelection();
            (void)save;
        }
    } else {
        rapport.overridden = false;
        UpdateRapport();
    }
    UpdateStatusBar();
    refresh();
}
