/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  File menu handlers (Open / Save / Save As) plus the surrounding
    QFileDialog + QMessageBox glue. The legacy unit filehandling.cpp
    had VCL TOpenDialog / TSaveDialog drag-and-drop handling, recent-
    files list (mru.cpp), and the "modified" prompt shown on close.
    This port lands just Open / Save / Save As -- the MRU list and
    the close-prompt are deferred along with the rest of the menu
    chrome.                                                         */

#include "mainwindow.h"
#include "fileformat.h"
#include "loadoptions.h"
#include "undoredo.h"
#include "language.h"

#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>

static QString fileFilter()
{
    return LANG_STR("DB-WEAVE files (*.dbw);;All files (*.*)",
                    "DB-WEAVE-Dateien (*.dbw);;Alle Dateien (*.*)");
}

void TDBWFRM::FileOpen()
{
    const QString dir
        = filename.isEmpty() ? QString() : QFileInfo((QString)filename).absolutePath();
    const QString chosen = QFileDialog::getOpenFileName(
        this, LANG_STR("Open pattern", "Muster öffnen"), dir, fileFilter());
    if (chosen.isEmpty())
        return;

    /*  Close any file the loader from a previous Load left open. */
    if (file && file->IsOpen())
        file->Close();

    filename = chosen;
    LOADSTAT stat = UNKNOWN_FAILURE;
    const bool ok = Load(stat, LOADALL);
    if (!ok) {
        QMessageBox::warning(
            this, QStringLiteral("DB-WEAVE"),
            LANG_STR("Could not open '%1' (status %2).",
                     "'%1' konnte nicht geöffnet werden (Status %2).")
                .arg(chosen)
                .arg(int(stat)));
        return;
    }
    AddToMRU(chosen);
    SetAppTitle();
    refresh();
}

void TDBWFRM::FileSaveAs()
{
    const QString dir = filename.isEmpty() ? QString() : (QString)filename;
    const QString chosen = QFileDialog::getSaveFileName(
        this, LANG_STR("Save pattern as", "Muster speichern unter"), dir, fileFilter());
    if (chosen.isEmpty())
        return;

    if (file && file->IsOpen())
        file->Close();
    filename = chosen;
    FileSave();
}

void TDBWFRM::FileSave()
{
    if (filename.isEmpty()) {
        FileSaveAs();
        return;
    }
    if (!Save()) {
        QMessageBox::warning(
            this, QStringLiteral("DB-WEAVE"),
            LANG_STR("Could not save '%1'.", "'%1' konnte nicht gespeichert werden.")
                .arg((QString)filename));
        return;
    }
    AddToMRU((QString)filename);
    SetModified(false);
    SetAppTitle();
}

/*-----------------------------------------------------------------*/
/*  Recent-files list. Storage via QSettings: six string keys under
    a "mru" group. The in-memory QStringList is trimmed to 6; the
    menu captions alias "&1" .. "&6" so Alt+digit opens the slot. */

static constexpr int MRU_MAX = 6;

void TDBWFRM::AddToMRU(const QString& _filename)
{
    if (_filename.isEmpty())
        return;
    mru.removeAll(_filename);
    mru.prepend(_filename);
    while (mru.size() > MRU_MAX)
        mru.removeLast();
    UpdateMRUMenu();
    SaveMRU();
}

void TDBWFRM::LoadMRU()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("mru"));
    mru.clear();
    for (int i = 0; i < MRU_MAX; i++) {
        const QString v = settings.value(QString::number(i)).toString();
        if (!v.isEmpty())
            mru.append(v);
    }
    settings.endGroup();
    UpdateMRUMenu();
}

void TDBWFRM::SaveMRU()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("mru"));
    settings.remove(QString()); /* wipe the group */
    for (int i = 0; i < mru.size(); i++)
        settings.setValue(QString::number(i), mru.at(i));
    settings.endGroup();
}

void TDBWFRM::UpdateMRUMenu()
{
    for (int i = 0; i < MRU_MAX; i++) {
        QAction* a = mruActions[i];
        if (!a)
            continue;
        if (i < mru.size()) {
            const QString path = mru.at(i);
            const QString shown = QFileInfo(path).fileName();
            a->setText(QStringLiteral("&%1 %2").arg(i + 1).arg(shown.isEmpty() ? path : shown));
            a->setToolTip(path);
            a->setVisible(true);
        } else {
            a->setVisible(false);
        }
    }
}

void TDBWFRM::OpenFromMRU(int _index)
{
    if (_index < 0 || _index >= mru.size())
        return;
    const QString path = mru.at(_index);
    if (!QFileInfo::exists(path)) {
        QMessageBox::warning(this, QStringLiteral("DB-WEAVE"),
                             LANG_STR("'%1' no longer exists.", "'%1' existiert nicht mehr.")
                                 .arg(path));
        mru.removeAt(_index);
        UpdateMRUMenu();
        SaveMRU();
        return;
    }
    if (file && file->IsOpen())
        file->Close();
    filename = path;
    LOADSTAT stat = UNKNOWN_FAILURE;
    if (!Load(stat, LOADALL)) {
        QMessageBox::warning(
            this, QStringLiteral("DB-WEAVE"),
            LANG_STR("Could not open '%1' (status %2).",
                     "'%1' konnte nicht geöffnet werden (Status %2).")
                .arg(path)
                .arg(int(stat)));
        return;
    }
    AddToMRU(path);
    SetAppTitle();
    refresh();
}

/*-----------------------------------------------------------------*/
#include "loadpartsdialog.h"

void TDBWFRM::LoadPartsClick()
{
    LoadPartsDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    const LOADPARTS parts = dlg.getLoadParts();

    const QString dir
        = filename.isEmpty() ? QString() : QFileInfo((QString)filename).absolutePath();
    const QString chosen = QFileDialog::getOpenFileName(
        this, LANG_STR("Load parts from", "Teile laden aus"), dir, fileFilter());
    if (chosen.isEmpty())
        return;

    /*  Swap the working filename in so Load() reads the picked
        file, then restore ours afterwards so the document stays
        tied to its original path.                              */
    const AnsiString savefn = filename;
    filename = chosen;
    if (file && file->IsOpen())
        file->Close();
    LOADSTAT stat = UNKNOWN_FAILURE;
    const bool ok = Load(stat, parts);
    filename = savefn;
    if (file && file->IsOpen())
        file->Close();

    if (!ok) {
        QMessageBox::warning(
            this, QStringLiteral("DB-WEAVE"),
            LANG_STR("Could not load parts from '%1' (status %2).",
                     "Teile aus '%1' konnten nicht geladen werden (Status %2).")
                .arg(chosen)
                .arg(int(stat)));
        return;
    }
    RecalcFreieSchaefte();
    RecalcFreieTritte();
    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}
