/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  In-house two-language (English / German) string table. Kept as a
    compile-time lookup instead of Qt's QTranslator because every
    legacy call site uses LANG_STR(EN, GE) inline -- porting them to
    tr()/.ts files is Phase-12 cleanup, not this phase.

    Source files in the port are UTF-8, matching Qt 6's default
    QString(const char*) interpretation. Legacy files were ISO-8859-1;
    the umlauts have been transcoded.
*/

#ifndef DBWEAVE_UI_LANGUAGE_H
#define DBWEAVE_UI_LANGUAGE_H

#include "vcl_compat.h"   /* AnsiString -> QString */

enum LANGUAGES { EN /*englisch*/,
                 GE /*deutsch*/};

/*  Globale Variable, die die eingestellte Sprache festhaelt.
    Umschalten mittels SwitchLanguage. */
extern LANGUAGES active_language;

void SwitchLanguage (LANGUAGES _language);

/*  LANG_C_H(OBJ, LANG, CAPTION, HINT) is the legacy VCL helper that
    sets OBJ->Caption and OBJ->Hint when the named language is active.
    The Qt equivalent is to call setText() and setToolTip()/
    setStatusTip() on QAction/QWidget. The macro keeps the legacy
    call-site shape and will be replaced with explicit Qt calls when
    the first menu is wired up -- for now it stands so lang_main.cpp
    can eventually be ported mechanically. */
#define LANG_C_H(OBJ, LANG, CAPTION, HINT) \
    if (active_language==LANG) { \
        (OBJ)->setText(QStringLiteral(CAPTION)); \
        (OBJ)->setToolTip(QStringLiteral(HINT)); \
    }

/*  LANG_STR(EN, GE) picks one of the two string literals based on the
    active language and wraps it in a QString. */
#define LANG_STR(STR_EN, STR_GE) \
    AnsiString(active_language==GE ? (STR_GE) : (STR_EN))

#endif
