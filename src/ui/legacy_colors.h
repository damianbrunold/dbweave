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

/*  Lightened from the legacy Windows "3DFace" (192,192,192) so the
    canvas blends smoothly into the chrome palette (~232,232,232).
    The shadow used for grid lines and strongline dividers is pulled
    up in step so line contrast against the lighter face is kept. */
inline QColor legacyBtnFace()
{
    return QColor(224, 224, 224);
}
inline QColor legacyBtnShadow()
{
    return QColor(160, 160, 160);
}

#endif
