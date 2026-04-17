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
enum DARSTELLUNG
{
	AUSGEFUELLT=0,
	STRICH,
	KREUZ,
	PUNKT,
	KREIS,
	STEIGEND,
	FALLEND,
	SMALLKREUZ,
	SMALLKREIS,
	NUMBER
};
/*-----------------------------------------------------------------*/
enum FELD
{
	INVALID,
	GEWEBE,
	EINZUG,
	TRITTFOLGE,
	AUFKNUEPFUNG,
	SCHUSSFARBEN,
	KETTFARBEN,
	BLATTEINZUG,
	ISEMPTY,      // Ab hier sind es unsichtbare Felder
	HLINEHORZ1,   // Zwar nicht unsichtbar aber nicht per Tastatur bedienbar...
	HLINEHORZ2,
	HLINEVERT1,
	HLINEVERT2
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
