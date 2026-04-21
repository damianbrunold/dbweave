/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Small utilities for file-dialog defaulting. On first use a dialog
    lands in the platform's Documents folder; on subsequent uses it
    lands in whichever folder the user last picked for the same
    category. Categories are short stable keys ("Pattern",
    "Template", "ImportBitmap", "ImportWIF", "Export", "ExportInfo")
    stored under the "LastDir" Settings category.                   */

#ifndef DBWEAVE_UI_FILEDIALOG_HELPERS_H
#define DBWEAVE_UI_FILEDIALOG_HELPERS_H

#include <QString>

/*  Preferred starting directory for a file dialog in `_category`.
    Returns the last-remembered folder for that category, or the
    user's Documents folder if none has been stored yet.          */
QString lastDirFor(const char* _category);

/*  Persist the directory of `_path` under `_category` so the next
    dialog in the same category opens there. Pass the full chosen
    filename -- this function extracts the directory portion. No-op
    when `_path` is empty.                                         */
void rememberDirFor(const char* _category, const QString& _path);

#endif
