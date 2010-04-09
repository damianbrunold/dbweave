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

#include <vcl\vcl.h>
#include <mem.h>
#pragma hdrstop

#include "assert.h"
#include "dbw3_form.h"
#include "dbw3_strings.h"
#include "felddef.h"
#include "datamodule.h"
#include "palette.h"
#include "version.h"
#include "properties.h"
#include "print.h"
#include "undoredo.h"
#include "cursor.h"

#include <limits>
#include <locale>
#include <algorithm>
#include <vector>
#include <string>
#include <istream>
#include <fstream>

using namespace std;

struct WifWeaving
{
	int shafts;
	int treadles;
	bool risingshed;
	WifWeaving() { shafts = treadles = 0; risingshed = false; }
};

struct WifWeftWarp
{
	int threads;
	int color;
	WifWeftWarp() { threads = 0; color = 1; }
};

struct WifColorPalette
{
	int entries;
	int beginrange;
	int endrange;
	WifColorPalette() { entries = beginrange = 0; endrange = 65535; }
};

struct WifColorEntry
{
	int red;
	int green;
	int blue;
	WifColorEntry() { red = green = blue = 0; }
};

struct WifThreadingEntry
{
    vector<int> harnesses;
};

struct WifThreading
{
    vector<WifThreadingEntry> entries;
};

struct WifTieupEntry
{
    vector<int> harnesses;
};

struct WifTieup
{
    vector<WifTieupEntry> entries;
};

struct WifTreadlingEntry
{
    vector<int> treadles;
};

struct WifTreadling
{
    vector<WifTreadlingEntry> entries;
};

class WifReader
{
private:
    TDBWFRM* frm;

	string filename;

	WifWeaving weaving;
	WifWeftWarp weft;
	WifWeftWarp warp;
	WifColorPalette colorpalette;
	vector<WifColorEntry> colortable;
    WifThreading threading;
    WifTieup tieup;
    WifTreadling treadling;

public:
    __fastcall WifReader (TDBWFRM* _frm);
    virtual __fastcall ~WifReader();
    void __fastcall Read (LPCSTR _filename);

protected:
	void FirstPass();
	void SecondPass();
	void ThirdPass();
    void CopyData();

    string GetToken (ifstream& strm);
	string GetField (string line);
	int    GetFieldEntryInt (string line, int entry);
	string GetFieldEntryString (string line);
	double GetFieldEntryDouble (string line);
	bool   GetFieldEntryBool (string line);

    string ReadContents (ifstream& strm);
    string ReadWeaving (ifstream& strm);
    string ReadWeft (ifstream& strm);
    string ReadWarp (ifstream& strm);
    string ReadColorPalette (ifstream& strm);
    string ReadColorTable (ifstream& strm);

    string ReadThreading (ifstream& strm);
    string ReadTreadling (ifstream& strm);
    string ReadLiftplan (ifstream& strm);
    string ReadTieup (ifstream& strm);

    vector<int> split(string line);
};

__fastcall WifReader::WifReader (TDBWFRM* _frm)
{
	frm = _frm;
}

__fastcall WifReader::~WifReader()
{
}

void __fastcall WifReader::Read (LPCSTR _filename)
{
	filename = _filename;
	FirstPass();
	SecondPass();
	ThirdPass();
    CopyData();
}

template <class charT, class traits> inline
basic_istream<charT, traits>& eatline (basic_istream<charT, traits>& strm)
{
	while (strm) {
		char c;
		strm >> c;
        if (c=='\n') break;
	}
	return strm;
}

string tolower(string s)
{
    for (unsigned int i=0; i<s.length(); i++)
        s[i] = tolower(s[i]);
	return s;
}

string WifReader::GetToken (ifstream& strm)
{
	string s = "";
	char c;
	while (strm) {
		strm >> c;
		if (c=='[') {
			string section;
			do {
				strm >> section;
				s += section;
			} while ((strm && s.length()==0) || s[s.length()-1]!=']');
			s = c + s;
			break;
		} else if (c!='\n') {
			strm >> eatline;
		}
	}

	return tolower(s);
}

string WifReader::GetField (string line)
{
	string::size_type idx = line.find('=');
	if (idx==string::npos) return "";
	return tolower(line.substr (0, idx));
}

int WifReader::GetFieldEntryInt (string line, int entry)
{
	string::size_type idx = line.find('=');
	if (idx==string::npos) return 0;
	for (string::size_type i=idx+1; i<line.length(); i++) {
		if (line[i]==',') {
			entry--;
			i++;
		}
		if (entry==0) {
			int nr = 0;
			while (i<line.length() && isdigit(line[i])) {
				nr = 10*nr + static_cast<int>(line[i]-'0');
				i++;
			}
            return nr;
		}
	}
	return 0;
}

vector<int> WifReader::split(string line) {
    vector<int> entries;
    string::size_type idx = line.find(',');
    while (idx != string::npos) {
        int n = atoi(line.substr(0, idx).c_str());
        entries.push_back(n);
        line = line.substr(idx + 1);
        idx = line.find(',');
    }
    if (line.length() > 0) {
        int n = atoi(line.c_str());
        entries.push_back(n);
    }
    return entries;
}

string WifReader::GetFieldEntryString (string line)
{
	string::size_type idx = line.find('=');
	if (idx==string::npos) return "";
	return line.substr(idx + 1);
}

double WifReader::GetFieldEntryDouble (string line)
{
	return 0.0;
}

bool WifReader::GetFieldEntryBool (string line)
{
	string::size_type idx = line.find('=');
	if (idx==string::npos) return 0;
	string data;
	while (++idx<line.length() && isalnum(line[idx])) {
		data += toupper(line[idx]);
	}
	return (data=="TRUE") || (data=="YES") || (data=="1") || (data=="ON");
}

string WifReader::ReadContents (ifstream& strm)
{
	return GetToken(strm);
}

string WifReader::ReadWeaving (ifstream& strm)
{
	char buff[250];
	while (strm) {
		strm.getline (buff, 250);
		string line = buff;
		if (line.length()>0 && line[0]=='[') {
			string::size_type idx = line.find(']');
			if (idx==string::npos) return "";
			else {
				return tolower(line.substr(0, idx+1));
			}
		}

		string field = GetField(line);
		if (field=="shafts") weaving.shafts = GetFieldEntryInt (line, 0);
		else if (field=="treadles") weaving.treadles = GetFieldEntryInt (line, 0);
		else if (field=="rising shed") weaving.risingshed = GetFieldEntryBool(line);
	}

	return GetToken(strm);
}

string WifReader::ReadWeft (ifstream& strm)
{
	char buff[250];
	while (strm) {
		strm.getline (buff, 250);
		string line = buff;
		if (line.length()>0 && line[0]=='[') {
			string::size_type idx = line.find(']');
			if (idx==string::npos) return "";
			else {
				return tolower(line.substr(0, idx+1));
			}
		}

		string field = GetField(line);
		if (field=="color") weft.color = GetFieldEntryInt (line, 0);
		else if (field=="threads") weft.threads = GetFieldEntryInt (line, 0);
	}

	return GetToken(strm);
}

string WifReader::ReadWarp (ifstream& strm)
{
	char buff[250];
	while (strm) {
		strm.getline (buff, 250);
		string line = buff;
		if (line.length()>0 && line[0]=='[') {
			string::size_type idx = line.find(']');
			if (idx==string::npos) return "";
			else {
				return tolower(line.substr(0, idx+1));
			}
		}

		string field = GetField(line);
		if (field=="color") warp.color = GetFieldEntryInt (line, 0);
		else if (field=="threads") warp.threads = GetFieldEntryInt (line, 0);
	}

	return GetToken(strm);
}

string WifReader::ReadColorPalette (ifstream& strm)
{
	char buff[250];
	while (strm) {
		strm.getline (buff, 250);
		string line = buff;
		if (line.length()>0 && line[0]=='[') {
			string::size_type idx = line.find(']');
			if (idx==string::npos) return "";
			else {
				return tolower(line.substr(0, idx+1));
			}
		}

		string field = GetField(line);
		if (field=="entries") colorpalette.entries = GetFieldEntryInt (line, 0);
		else if (field=="range") {
			colorpalette.beginrange = GetFieldEntryInt (line, 0);
			colorpalette.endrange = GetFieldEntryInt (line, 1);
		}
	}

	return GetToken(strm);
}

string WifReader::ReadColorTable (ifstream& strm)
{
	char buff[250];
	while (strm) {
		strm.getline (buff, 250);
		string line = buff;
		if (line.length()>0 && line[0]=='[') {
			string::size_type idx = line.find(']');
			if (idx==string::npos) return "";
			else {
				return tolower(line.substr(0, idx+1));
			}
		}

		string field = GetField(line);
		if (field=="entries") colorpalette.entries = GetFieldEntryInt (line, 0);
		else if (field=="range") {
			colorpalette.beginrange = GetFieldEntryInt (line, 0);
			colorpalette.endrange = GetFieldEntryInt (line, 1);
		}
	}

	return GetToken(strm);
}

void WifReader::FirstPass()
{
	try {
		ifstream strm(filename.c_str(), ios::in);
		strm >> noskipws;
		string s = GetToken (strm);
		while (s!="") {
            if (s=="[contents]") s = ReadContents(strm);
            else if (s=="[weaving]") s = ReadWeaving(strm);
            else if (s=="[color palette]") s = ReadColorPalette(strm);
			else s = GetToken(strm);
		}
	} catch (...) {
	}
}

string WifReader::ReadThreading (ifstream& strm)
{
	char buff[250];
	while (strm) {
		strm.getline (buff, 250);
		string line = buff;
		if (line.length()>0 && line[0]=='[') {
			string::size_type idx = line.find(']');
			if (idx==string::npos) return "";
			else {
				return tolower(line.substr(0, idx+1));
			}
		}

		int n = atoi(GetField(line).c_str()) - 1;
        WifThreadingEntry entry;
        entry.harnesses = split(GetFieldEntryString(line));
        threading.entries[n] = entry;
	}

	return GetToken(strm);
}

string WifReader::ReadTreadling (ifstream& strm)
{
	char buff[250];
	while (strm) {
		strm.getline (buff, 250);
		string line = buff;
		if (line.length()>0 && line[0]=='[') {
			string::size_type idx = line.find(']');
			if (idx==string::npos) return "";
			else {
				return tolower(line.substr(0, idx+1));
			}
		}

		int n = atoi(GetField(line).c_str()) - 1;
        WifTreadlingEntry entry;
        entry.treadles = split(GetFieldEntryString(line));
        treadling.entries[n] = entry;
	}

	return GetToken(strm);
}

string WifReader::ReadLiftplan (ifstream& strm)
{
	return GetToken(strm);
}

string WifReader::ReadTieup (ifstream& strm)
{
	char buff[250];
	while (strm) {
		strm.getline (buff, 250);
		string line = buff;
		if (line.length()>0 && line[0]=='[') {
			string::size_type idx = line.find(']');
			if (idx==string::npos) return "";
			else {
				return tolower(line.substr(0, idx+1));
			}
		}

		int n = atoi(GetField(line).c_str()) - 1;
        WifTieupEntry entry;
        entry.harnesses = split(GetFieldEntryString(line));
        tieup.entries[n] = entry;
	}

	return GetToken(strm);
}

void WifReader::SecondPass()
{
	try {
		ifstream strm(filename.c_str(), ios::in);
		strm >> noskipws;
		string s = GetToken (strm);
		while (s!="") {
            if (s=="[weft]") s = ReadWeft(strm);
            else if (s=="[warp]") s = ReadWarp(strm);
            else if (s=="[color table]") s = ReadColorTable(strm);
			else s = GetToken(strm);
		}
	} catch (...) {
	}
}

void WifReader::ThirdPass()
{
    threading.entries.resize(warp.threads);
    treadling.entries.resize(weft.threads);
    tieup.entries.resize(weaving.treadles);
	try {
		ifstream strm(filename.c_str(), ios::in);
		strm >> noskipws;
		string s = GetToken (strm);
		while (s!="") {
            if (s=="[threading]") s = ReadThreading(strm);
            else if (s=="[treadling]") s = ReadTreadling(strm);
            else if (s=="[liftplan]") s = ReadLiftplan(strm);
            else if (s=="[tieup]") s = ReadTieup(strm);
			else s = GetToken(strm);
		}
	} catch (...) {
	}
}

void WifReader::CopyData()
{
    frm->einzug.Clear();
    for (unsigned int i = 0; i < threading.entries.size(); i++) {
        int val = threading.entries[i].harnesses[0];
        frm->einzug.feld.Set(i, val);
    }
    frm->aufknuepfung.Clear();
    for (unsigned int i = 0; i < tieup.entries.size(); i++) {
        vector<int> harnesses = tieup.entries[i].harnesses;
        for (unsigned int j = 0; j < harnesses.size(); j++) {
            int val = harnesses[j];
            frm->aufknuepfung.feld.Set(i, val - 1, 1);
        }
    }
    frm->trittfolge.Clear();
    for (unsigned int i = 0; i < treadling.entries.size(); i++) {
        vector<int> treadles = treadling.entries[i].treadles;
        for (unsigned int j = 0; j < treadles.size(); j++) {
            int val = treadles[j];
            frm->trittfolge.feld.Set(val - 1, i, 1);
        }
    }
    frm->RecalcGewebe();
    frm->SetModified();
}

void __fastcall TDBWFRM::ImportWIFClick(TObject *Sender)
{
    Data->ImportDialog->FileName = "*.wif";

    // Voreinstellung des Filters
    Data->ImportDialog->FilterIndex = 1;

    if (Data->ImportDialog->Execute()) {
        // Eventuelle Änderungen speichern
        if (!AskSave()) return;

        // Daten aus WIF-File importieren
        TCursor oldCursor = Cursor;
        Cursor = crHourGlass;
        WifReader reader (this);
        reader.Read(Data->ImportDialog->FileName.c_str());
        Cursor = oldCursor;

        filename = "";
        Data->SaveDialog->FileName = DATEI_UNBENANNT;
        Data->OpenDialog->FilterIndex = 1;
        Data->SaveDialog->FilterIndex = 1;
    	saved = false;
        mousedown = false;
        SetAppTitle();
        FinalizeLoad();
        if (!righttoleft) sb_horz1->Position = scroll_x1;
        else sb_horz1->Position = sb_horz1->Max - scroll_x1;
        if (toptobottom) sb_vert1->Position = scroll_y1;
        else sb_vert1->Position = sb_vert1->Max - scroll_y1;
        UpdateScrollbars();
        dbw3_assert(cursorhandler);
        cursorhandler->CheckCursorPos();
        if (undo) undo->Clear();
        Invalidate();
        if (undo) undo->Snapshot();
    }
}


