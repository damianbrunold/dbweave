/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*-----------------------------------------------------------------*/
#ifndef DBWEAVE_DOMAIN_ENUMS_H
#define DBWEAVE_DOMAIN_ENUMS_H
/*-----------------------------------------------------------------*/
// FELD definiert die verschiedenen Felder, die fuer eine vollstaendige
// Patrone noetig sind. Die Reihenfolge ist relevant: sie bestimmt,
// die 'Tabreihenfolge' fuer die Tastatureingabe.
// DARSTELLUNG wird bei den Feldern Einzug, Aufknuepfung und
// Trittfolge verwendet, um die Symbole zu bestimmen, die fuer die
// Darstellung eines gefuellten Feldes verwendet werden.
enum DARSTELLUNG {
    AUSGEFUELLT = 0,
    STRICH,
    KREUZ,
    PUNKT,
    KREIS,
    STEIGEND,
    FALLEND,
    SMALLKREUZ,
    SMALLKREIS,
    NUMBER,
    /*  Two extra symbols introduced to match the web-based dbweave
        editor. Append-only so saved files stay binary-compatible
        (viewtype is persisted as the int value of this enum). */
    HDASH, /* horizontal bar (mirror of STRICH along the diagonal) */
    PLUS   /* '+' shape: centred vertical + horizontal stroke      */
};
/*-----------------------------------------------------------------*/
/*  TOOL selects the behaviour of a mouse-drag on the gewebe canvas.
    TOOL_POINT is the default paint-single-cell-by-click mode; the
    remaining modes treat the drag as a two-point geometric shape
    (anchor + release cell) rasterised onto gewebe on release.    */
enum TOOL {
    TOOL_POINT = 0,
    TOOL_LINE,
    TOOL_RECTANGLE,
    TOOL_FILLEDRECTANGLE,
    TOOL_ELLIPSE,
    TOOL_FILLEDELLIPSE
};
/*-----------------------------------------------------------------*/
enum FELD {
    INVALID,
    GEWEBE,
    EINZUG,
    TRITTFOLGE,
    AUFKNUEPFUNG,
    SCHUSSFARBEN,
    KETTFARBEN,
    BLATTEINZUG,
    ISEMPTY,    // Ab hier sind es unsichtbare Felder
    HLINEHORZ1, // Zwar nicht unsichtbar aber nicht per Tastatur bedienbar...
    HLINEHORZ2,
    HLINEVERT1,
    HLINEVERT2
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
