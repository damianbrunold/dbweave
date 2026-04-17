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
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("dbw.res");
USEFORM("datamodule.cpp", Data);
USEFORM("about.cpp", AboutBox);
USEFORM("blockmuster_form.cpp", BlockmusterForm);
USEFORM("cursordir_form.cpp", CursorDirForm);
USEFORM("cursorgoto_form.cpp", CursorGotoForm);
USEFORM("dbw3_form.cpp", DBWFRM);
USEFORM("einstellverh_form.cpp", EinstellVerhaeltnisForm);
USEFORM("einzugassistent_form.cpp", EinzugassistentForm);
USEFORM("entervv_form.cpp", EnterVVForm);
USEFORM("entwurfsinfo_form.cpp", EntwurfsinfoForm);
USEFORM("envoptions_form.cpp", EnvironmentOptionsForm);
USEFORM("farbauswahl_form.cpp", ChooseHSVForm);
USEFORM("farbauswahl1_form.cpp", ChooseRGBForm);
USEFORM("farbauswahl2_form.cpp", ChoosePaletteForm);
USEFORM("farbpalette_form.cpp", FarbPalette);
USEFORM("farbverlauf_form.cpp", FarbverlaufForm);
USEFORM("fixeinzug_form.cpp", FixeinzugForm);
USEFORM("importbmp_form.cpp", ImportBmpForm);
USEFORM("loadparts_form.cpp", LoadPartsForm);
USEFORM("overview_form.cpp", OverviewForm);
USEFORM("pagesetup_form.cpp", PageSetupForm);
USEFORM("printcancel_form.cpp", PrintCancelForm);
USEFORM("printpreview_form.cpp", PrintPreviewForm);
USEFORM("printselection_form.cpp", PrintSelectionForm);
USEFORM("properties_form.cpp", FilePropertiesForm);
USEFORM("rapport_form.cpp", RapportForm);
USEFORM("selcolor_form.cpp", SelColorForm);
USEFORM("splash_form.cpp", Splashscreen);
USEFORM("steuerung_form.cpp", STRGFRM);
USEFORM("strggoto_form.cpp", StrgGotoForm);
USEFORM("strginfo_form.cpp", StrgInfoForm);
USEFORM("strgoptloom_form.cpp", StrgOptLoomForm);
USEFORM("strgpatronicindparms_form.cpp", StrgPatronicIndParmsForm);
USEFORM("techinfo_form.cpp", TechinfoForm);
USEFORM("toolpalette_form.cpp", ToolpaletteForm);
USEFORM("userdef_entername_form.cpp", UserdefEnternameForm);
USEFORM("userdefselect_form.cpp", UserdefSelectForm);
USEFORM("xoptions_form.cpp", XOptionsForm);
USEUNIT("assert.cpp");
USEUNIT("aufknuepfung.cpp");
USEUNIT("bereiche.cpp");
USEUNIT("blockmuster.cpp");
USEUNIT("blockmuster_muster.cpp");
USEUNIT("clear.cpp");
USEUNIT("colors.cpp");
USEUNIT("commandline.cpp");
USEUNIT("comutil.cpp");
USEUNIT("cursor.cpp");
USEUNIT("debugchecks.cpp");
USEUNIT("delete.cpp");
USEUNIT("dialogs.cpp");
USEUNIT("draw.cpp");
USEUNIT("einzug.cpp");
USEUNIT("einzugtrittfolge.cpp");
USEUNIT("export.cpp");
USEUNIT("exportbitmap.cpp");
USEUNIT("felddef.cpp");
USEUNIT("fileformat.cpp");
USEUNIT("filehandling.cpp");
USEUNIT("fileload.cpp");
USEUNIT("filesave.cpp");
USEUNIT("highlight.cpp");
USEUNIT("hilfslinien.cpp");
USEUNIT("idle.cpp");
USEUNIT("importbmp.cpp");
USEUNIT("init.cpp");
USEUNIT("insert.cpp");
USEUNIT("insertbindung.cpp");
USEUNIT("invalidate.cpp");
USEUNIT("kbdhandling.cpp");
USEUNIT("kbdscroll.cpp");
USEUNIT("language.cpp");
USEUNIT("lang_main.cpp");
USEUNIT("mousehandling.cpp");
USEUNIT("move.cpp");
USEUNIT("mru.cpp");
USEUNIT("osversion.cpp");
USEUNIT("palette.cpp");
USEUNIT("popupmenu.cpp");
USEUNIT("properties.cpp");
USEUNIT("print.cpp");
USEUNIT("printdraw.cpp");
USEUNIT("printextent.cpp");
USEUNIT("printinit.cpp");
USEUNIT("printprint.cpp");
USEUNIT("range.cpp");
USEUNIT("rangecolors.cpp");
USEUNIT("rapport.cpp");
USEUNIT("rapportieren.cpp");
USEUNIT("recalc.cpp");
USEUNIT("redraw.cpp");
USEUNIT("schlagpatrone.cpp");
USEUNIT("scrolling.cpp");
USEUNIT("selection.cpp");
USEUNIT("setaufknuepfung.cpp");
USEUNIT("setblatteinzug.cpp");
USEUNIT("setcolors.cpp");
USEUNIT("seteinzug.cpp");
USEUNIT("setfarbe.cpp");
USEUNIT("setgewebe.cpp");
USEUNIT("settings.cpp");
USEUNIT("settrittfolge.cpp");
USEUNIT("statusbar.cpp");
USEUNIT("steigung.cpp");
USEUNIT("steuerung.cpp");
USEUNIT("steuerung_draw.cpp");
USEUNIT("steuerung_kbd.cpp");
USEUNIT("steuerung_mouse.cpp");
USEUNIT("steuerung_popup.cpp");
USEUNIT("steuerung_pos.cpp");
USEUNIT("steuerung_weben.cpp");
USEUNIT("toolbar.cpp");
USEUNIT("tools.cpp");
USEUNIT("trittfolge.cpp");
USEUNIT("undoredo.cpp");
USEUNIT("userdef.cpp");
USEUNIT("utilities.cpp");
USEUNIT("zentralsymm.cpp");
USEUNIT("zoom.cpp");
USERC("version.rc");
USEUNIT("import.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        Application->Initialize();
        Application->Title = "DB-WEAVE";
        Application->HelpFile = "";
        Application->CreateForm(__classid(TData), &Data);
                 Application->CreateForm(__classid(TDBWFRM), &DBWFRM);
                 Application->CreateForm(__classid(TFarbPalette), &FarbPalette);
                 Application->CreateForm(__classid(TToolpaletteForm), &ToolpaletteForm);
                 Application->CreateForm(__classid(TSTRGFRM), &STRGFRM);
                 Application->Run();
    }
    catch (Exception &exception)
    {
        Application->ShowException(&exception);
    }
    return 0;
}
//---------------------------------------------------------------------------



