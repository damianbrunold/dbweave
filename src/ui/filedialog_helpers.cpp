/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "filedialog_helpers.h"
#include "settings.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

QString lastDirFor(const char* _category)
{
    Settings settings;
    settings.SetCategory(QString("LastDir"));
    const QString saved = settings.Load(QString(_category), QString());
    const QString dir = (QString)saved;
    if (!dir.isEmpty() && QFileInfo(dir).isDir())
        return dir;
    const QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return docs.isEmpty() ? QDir::homePath() : docs;
}

void rememberDirFor(const char* _category, const QString& _path)
{
    if (_path.isEmpty())
        return;
    const QString dir = QFileInfo(_path).absolutePath();
    if (dir.isEmpty())
        return;
    Settings settings;
    settings.SetCategory(QString("LastDir"));
    settings.Save(QString(_category), QString(dir));
}
