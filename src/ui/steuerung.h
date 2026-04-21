/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy steuerung_form.cpp (TSTRGFRM) -- the secondary
    weaving window. Replaces the earlier bare-bones LoomDialog.

    Structurally this is a modal QDialog that hosts a QMenuBar,
    QToolBar, a SteuerungCanvas with a vertical QScrollBar beside
    it, and a status-bar-style footer. */

#ifndef DBWEAVE_UI_STEUERUNG_H
#define DBWEAVE_UI_STEUERUNG_H

#include <QDialog>
#include <cstdint>
#include <memory>

#include "dbw3_base.h"       /* Klammer, DARSTELLUNG */
#include "loom.h"            /* LOOMINTERFACE, StWeaveController */

class QAction;
class QActionGroup;
class QLabel;
class QMenu;
class QMenuBar;
class QScrollBar;
class QToolBar;
class QStatusBar;

class TDBWFRM;
class TData;
class FeldSchussfarben;
class FeldKettfarben;
class FeldEinzug;
class FeldGewebe;
class FeldAufknuepfung;
class FeldTrittfolge;

class SteuerungCanvas;

/*  Named after the legacy class for continuity with the German
    domain vocabulary (Steuerung = loom control).                */
class TSTRGFRM : public QDialog
{
    Q_OBJECT

public:
    static constexpr int MAXKLAMMERN = 9;

    explicit TSTRGFRM(TDBWFRM* _main, QWidget* _parent = nullptr);
    ~TSTRGFRM() override;

    /*  Copy klammer + weave-position state back to TDBWFRM. Called
        explicitly by LoomControlClick after exec() returns so the
        main window picks up the secondary window's final state. */
    void pushStateToMain();

    /*  Layout / position helpers ported from legacy
        steuerung_form.cpp + steuerung_pos.cpp.                    */
    void CalcSizes();
    void CalcTritte();
    void UpdateScrollbar();
    void AutoScroll();
    void UpdateStatusbar();
    void _ResetCurrentPos();
    bool IsValidWeavePosition() const;
    void ValidateWeavePosition();
    void WeaveKlammerRight();
    void WeaveKlammerLeft();
    void WeaveRepetitionInc();
    void WeaveRepetitionDec();
    void GotoKlammer(int _klammer);
    void GotoLastPosition();
    void UpdateLastPosition();

    /*  Called after state changes that may enable / disable
        the nine Goto-klammer actions. Replaces the legacy
        IdleHandler.                                              */
    void refreshGotoActions();

    /*  Full refresh: CalcSizes + canvas->update() +
        UpdateStatusbar + refreshGotoActions. Used as the "touch
        everything" entry point from handlers in later slices.   */
    void refresh();

    bool Weaving() const
    {
        return weaving;
    }
    void SetWeaving(bool _w = true)
    {
        weaving = _w;
    }

    bool Modified() const
    {
        return modified;
    }
    void SetModified()
    {
        modified = true;
    }

    /*  --- State mirrored from / into TDBWFRM ------------------ */
    Klammer klammern[MAXKLAMMERN];

    /*  Non-owning pointers into TDBWFRM's field containers. */
    TDBWFRM* frm = nullptr;
    TData* data = nullptr;
    FeldSchussfarben* schussfarben = nullptr;
    FeldKettfarben* kettfarben = nullptr;
    FeldEinzug* einzug = nullptr;
    FeldGewebe* gewebe = nullptr;
    FeldAufknuepfung* aufknuepfung = nullptr;
    FeldTrittfolge* trittfolge = nullptr;

    /*  Geometry / zoom. */
    int zoom[10] = { 5, 7, 9, 11, 13, 15, 17, 19, 21, 23 };
    int currentzoom = 4;
    int gridsize = 0;
    int tritte = 0;
    int rapportx = 0;
    int rapporty = 0;
    bool fewithraster = false;

    /*  Canvas layout. Populated by CalcSizes; consumed by the
        drawing and mouse routines. Units: viewport pixels in the
        canvas widget's local coordinates.                          */
    int maxi = 0;             /* visible gewebe columns            */
    int maxj = 0;             /* visible rows (schuesse)           */
    int trittCols = 0;        /* visible trittfolge cols (legacy tr -- renamed to avoid collision with QObject::tr) */
    int x1 = 0;               /* column where schlagpatrone starts */
    int klammerwidth = 0;     /* MAXKLAMMERN * 11 px               */
    int top = 0;              /* canvas-local drawable top         */
    int bottom = 0;           /* canvas-local drawable bottom-y    */
    int left = 0;              /* canvas-local drawable left        */
    int right = 0;             /* canvas-local drawable right       */
    int dx = 0;                /* visual gap between gewebe / SP    */


    /*  View state. */
    bool schlagpatrone = false;
    DARSTELLUNG schlagpatronendarstellung = AUSGEFUELLT;
    QString filename;

    /*  Weaving / position state. */
    int current_klammer = 0;
    int weave_position = 0;
    int weave_klammer = 0;
    int weave_repetition = 1;
    int last_position = -1;
    int last_klammer = -1;
    int last_repetition = -1;
    bool schussselected = true;
    int scrolly = 0;
    bool firstschuss = true;
    bool weaving = false;
    bool modified = false;

    /*  Loom options. Populated by LoadSettings. `port` is a plain
        int (1..8 mapping to COM1..COM8) so this header stays
        buildable without the loom-only loomsettings.h; the
        LoomOptionsDialog maps between enum and int at its edges. */
    int port = 1; /* 1 == COM1 */
    LOOMINTERFACE intrf = intrf_dummy;
    bool reverse = false;   /* ReverseSchaft -- schaft order flip */
    bool backwards = false; /* WeaveBackwards -- weave direction  */
    int numberOfShafts = 24;
    int slipsBytes = 4;
    bool loop = true;

public:
    /*  Widgets exposed to the canvas and sub-stages. */
    SteuerungCanvas* canvas = nullptr;
    QScrollBar* scrollbar = nullptr;
    QToolBar* toolbar = nullptr;
    QStatusBar* statusbar = nullptr;
    QMenuBar* menubar = nullptr;

    /*  QAction skeleton -- populated by the ctor, wired per
        sub-stage. All disabled in 7a.                          */
    QAction* actStart = nullptr;
    QAction* actStop = nullptr;
    QAction* actReverse = nullptr;
    QAction* actOptionsLoom = nullptr;
    QAction* actLoop = nullptr;
    QAction* actReverseSchaft = nullptr;
    QAction* actViewPatrone = nullptr;
    QAction* actViewFarbeffekt = nullptr;
    QAction* actViewGewebesimulation = nullptr;
    QAction* actZoomIn = nullptr;
    QAction* actZoomNormal = nullptr;
    QAction* actZoomOut = nullptr;
    QAction* actSetCurrentPos = nullptr;
    QAction* actGotoLastPos = nullptr;
    QAction* actGotoKlammer[MAXKLAMMERN] = {};
    QAction* actSbGoto[MAXKLAMMERN] = {};
    QAction* actSchafts[8] = {}; /* 4 / 8 / 12 / 16 / 20 / 24 / 28 / 32 */
    QAction* actBeenden = nullptr;

    /*  Status-bar labels (filled by UpdateStatusbar in 7b+).   */
    QLabel* labPosition = nullptr;
    QLabel* labKlammer = nullptr;
    QLabel* labRepetition = nullptr;

    /*  Drawing (7c). currentPainter is set by the canvas for the
        duration of paintEvent; the Draw* methods paint through
        it.                                                       */
    class QPainter* currentPainter = nullptr;

    /*  Mouse drag state (7d). Populated by FormMouseDown when the
        user presses inside a klammer column; consumed by
        FormMouseMove to resize / move the klammer.                */
    bool dragging = false;
    int drag_klammer = 0;
    int drag_j = 0;
    enum DragStyle { DRAG_TOP, DRAG_MIDDLE, DRAG_BOTTOM };
    DragStyle drag_style = DRAG_MIDDLE;

    void FormMouseDown(class QMouseEvent* _e);
    void FormMouseMove(class QMouseEvent* _e);
    void FormMouseUp(class QMouseEvent* _e);

    /*  Keyboard (7e). Arrow keys / PgUp / PgDown / Ctrl-variants
        navigate the current schuss / klammer depending on the
        schussselected flag. Enter toggles the flag; digits set the
        current klammer's repetitions.                             */
    void FormKeyDown(class QKeyEvent* _e);

    /*  Weaving loop (7f). */
    std::unique_ptr<StWeaveController> controller;
    bool stopit = true;
    bool tempquit = false;
    int maxweave = 0;

    void AllocInterface();
    int MaxSchaefte() const;
    void WeaveStartClick();
    void WeaveStopClick();
    void WeaveTempQuit();
    bool AtBegin() const;
    std::uint32_t GetSchaftDaten(int _pos) const;
    void Weben();
    void NextTritt();
    void PrevTritt();

    /*  Settings persistence (7h). QSettings group "Loom" matches
        legacy Settings category.                                   */
    void LoadSettings();
    void SaveSettings() const;

    /*  Popup menu (7e). Built once by buildPopupMenu; shown from
        SteuerungCanvas::contextMenuEvent.                        */
    class QMenu* popupMenu = nullptr;
    void showPopup(const QPoint& _globalPos);

    void paintAll();

    void DrawGrid();
    void DrawData();
    void DrawKlammern();
    void DrawKlammer(int _i);
    void DrawSelection();
    void DrawLastPos();

private:
    void _DrawKlammer(int _i);
    void _DrawPositionSelected();
    void _DrawKlammerSelected();
    void _DrawLastPos();
    void DrawGewebe(int _i, int _j);
    void DrawGewebeNormal(int _i, int _j, int _x, int _y, int _xx, int _yy);
    void DrawGewebeFarbeffekt(int _i, int _j, int _x, int _y, int _xx, int _yy);
    void DrawGewebeSimulation(int _i, int _j, int _x, int _y, int _xx, int _yy);

public:

private:
    void buildMenus();
    void buildToolbar();
    void buildCentralArea();
    void buildStatusbar();
    void buildPopupMenu();

    void pullStateFromMain();

protected:
    void showEvent(QShowEvent* _e) override;
    void closeEvent(class QCloseEvent* _e) override;
};

#endif
