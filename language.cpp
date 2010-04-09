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
#include <vcl\vcl.h>
#pragma hdrstop
#include "language.h"
#include "dbw3_form.h"
#include "assert.h"
#include "datamodule.h"
#include "farbpalette_form.h"
#include "steuerung_form.h"
#include "toolpalette_form.h"
/*-----------------------------------------------------------------*/
LANGUAGES active_language = EN;
/*-----------------------------------------------------------------*/
void SwitchLanguage (LANGUAGES _language)
{
    if (active_language==_language) return;

    active_language = _language;

    DBWFRM->ReloadLanguage();
    DBWFRM->SetAppTitle();
    Data->ReloadLanguage();
    FarbPalette->ReloadLanguage();
    ToolpaletteForm->ReloadLanguage();
    STRGFRM->ReloadLanguage();

    if (active_language==EN) Application->HelpFile = "dbwe.hlp";
    else if (active_language==GE) Application->HelpFile = "dbwg.hlp";
    else dbw3_assert(false);
}
/*-----------------------------------------------------------------*/

