/*  DB-WEAVE Qt 6 port - VCL compatibility shim (Phase 1)
    Copyright (C) 1998-2026  Damian Brunold

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Thin compatibility layer that lets legacy C++Builder / VCL source
    files compile under Qt 6 with minimal mechanical edits.

    Scope of this header:
      - map AnsiString onto QString (UTF-16 vs VCL's 1-byte-per-codepage:
        the legacy code treats strings opaquely at API boundaries, so the
        substitution is safe; pixel-level byte access is not attempted).
      - neutralise Borland-specific keyword macros (__fastcall, __published).
      - provide True/False for VCL-style Pascal literals.

    Not scope:
      - TCanvas, TBitmap drawing primitives (rewrite against QPainter /
        QImage during each module's port).
      - TForm, TEdit, TButton and other widgets (rebuild via Qt Designer).
*/

#ifndef DBWEAVE_COMPAT_VCL_COMPAT_H
#define DBWEAVE_COMPAT_VCL_COMPAT_H

#include <QString>

using AnsiString = QString;
using String     = QString;

#ifndef True
#   define True  true
#endif
#ifndef False
#   define False false
#endif

/*  Borland-specific calling convention and access specifier macros. */
#ifndef __fastcall
#   define __fastcall
#endif
#ifndef __published
#   define __published public
#endif

#endif /* DBWEAVE_COMPAT_VCL_COMPAT_H */
