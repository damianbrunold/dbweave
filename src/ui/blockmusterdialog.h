/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy blockmuster_form.cpp (TBlockmusterForm).
    Substitution-pattern editor: the user builds up a Grundmuster
    (base pattern) plus up to 9 numbered sub-patterns, each a 12x12
    small weave cartridge. On apply, every base-pattern cell of the
    main pattern's aufknuepfung is replaced by the corresponding
    numbered pattern at block size (mx+1, my+1).

    Keyboard cursor navigation from the legacy form is deferred in
    favour of the mouse-driven click-to-toggle flow + the menu
    actions (roll/mirror/invert/rotate/central-symmetry/undo/redo/
    copy-from/preset-bindungen). The dialog preserves the legacy
    BlockUndo ring semantics.                                      */

#ifndef DBWEAVE_UI_BLOCKMUSTER_DIALOG_H
#define DBWEAVE_UI_BLOCKMUSTER_DIALOG_H

#include <QDialog>
#include "blockmuster.h"

class QAction;
class QActionGroup;
class QLabel;
class TDBWFRM;
class BlockmusterCanvas;

class BlockmusterDialog : public QDialog
{
    Q_OBJECT
public:
    BlockmusterDialog(TDBWFRM* _frm, BlockUndo& _undo, PMUSTERARRAY _bindungen, int& _current,
                      const QString& _caption, bool _withRange);

    /*  Computed on Apply: bounding box of any filled cell across
        all 10 Musters. Used by the caller to size the block expand
        (legacy's mx+1 / my+1). */
    int mx = -1;
    int my = -1;

    /*  Einzug/Trittfolge direction toggles — the legacy form
        persisted these to TDBWFRM; we mirror the same state here
        so the caller can read them after Apply. */
    bool einzugZ = true;
    bool trittfolgeZ = true;

protected:
    void accept() override;

private:
    TDBWFRM* frm = nullptr;
    BlockUndo& undo;
    PMUSTERARRAY bindungen = nullptr;
    int& current;
    bool withRange;

    BlockmusterCanvas* canvas = nullptr;
    QLabel* description = nullptr;
    QLabel* usedLabel = nullptr;

    QAction* bindungActs[10] = {};
    QAction* einzugZAct = nullptr;
    QAction* einzugSAct = nullptr;
    QAction* trittfolgeZAct = nullptr;
    QAction* trittfolgeSAct = nullptr;
    QAction* undoAct = nullptr;
    QAction* redoAct = nullptr;

    void selectBindung(int _b);
    void refreshDescription();
    void refreshUsed();
    void calcRange();

    static char toggle(char _s, int _current);

    void editDelete();
    void editMirrorH();
    void editMirrorV();
    void editRotate();
    void editInvert();
    void editCentralsym();
    void rollUp();
    void rollDown();
    void rollLeft();
    void rollRight();

    void musterKoeper(int _h, int _s);
    void musterAtlas(int _n);
    void musterPanama(int _h, int _s);

    void grabFrom(int _from);

    void doUndo();
    void doRedo();

    friend class BlockmusterCanvas;
};

#endif
