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
#include <vcl\registry.hpp>
/*-----------------------------------------------------------------*/
#include "dbw3_form.h"
#include "dbw3_strings.h"
#include "regbase.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::AddToMRU (const String& _filename)
{
    if (_filename=="") return;
    int i;

    // Wenn Datei schon in MRU: Eintrag nach oben schieben
    for (i=0; i<6; i++) {
        if (mru[i]==_filename) {
            if (i>0) {
                String temp = mru[i];
                for (int j=i; j>0; j--)
                    mru[j] = mru[j-1];
                mru[0] = temp;
            }
            UpdateMRU();
            SaveMRU();
            return;
        }
    }

    // Ansonsten wird alles um einen Platz nach unten
    // geschoben und der Dateiname im ersten Eintrag
    // vermerkt.
    for (i=5; i>0; i--)
        mru[i] = mru[i-1];
    mru[0] = _filename;

    UpdateMRU();
    SaveMRU();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::UpdateMRU()
{
    UpdateMRUMenu (1, FileMRU1, mru[0]);
    UpdateMRUMenu (2, FileMRU2, mru[1]);
    UpdateMRUMenu (3,FileMRU3, mru[2]);
    UpdateMRUMenu (4, FileMRU4, mru[3]);
    UpdateMRUMenu (5, FileMRU5, mru[4]);
    UpdateMRUMenu (6, FileMRU6, mru[5]);
    FileMRUSeparator->Visible = FileMRU1->Visible || FileMRU2->Visible ||
                                FileMRU3->Visible || FileMRU4->Visible ||
                                FileMRU5->Visible || FileMRU6->Visible;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::UpdateMRUMenu (int _item, TMenuItem* _menuitem, String _filename)
{
    _menuitem->Visible = _filename!="";

    //xxxy Eigene Dateien oder so?!
    // Bestimmen ob Datei im Daten-Verzeichnis ist, falls
    // nicht, ganzen Pfad anzeigen!
    String path = ExtractFilePath(_filename).LowerCase();
    String datapath = (ExtractFilePath(Application->ExeName)).LowerCase();
    if (path==datapath)
        _menuitem->Caption = (String)"&" + IntToStr(_item) + " " + ExtractFileName (_filename);
    else
        _menuitem->Caption = (String)"&" + IntToStr(_item) + " " + _filename;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FileMRU1Click(TObject *Sender)
{
    // MRU 1
    DateiLaden (mru[0], true);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FileMRU2Click(TObject *Sender)
{
    // MRU 2
    DateiLaden (mru[1], true);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FileMRU3Click(TObject *Sender)
{
    // MRU 3
    DateiLaden (mru[2], true);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FileMRU4Click(TObject *Sender)
{
    // MRU 4
    DateiLaden (mru[3], true);
}
//---------------------------------------------------------------------------
void __fastcall TDBWFRM::FileMRU5Click(TObject *Sender)
{
    // MRU 5
    DateiLaden (mru[4], true);
}
//---------------------------------------------------------------------------
void __fastcall TDBWFRM::FileMRU6Click(TObject *Sender)
{
    // MRU 6
    DateiLaden (mru[5], true);
}
/*-----------------------------------------------------------------*/
void WriteRegistryString (Registry::TRegistry* _registry, const String& _name, const String& _value)
{
    if (_value.IsEmpty()) _registry->DeleteValue (_name);
    else _registry->WriteString (_name, _value);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SaveMRU()
{
    try {
        Registry::TRegistry* registry = new Registry::TRegistry;
        if (registry->OpenKey (DBW_REGMRU, true)) {
            WriteRegistryString (registry, "mru0", mru[0]);
            WriteRegistryString (registry, "mru1", mru[1]);
            WriteRegistryString (registry, "mru2", mru[2]);
            WriteRegistryString (registry, "mru3", mru[3]);
            WriteRegistryString (registry, "mru4", mru[4]);
            WriteRegistryString (registry, "mru5", mru[5]);
            registry->CloseKey();
        }
        delete registry;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void ReadRegistryString (Registry::TRegistry* _registry, const String& _name, String& _value)
{
    if (!_registry->ValueExists (_name)) _value = "";
    else _value = _registry->ReadString (_name);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::LoadMRU()
{
    try {
        Registry::TRegistry* registry = new Registry::TRegistry;
        if (registry->OpenKey (DBW_REGMRU, true)) {
            ReadRegistryString (registry, "mru0", mru[0]);
            ReadRegistryString (registry, "mru1", mru[1]);
            ReadRegistryString (registry, "mru2", mru[2]);
            ReadRegistryString (registry, "mru3", mru[3]);
            ReadRegistryString (registry, "mru4", mru[4]);
            ReadRegistryString (registry, "mru5", mru[5]);
            registry->CloseKey();
        }
        delete registry;
    } catch (...) {
    }
}
//---------------------------------------------------------------------------