/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef steuerung_formH
#define steuerung_formH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>
#include <ComCtrls.hpp>
#include "combase.h"
#include "strgoptloom_form.h"
#include "dbw3_base.h"
#include <Buttons.hpp>
#include <ToolWin.hpp>
#include "ComPort.hpp"

// Forward-Deklaration
class StWeaveController;

#define WM_SWITCHSHORTCUTS (WM_APP+400)
#define WM_AUTOSTARTWEAVE  (WM_APP+401)

const int MAXKLAMMERN = 9;

class TSTRGFRM : public TForm
{
__published:	// IDE-managed Components
    TMainMenu *MainMenu;
    TMenuItem *WebenBeenden;
    TMenuItem *MenuWeave;
    TMenuItem *WeaveStart;
    TMenuItem *WeaveStop;
    TMenuItem *N1;
    TMenuItem *MenuOptions;
    TMenuItem *OptionsLoom;
    TStatusBar *statusbar;
    TMenuItem *WeaveBackwards;
    TMenuItem *wb;
    TScrollBar *scrollbar;
    TToolBar *toolbar;
    TSpeedButton *SBStop;
    TToolButton *ToolButton1;
    TSpeedButton *SBSetCurrentPos;
    TSpeedButton *SBReverse;
    TToolButton *ToolButton2;
    TSpeedButton *SBOptions;
    TSpeedButton *SBStart;
    TMenuItem *MenuAnsicht;
    TMenuItem *ViewPatrone;
    TMenuItem *ViewFarbeffekt;
    TMenuItem *ViewGewebesimulation;
    TMenuItem *Loop;
    TMenuItem *SetCurrentPos;
    TMenuItem *GotoLastPos;
    TMenuItem *MenuPosition;
    TMenuItem *MenuGotoKlammer;
    TMenuItem *GotoKlammer1;
    TMenuItem *GotoKlammer2;
    TMenuItem *GotoKlammer3;
    TMenuItem *GotoKlammer4;
    TMenuItem *GotoKlammer5;
    TMenuItem *GotoKlammer6;
    TMenuItem *GotoKlammer7;
    TMenuItem *GotoKlammer8;
    TMenuItem *GotoKlammer9;
    TToolButton *ToolButton3;
    TSpeedButton *sbGoto1;
    TSpeedButton *sbGoto2;
    TSpeedButton *sbGoto3;
    TSpeedButton *sbGoto4;
    TSpeedButton *sbGoto5;
    TSpeedButton *sbGoto6;
    TSpeedButton *sbGoto7;
    TSpeedButton *sbGoto8;
    TSpeedButton *sbGoto9;
    TMenuItem *N3;
    TMenuItem *ZoomIn;
    TMenuItem *ZoomNormal;
    TMenuItem *ZoomOut;
    TMenuItem *ReverseSchaft;
    TPopupMenu *PopupMenu;
    TMenuItem *Cancel;
    TMenuItem *N4;
    TMenuItem *StartWeben;
    TMenuItem *StopWeben;
    TMenuItem *N5;
    TMenuItem *ReverseWeben;
    TMenuItem *GotoLast;
    TMenuItem *GotoMenu;
    TMenuItem *GotoKlammer1Popup;
    TMenuItem *GotoKlammer2Popup;
    TMenuItem *GotoKlammer3Popup;
    TMenuItem *GotoKlammer4Popup;
    TMenuItem *GotoKlammer5Popup;
    TMenuItem *GotoKlammer6Popup;
    TMenuItem *GotoKlammer7Popup;
    TMenuItem *GotoKlammer8Popup;
    TMenuItem *GotoKlammer9Popup;
    TMenuItem *N6;
    TMenuItem *ViewMenu;
    TMenuItem *ViewPatronePopup;
    TMenuItem *ViewFarbeffektPopup;
    TMenuItem *ViewGewebesimulationPopup;
    TMenuItem *N7;
    TMenuItem *ZoomInPopup;
    TMenuItem *NormalPopup;
    TMenuItem *ZoomOutPopup;
    TMenuItem *N8;
    TMenuItem *MenuSchaefte;
    TMenuItem *Schafts8;
    TMenuItem *Schafts12;
    TMenuItem *Schafts16;
    TMenuItem *Schafts24;
    TMenuItem *Schafts32;
    TMenuItem *Schafts4;
    TMenuItem *Schafts20;
    TMenuItem *Schafts28;
    TComPort *comport;
    void __fastcall WebenBeendenClick(TObject *Sender);
    void __fastcall WeaveStartClick(TObject *Sender);
    void __fastcall WeaveStopClick(TObject *Sender);
    void __fastcall OptionsLoomClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall FormResize(TObject *Sender);
    void __fastcall FormKeyPress(TObject *Sender, char &Key);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall WeaveBackwardsClick(TObject *Sender);
    void __fastcall scrollbarScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos);
    void __fastcall SBStartClick(TObject *Sender);
    void __fastcall SBStopClick(TObject *Sender);
    void __fastcall SBReverseClick(TObject *Sender);
    void __fastcall SBOptionsClick(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall FormMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
    void __fastcall ViewPatroneClick(TObject *Sender);
    void __fastcall ViewFarbeffektClick(TObject *Sender);
    void __fastcall ViewGewebesimulationClick(TObject *Sender);
    void __fastcall LoopClick(TObject *Sender);
    void __fastcall SetCurrentPosClick(TObject *Sender);
    void __fastcall SBSetCurrentPosClick(TObject *Sender);
    void __fastcall GotoLastPosClick(TObject *Sender);
    void __fastcall GotoKlammer1Click(TObject *Sender);
    void __fastcall GotoKlammer2Click(TObject *Sender);
    void __fastcall GotoKlammer3Click(TObject *Sender);
    void __fastcall GotoKlammer4Click(TObject *Sender);
    void __fastcall GotoKlammer5Click(TObject *Sender);
    void __fastcall GotoKlammer6Click(TObject *Sender);
    void __fastcall GotoKlammer7Click(TObject *Sender);
    void __fastcall GotoKlammer8Click(TObject *Sender);
    void __fastcall GotoKlammer9Click(TObject *Sender);
    void __fastcall sbGoto1Click(TObject *Sender);
    void __fastcall sbGoto2Click(TObject *Sender);
    void __fastcall sbGoto3Click(TObject *Sender);
    void __fastcall sbGoto4Click(TObject *Sender);
    void __fastcall sbGoto5Click(TObject *Sender);
    void __fastcall sbGoto6Click(TObject *Sender);
    void __fastcall sbGoto7Click(TObject *Sender);
    void __fastcall sbGoto8Click(TObject *Sender);
    void __fastcall sbGoto9Click(TObject *Sender);
    void __fastcall ZoomInClick(TObject *Sender);
    void __fastcall ZoomNormalClick(TObject *Sender);
    void __fastcall ZoomOutClick(TObject *Sender);
    void __fastcall ReverseSchaftClick(TObject *Sender);
    void __fastcall CancelClick(TObject *Sender);
    void __fastcall StartWebenClick(TObject *Sender);
    void __fastcall StopWebenClick(TObject *Sender);
    void __fastcall ReverseWebenClick(TObject *Sender);
    void __fastcall GotoLastClick(TObject *Sender);
    void __fastcall GotoKlammer1PopupClick(TObject *Sender);
    void __fastcall GotoKlammer2PopupClick(TObject *Sender);
    void __fastcall GotoKlammer3PopupClick(TObject *Sender);
    void __fastcall GotoKlammer4PopupClick(TObject *Sender);
    void __fastcall GotoKlammer5PopupClick(TObject *Sender);
    void __fastcall GotoKlammer6PopupClick(TObject *Sender);
    void __fastcall GotoKlammer7PopupClick(TObject *Sender);
    void __fastcall GotoKlammer8PopupClick(TObject *Sender);
    void __fastcall GotoKlammer9PopupClick(TObject *Sender);
    void __fastcall ViewPatronePopupClick(TObject *Sender);
    void __fastcall ViewFarbeffektPopupClick(TObject *Sender);
    void __fastcall ViewGewebesimulationPopupClick(TObject *Sender);
    void __fastcall ZoomInPopupClick(TObject *Sender);
    void __fastcall NormalPopupClick(TObject *Sender);
    void __fastcall ZoomOutPopupClick(TObject *Sender);
    void __fastcall Schafts8Click(TObject *Sender);
    void __fastcall Schafts12Click(TObject *Sender);
    void __fastcall Schafts16Click(TObject *Sender);
    void __fastcall Schafts24Click(TObject *Sender);
    void __fastcall Schafts32Click(TObject *Sender);
    void __fastcall Schafts4Click(TObject *Sender);
    void __fastcall Schafts20Click(TObject *Sender);
    void __fastcall Schafts28Click(TObject *Sender);
protected:
    void __fastcall WMSwitchShortcuts(TMessage& _message);
    void __fastcall WMAutoStartWeave(TMessage& _message);
BEGIN_MESSAGE_MAP
    MESSAGE_HANDLER(WM_SWITCHSHORTCUTS, TMessage, WMSwitchShortcuts)
    MESSAGE_HANDLER(WM_AUTOSTARTWEAVE, TMessage, WMAutoStartWeave)
END_MESSAGE_MAP(TForm)

private:	// User declarations
    StWeaveController* controller;
    bool stopit;
    bool tempquit;
    int maxweave;

    // Einstellungen
    PORT port;  // für Seriell
    LPT  lpt;   // für Parallel
    int delay;
    LOOMINTERFACE intrf;
    bool reverse;  // Schäfte von "hinten" her verteilen

public:
    // Status
    bool weaving;
    bool modified;
    int current_klammer;
    int weave_position;
    int weave_klammer;
    int weave_repetition;
    int last_position;
    int last_klammer;
    int last_repetition;
    bool schussselected;
    int scrolly;
    bool firstschuss;

private:
    // Draginfos
    bool dragging;
    int drag_klammer;
    int drag_j;
    enum { DRAG_TOP, DRAG_MIDDLE, DRAG_BOTTOM } drag_style;

    // Verfügbarer Bereich
    int top, left, bottom, right;

    // Ausmasse
    int height;
    int dx;
    int klammerwidth;
    int maxi;
    int maxj;
    int tr;
    int x1;


public:
    Klammer klammern[MAXKLAMMERN];
    int zoom[10];
    int currentzoom;

public:
    // Daten
    int gridsize;
    int tritte;
    bool schlagpatrone;
    DARSTELLUNG schlagpatronendarstellung;
    int rapportx;
    int rapporty;
    FeldSchussfarben* schussfarben;
    FeldKettfarben*   kettfarben;
    FeldEinzug*       einzug;
    FeldGewebe*       gewebe;
    FeldAufknuepfung* aufknuepfung;
    FeldTrittfolge*   trittfolge;
    bool fewithraster;
    String filename;

public:		// User declarations
    __fastcall TSTRGFRM(TComponent* Owner);
    virtual __fastcall ~TSTRGFRM();

private:
    void __fastcall WeaveTempQuit();
    void __fastcall Weben();
    DWORD __fastcall GetSchaftDaten(int _pos);
    void __fastcall LoadSettings();
    void __fastcall SaveSettings();
    void __fastcall CalcSizes();
    void __fastcall CalcTritte();
    void __fastcall UpdateStatusbar();
    void __fastcall DrawGrid();
    void __fastcall DrawData();
    void __fastcall DrawKlammern();
    void __fastcall DrawKlammer (int i);
    void __fastcall DrawSelection();
    void __fastcall ClearSelection();
    void __fastcall DrawPositionSelection();
    void __fastcall ClearPositionSelection();
    void __fastcall DrawLastPos();
    void __fastcall ClearLastPos();
    void __fastcall DrawKlammerSelection();
    void __fastcall ClearKlammerSelection();
    void __fastcall NextTritt();
    void __fastcall PrevTritt();
    void __fastcall AutoScroll();
    bool __fastcall AtBegin();
    void __fastcall AllocInterface();

    void __fastcall WeaveKlammerRight();
    void __fastcall WeaveKlammerLeft();
    void __fastcall WeaveRepetitionInc();
    void __fastcall WeaveRepetitionDec();
    bool __fastcall IsValidWeavePosition();
    void __fastcall GotoLastPosition();
    void __fastcall UpdateLastPosition();
    void __fastcall GotoKlammer (int _klammer);

    void __fastcall IdleHandler (TObject* Sender, bool& Done);
    void __fastcall DisplayHint (TObject* Sender);

    void __fastcall HandleKlammerKeyDown (WORD& Key, TShiftState Shift);
    void __fastcall HandleSchussKeyDown (WORD& Key, TShiftState Shift);

    void __fastcall UpdateScrollbar();

    int __fastcall MaxSchaefte();

public:
    void __fastcall ValidateWeavePosition();
    void __fastcall _ResetCurrentPos();
    bool __fastcall Weaving();
    void __fastcall SetWeaving(bool _weaving=true);
    void __fastcall SetModified();
    bool __fastcall Modified();

    void __fastcall ReloadLanguage();

private:
    void __fastcall _DrawSelection();
    void __fastcall _DrawPositionSelected();
    void __fastcall _DrawLastPos();
    void __fastcall _DrawKlammerSelected();
    void __fastcall _DrawKlammer(int i);

    void __fastcall DrawGewebe (int _i, int _j);
    void __fastcall DrawGewebeNormal (int _i, int _j, int _x, int _y, int _xx, int _yy);
    void __fastcall DrawGewebeFarbeffekt (int _i, int _j, int _x, int _y, int _xx, int _yy);
    void __fastcall DrawGewebeSimulation (int _i, int _j, int _x, int _y, int _xx, int _yy);
};

extern PACKAGE TSTRGFRM *STRGFRM;

#endif

