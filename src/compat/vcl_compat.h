/*  DB-WEAVE Qt 6 port (Phase 12 cleanup)
    Copyright (C) 1998-2026  Damian Brunold

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  What started as a full VCL-to-Qt shim in Phase 1 is now just a
    thin alias: AnsiString / String remain as typedefs for QString
    so the handful of ported modules that still type
    "AnsiString(\"foo\")" keep compiling unchanged. The legacy
    __fastcall / __published / True / False / TStringList helpers
    were dropped in Phase 12; nothing in the port references them
    any more. */

#ifndef DBWEAVE_COMPAT_VCL_COMPAT_H
#define DBWEAVE_COMPAT_VCL_COMPAT_H

#include <QString>

using AnsiString = QString;
using String = QString;

#endif
