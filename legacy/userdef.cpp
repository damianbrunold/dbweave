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
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "userdef.h"
#include "dbw3_form.h"
#include "settings.h"
#include "dbw3_strings.h"
#include "assert.h"
#include "undoredo.h"
#include "datamodule.h"
#include "userdef_entername_form.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::LoadUserdefMenu()
{
    try {
        Settings settings;
        bool menuvisible = false;
        for (int i=0; i<MAXUSERDEF; i++) {
            settings.SetCategory (String("Userdef")+IntToStr(i));
            userdef[i].description = settings.Load ("Description", "");
            userdef[i].sizex = settings.Load ("Sizex", 0);
            userdef[i].sizey = settings.Load ("Sizey", 0);
            userdef[i].data = settings.Load ("Data", "");
            bool visible = userdef[i].data!="";
            if (visible) menuvisible = true;
            switch (i) {
                case 0:
                    Userdef1->Visible = visible;
                    Userdef1->Caption = userdef[i].description;
                    break;
                case 1:
                    Userdef2->Visible = visible;
                    Userdef2->Caption = userdef[i].description;
                    break;
                case 2:
                    Userdef3->Visible = visible;
                    Userdef3->Caption = userdef[i].description;
                    break;
                case 3:
                    Userdef4->Visible = visible;
                    Userdef4->Caption = userdef[i].description;
                    break;
                case 4:
                    Userdef5->Visible = visible;
                    Userdef5->Caption = userdef[i].description;
                    break;
                case 5:
                    Userdef6->Visible = visible;
                    Userdef6->Caption = userdef[i].description;
                    break;
                case 6:
                    Userdef7->Visible = visible;
                    Userdef7->Caption = userdef[i].description;
                    break;
                case 7:
                    Userdef8->Visible = visible;
                    Userdef8->Caption = userdef[i].description;
                    break;
                case 8:
                    Userdef9->Visible = visible;
                    Userdef9->Caption = userdef[i].description;
                    break;
                case 9:
                    Userdef10->Visible = visible;
                    Userdef10->Caption = userdef[i].description;
                    break;
            }
        }
        MenuWeitere->Visible = menuvisible;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::PasteUserdef(bool _transparent)
{
    int i = SelectUserdef(USERDEF_PASTEMUSTER);
    if (i>=0 && i<MAXUSERDEF) InsertUserdef (i, _transparent);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Userdef1Click(TObject *Sender)
{
    InsertUserdef (0, GetKeyState(VK_CONTROL)&0x8000);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Userdef2Click(TObject *Sender)
{
    InsertUserdef (1, GetKeyState(VK_CONTROL)&0x8000);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Userdef3Click(TObject *Sender)
{
    InsertUserdef (2, GetKeyState(VK_CONTROL)&0x8000);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Userdef4Click(TObject *Sender)
{
    InsertUserdef (3, GetKeyState(VK_CONTROL)&0x8000);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Userdef5Click(TObject *Sender)
{
    InsertUserdef (4, GetKeyState(VK_CONTROL)&0x8000);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Userdef6Click(TObject *Sender)
{
    InsertUserdef (5, GetKeyState(VK_CONTROL)&0x8000);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Userdef7Click(TObject *Sender)
{
    InsertUserdef (6, GetKeyState(VK_CONTROL)&0x8000);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Userdef8Click(TObject *Sender)
{
    InsertUserdef (7, GetKeyState(VK_CONTROL)&0x8000);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Userdef9Click(TObject *Sender)
{
    InsertUserdef (8, GetKeyState(VK_CONTROL)&0x8000);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Userdef10Click(TObject *Sender)
{
    InsertUserdef (9, GetKeyState(VK_CONTROL)&0x8000);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::InsertUserdef (int _i, bool _transparent)
{
    dbw3_assert(_i>=0);
    dbw3_assert(_i<MAXUSERDEF);

    // Daten genügend lang?
    if (userdef[_i].data.Length() < userdef[_i].sizex*userdef[_i].sizey) {
        dbw3_assert(false);
        return;
    }

    if (userdef[_i].sizex!=0 && kbd_field==GEWEBE) {
        int x = gewebe.kbd.i+scroll_x1;
        int y = gewebe.kbd.j+scroll_y2;
        int xx = x + userdef[_i].sizex;
        int yy = y + userdef[_i].sizey;
        if (xx>=Data->MAXX1) xx = Data->MAXX1-1;
        if (yy>=Data->MAXY2) yy = Data->MAXY2-1;

        try {
            for (int i=x; i<xx; i++)
                for (int j=y; j<yy; j++) {
                    int idx = (i-x)*userdef[_i].sizey + (j-y);
                    char s = userdef[_i].data[idx+1];
                    if (s!='k') gewebe.feld.Set (i, j, char(s-'k'));
                    else if (!_transparent) gewebe.feld.Set (i, j, 0);
                }
        } catch (...) {
        }

        RecalcAll();
        CalcRangeKette();
        CalcRangeSchuesse();
        UpdateRapport();
        SetCursor (xx, y, true);
        selection.begin.i = x;
        selection.begin.j = y;
        selection.end.i = xx - 1;
        selection.end.j = yy - 1;
        if (selection.end.i>=scroll_x1+gewebe.pos.width/gewebe.gw) selection.end.i = scroll_x1 + gewebe.pos.width/gewebe.gw - 1;
        if (selection.end.j>=scroll_y2+gewebe.pos.height/gewebe.gh) selection.end.j = scroll_y2 + gewebe.pos.height/gewebe.gh - 1;
        selection.feld = GEWEBE;
        Invalidate();
        SetModified();
        undo->Snapshot();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::UserdefAddClick(TObject *Sender)
{
    if (kette.count()>50 || schuesse.count()>50) {
        Application->MessageBox (USERDEF_TOOLARGE, APP_TITLE, MB_OK);
        return;
    }

    int i = SelectUserdef(USERDEF_SELMUSTER);
    if (i>=0 && i<MAXUSERDEF) {
        String descr = GetUserdefName (userdef[i].description=="" ? String(USERDEF_MUSTER)+IntToStr(i+1) : userdef[i].description);
        if (descr!="") {
            Settings settings;
            settings.SetCategory (String("Userdef")+IntToStr(i));
            settings.Save ("Description", descr);
            settings.Save ("Sizex", kette.count());
            settings.Save ("Sizey", schuesse.count());
            String data = "";
            for (int i=kette.a; i<=kette.b; i++)
                for (int j=schuesse.a; j<=schuesse.b; j++) {
                    char s = gewebe.feld.Get(i, j);
                    data += (s>0 ? char(s+'k') : 'k');
                }
            settings.Save ("Data", data);
            LoadUserdefMenu();
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::UserdefAddSelClick(TObject *Sender)
{
    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid() && selection.feld==GEWEBE) {
        if (selection.end.i-selection.begin.i+1>50 || selection.end.j-selection.begin.j+1>50) {
            Application->MessageBox (USERDEF_TOOLARGE, APP_TITLE, MB_OK);
            selection = savesel;
            return;
        }

        int i = SelectUserdef(USERDEF_SELMUSTER);
        if (i>=0 && i<MAXUSERDEF) {
            String descr = GetUserdefName (userdef[i].description=="" ? String(USERDEF_MUSTER)+IntToStr(i+1) : userdef[i].description);
            if (descr!="") {
                Settings settings;
                settings.SetCategory (String("Userdef")+IntToStr(i));
                settings.Save ("Description", descr);
                settings.Save ("Sizex", selection.end.i-selection.begin.i+1);
                settings.Save ("Sizey", selection.end.j-selection.begin.j+1);
                String data = "";
                for (int i=selection.begin.i; i<=selection.end.i; i++)
                    for (int j=selection.begin.j; j<=selection.end.j; j++) {
                        char s = gewebe.feld.Get(i, j);
                        data += (s>0 ? char(s+'k') : 'k');
                    }
                settings.Save ("Data", data);
                LoadUserdefMenu();
                SetCursor (selection.end.i+1, selection.begin.j, true);
            }
        }
    }
    selection = savesel;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::UserdefRemoveClick(TObject *Sender)
{
    int i = SelectUserdef(USERDEF_SELDELETE);
    if (i>=0 && i<MAXUSERDEF) {
        Settings settings;
        settings.SetCategory (String("Userdef")+IntToStr(i));
        settings.Save ("Description", "");
        settings.Save ("Sizex", 0);
        settings.Save ("Sizey", 0);
        settings.Save ("Data", "");
    }
    LoadUserdefMenu();
}
/*-----------------------------------------------------------------*/
