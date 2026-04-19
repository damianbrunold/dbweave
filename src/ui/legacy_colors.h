/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    Hard-coded legacy Windows-95 system colours the whole port shares.
    Legacy dbweave rendered on clBtnFace (light grey) / clBtnShadow
    (medium grey); reproduce those verbatim so the canvas stays
    readable under dark desktop themes until a properly themed
    palette arrives.
*/

#ifndef DBWEAVE_UI_LEGACY_COLORS_H
#define DBWEAVE_UI_LEGACY_COLORS_H

#include <QColor>

inline QColor legacyBtnFace()
{
    return QColor(192, 192, 192);
}
inline QColor legacyBtnShadow()
{
    return QColor(128, 128, 128);
}

#endif
