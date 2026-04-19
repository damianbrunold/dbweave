/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Right-click context menu. Legacy mounted two VCL TPopupMenus
    (one for the document area, one for the View toggles) and
    updated their entries' Visible state in UpdatePopupMenu1. The
    Qt port builds the menu on demand via QMenu::exec so the
    enabled/disabled state reflects the live selection without an
    explicit Update step.                                        */

#include "mainwindow.h"
#include "language.h"

#include <QAction>
#include <QMenu>

void TDBWFRM::handleContextMenu(const QPoint& _globalPos)
{
    QMenu menu(this);

    RANGE sel = selection;
    sel.Normalize();
    const bool haveSelection = sel.Valid();

    if (haveSelection) {
        /*  Edit-style popup: mirrors the Edit menu, gated on what
            makes sense for the current selection. Rotate is
            disabled unless the selection is square; Central
            symmetry needs GEWEBE / AUFKNUEPFUNG or (TRITTFOLGE and
            not single-treadle mode).                              */
        const bool square = (sel.end.i - sel.begin.i) == (sel.end.j - sel.begin.j);
        const bool rotateOK = square
                              && (sel.feld == GEWEBE || sel.feld == AUFKNUEPFUNG
                                  || (sel.feld == TRITTFOLGE && ViewSchlagpatrone
                                      && ViewSchlagpatrone->isChecked()));
        const bool singleTritt
            = trittfolge.einzeltritt && !(ViewSchlagpatrone && ViewSchlagpatrone->isChecked());
        const bool centralOK = sel.feld == GEWEBE || sel.feld == AUFKNUEPFUNG
                               || (sel.feld == TRITTFOLGE && !singleTritt);

        QAction* aCut = menu.addAction(LANG_STR("Cu&t", "A&usschneiden"));
        QAction* aCopy = menu.addAction(LANG_STR("&Copy", "&Kopieren"));
        QAction* aPaste = menu.addAction(LANG_STR("&Paste", "&Einfügen"));
        QAction* aPasteT = menu.addAction(
            LANG_STR("Paste T&ransparent", "&Transparent einfügen"));
        QAction* aDel = menu.addAction(LANG_STR("&Delete", "&Löschen"));
        menu.addSeparator();
        QAction* aInv = menu.addAction(LANG_STR("&Invert", "&Invertieren"));
        QAction* aMH = menu.addAction(LANG_STR("Mirror &Horizontal", "Spiegeln &horizontal"));
        QAction* aMV = menu.addAction(LANG_STR("Mirror &Vertical", "Spiegeln &vertikal"));
        QAction* aRot = menu.addAction(LANG_STR("R&otate 90\xc2\xb0", "R&otieren 90\xc2\xb0"));
        QAction* aCent = menu.addAction(
            LANG_STR("Central &symmetry", "Zentral&symmetrisch"));
        aRot->setEnabled(rotateOK);
        aCent->setEnabled(centralOK);
        menu.addSeparator();
        QAction* aRU = menu.addAction(LANG_STR("Roll &Up", "Nach &oben rollen"));
        QAction* aRD = menu.addAction(LANG_STR("Roll D&own", "Nach u&nten rollen"));
        QAction* aRL = menu.addAction(LANG_STR("Roll &Left", "Nach &links rollen"));
        QAction* aRR = menu.addAction(LANG_STR("Roll R&ight", "Nach &rechts rollen"));

        connect(aCut, &QAction::triggered, this, [this] { CutSelection(); });
        connect(aCopy, &QAction::triggered, this, [this] { CopySelection(); });
        connect(aPaste, &QAction::triggered, this, [this] { PasteSelection(false); });
        connect(aPasteT, &QAction::triggered, this, [this] { PasteSelection(true); });
        connect(aDel, &QAction::triggered, this, [this] { DeleteSelection(); });
        connect(aInv, &QAction::triggered, this, [this] { InvertSelection(); });
        connect(aMH, &QAction::triggered, this, [this] { MirrorHorzSelection(); });
        connect(aMV, &QAction::triggered, this, [this] { MirrorVertSelection(); });
        connect(aRot, &QAction::triggered, this, [this] { RotateSelection(); });
        connect(aCent, &QAction::triggered, this, [this] { CentralsymSelection(); });
        connect(aRU, &QAction::triggered, this, [this] { RollUpSelection(); });
        connect(aRD, &QAction::triggered, this, [this] { RollDownSelection(); });
        connect(aRL, &QAction::triggered, this, [this] { RollLeftSelection(); });
        connect(aRR, &QAction::triggered, this, [this] { RollRightSelection(); });
    } else {
        /*  View-toggle popup. Reuses the existing checkable QActions
            that live on TDBWFRM so toggling via the popup stays in
            sync with the View menu and the canvas refresh.         */
        if (ViewEinzug)
            menu.addAction(ViewEinzug);
        if (ViewTrittfolge)
            menu.addAction(ViewTrittfolge);
        if (ViewSchlagpatrone)
            menu.addAction(ViewSchlagpatrone);
        if (ViewBlatteinzug)
            menu.addAction(ViewBlatteinzug);
        if (ViewFarbe)
            menu.addAction(ViewFarbe);
        menu.addSeparator();
        if (RappViewRapport)
            menu.addAction(RappViewRapport);
        if (ViewHlines)
            menu.addAction(ViewHlines);
        if (ViewFarbpalette)
            menu.addAction(ViewFarbpalette);
        menu.addSeparator();
        QMenu* sub = menu.addMenu(LANG_STR("&Cloth display", "&Gewebeansicht"));
        if (GewebeNormal)
            sub->addAction(GewebeNormal);
        if (GewebeFarbeffekt)
            sub->addAction(GewebeFarbeffekt);
        if (GewebeSimulation)
            sub->addAction(GewebeSimulation);
        if (GewebeNone)
            sub->addAction(GewebeNone);
        menu.addSeparator();
        QAction* aPaste = menu.addAction(LANG_STR("&Paste", "&Einfügen"));
        QAction* aPasteT = menu.addAction(
            LANG_STR("Paste T&ransparent", "&Transparent einfügen"));
        connect(aPaste, &QAction::triggered, this, [this] { PasteSelection(false); });
        connect(aPasteT, &QAction::triggered, this, [this] { PasteSelection(true); });
    }

    menu.exec(_globalPos);
}
