/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope:
      * TDBWFRM::UpdateStatusBar -- rebuilds five QLabels that sit
        permanently on the QStatusBar:
          sbField   -- current field + cursor row/col
          sbSelect  -- "Size WxH" when a multi-cell selection is
                       active
          sbRange   -- "Range N" with the paint colour
          sbRapport -- "Size KxS  Rapport KxS" (kette/schuesse)
          sbZoom    -- "Zoom: 13 px"
    Deferred: per-panel owner-drawn coloured backgrounds (legacy
    OnDrawStatusBar), range popup on panel click, palette-colour
    preview swatch, hint text panel.
*/

#include "mainwindow.h"
#include "datamodule.h"
#include "language.h"

#include <QLabel>
#include <QStatusBar>
#include <QString>

static QString fieldName(FELD _f)
{
    switch (_f) {
    case GEWEBE:
        return LANG_STR("Pattern", "Bindung");
    case EINZUG:
        return LANG_STR("Threading", "Einzug");
    case AUFKNUEPFUNG:
        return LANG_STR("Tie-up", "Aufknüpfung");
    case TRITTFOLGE:
        return LANG_STR("Treadling", "Trittfolge");
    case BLATTEINZUG:
        return LANG_STR("Reed threading", "Blatteinzug");
    case KETTFARBEN:
        return LANG_STR("Warp colors", "Kettfarben");
    case SCHUSSFARBEN:
        return LANG_STR("Weft colors", "Schussfarben");
    default:
        return QString();
    }
}

void TDBWFRM::UpdateStatusBar()
{
    if (!sbField)
        return; /* ctor hasn't run the setup yet */

    /*  Field + cursor position. */
    int x = -1, y = -1, sx = 0, sy = 0;
    switch (kbd_field) {
    case GEWEBE:
        x = gewebe.kbd.i + 1;
        y = gewebe.kbd.j + 1;
        sx = scroll_x1;
        sy = scroll_y2;
        break;
    case EINZUG:
        x = einzug.kbd.i + 1;
        y = einzug.kbd.j + 1;
        sx = scroll_x1;
        sy = scroll_y1;
        break;
    case AUFKNUEPFUNG:
        x = aufknuepfung.kbd.i + 1;
        y = aufknuepfung.kbd.j + 1;
        sx = scroll_x2;
        sy = scroll_y1;
        break;
    case TRITTFOLGE:
        x = trittfolge.kbd.i + 1;
        y = trittfolge.kbd.j + 1;
        sx = scroll_x2;
        sy = scroll_y2;
        break;
    case BLATTEINZUG:
        x = blatteinzug.kbd.i + 1;
        sx = scroll_x1;
        break;
    case KETTFARBEN:
        x = kettfarben.kbd.i + 1;
        sx = scroll_x1;
        break;
    case SCHUSSFARBEN:
        y = schussfarben.kbd.j + 1;
        sy = scroll_y2;
        break;
    default:
        break;
    }
    QString field = fieldName(kbd_field);
    if (kbd_field == TRITTFOLGE && ViewSchlagpatrone && ViewSchlagpatrone->isChecked())
        field = LANG_STR("Pegplan", "Schlagpatrone");

    QString pos;
    if (x >= 0)
        pos += LANG_STR("  Col %1", "  Sp %1").arg(x + sx);
    if (y >= 0)
        pos += LANG_STR("  Row %1", "  Ze %1").arg(y + sy);
    sbField->setText(field + pos);

    /*  Selection size (only when > 1x1). */
    QString selText;
    if (selection.Valid()) {
        RANGE sel = selection;
        sel.Normalize();
        const int dx = sel.end.i - sel.begin.i + 1;
        const int dy = sel.end.j - sel.begin.j + 1;
        if (dx > 1 || dy > 1)
            selText = LANG_STR("Selection %1x%2", "Selektion %1x%2").arg(dx).arg(dy);
    }
    sbSelect->setText(selText);

    /*  Current range. */
    QString rangeText;
    if (currentrange >= 1 && currentrange <= 9)
        rangeText = LANG_STR("Range %1", "Bereich %1").arg(currentrange);
    else if (currentrange == AUSHEBUNG)
        rangeText = LANG_STR("Lift out", "Aushebung");
    else if (currentrange == ANBINDUNG)
        rangeText = LANG_STR("Binding", "Anbindung");
    else if (currentrange == ABBINDUNG)
        rangeText = LANG_STR("Unbinding", "Abbindung");
    sbRange->setText(rangeText);

    /*  Size + rapport. */
    QString rap;
    if (kette.a != -1 || schuesse.a != -1) {
        const int kw = (kette.a != -1) ? (kette.b - kette.a + 1) : 0;
        const int sw = (schuesse.a != -1) ? (schuesse.b - schuesse.a + 1) : 0;
        rap = LANG_STR("Size %1x%2", "Grösse %1x%2").arg(kw).arg(sw);
    }
    if (rapport.kr.b != -1 || rapport.sr.b != -1) {
        const int kw = (rapport.kr.b != -1) ? (rapport.kr.b - rapport.kr.a + 1) : 0;
        const int sw = (rapport.sr.b != -1) ? (rapport.sr.b - rapport.sr.a + 1) : 0;
        if (!rap.isEmpty())
            rap += QStringLiteral("  ");
        rap += rapport.overridden ? QStringLiteral("Rapport* %1x%2")
                                  : QStringLiteral("Rapport %1x%2");
        rap = rap.arg(kw).arg(sw);
    }
    sbRapport->setText(rap);

    /*  Lock indicator -- replaces the legacy zoom-in-pixels panel.
        Only shown while OptionsLockGewebe is checked; blank otherwise
        so the space doesn't draw attention when not meaningful.    */
    if (OptionsLockGewebe && OptionsLockGewebe->isChecked())
        sbZoom->setText(LANG_STR("Pattern locked", "Bindung gesperrt"));
    else
        sbZoom->setText(QString());
}
