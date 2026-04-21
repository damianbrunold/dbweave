/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  User-defined patterns. A fixed-size array of up to 10 named
    patterns persisted via Settings (QSettings), accessible from
    the Pattern menu to insert, replace, or delete.                */

#ifndef DBWEAVE_UI_USERDEF_H
#define DBWEAVE_UI_USERDEF_H

#include <QString>

#define MAXUSERDEF 10

struct UserdefPattern {
    QString description;
    int sizex = 0;
    int sizey = 0;
    QString data;
};

#endif
