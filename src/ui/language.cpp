/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "language.h"
#include "datamodule.h"
#include "mainwindow.h"

LANGUAGES active_language = EN;

void SwitchLanguage(LANGUAGES _language)
{
    if (active_language == _language)
        return;

    active_language = _language;

    if (DBWFRM)
        DBWFRM->ReloadLanguage();
    if (Data)
        Data->ReloadLanguage();
}
