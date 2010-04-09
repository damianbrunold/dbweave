/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*-----------------------------------------------------------------*/
#ifndef enumsH
#define enumsH
/*-----------------------------------------------------------------*/
// FELD definiert die verschiedenen Felder, die fuer eine vollstaendige
// Patrone noetig sind. Die Reihenfolge ist relevant: sie bestimmt,
// die 'Tabreihenfolge' fuer die Tastatureingabe.
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
    HLINEHORZ1, // Zwar nicht unsichtbar aber nicht per Tastatur bedienbar...
    HLINEHORZ2,
    HLINEVERT1,
    HLINEVERT2
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
