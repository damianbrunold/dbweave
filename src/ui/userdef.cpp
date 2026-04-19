/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "userdef.h"
#include "userdefdialogs.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "settings.h"
#include "undoredo.h"
#include "assert_compat.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>

/*-----------------------------------------------------------------*/
void TDBWFRM::LoadUserdefMenu()
{
    Settings settings;
    bool menuvisible = false;
    for (int i = 0; i < MAXUSERDEF; i++) {
        settings.SetCategory(AnsiString(QStringLiteral("Userdef%1").arg(i)));
        userdef[i].description = settings.Load(AnsiString("Description"), AnsiString(""));
        userdef[i].sizex = settings.Load(AnsiString("Sizex"), 0);
        userdef[i].sizey = settings.Load(AnsiString("Sizey"), 0);
        userdef[i].data = settings.Load(AnsiString("Data"), AnsiString(""));
        const bool visible = !userdef[i].data.isEmpty();
        if (visible)
            menuvisible = true;
        if (UserdefAct[i]) {
            UserdefAct[i]->setVisible(visible);
            UserdefAct[i]->setText(userdef[i].description);
        }
    }
    if (MenuWeitere)
        MenuWeitere->menuAction()->setVisible(menuvisible);
}

/*-----------------------------------------------------------------*/
int TDBWFRM::SelectUserdef(const QString& _title)
{
    UserdefSelectDialog dlg(userdef, _title, this);
    if (dlg.exec() != QDialog::Accepted)
        return -1;
    return dlg.selectedIndex();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::InsertUserdef(int _i, bool _transparent)
{
    dbw3_assert(_i >= 0);
    dbw3_assert(_i < MAXUSERDEF);

    /*  Data has to have at least sizex*sizey characters. */
    if (userdef[_i].data.length() < userdef[_i].sizex * userdef[_i].sizey) {
        dbw3_assert(false);
        return;
    }

    if (userdef[_i].sizex != 0 && kbd_field == GEWEBE) {
        const int x = gewebe.kbd.i + scroll_x1;
        const int y = gewebe.kbd.j + scroll_y2;
        int xx = x + userdef[_i].sizex;
        int yy = y + userdef[_i].sizey;
        if (xx >= Data->MAXX1)
            xx = Data->MAXX1 - 1;
        if (yy >= Data->MAXY2)
            yy = Data->MAXY2 - 1;

        const QString data = userdef[_i].data;
        for (int i = x; i < xx; i++) {
            for (int j = y; j < yy; j++) {
                const int idx = (i - x) * userdef[_i].sizey + (j - y);
                if (idx >= data.length())
                    continue;
                const QChar s = data.at(idx);
                if (s != QChar('k'))
                    gewebe.feld.Set(i, j, char(s.unicode() - 'k'));
                else if (!_transparent)
                    gewebe.feld.Set(i, j, 0);
            }
        }

        RecalcAll();
        CalcRangeKette();
        CalcRangeSchuesse();
        UpdateRapport();
        SetCursor(xx, y);
        selection.begin.i = x;
        selection.begin.j = y;
        selection.end.i = xx - 1;
        selection.end.j = yy - 1;
        if (gewebe.gw > 0 && selection.end.i >= scroll_x1 + gewebe.pos.width / gewebe.gw)
            selection.end.i = scroll_x1 + gewebe.pos.width / gewebe.gw - 1;
        if (gewebe.gh > 0 && selection.end.j >= scroll_y2 + gewebe.pos.height / gewebe.gh)
            selection.end.j = scroll_y2 + gewebe.pos.height / gewebe.gh - 1;
        selection.feld = GEWEBE;
        refresh();
        SetModified();
        if (undo)
            undo->Snapshot();
    }
}

/*-----------------------------------------------------------------*/
void TDBWFRM::PasteUserdef(bool _transparent)
{
    const int i = SelectUserdef(QStringLiteral("Paste user pattern"));
    if (i >= 0 && i < MAXUSERDEF)
        InsertUserdef(i, _transparent);
}

/*-----------------------------------------------------------------*/
/*  Helper: serialise a sub-rect of gewebe.feld into the legacy
    'k'-based alphabet. Extracted because both UserdefAddClick and
    UserdefAddSelClick use the same format. */
static AnsiString encodeRect(TDBWFRM* _frm, int _i1, int _i2, int _j1, int _j2)
{
    QString out;
    out.reserve((_i2 - _i1 + 1) * (_j2 - _j1 + 1));
    for (int i = _i1; i <= _i2; i++)
        for (int j = _j1; j <= _j2; j++) {
            const char s = _frm->gewebe.feld.Get(i, j);
            out.append(QChar(s > 0 ? char(s + 'k') : 'k'));
        }
    return out;
}

/*-----------------------------------------------------------------*/
void TDBWFRM::UserdefAddClick()
{
    if (kette.count() > 50 || schuesse.count() > 50) {
        QMessageBox::information(
            this, QStringLiteral("DB-WEAVE"),
            QStringLiteral("Pattern too large to save as a user-defined pattern "
                           "(maximum 50 × 50)."));
        return;
    }

    const int i = SelectUserdef(QStringLiteral("Select pattern slot"));
    if (i < 0 || i >= MAXUSERDEF)
        return;

    const QString fallback = QStringLiteral("Pattern %1").arg(i + 1);
    const QString seed = userdef[i].description.isEmpty() ? fallback : userdef[i].description;
    const QString descr = getUserdefName(this, seed);
    if (descr.isEmpty())
        return;

    Settings settings;
    settings.SetCategory(AnsiString(QStringLiteral("Userdef%1").arg(i)));
    settings.Save(AnsiString("Description"), AnsiString(descr));
    settings.Save(AnsiString("Sizex"), kette.count());
    settings.Save(AnsiString("Sizey"), schuesse.count());
    settings.Save(AnsiString("Data"), encodeRect(this, kette.a, kette.b, schuesse.a, schuesse.b));
    LoadUserdefMenu();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::UserdefAddSelClick()
{
    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid() && selection.feld == GEWEBE) {
        const int w = selection.end.i - selection.begin.i + 1;
        const int h = selection.end.j - selection.begin.j + 1;
        if (w > 50 || h > 50) {
            QMessageBox::information(
                this, QStringLiteral("DB-WEAVE"),
                QStringLiteral("Pattern too large to save as a user-defined pattern "
                               "(maximum 50 × 50)."));
            selection = savesel;
            return;
        }

        const int i = SelectUserdef(QStringLiteral("Select pattern slot"));
        if (i >= 0 && i < MAXUSERDEF) {
            const QString fallback = QStringLiteral("Pattern %1").arg(i + 1);
            const QString seed
                = userdef[i].description.isEmpty() ? fallback : userdef[i].description;
            const QString descr = getUserdefName(this, seed);
            if (!descr.isEmpty()) {
                Settings settings;
                settings.SetCategory(AnsiString(QStringLiteral("Userdef%1").arg(i)));
                settings.Save(AnsiString("Description"), AnsiString(descr));
                settings.Save(AnsiString("Sizex"), w);
                settings.Save(AnsiString("Sizey"), h);
                settings.Save(AnsiString("Data"),
                              encodeRect(this, selection.begin.i, selection.end.i,
                                         selection.begin.j, selection.end.j));
                LoadUserdefMenu();
                SetCursor(selection.end.i + 1, selection.begin.j);
            }
        }
    }
    selection = savesel;
}

/*-----------------------------------------------------------------*/
void TDBWFRM::UserdefRemoveClick()
{
    const int i = SelectUserdef(QStringLiteral("Select pattern to delete"));
    if (i >= 0 && i < MAXUSERDEF) {
        Settings settings;
        settings.SetCategory(AnsiString(QStringLiteral("Userdef%1").arg(i)));
        settings.Save(AnsiString("Description"), AnsiString(""));
        settings.Save(AnsiString("Sizex"), 0);
        settings.Save(AnsiString("Sizey"), 0);
        settings.Save(AnsiString("Data"), AnsiString(""));
    }
    LoadUserdefMenu();
}
