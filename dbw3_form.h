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

#ifndef dbw3_formH
#define dbw3_formH

#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Menus.hpp>
#include <vcl\ComCtrls.hpp>
#include <vcl\Dialogs.hpp>
#include <vcl\messages.hpp>
#include <vcl\ExtCtrls.hpp>
#include <Buttons.hpp>
#include <ToolWin.hpp>
#include <objbase.h>
#include "enums.h"
#include "dbw3_base.h"
#include "print_base.h"
#include "blockmuster.h"
#include "hilfslinien.h"
#include "fileload.h"
#include "tools.h"
#include "userdef.h"
#include "assert.h"

// Forward-Deklarationen
class FfWriter;
class FfReader;
class FfFile;
class StWeaveController;
class UrUndo;
class BlockUndo;
class CrCursorHandler;
class RpRapport;
class EinzugRearrange;

class TDBWFRM : public TForm
{
__published:    // IDE-managed Components
    TMainMenu *DBWMENU;
    TMenuItem *MenuAnsicht;
    TMenuItem *ViewBlatteinzug;
    TMenuItem *ViewFarbe;
    TMenuItem *ViewEinzug;
    TMenuItem *ViewTrittfolge;
    TStatusBar *Statusbar;
    TPopupMenu *Popupmenu;
    TMenuItem *ViewBlatteinzugPopup;
    TMenuItem *ViewFarbePopup;
    TMenuItem *ViewEinzugPopup;
    TMenuItem *ViewTrittfolgePopup;
    TMenuItem *N1;
    TMenuItem *N2;
    TMenuItem *GewebeFarbeffekt;
    TMenuItem *N4;
    TMenuItem *MenuExtras;
    TMenuItem *ViewSchlagpatrone;
    TScrollBar *sb_horz1;
    TScrollBar *sb_horz2;
    TScrollBar *sb_vert1;
    TScrollBar *sb_vert2;
    TMenuItem *GewebeSimulation;
    TMenuItem *GewebeNormal;
    TMenuItem *Gewebe;
    TMenuItem *N7;
    TMenuItem *ViewZoomIn;
    TMenuItem *ViewZoomOut;
    TMenuItem *ViewZoomInPopup;
    TMenuItem *ViewZoomOutPopup;
    TMenuItem *MenuDatei;
    TMenuItem *FileOpen;
    TMenuItem *FileSave;
    TMenuItem *FileSaveas;
    TMenuItem *FileNew;
    TMenuItem *FileMRUSeparator;
    TMenuItem *FileExit;
    TMenuItem *MenuHelp;
    TMenuItem *HelpAbout;
    TMenuItem *HelpBsoftOnline;
    TMenuItem *N3;
    TMenuItem *EinstellVerhaeltnis;
    TMenuItem *FilePrint;
    TMenuItem *FilePrintSetup;
    TMenuItem *N8;
    TMenuItem *N10;
    TMenuItem *ViewRedraw;
    TMenuItem *FileMRU1;
    TMenuItem *N11;
    TMenuItem *FileMRU2;
    TMenuItem *FileMRU3;
    TMenuItem *FileMRU6;
    TMenuItem *OptionsLockGewebe;
    TMenuItem *MenuRapport;
	TMenuItem *RappRapportieren;
    TMenuItem *ViewFarbpalette;
    TMenuItem *FileMRU5;
    TMenuItem *FileMRU4;
    TMenuItem *RappViewRapport;
    TMenuItem *N5;
    TMenuItem *MenuEinzug;
    TMenuItem *MenuTrittfolge;
    TMenuItem *EzChorig2;
    TMenuItem *EzFixiert;
    TMenuItem *TfBelassen;
    TMenuItem *N9;
    TMenuItem *N12;
    TMenuItem *TfGeradeZ;
    TMenuItem *TfGeradeS;
    TMenuItem *TfGesprungen;
    TMenuItem *EzGeradeZ;
    TMenuItem *EzGeradeS;
    TMenuItem *EzMinimalZ;
    TMenuItem *EzMinimalS;
    TMenuItem *MenuBearbeiten;
    TMenuItem *EditUndo;
    TMenuItem *EditRedo;
    TMenuItem *N13;
    TMenuItem *EditCut;
    TMenuItem *EditCopy;
    TMenuItem *EditPaste;
    TMenuItem *EditPasteTransparent;
    TMenuItem *N14;
    TMenuItem *EditInvert;
    TMenuItem *EditMirrorHorz;
    TMenuItem *EditMirrorVert;
    TMenuItem *EditRotate;
    TMenuItem *EditDelete;
    TMenuItem *GewebeNone;
    TMenuItem *FileProps;
    TMenuItem *N15;
    TMenuItem *Weave;
	TMenuItem *RappReduzieren;
	TMenuItem *FileSetPage;
    TMenuItem *RappOverride;
	TMenuItem *TfMinimalZ;
	TMenuItem *TfMinimalS;
	TTimer *CursorTimer;
    TMenuItem *PopupCancel;
    TMenuItem *N16;
    TToolBar *Toolbar;
    TSpeedButton *SBFileNew;
    TSpeedButton *SBFileOpen;
    TSpeedButton *SBFilePrint;
    TSpeedButton *SBPreview;
    TSpeedButton *SBEditCut;
    TSpeedButton *SBEditCopy;
    TSpeedButton *SBEditPaste;
    TToolButton *ToolButton1;
    TToolButton *ToolButton2;
    TMenuItem *MenuFarbe;
    TMenuItem *DefineColors;
    TSpeedButton *SBSave;
    TMenuItem *FilePrintpreview;
    TMenuItem *FileRevert;
    TToolButton *ToolButton3;
    TSpeedButton *SBZoomIn;
    TSpeedButton *SBZoomOut;
    TMenuItem *FileNewTemplate;
    TMenuItem *EditFillKoeper;
    TMenuItem *ViewOnlyGewebe;
    TMenuItem *ViewZoomNormal;
    TMenuItem *OptPrintSelection;
    TToolButton *ToolSeparator;
    TSpeedButton *SBMoveUp;
    TSpeedButton *SBMoveDown;
    TSpeedButton *SBMoveLeft;
    TSpeedButton *SBMoveRight;
    TMenuItem *N18;
    TMenuItem *SchaftMoveUp;
    TMenuItem *SchaftMoveDown;
    TMenuItem *TrittMoveLeft;
    TMenuItem *TrittMoveRight;
    TMenuItem *MenuMove;
    TMenuItem *MenuInsert;
    TMenuItem *MenuDelete;
    TMenuItem *InsertSchaft;
    TMenuItem *InsertTritt;
    TMenuItem *InsertKette;
    TMenuItem *InsertSchuss;
    TMenuItem *DeleteSchaft;
    TMenuItem *DeleteTritt;
    TMenuItem *DeleteKette;
    TMenuItem *DeleteSchuss;
    TSpeedButton *SBUndo;
    TToolButton *ToolButton5;
    TSpeedButton *SBRedo;
    TMenuItem *CopyEinzugTrittfolge;
    TMenuItem *CopyTrittfolgeEinzug;
    TMenuItem *N19;
    TMenuItem *EditCursordirection;
    TMenuItem *N21;
    TMenuItem *ViewHlines;
    TMenuItem *MenuRoll;
    TMenuItem *RollUp;
    TMenuItem *RollDown;
    TMenuItem *RollLeft;
    TMenuItem *RollRight;
    TMenuItem *CursorLocked;
    TMenuItem *MenuBereich;
    TMenuItem *Overview;
    TMenuItem *N6;
    TMenuItem *KettfarbenWieSchussfarben;
    TMenuItem *SchussfarbenWieKettfarben;
    TMenuItem *AktuellerBereich1;
    TMenuItem *Range1;
    TMenuItem *Range2;
    TMenuItem *Range3;
    TMenuItem *Range4;
    TMenuItem *Range5;
    TMenuItem *Range6;
    TMenuItem *Range7;
    TMenuItem *Range8;
    TMenuItem *Range9;
    TMenuItem *ImportBitmap;
    TMenuItem *EditBlockmuster;
    TMenuItem *RangePatterns;
    TMenuItem *ViewToolpalette;
    TMenuItem *ViewInfos;
    TMenuItem *GewebeNonePopup;
    TMenuItem *GewebeNormalPopup;
    TMenuItem *GewebeFarbeffektPopup;
    TMenuItem *GewebeSimulationPopup;
    TMenuItem *DateiExport;
    TMenuItem *N17;
    TMenuItem *EditSwapside;
    TMenuItem *Farbverlauf;
    TMenuItem *EinzugAssistent;
    TMenuItem *N22;
    TMenuItem *ClearEinzug;
    TMenuItem *EzBelassen;
    TMenuItem *MenuSchlagpatrone;
    TMenuItem *ClearTrittfolge;
    TMenuItem *ClearSchlagpatrone;
    TMenuItem *SpSpiegeln;
    TMenuItem *TfSpiegeln;
    TMenuItem *EzSpiegeln;
    TMenuItem *ClearAufknuepfung;
    TMenuItem *MenuAufknuepfung;
    TMenuItem *AufInvert;
    TMenuItem *AufRollen;
    TMenuItem *AufRollUp;
    TMenuItem *AufRollDown;
    TMenuItem *AufRollLeft;
    TMenuItem *AufRollRight;
    TMenuItem *AufZentralsymm;
    TMenuItem *SpInvert;
    TMenuItem *EditCentralsym;
    TMenuItem *EzChorig3;
    TMenuItem *N23;
    TMenuItem *SetKettfarbe;
    TMenuItem *SetSchussfarbe;
    TMenuItem *MenuEinfuegen;
    TMenuItem *Koeper2_2;
    TMenuItem *Koeper2_1;
    TMenuItem *Koeper;
    TMenuItem *Atlas;
    TMenuItem *Atlas5;
    TMenuItem *Atlas7;
    TMenuItem *Atlas8;
    TMenuItem *Atlas9;
    TMenuItem *Atlas10;
    TMenuItem *Atlas6;
    TMenuItem *gleichseitig;
    TMenuItem *Kettkoeper;
    TMenuItem *Koeper3_3;
    TMenuItem *Koeper4_4;
    TMenuItem *Koeper5_5;
    TMenuItem *Koeper3_1;
    TMenuItem *Koeper4_1;
    TMenuItem *Koeper5_1;
    TMenuItem *Koeper3_2;
    TMenuItem *Koeper4_2;
    TMenuItem *Koeper5_2;
    TMenuItem *Koeper4_3;
    TMenuItem *Koeper5_3;
    TSpeedButton *SBInvert;
    TSpeedButton *SBMirrorH;
    TSpeedButton *SBMirrorV;
    TSpeedButton *SBRotate;
    TSpeedButton *SBCentralsym;
    TPopupMenu *PopupMenu1;
    TMenuItem *Popup1Cancel;
    TMenuItem *N24;
    TMenuItem *Popup1Copy;
    TMenuItem *Popup1Cut;
    TMenuItem *N25;
    TMenuItem *Popup1Invert;
    TMenuItem *Popup1MirrorH;
    TMenuItem *Popup1MirrorV;
    TMenuItem *Popup1Rotate;
    TMenuItem *Popup1Centralsym;
    TMenuItem *Popup1Delete;
    TMenuItem *N26;
    TMenuItem *Popup1Rollen;
    TMenuItem *Popup1RollUp;
    TMenuItem *Popup1RollDown;
    TMenuItem *Popup1RollLeft;
    TMenuItem *Popup1RollRight;
    TMenuItem *CursorGoto;
    TMenuItem *CursorMenu;
    TMenuItem *LoadParts;
    TMenuItem *N27;
    TMenuItem *Steigung;
    TMenuItem *SteigungInc;
    TMenuItem *SteigungDec;
    TMenuItem *AufSteigung;
    TMenuItem *AufSteigungInc;
    TMenuItem *AufSteigungDec;
    TMenuItem *Basestyle;
    TMenuItem *OptSwiss;
    TMenuItem *OptSkandinavisch;
    TMenuItem *OptAmerican;
    TMenuItem *XOptions;
    TMenuItem *ReplaceColor;
    TSpeedButton *SBHighlight;
    TMenuItem *SwitchColors;
    TMenuItem *MenuOptions;
    TMenuItem *XOptionsGlobal;
    TMenuItem *N20;
    TMenuItem *MenuLancierung;
    TMenuItem *KettLancierung;
    TMenuItem *SchussLancierung;
    TMenuItem *MenuWeitere;
    TMenuItem *Userdef1;
    TMenuItem *Userdef2;
    TMenuItem *Userdef3;
    TMenuItem *Userdef4;
    TMenuItem *Userdef5;
    TMenuItem *Userdef6;
    TMenuItem *Userdef7;
    TMenuItem *Userdef8;
    TMenuItem *Userdef9;
    TMenuItem *Userdef10;
    TMenuItem *N28;
    TMenuItem *AdminUserdef;
    TMenuItem *UserdefAdd;
    TMenuItem *UserdefRemove;
    TMenuItem *UserdefAddSel;
    TMenuItem *N29;
    TMenuItem *RangeAushebung;
    TMenuItem *RangeAnbindung;
    TMenuItem *RangeAbbindung;
    TMenuItem *Inverserepeat;
    TMenuItem *PasteDividerPopup;
    TMenuItem *PastePopup;
    TMenuItem *PasteTransparentlyPopup;
    TMenuItem *Paste1Popup;
    TMenuItem *Paste1TransparentlyPopup;
    TMenuItem *N30;
    TMenuItem *OptEnvironment;
    TTimer *Splashtimer;
    TMenuItem *ViewmodePopup;
    TMenuItem *HelpTechinfo;
    TPopupMenu *PopupRange;
    TMenuItem *PopupRangeCancel;
    TMenuItem *N31;
    TMenuItem *PopupRange1;
    TMenuItem *PopupRange2;
    TMenuItem *PopupRange3;
    TMenuItem *PopupRange4;
    TMenuItem *PopupRange5;
    TMenuItem *PopupRange6;
    TMenuItem *PopupRange7;
    TMenuItem *PopupRange8;
    TMenuItem *PopupRange9;
    TMenuItem *N32;
    TMenuItem *PopupRangeLiftout;
    TMenuItem *PopupRangeBinding;
    TMenuItem *PopupRangeUnbinding;
    TMenuItem *ExportBitmap;
    TMenuItem *ExportWIF;
    TMenuItem *ExportDBW35;
	TMenuItem *DateiImport;
	TMenuItem *ImportWIF;
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall ViewBlatteinzugClick(TObject *Sender);
    void __fastcall ViewFarbeClick(TObject *Sender);
    void __fastcall ViewEinzugClick(TObject *Sender);
    void __fastcall ViewTrittfolgeClick(TObject *Sender);
    void __fastcall ViewBlatteinzugPopupClick(TObject *Sender);
    void __fastcall ViewFarbePopupClick(TObject *Sender);
    void __fastcall ViewEinzugPopupClick(TObject *Sender);
    void __fastcall FormResize(TObject *Sender);
    void __fastcall FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall FormMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
    void __fastcall FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall GewebeFarbeffektClick(TObject *Sender);
    void __fastcall ViewSchlagpatroneClick(TObject *Sender);
    void __fastcall ViewTrittfolgePopupClick(TObject *Sender);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall GewebeNormalPopupClick(TObject *Sender);
    void __fastcall GewebeFarbeffektPopupClick(TObject *Sender);
    void __fastcall GewebeSimulationPopupClick(TObject *Sender);
    void __fastcall GewebeNormalClick(TObject *Sender);
    void __fastcall GewebeSimulationClick(TObject *Sender);
    void __fastcall ViewZoomInPopupClick(TObject *Sender);
    void __fastcall ViewZoomOutPopupClick(TObject *Sender);
    void __fastcall ViewZoomInClick(TObject *Sender);
    void __fastcall ViewZoomOutClick(TObject *Sender);    
    void __fastcall FileOpenClick(TObject *Sender);
    void __fastcall FileSaveClick(TObject *Sender);
    void __fastcall FileSaveasClick(TObject *Sender);
    void __fastcall FileExitClick(TObject *Sender);
    void __fastcall FileNewClick(TObject *Sender);
    void __fastcall HelpAboutClick(TObject *Sender);
    void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
    void __fastcall HelpBsoftOnlineClick(TObject *Sender);
    void __fastcall EinstellVerhaeltnisClick(TObject *Sender);
    void __fastcall FilePrintClick(TObject *Sender);
    void __fastcall FilePrintSetupClick(TObject *Sender);
    void __fastcall ViewRedrawClick(TObject *Sender);
    void __fastcall sb_vert1Scroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos);
    void __fastcall sb_vert2Scroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos);
    void __fastcall sb_horz1Scroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos);
    void __fastcall sb_horz2Scroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos);
    void __fastcall FileMRU1Click(TObject *Sender);
    void __fastcall FileMRU2Click(TObject *Sender);
    void __fastcall FileMRU3Click(TObject *Sender);
    void __fastcall FileMRU4Click(TObject *Sender);
    void __fastcall FileMRU5Click(TObject *Sender);
    void __fastcall FileMRU6Click(TObject *Sender);
    void __fastcall OptionsLockGewebeClick(TObject *Sender);
    void __fastcall RappViewRapportClick(TObject *Sender);
    void __fastcall RappRapportierenClick(TObject *Sender);
    void __fastcall ViewFarbpaletteClick(TObject *Sender);
    void __fastcall EzFixiertClick(TObject *Sender);
    void __fastcall EzGeradeZClick(TObject *Sender);
    void __fastcall EzGeradeSClick(TObject *Sender);
    void __fastcall EzChorig2Click(TObject *Sender);
    void __fastcall EzChorig3Click(TObject *Sender);
    void __fastcall TfBelassenClick(TObject *Sender);
    void __fastcall TfGeradeZClick(TObject *Sender);
    void __fastcall TfGeradeSClick(TObject *Sender);
    void __fastcall TfGesprungenClick(TObject *Sender);
    void __fastcall EzMinimalZClick(TObject *Sender);
    void __fastcall EzMinimalSClick(TObject *Sender);
    void __fastcall EditCutClick(TObject *Sender);
    void __fastcall EditCopyClick(TObject *Sender);
    void __fastcall EditPasteClick(TObject *Sender);
    void __fastcall EditPasteTransparentClick(TObject *Sender);
    void __fastcall EditInvertClick(TObject *Sender);
    void __fastcall EditMirrorHorzClick(TObject *Sender);
    void __fastcall EditMirrorVertClick(TObject *Sender);
    void __fastcall EditRotateClick(TObject *Sender);
    void __fastcall EditDeleteClick(TObject *Sender);
    void __fastcall GewebeNonePopupClick(TObject *Sender);
    void __fastcall GewebeNoneClick(TObject *Sender);
    void __fastcall FilePropsClick(TObject *Sender);
    void __fastcall WeaveClick(TObject *Sender);
	void __fastcall RappReduzierenClick(TObject *Sender);
	void __fastcall FileSetPageClick(TObject *Sender);
    void __fastcall RappOverrideClick(TObject *Sender);
	void __fastcall TfMinimalZClick(TObject *Sender);
	void __fastcall TfMinimalSClick(TObject *Sender);
    void __fastcall EditUndoClick(TObject *Sender);
    void __fastcall EditRedoClick(TObject *Sender);
	void __fastcall CursorTimerTimer(TObject *Sender);
    void __fastcall SBFileNewClick(TObject *Sender);
    void __fastcall SBFileOpenClick(TObject *Sender);
    void __fastcall SBPreviewClick(TObject *Sender);
    void __fastcall SBFilePrintClick(TObject *Sender);
    void __fastcall SBEditCutClick(TObject *Sender);
    void __fastcall SBEditCopyClick(TObject *Sender);
    void __fastcall SBEditPasteClick(TObject *Sender);
    void __fastcall DefineColorsClick(TObject *Sender);
    void __fastcall SBSaveClick(TObject *Sender);
    void __fastcall FormActivate(TObject *Sender);
    void __fastcall FormDeactivate(TObject *Sender);
    void __fastcall FilePrintpreviewClick(TObject *Sender);
    void __fastcall FileRevertClick(TObject *Sender);
    void __fastcall SBZoomInClick(TObject *Sender);
    void __fastcall SBZoomOutClick(TObject *Sender);
    void __fastcall StatusbarDblClick(TObject *Sender);
    void __fastcall FileNewTemplateClick(TObject *Sender);
    void __fastcall EditFillKoeperClick(TObject *Sender);
    void __fastcall ViewOnlyGewebeClick(TObject *Sender);
    void __fastcall ViewZoomNormalClick(TObject *Sender);
    void __fastcall OptPrintSelectionClick(TObject *Sender);
    void __fastcall SchaftMoveUpClick(TObject *Sender);
    void __fastcall SchaftMoveDownClick(TObject *Sender);
    void __fastcall TrittMoveLeftClick(TObject *Sender);
    void __fastcall TrittMoveRightClick(TObject *Sender);
    void __fastcall SBMoveUpClick(TObject *Sender);
    void __fastcall SBMoveDownClick(TObject *Sender);
    void __fastcall SBMoveLeftClick(TObject *Sender);
    void __fastcall SBMoveRightClick(TObject *Sender);
    void __fastcall InsertSchaftClick(TObject *Sender);
    void __fastcall InsertTrittClick(TObject *Sender);
    void __fastcall InsertKetteClick(TObject *Sender);
    void __fastcall InsertSchussClick(TObject *Sender);
    void __fastcall DeleteSchaftClick(TObject *Sender);
    void __fastcall DeleteTrittClick(TObject *Sender);
    void __fastcall DeleteKetteClick(TObject *Sender);
    void __fastcall DeleteSchussClick(TObject *Sender);
    void __fastcall SBUndoClick(TObject *Sender);
    void __fastcall SBRedoClick(TObject *Sender);
    void __fastcall CopyEinzugTrittfolgeClick(TObject *Sender);
    void __fastcall CopyTrittfolgeEinzugClick(TObject *Sender);
    void __fastcall EditCursordirectionClick(TObject *Sender);
    void __fastcall EditBlockmusterClick(TObject *Sender);
    void __fastcall ViewHlinesClick(TObject *Sender);
    void __fastcall RollUpClick(TObject *Sender);
    void __fastcall RollDownClick(TObject *Sender);
    void __fastcall RollLeftClick(TObject *Sender);
    void __fastcall RollRightClick(TObject *Sender);
    void __fastcall CursorLockedClick(TObject *Sender);
    void __fastcall Range1Click(TObject *Sender);
    void __fastcall Range2Click(TObject *Sender);
    void __fastcall Range3Click(TObject *Sender);
    void __fastcall Range4Click(TObject *Sender);
    void __fastcall Range5Click(TObject *Sender);
    void __fastcall Range6Click(TObject *Sender);
    void __fastcall Range7Click(TObject *Sender);
    void __fastcall Range8Click(TObject *Sender);
    void __fastcall Range9Click(TObject *Sender);
    void __fastcall ImportBitmapClick(TObject *Sender);
    void __fastcall OverviewClick(TObject *Sender);
    void __fastcall RangePatternsClick(TObject *Sender);
    void __fastcall KettfarbenWieSchussfarbenClick(TObject *Sender);
    void __fastcall SchussfarbenWieKettfarbenClick(TObject *Sender);
    void __fastcall ViewToolpaletteClick(TObject *Sender);
    void __fastcall ViewInfosClick(TObject *Sender);
    void __fastcall DateiExportClick(TObject *Sender);
    void __fastcall EditSwapsideClick(TObject *Sender);
    void __fastcall FarbverlaufClick(TObject *Sender);
    void __fastcall EinzugAssistentClick(TObject *Sender);
    void __fastcall ClearEinzugClick(TObject *Sender);
    void __fastcall EzBelassenClick(TObject *Sender);
    void __fastcall EzSpiegelnClick(TObject *Sender);
    void __fastcall ClearTrittfolgeClick(TObject *Sender);
    void __fastcall TfSpiegelnClick(TObject *Sender);
    void __fastcall ClearSchlagpatroneClick(TObject *Sender);
    void __fastcall SpSpiegelnClick(TObject *Sender);
    void __fastcall ClearAufknuepfungClick(TObject *Sender);
    void __fastcall AufInvertClick(TObject *Sender);
    void __fastcall AufRollUpClick(TObject *Sender);
    void __fastcall AufRollDownClick(TObject *Sender);
    void __fastcall AufRollLeftClick(TObject *Sender);
    void __fastcall AufRollRightClick(TObject *Sender);
    void __fastcall AufZentralsymmClick(TObject *Sender);
    void __fastcall SpInvertClick(TObject *Sender);
    void __fastcall EditCentralsymClick(TObject *Sender);
    void __fastcall SetKettfarbeClick(TObject *Sender);
    void __fastcall SetSchussfarbeClick(TObject *Sender);
    void __fastcall Koeper2_2Click(TObject *Sender);
    void __fastcall Koeper3_3Click(TObject *Sender);
    void __fastcall Koeper4_4Click(TObject *Sender);
    void __fastcall Koeper5_5Click(TObject *Sender);
    void __fastcall Koeper2_1Click(TObject *Sender);
    void __fastcall Koeper3_1Click(TObject *Sender);
    void __fastcall Koeper4_1Click(TObject *Sender);
    void __fastcall Koeper5_1Click(TObject *Sender);
    void __fastcall Koeper3_2Click(TObject *Sender);
    void __fastcall Koeper4_2Click(TObject *Sender);
    void __fastcall Koeper5_2Click(TObject *Sender);
    void __fastcall Koeper4_3Click(TObject *Sender);
    void __fastcall Koeper5_3Click(TObject *Sender);
    void __fastcall Atlas5Click(TObject *Sender);
    void __fastcall Atlas6Click(TObject *Sender);
    void __fastcall Atlas7Click(TObject *Sender);
    void __fastcall Atlas8Click(TObject *Sender);
    void __fastcall Atlas9Click(TObject *Sender);
    void __fastcall Atlas10Click(TObject *Sender);
    void __fastcall SBInvertClick(TObject *Sender);
    void __fastcall SBMirrorHClick(TObject *Sender);
    void __fastcall SBMirrorVClick(TObject *Sender);
    void __fastcall SBRotateClick(TObject *Sender);
    void __fastcall SBCentralsymClick(TObject *Sender);
    void __fastcall Popup1CancelClick(TObject *Sender);
    void __fastcall Popup1CopyClick(TObject *Sender);
    void __fastcall Popup1CutClick(TObject *Sender);
    void __fastcall Popup1InvertClick(TObject *Sender);
    void __fastcall Popup1MirrorHClick(TObject *Sender);
    void __fastcall Popup1MirrorVClick(TObject *Sender);
    void __fastcall Popup1RotateClick(TObject *Sender);
    void __fastcall Popup1CentralsymClick(TObject *Sender);
    void __fastcall Popup1DeleteClick(TObject *Sender);
    void __fastcall Popup1RollUpClick(TObject *Sender);
    void __fastcall Popup1RollDownClick(TObject *Sender);
    void __fastcall Popup1RollLeftClick(TObject *Sender);
    void __fastcall Popup1RollRightClick(TObject *Sender);
    void __fastcall FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall CursorGotoClick(TObject *Sender);
    void __fastcall LoadPartsClick(TObject *Sender);
    void __fastcall SteigungIncClick(TObject *Sender);
    void __fastcall SteigungDecClick(TObject *Sender);
    void __fastcall AufSteigungIncClick(TObject *Sender);
    void __fastcall AufSteigungDecClick(TObject *Sender);
    void __fastcall OptSwissClick(TObject *Sender);
    void __fastcall OptSkandinavischClick(TObject *Sender);
    void __fastcall OptAmericanClick(TObject *Sender);
    void __fastcall XOptionsClick(TObject *Sender);
    void __fastcall ReplaceColorClick(TObject *Sender);
    void __fastcall SBHighlightMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall SBHighlightMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall SwitchColorsClick(TObject *Sender);
    void __fastcall XOptionsGlobalClick(TObject *Sender);
    void __fastcall KettLancierungClick(TObject *Sender);
    void __fastcall SchussLancierungClick(TObject *Sender);
    void __fastcall Userdef1Click(TObject *Sender);
    void __fastcall Userdef2Click(TObject *Sender);
    void __fastcall Userdef3Click(TObject *Sender);
    void __fastcall Userdef4Click(TObject *Sender);
    void __fastcall Userdef5Click(TObject *Sender);
    void __fastcall Userdef6Click(TObject *Sender);
    void __fastcall Userdef7Click(TObject *Sender);
    void __fastcall Userdef8Click(TObject *Sender);
    void __fastcall Userdef9Click(TObject *Sender);
    void __fastcall Userdef10Click(TObject *Sender);
    void __fastcall UserdefAddClick(TObject *Sender);
    void __fastcall UserdefRemoveClick(TObject *Sender);
    void __fastcall UserdefAddSelClick(TObject *Sender);
    void __fastcall RangeAushebungClick(TObject *Sender);
    void __fastcall RangeAnbindungClick(TObject *Sender);
    void __fastcall RangeAbbindungClick(TObject *Sender);
    void __fastcall InverserepeatClick(TObject *Sender);
    void __fastcall PastePopupClick(TObject *Sender);
    void __fastcall PasteTransparentlyPopupClick(TObject *Sender);
    void __fastcall Popup1PasteClick(TObject *Sender);
    void __fastcall Popup1PasteTransparentlyClick(TObject *Sender);
    void __fastcall OptEnvironmentClick(TObject *Sender);
    void __fastcall SplashtimerTimer(TObject *Sender);
    void __fastcall FormMouseWheel(TObject *Sender, TShiftState Shift, int WheelDelta, TPoint &MousePos, bool &Handled);
    void __fastcall HelpTechinfoClick(TObject *Sender);
    void __fastcall PopupRangeCancelClick(TObject *Sender);
    void __fastcall PopupRange1Click(TObject *Sender);
    void __fastcall PopupRange2Click(TObject *Sender);
    void __fastcall PopupRange3Click(TObject *Sender);
    void __fastcall PopupRange4Click(TObject *Sender);
    void __fastcall PopupRange5Click(TObject *Sender);
    void __fastcall PopupRange6Click(TObject *Sender);
    void __fastcall PopupRange7Click(TObject *Sender);
    void __fastcall PopupRange8Click(TObject *Sender);
    void __fastcall PopupRange9Click(TObject *Sender);
    void __fastcall PopupRangeBindingClick(TObject *Sender);
    void __fastcall PopupRangeUnbindingClick(TObject *Sender);
    void __fastcall PopupRangeLiftoutClick(TObject *Sender);
    void __fastcall StatusbarClick(TObject *Sender);
    void __fastcall PopupRangePopup(TObject *Sender);
    void __fastcall StatusbarMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
	void __fastcall ImportWIFClick(TObject *Sender);
private:    // User declarations
    bool bInitialized;

    bool mousedown;
    PT  md_pt; // mousedown_point (pixelkoord)
    PT  md;    // mousedown;
    PT  dragto;
    FELD md_feld;

    int lastfarbei;
	int lastfarbej;
	int lastblatteinzugi;

    bool bSelectionCleared;

    bool rangepopupactive;

    bool hDragging;
    bool vDragging;
    int  lastvdrag;
    int  lasthdrag;

    String mru[6];

    StWeaveController* controller;
	UrUndo* undo;
    BlockUndo* blockundo;
    BlockUndo* bereichundo;
    CrCursorHandler* cursorhandler;
    RpRapport* rapporthandler;
    EinzugRearrange* einzughandler;

public:
    bool handlecommandline;

    bool dohighlight;

public:
    // OS-Info
    bool IsNT; // True falls Rechner ein NT-System hat

    bool showweaveposition;
    int weave_position;

    // Tools
    TOOL tool;
    LINESIZE linesize;

    Klammer klammern[9];
    bool weaving;

    int gridheight; // Feldbreite
    int gridwidth;  // Feldhöhe
    int divider;    // Mindestbreite/-höhe der Unterteilungen

    RAPPORT rapport; // Rapport
    SZ kette;        // Belegter Bereich der Kette
    SZ schuesse;     // Belegter Bereich der Schuesse

    double faktor_kette; // Kett-/Schussverhältnis, hat
    double faktor_schuss; // Auswirkungen auf gridheight/gridwidth!

    FeldKettfarben   kettfarben;
    FeldBlatteinzug  blatteinzug;
    FeldEinzug       einzug;
    FeldGewebe       gewebe;
    FeldAufknuepfung aufknuepfung;
    FeldTrittfolge   trittfolge;
    FeldSchussfarben schussfarben;

    // Blockmuster
    Muster blockmuster[10];
    Muster bereichmuster[10];
    int currentbm;
    bool   einzugZ;
    bool   trittfolgeZ;

    // Hilfslinien
    Hilfslinien hlines;
    HlineBar hlinehorz1;
    HlineBar hlinehorz2;
    HlineBar hlinevert1;
    HlineBar hlinevert2;
    bool hlinedrag;
    bool hlinepredrag;
    Hilfslinie* hline; // gedraggte Hilfslinie

    // Allzweck-Buffers
    char* xbuf;
    char* ybuf;

    // Benutzerdefinierter Einzug
    short* fixeinzug;
    short  firstfree; // erster freier Schaft des fixierten Einzugs
    int    fixsize; // "Breite" des fixeinzugs

    // Freie Schäfte und Tritte
    bool* freieschaefte;
    bool* freietritte;

#ifdef _DEBUG
    int xbufsize;
    int ybufsize;
    int fixeinzugsize;
    int freieschaeftesize;
    int freietrittesize;
#endif

    // Beim Umschalten auf Schlagpatrone wird
    // erstens automatisch auf Fixiert resp. Belassen
    // geschaltet. Zweitens wird die vorherige
    // Einstellung hier gespeichert. Beim Rückschalten
    // wird sie restauriert.
    TMenuItem* savedeinzugstyle;
    TMenuItem* savedtrittfolgenstyle;

    int hvisible; // Anzahl sichtbare Felder Einzug
    int wvisible; // Anzahl sichtbare Felder Trittfolge

    TColor strongclr;

    DARSTELLUNG schlagpatronendarstellung;
    DARSTELLUNG darst_aushebung;
    DARSTELLUNG darst_anbindung;
    DARSTELLUNG darst_abbindung;

    bool einzugunten;  // true: Einzug wird unterhalb Gewebe dargestellt
    bool righttoleft;  // true: Einzug/Gewebe werden von rechts nach links bearbeitet
    bool toptobottom;  // true: Einzug/Aufknüpfung werden von oben nach unten bearbeitet
    bool fewithraster; // true: Farbeffekt wird mit Raster dargestellt
    bool sinkingshed;  // true: gesetztes Feld ist eine Kettsenkung
    bool palette2;     // true: zweite Palette ist aktiv

    FELD kbd_field;

    RANGE selection;

    int scroll_x1, scroll_x2;
    int scroll_y1, scroll_y2;

    char currentrange; // aktueller Bereich (1-9, 10, 11, -12)

    int currentzoom;
    int zoom[10];

    bool modified;
	bool saved;
    FfFile* file;
    String filename;

	// Drucken: Seiteneinstellung
	PrBorders borders;
	PrHeaderFooter header;
	PrHeaderFooter footer;

    // Druckbereich
    SZ printkette;
    SZ printschuesse;
    SZ printschaefte;
    SZ printtritte;

    // Userdef Patterns
    UserdefPattern userdef[MAXUSERDEF];

public:
    // Funktionen
	void __fastcall InitBorders();
    void __fastcall InitZoom();
    void __fastcall InitPrintRange();
    void __fastcall CalcGrid();
    void __fastcall LoadOptions();

    void __fastcall DebugChecks();

    void __fastcall EditFixeinzug(TObject *Sender);

    void __fastcall HandleCommandlineOpen();
    void __fastcall HandleCommandlinePrint();
    void __fastcall HandleCommandlineArguments();

    void __fastcall LoadUserdefMenu();
    void __fastcall PasteUserdef(bool _transparent);
    int __fastcall SelectUserdef (String _title="");
    void __fastcall InsertUserdef (int _i, bool _transparent);

    void __fastcall AllocBuffers (bool _norecalc);
    void __fastcall AllocBuffersX1();
    void __fastcall AllocBuffersX2();
    void __fastcall AllocBuffersY1();
    void __fastcall AllocBuffersY2();
    void __fastcall InitTools();
    void __fastcall SetAusmasse (int x1, int y1, int x2, int y2, int vx2, int vy1);

    void __fastcall DrawDragTool (int _i, int _j, int _i1, int _j1);
    void __fastcall DeleteDragTool (int _i, int _j, int _i1, int _j1);
    void __fastcall DrawTool (int _i, int _j, int _i1, int _j1);
    void __fastcall DrawToolLine (int _i, int _j, int _i1, int _j1);
    void __fastcall DrawToolRectangle (int _i, int _j, int _i1, int _j1, bool _filled);
    void __fastcall DrawToolEllipse (int _i, int _j, int _i1, int _j1, bool _filled);

    void __fastcall UpdateToolButtons();
    int __fastcall UpdateToolButton (TSpeedButton* _sb, int _x);
    void __fastcall UpdatePopupMenu1();

    void __fastcall SwitchRange (int _range);

    void __fastcall ShowOptions (bool _global=false);

    void __fastcall DrawHighlight();
    void __fastcall ClearHighlight();
    void __fastcall _DrawHighlight (TColor _col);

    void __fastcall AtlasEinfuegen (int _n);
    void __fastcall KoeperEinfuegen (int _h, int _s);

    void __fastcall ToggleGewebe (int _i, int _j);
    void __fastcall ToggleAufknuepfung (int _i, int _j);

    void __fastcall ExtendTritte();
    void __fastcall ExtendSchaefte();
    void __fastcall _ExtendTritte (int _max);
    void __fastcall _ExtendSchaefte (int _max);

    void __fastcall RecalcFreieSchaefte();
    void __fastcall RecalcFreieTritte();

    void __fastcall HandleHomeKey (TShiftState Shift);
    void __fastcall HandleEndKey (TShiftState Shift);
    void __fastcall HandlePageUpKey (TShiftState Shift);
    void __fastcall HandlePageDownKey (TShiftState Shift);

    void __fastcall HandleHomeKey (FELD _feld, INPUTPOS _kbd, bool _left, bool _ctrl, int& _scroll1, int _scroll2, TScrollBar* _sb);
    void __fastcall HandleEndKey (FELD _feld, INPUTPOS _kbd, int _width, bool _left, bool _ctrl, int& _scroll1, int _scroll2, TScrollBar* _sb);
    void __fastcall HandlePageUpKey (FELD _feld, INPUTPOS _kbd, int _width, bool _top, bool _ctrl, int _scroll1, int& _scroll2, TScrollBar* _sb);
    void __fastcall HandlePageDownKey (FELD _feld, INPUTPOS _kbd, bool _top, bool _ctrl, int _scroll1, int& _scroll2, TScrollBar* _sb);

    void __fastcall OnShowPrinterSetupDialog (TObject* Sender);
    void __fastcall OnShowPrintDialog (TObject* Sender);

    void __fastcall UpdateMoveMenu();
    void __fastcall UpdateInsertMenu();
    void __fastcall UpdateDeleteMenu();

    bool __fastcall LoadNormalTemplate();
    bool __fastcall AskSave();
    void __fastcall CloseDataFile();
    void __fastcall InitFelder();
    void __fastcall ClearFelder();
    void __fastcall InitDimensions();
    void __fastcall RecalcDimensions();

    void __fastcall ToggleBlatteinzug();
    void __fastcall ToggleFarbe();
    void __fastcall ToggleEinzug();
    void __fastcall ToggleTrittfolge();
    void __fastcall ToggleSchlagpatrone();

    void __fastcall SelectGewebeNormal();
    void __fastcall SelectGewebeFarbeffekt();
    void __fastcall SelectGewebeSimulation();
    void __fastcall SelectGewebeNone();

    void __fastcall DrawDividers();
    void __fastcall DrawBlatteinzug();
    void __fastcall DrawKettfarben();
    void __fastcall DrawEinzug();
    void __fastcall DrawAufknuepfung();
    void __fastcall DrawTrittfolge();
    void __fastcall DrawGewebe();
    void __fastcall DrawSchussfarben();
    void __fastcall DrawHilfslinien();
    void __fastcall DrawHlineBars();
    void __fastcall DrawHilfslinie (Hilfslinie* _hline);
    void __fastcall DeleteHilfslinie (Hilfslinie* _hline);
    void __fastcall _DrawHilfslinie (Hilfslinie* _hline, bool _draw);

    void __fastcall _ClearEinzug(); // Nur Daten!
    void __fastcall _DrawEinzug();
    void __fastcall _ClearAufknuepfung();
    void __fastcall _DrawAufknuepfung();
    void __fastcall _ClearSchlagpatrone();
    void __fastcall _DrawSchlagpatrone();

    void __fastcall Physical2Logical (int _x, int _y, FELD& _feld, int& _i, int& _j);
    void __fastcall RecalcGewebe();
    void __fastcall RecalcAll();
    void __fastcall RecalcSchlagpatrone();
    void __fastcall RecalcTrittfolgeAufknuepfung();
    void __fastcall RecalcFixEinzug();
    void __fastcall UpdateEinzugFixiert();
    void __fastcall MinimizeAufknuepfung();
    bool __fastcall IsEmptyEinzug (int _i);
    bool __fastcall IsEmptyTrittfolge (int _j);
    void __fastcall RecalcTrittfolgeEmpty (int _j);
    bool __fastcall HitCheck (const GRIDPOS& _grid, int _x, int _y, int _scrollx, int _scrolly, int& _i, int& _j, int _gw, int _gh, bool _righttoleft=false, bool _toptobottom=false);
    void __fastcall SetScrollbar (TScrollBar* _sb, int _max, int _i, int _pos, bool _invers=false);
    void __fastcall UpdateScrollbars();

    bool __fastcall _IsEmptySchaft (int _j);
    bool __fastcall _IsEmptyTritt (int _i);
    int __fastcall GetEmptySchaft();
    int __fastcall GetEmptyTritt();
    void __fastcall InsertKettfaden (int _i);
    void __fastcall InsertSchussfaden (int _j);
    void __fastcall DeleteKettfaden (int _i);
    void __fastcall DeleteSchussfaden (int _j);

    // Felder zeichnen (relative Koordinaten)
    void __fastcall DrawEinzug (int _i, int _j, TColor _col=clBlack);
    void __fastcall DrawEinzugRahmen (int _i, int _j);
    void __fastcall DrawAufknuepfung (int _i, int _j, int _pegplan=false, TColor _col=clBlack);
    void __fastcall DrawAufknuepfungRahmen (int _i, int _j);
    void __fastcall DrawTrittfolge (int _i, int _j, TColor _col=clBlack);
    void __fastcall DrawTrittfolgeRahmen (int _i, int _j);
    void __fastcall DrawGewebe (int _i, int _j, TColor _col=clBlack);
    void __fastcall DrawGewebeNormal (int _i, int _j, int _x, int _y, int _xx, int _yy, TColor _col=clBlack);
    void __fastcall DrawGewebeFarbeffekt (int _i, int _j, int _x, int _y, int _xx, int _yy);
    void __fastcall DrawGewebeSimulation (int _i, int _j, int _x, int _y, int _xx, int _yy);
    void __fastcall DrawGewebeRapport (int _i, int _j, int _x, int _y, int _xx, int _yy);
    void __fastcall DrawGewebeRahmen (int _i, int _j);
    void __fastcall DrawGewebeRahmen (int _i, int _j, int _x, int _y, int _xx, int _yy);

    void __fastcall DrawGewebeSchuss (int _j);
    void __fastcall DrawGewebeKette (int _i);
    void __fastcall DeleteGewebeSchuss (int _j);
    void __fastcall DeleteGewebeKette (int _i);

    void __fastcall CalcRangeSchuesse();
    void __fastcall CalcRangeKette();
    void __fastcall CalcRange();
    void __fastcall UpdateRange (int _i, int _j, bool _set);
    bool __fastcall IsEmptySchuss (int _j);
    bool __fastcall IsEmptyKette (int _i);
    bool __fastcall IsEmptySchussNurGewebe (int _j);
    bool __fastcall IsEmptyKetteNurGewebe (int _i);

    bool __fastcall KettfadenEqual (int _a, int _b);

    int __fastcall GetFirstSchaft();
    int __fastcall GetLastSchaft();
    int __fastcall GetFirstTritt();
    int __fastcall GetLastTritt();

    void __fastcall DrawRapport();
    void __fastcall CalcRapport();
    void __fastcall UpdateRapport();
    void __fastcall ClearRapport();
    bool __fastcall IsInRapport (int _i, int _j);

    void __fastcall ClearSelection (bool _cleardata=true);
    void __fastcall DrawSelection();
    void __fastcall ResizeSelection (int _i, int _j, FELD _feld, bool _square);
    void __fastcall CutSelection();
    bool __fastcall CopySelection (bool _movecursor=true);
    void __fastcall PasteSelection (bool _transparent);
    void __fastcall CalcSelectionRect (int& _x, int& _y, int& _xx, int& _yy);

    void __fastcall DateiNeu();
    void __fastcall DateiSpeichern();
    void __fastcall DateiExportieren(const String& _filename);
    void __fastcall DateiLaden (const String& _filename, bool _save, bool _addtomru=true);
    void __fastcall SetModified();
    void __fastcall SetAppTitle();
    bool __fastcall CheckExistence(const String& _filename);
    void __fastcall ResizeFelder();

    void __fastcall DoExportBitmap (const String& _filename);

    bool __fastcall Save(bool _format37=true);
	bool __fastcall SaveSignatur (FfWriter* _writer);
    bool __fastcall SaveVersion (FfWriter* _writer, bool _format37);
    bool __fastcall SaveFileinfo (FfWriter* _writer);
    bool __fastcall SaveData (FfWriter* _writer, bool _format37);
    bool __fastcall SaveView (FfWriter* _writer, bool _format37);
    bool __fastcall SavePrint (FfWriter* _writer, bool _format37);

    bool __fastcall Load (LOADSTAT& _stat, LOADPARTS _loadparts=LOADALL);
    void __fastcall FinalizeLoad();

    short __fastcall GetFreeEinzug();
    short __fastcall GetFreeTritt();
    void __fastcall CopyTritt (int _von, int _nach);

    void __fastcall SetEinzug (int _i, int _j);
    void __fastcall SetTrittfolge (int _i, int _j, bool _set, int _range);
    void __fastcall SetAufknuepfung (int _i, int _j, bool _set, int _range);
    void __fastcall SetGewebe (int _i, int _j, bool _set, int _range);
    void __fastcall SetKettfarben (int _i);
    void __fastcall SetSchussfarben (int _j);
    void __fastcall SetBlatteinzug (int _i);

    void __fastcall DoSetEinzug (int _i, int _j);
    void __fastcall DoSetTrittfolge (int _i, int _j, bool _set, int _range);
    void __fastcall DoSetAufknuepfung (int _i, int _j, bool _set, int _range);
    void __fastcall DoSetGewebe (int _i, int _j, bool _set, int _range);
    void __fastcall DoSetKettfarben (int _i);
    void __fastcall DoSetSchussfarben (int _j);
    void __fastcall DoSetBlatteinzug (int _i);

    void __fastcall MoveTritt (int _von, int _nach);
    bool __fastcall IsEmptyTritt (int _i);
    int  __fastcall GetFirstNonemptyTritt (int _i);
    void __fastcall EliminateEmptyTritt();
    void __fastcall EliminateEmptySchaft();
    void __fastcall MergeTritte();
    bool __fastcall AufknuepfungsspalteEqual (int _i1, int _i2);

    void __fastcall UpdateIsEmpty (int _from, int _to);

    bool __fastcall TrittfolgeEqual (int _j1, int _j2);
    bool __fastcall EinzugEqual (int _i1, int _i2);

    void __fastcall RedrawTritt (int _i);
    void __fastcall RedrawAufknuepfungTritt (int _i);
    void __fastcall RedrawGewebe (int _i, int _j);
    void __fastcall ClearGewebe (int _i, int _j);
    void __fastcall RedrawAufknuepfung (int _i, int _j);

    void __fastcall RearrangeSchaefte();
    void __fastcall RearrangeTritte();
    void __fastcall SwitchTritte (int _a, int _b);

    void __fastcall BlockExpandEinzug (int _count);
    void __fastcall BlockExpandTrittfolge (int _count);
    void __fastcall BlockExpandAufknuepfung (int _x, int _y);

    void __fastcall BereicheFillPattern (int _x, int _y);

    void __fastcall IncrementSteigung (int _i, int _j, int _ii, int _jj, FELD _feld);
    void __fastcall DecrementSteigung (int _i, int _j, int _ii, int _jj, FELD _feld);

    void __fastcall ClearKettfaden (int _i);
    void __fastcall ClearSchussfaden (int _j);
    void __fastcall CopyKettfaden (int _von, int _nach, bool _withcolors);
    void __fastcall CopySchussfaden (int _von, int _nach, bool _withcolors);
    void __fastcall RapportSchuss (int _ry, bool _withcolors);
    void __fastcall RapportKette (int _rx, bool _withcolors);

    void __fastcall DrawCursor();
    void __fastcall DeleteCursor();
    void __fastcall GotoNextField();
    void __fastcall GotoPrevField();
    void __fastcall SetCursor (int _i, int _j, bool _clearselection=true);

    void __fastcall UpdateStatusBar();
    void __fastcall RecalcStatusBar();
	void __fastcall OnDrawStatusBar (TStatusBar* StatusBar, TStatusPanel* Panel, const Windows::TRect &Rect);
    void __fastcall DisplayHint (TObject* Sender);

    void __fastcall UpdateMRUMenu (int _item, TMenuItem* _menuitem, String _filename);
    void __fastcall AddToMRU (const String& _filename);
    void __fastcall UpdateMRU();
    void __fastcall SaveMRU();
    void __fastcall LoadMRU();

    void __fastcall DrawDivDragH (int _x);
    void __fastcall DrawDivDragV (int _y);
    void __fastcall DivDragStatus();

    int __fastcall SelectColorIndex(int _index);
    bool __fastcall SelectColor (COLORREF& _col);

    void __fastcall IdleHandler (TObject* Sender, bool& Done);

    void __fastcall GetSelectionLine (const RANGE& _selection, int _j, char* _buff);

    void __fastcall InvalidateFeld (GRIDPOS _gridpos);

protected:
    void __fastcall WMEraseBkgnd (TWMEraseBkgnd& _msg);
BEGIN_MESSAGE_MAP
    MESSAGE_HANDLER (WM_ERASEBKGND, TWMEraseBkgnd, WMEraseBkgnd)
END_MESSAGE_MAP (TForm)

public:        // User declarations
    __fastcall TDBWFRM(TComponent* Owner);
    virtual __fastcall ~TDBWFRM();
    void __fastcall ReloadLanguage();
};

extern PACKAGE TDBWFRM *DBWFRM;

inline void __fastcall ClearCell (TCanvas* _canvas, int _x, int _y, int _xx, int _yy)
{
    _canvas->Pen->Color = clBtnFace;
    _canvas->Brush->Color = clBtnFace;
    _canvas->Rectangle (_x+1, _y+1, _xx, _yy);
}

inline void __fastcall PaintCell (TCanvas* _canvas, DARSTELLUNG _darstellung, int _x, int _y, int _xx, int _yy, TColor _color=clBlack, bool _dontclear=false, int _number=-1, TColor _bkground=clBtnFace)
{
    if (!_dontclear) ClearCell (_canvas, _x, _y, _xx, _yy);

    _canvas->Pen->Color = _color;
    _canvas->Brush->Color = _color;

    switch (_darstellung) {
        case AUSGEFUELLT:
            _canvas->Rectangle (_x+2, _y+2, _xx-1, _yy-1);
            break;

        case STRICH:
            _canvas->MoveTo ((_x+_xx)/2, _yy-2);
            _canvas->LineTo ((_x+_xx)/2, _y+1);
            _canvas->MoveTo ((_x+_xx)/2+1, _yy-2);
            _canvas->LineTo ((_x+_xx)/2+1, _y+1);
            break;

        case KREUZ:
            _canvas->MoveTo (_x+2, _yy-2);
            _canvas->LineTo (_xx-1, _y+1);
            _canvas->MoveTo (_x+2, _y+2);
            _canvas->LineTo (_xx-1, _yy-1);
            break;

        case PUNKT: {
            int x = (_x+_xx)/2;
            int y = (_y+_yy)/2;
            _canvas->MoveTo (x, y); _canvas->LineTo (x+2, y);
            _canvas->MoveTo (x, y+1); _canvas->LineTo (x+2, y+1);
            break;
        }

        case KREIS:
            _canvas->Arc (_x+2, _y+2, _xx-1, _yy-1, _x, _y, _x, _y);
            break;

        case STEIGEND:
            _canvas->MoveTo (_x+2, _yy-2);
            _canvas->LineTo (_xx-1, _y+1);
            break;

        case FALLEND:
            _canvas->MoveTo (_x+2, _y+2);
            _canvas->LineTo (_xx-1, _yy-1);
            break;

        case SMALLKREIS:
            if (_xx-_x>=9) _canvas->Arc (_x+4, _y+4, _xx-3, _yy-3, _x, _y, _x, _y);
            else {
                int x = (_x+_xx)/2;
                int y = (_y+_yy)/2;
                _canvas->MoveTo (x, y); _canvas->LineTo (x+2, y);
                _canvas->MoveTo (x, y+1); _canvas->LineTo (x+2, y+1);
            }
            break;

        case SMALLKREUZ:
            if (_xx-_x>=9) {
                _canvas->MoveTo (_x+4, _yy-4);
                _canvas->LineTo (_xx-3, _y+3);
                _canvas->MoveTo (_x+4, _y+4);
                _canvas->LineTo (_xx-3, _yy-3);
            } else {
                _canvas->MoveTo (_x+2, _yy-2);
                _canvas->LineTo (_xx-1, _y+1);
                _canvas->MoveTo (_x+2, _y+2);
                _canvas->LineTo (_xx-1, _yy-1);
            }
            break;

        case NUMBER:
            if (_number>=0) {
                _canvas->Brush->Color = _bkground;
                int oldflags = _canvas->TextFlags;
                int oldsize = _canvas->Font->Height;
                _canvas->TextFlags = _canvas->TextFlags & ~ETO_OPAQUE;
                _canvas->Font->Color = _color;
                if (DBWFRM->currentzoom>3) _canvas->Font->Height = DBWFRM->einzug.gh-2;
                else if (DBWFRM->currentzoom>=2) _canvas->Font->Height = 7;
                else _canvas->Font->Height = 6;
                String nr = IntToStr(_number+1);
                TSize sz = _canvas->TextExtent(nr);
                int xx = 0; if (_xx-_x>sz.cx) xx = (_xx-_x-sz.cx)/2;
                int yy = 0; if (_yy-_y>sz.cy) yy = (_yy-_y-sz.cy)/2;
                _canvas->TextRect (TRect(_x+1, _y+1, _xx-1, _yy-1), _x+xx, _y+yy, nr);
                _canvas->TextFlags = oldflags;
                _canvas->Font->Height = oldsize;
            } else {
                _canvas->Rectangle (_x+2, _y+2, _xx-1, _yy-1);
            }
            break;
    }
}

#endif
