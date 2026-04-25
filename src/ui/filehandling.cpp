/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  File menu handlers (New / Open / Save / Save As / New from
    template) plus the surrounding QFileDialog + QMessageBox glue,
    the AskSave "modified" prompt used before destructive document
    changes, and ResetDocument used by File > New.                  */

#include "mainwindow.h"
#include "patterncanvas.h"
#include "fileformat.h"
#include "loadoptions.h"
#include "undoredo.h"
#include "cursor.h"
#include "datamodule.h"
#include "palette.h"
#include "properties.h"
#include "blockmuster.h"
#include "language.h"
#include "filedialog_helpers.h"
#include <QCoreApplication>
#include <QDir>

#include <cstring>

#include <QAction>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>

static QString fileFilter()
{
    return LANG_STR("DB-WEAVE files (*.dbw);;All files (*.*)",
                    "DB-WEAVE-Dateien (*.dbw);;Alle Dateien (*.*)");
}

static QString templateFilter()
{
    return LANG_STR("DB-WEAVE templates (*.dbv);;All files (*.*)",
                    "DB-WEAVE-Vorlagen (*.dbv);;Alle Dateien (*.*)");
}

/*-----------------------------------------------------------------*/
/*  Wipe the undo history and seed it with one snapshot of the
    current document state. Called by the full-document load paths
    so the freshly-loaded contents become the only undoable point --
    without this, undo could walk back into the previously-loaded
    document or to an empty state from before any load.            */
void TDBWFRM::ResetUndoToCurrentState()
{
    if (!undo)
        return;
    delete undo;
    undo = new UrUndo(this);
    undo->Snapshot();
    undo->MarkClean();
}

/*-----------------------------------------------------------------*/
/*  Port of legacy DateiNeu: reset every bit of document state back
    to the values a freshly-constructed TDBWFRM would have. Does NOT
    prompt the user or clear the filename -- callers handle those so
    the same helper can be shared between FileNewClick (no filename)
    and FileNewTemplateClick (loads a .dbv first, then clears).   */
void TDBWFRM::ResetDocument()
{
    /*  Close any open backing file so the next Save won't try to
        write through a stale handle.                              */
    if (file && file->IsOpen())
        file->Close();

    /*  Reset Data->MAX* back to their ctor defaults so the cleared
        document has the same dimensions a freshly-constructed one
        would have.                                                 */
    Data->MAXX1 = DEFAULT_MAXX1;
    Data->MAXX2 = DEFAULT_MAXX2;
    Data->MAXY1 = DEFAULT_MAXY1;
    Data->MAXY2 = DEFAULT_MAXY2;

    /*  Resize the field buffers to match, then wipe every cell.
        Resize() preserves existing content -- Clear() is what
        actually zeroes the gewebe / einzug / aufknuepfung /
        trittfolge / colour strips back to empty.                 */
    AllocBuffers(true);
    gewebe.Clear();
    einzug.Clear();
    aufknuepfung.Clear();
    trittfolge.Clear();
    blatteinzug.Clear();
    kettfarben.Clear();
    schussfarben.Clear();

    /*  Recalc scratch buffers. */
    if (xbuf)
        std::memset(xbuf, 0, Data->MAXX1);
    if (ybuf)
        std::memset(ybuf, 0, Data->MAXY2);

    /*  Klammer (loom brace) ranges. */
    for (int i = 0; i < 9; i++) {
        klammern[i].first = 0;
        klammern[i].last = 1;
        klammern[i].repetitions = 0;
    }

    /*  Fix-einzug state. AllocBuffersX1 above already (re)allocated
        fixeinzug zero-initialised; just reset the companion counters. */
    firstfree = 1;
    fixsize = 0;

    /*  Scroll / viewport. */
    scroll_x1 = scroll_x2 = scroll_y1 = scroll_y2 = 0;
    currentzoom = 4;

    /*  Weft/warp ratio back to square cells. */
    faktor_kette = 1.0f;
    faktor_schuss = 1.0f;

    /*  Ranges + rapport + selection. Legacy sentinel for "no range"
        is a = b = -1.                                              */
    kette = SZ(-1, -1);
    schuesse = SZ(-1, -1);
    rapport = RAPPORT();
    rapport.kr = SZ(0, -1);
    rapport.sr = SZ(0, -1);
    ClearSelection();

    /*  Reset the visible strip caps to the ctor defaults (12 each).
        A subsequent template load would overwrite these anyway, but
        File > New starts from a clean default look.                */
    hvisible = DEFAULT_MAXY1;
    wvisible = DEFAULT_MAXX2;

    /*  Tool / range state. */
    tool = TOOL_POINT;
    currentrange = 1;
    if (rangeActions[0] && rangeActions[0]->actionGroup())
        rangeActions[0]->setChecked(true);

    /*  View-menu toggles back to defaults. */
    if (ViewSchlagpatrone)
        ViewSchlagpatrone->setChecked(false);
    trittfolge.einzeltritt = true;
    if (EzMinimalZ)
        EzMinimalZ->setChecked(true);
    if (TfMinimalZ)
        TfMinimalZ->setChecked(true);
    if (GewebeNormal)
        GewebeNormal->setChecked(true);
    if (RappViewRapport)
        RappViewRapport->setChecked(false);
    if (ViewHlines)
        ViewHlines->setChecked(true);
    if (Inverserepeat)
        Inverserepeat->setChecked(false);
    if (OptionsLockGewebe)
        OptionsLockGewebe->setChecked(false);
    fewithraster = false;
    /*  Orientation / shed / reed visibility follow the user's saved
        Grundeinstellung. Ensures that File > New creates a document
        with the preset the user configured, regardless of what a
        previously-loaded pattern set.                              */
    ApplyBaseStyleFromSettings();

    /*  Secondary state: hilfslinien, blockmuster, bereichmuster,
        metadata, print borders.                                   */
    hlines.DeleteAll();
    for (int i = 0; i < 10; i++) {
        blockmuster[i].Clear();
        bereichmuster[i].Clear();
    }
    currentbm = 0;
    if (Data->properties)
        Data->properties->Init();
    if (Data->palette)
        Data->palette->InitPalette();
    InitBorders();

    /*  Cursor + keyboard field. */
    kbd_field = GEWEBE;
    if (cursorhandler) {
        cursorhandler->SetCursorDirection(CD_UP);
    }

    /*  Undo stack: clear + seed with a snapshot of the empty state. */
    if (undo) {
        delete undo;
        undo = new UrUndo(this);
    }

    /*  Menu chrome catches up (Trittfolge vs Schlagpatrone). */
    UpdateSchlagpatroneMode();

    /*  Repaint + relayout. */
    RecalcGewebe();
    if (pattern_canvas)
        pattern_canvas->recomputeLayout();
    SetModified(false);
    refresh();
    if (undo)
        undo->Snapshot();
}

/*-----------------------------------------------------------------*/
/*  FileNewClick — prompt-save, reset the document and clear the
    filename so Save prompts for a new target. Mirrors legacy
    DateiNeu semantics minus the optional "normal.dbv" bootstrap
    (that's what FileNewTemplateClick is for).                    */
void TDBWFRM::FileNewClick()
{
    if (!AskSave())
        return;
    filename.clear();
    ResetDocument();
    SetAppTitle();
}

/*-----------------------------------------------------------------*/
/*  FileNewTemplateClick — ask the user for a .dbv file, load it and
    then clear the filename so the next Save prompts for a fresh
    target. If the user cancels the file-dialog we leave the current
    document untouched.                                           */
void TDBWFRM::FileNewTemplateClick()
{
    if (!AskSave())
        return;
    const QString dir
        = filename.isEmpty() ? lastDirFor("Template") : QFileInfo((QString)filename).absolutePath();
    const QString chosen = QFileDialog::getOpenFileName(
        this, LANG_STR("New from template", "Neu gemäss Vorlage"), dir, templateFilter());
    if (chosen.isEmpty())
        return;
    rememberDirFor("Template", chosen);
    if (file && file->IsOpen())
        file->Close();
    filename = chosen;
    LOADSTAT stat = UNKNOWN_FAILURE;
    if (!Load(stat, LOADALL)) {
        QMessageBox::warning(this, QStringLiteral("DB-WEAVE"),
                             LANG_STR("Could not load template '%1' (status %2).",
                                      "Vorlage '%1' konnte nicht geladen werden (Status %2).")
                                 .arg(chosen)
                                 .arg(int(stat)));
        return;
    }
    /*  The loaded file lives under its template path; reset filename
        so Save treats it as an unnamed document and prompts for a
        target. Flag modified because the template contents now exist
        without a backing file.                                     */
    filename.clear();
    SetModified(true);
    ResetUndoToCurrentState();
    SetAppTitle();
}

void TDBWFRM::FileOpen()
{
    if (!AskSave())
        return;
    const QString dir
        = filename.isEmpty() ? lastDirFor("Pattern") : QFileInfo((QString)filename).absolutePath();
    const QString chosen = QFileDialog::getOpenFileName(
        this, LANG_STR("Open pattern", "Muster öffnen"), dir, fileFilter());
    if (chosen.isEmpty())
        return;
    rememberDirFor("Pattern", chosen);

    /*  Close any file the loader from a previous Load left open. */
    if (file && file->IsOpen())
        file->Close();

    filename = chosen;
    LOADSTAT stat = UNKNOWN_FAILURE;
    const bool ok = Load(stat, LOADALL);
    if (!ok) {
        QMessageBox::warning(this, QStringLiteral("DB-WEAVE"),
                             LANG_STR("Could not open '%1' (status %2).",
                                      "'%1' konnte nicht geöffnet werden (Status %2).")
                                 .arg(chosen)
                                 .arg(int(stat)));
        return;
    }
    AddToMRU(chosen);
    SetModified(false);
    ResetUndoToCurrentState();
    refresh();
}

/*  Pick a pattern file and launch a separate DB-WEAVE process with
    it. The current document is left untouched -- no AskSave prompt,
    no state change in this window.                                 */
void TDBWFRM::FileOpenInNewWindow()
{
    const QString dir
        = filename.isEmpty() ? lastDirFor("Pattern") : QFileInfo((QString)filename).absolutePath();
    const QString chosen = QFileDialog::getOpenFileName(
        this, LANG_STR("Open pattern in new window", "Muster in neuem Fenster öffnen"), dir,
        fileFilter());
    if (chosen.isEmpty())
        return;
    rememberDirFor("Pattern", chosen);

    const QString exe = QCoreApplication::applicationFilePath();
    if (!QProcess::startDetached(exe, QStringList { chosen })) {
        QMessageBox::warning(
            this, QStringLiteral("DB-WEAVE"),
            LANG_STR("Could not start a new DB-WEAVE instance for '%1'.",
                     "Neue DB-WEAVE-Instanz für '%1' konnte nicht gestartet werden.")
                .arg(chosen));
    }
}

/*  FileRevertClick — reload the currently open file, discarding any
    unsaved changes. Legacy FileRevertClick does the same with no
    confirmation prompt; the menu action stays disabled when there is
    no filename or nothing has been modified, which is our gate here. */
void TDBWFRM::FileRevertClick()
{
    if (filename.isEmpty())
        return;
    if (!modified)
        return;
    if (file && file->IsOpen())
        file->Close();
    LOADSTAT stat = UNKNOWN_FAILURE;
    const bool ok = Load(stat, LOADALL);
    if (!ok) {
        QMessageBox::warning(this, QStringLiteral("DB-WEAVE"),
                             LANG_STR("Could not reload '%1' (status %2).",
                                      "'%1' konnte nicht neu geladen werden (Status %2).")
                                 .arg((QString)filename)
                                 .arg(int(stat)));
        return;
    }
    SetModified(false);
    ResetUndoToCurrentState();
    refresh();
}

void TDBWFRM::FileSaveAs()
{
    const QString dir = filename.isEmpty() ? lastDirFor("Pattern") : (QString)filename;
    QString chosen = QFileDialog::getSaveFileName(
        this, LANG_STR("Save pattern as", "Muster speichern unter"), dir, fileFilter());
    if (chosen.isEmpty())
        return;
    /*  Append the .dbw suffix if the user typed a bare name. The
        native "Save As" dialog on some platforms does not auto-
        append from the filter on its own; do it ourselves so the
        loader has something to recognise on the next Open.       */
    if (QFileInfo(chosen).suffix().isEmpty())
        chosen += QStringLiteral(".dbw");
    rememberDirFor("Pattern", chosen);

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
    /*  The just-saved state matches what's now on disk -- mark it as
        the new clean point so undoing back to it clears modified.   */
    if (undo)
        undo->MarkClean();
    SetAppTitle();
}

/*-----------------------------------------------------------------*/
/*  Port of legacy TDBWFRM::AskSave. Returns true when it's safe to
    discard the current document (no unsaved changes, user saved,
    or user explicitly discarded). Returns false when the user
    cancelled the prompt.

    Matches legacy semantics exactly: a modified but empty AND
    unnamed document (e.g. after a Grundeinstellung change on a
    fresh session) is treated as harmless -- no prompt.            */
bool TDBWFRM::AskSave()
{
    if (!modified)
        return true;
    /*  An unnamed document that hasn't accumulated any real content
        can be silently discarded. Real content = any used warp or
        weft range, any used shaft (einzug entry), or any used
        treadle.                                                    */
    if (filename.isEmpty()) {
        bool anyShaft = false;
        for (int j = 0; j < Data->MAXY1 && !anyShaft; j++)
            if (!IsFreeSchaft(j))
                anyShaft = true;
        const bool anyTritt = GetFirstTritt() <= GetLastTritt();
        const bool hasContent = kette.b != -1 || schuesse.b != -1 || anyShaft || anyTritt;
        if (!hasContent)
            return true;
    }
    const QString msg = LANG_STR("The pattern has unsaved changes. Save them now?",
                                 "Das Muster hat ungespeicherte Änderungen. Jetzt speichern?");
    QMessageBox::StandardButton choice = QMessageBox::question(
        this, QStringLiteral("DB-WEAVE"), msg,
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
    if (choice == QMessageBox::Cancel)
        return false;
    if (choice == QMessageBox::No)
        return true;
    /*  Yes: run the save path. FileSave delegates to FileSaveAs when
        no filename is set yet. If the user cancels the file-dialog
        we stay modified; report that as "not OK to close".          */
    FileSave();
    return !modified;
}

void TDBWFRM::closeEvent(QCloseEvent* _event)
{
    if (AskSave()) {
        SaveWindowState();
        _event->accept();
    } else {
        _event->ignore();
    }
}

/*-----------------------------------------------------------------*/
/*  Window geometry / dock layout persistence. QSettings under the
    "MainWindow" group captures:
      - geometry  Qt opaque blob (position + size + maximized
                  state + multi-screen offset).
      - state     Qt opaque blob (toolbar / QDockWidget layout for
                  palette / ranges / tools).
    Both keys are absent on a first-time run; LoadWindowState
    falls through to the caller-supplied default size.           */
bool TDBWFRM::LoadWindowState()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("MainWindow"));
    const QByteArray geom = settings.value(QStringLiteral("geometry")).toByteArray();
    const QByteArray state = settings.value(QStringLiteral("state")).toByteArray();
    settings.endGroup();
    bool any = false;
    if (!geom.isEmpty() && restoreGeometry(geom))
        any = true;
    if (!state.isEmpty() && restoreState(state))
        any = true;
    return any;
}

void TDBWFRM::SaveWindowState() const
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("MainWindow"));
    settings.setValue(QStringLiteral("geometry"), saveGeometry());
    settings.setValue(QStringLiteral("state"), saveState());
    settings.endGroup();
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
    if (!AskSave())
        return;
    const QString path = mru.at(_index);
    if (!QFileInfo::exists(path)) {
        QMessageBox::warning(
            this, QStringLiteral("DB-WEAVE"),
            LANG_STR("'%1' no longer exists.", "'%1' existiert nicht mehr.").arg(path));
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
        QMessageBox::warning(this, QStringLiteral("DB-WEAVE"),
                             LANG_STR("Could not open '%1' (status %2).",
                                      "'%1' konnte nicht geöffnet werden (Status %2).")
                                 .arg(path)
                                 .arg(int(stat)));
        return;
    }
    AddToMRU(path);
    SetModified(false);
    ResetUndoToCurrentState();
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
        = filename.isEmpty() ? lastDirFor("Pattern") : QFileInfo((QString)filename).absolutePath();
    const QString chosen = QFileDialog::getOpenFileName(
        this, LANG_STR("Load parts from", "Teile laden aus"), dir, fileFilter());
    if (chosen.isEmpty())
        return;
    rememberDirFor("Pattern", chosen);

    /*  Swap the working filename in so Load() reads the picked
        file, then restore ours afterwards so the document stays
        tied to its original path.                              */
    const QString savefn = filename;
    filename = chosen;
    if (file && file->IsOpen())
        file->Close();
    LOADSTAT stat = UNKNOWN_FAILURE;
    const bool ok = Load(stat, parts);
    filename = savefn;
    if (file && file->IsOpen())
        file->Close();

    if (!ok) {
        QMessageBox::warning(this, QStringLiteral("DB-WEAVE"),
                             LANG_STR("Could not load parts from '%1' (status %2).",
                                      "Teile aus '%1' konnten nicht geladen werden (Status %2).")
                                 .arg(chosen)
                                 .arg(int(stat)));
        return;
    }
    SetModified();
    refresh();
    if (undo)
        undo->Snapshot();
}
