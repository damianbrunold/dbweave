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
#include <stdio.h>
#include "print.h"

class WifWriter
{
private:
    TDBWFRM* frm;
    FILE* file;

    int firstschaft;
    int lastschaft;
    int firsttritt;
    int lasttritt;
    int firstschussfaden;
    int lastschussfaden;
    int firstkettfaden;
    int lastkettfaden;

public:
    __fastcall WifWriter (TDBWFRM* _frm);
    virtual __fastcall ~WifWriter();
    void __fastcall Write (LPCSTR _filename);

protected:
    void __fastcall CalcDimensions();
    bool __fastcall OpenFile (LPCSTR _filename);
    void __fastcall CloseFile();
    void __fastcall WriteSection (LPCSTR _section);
    void __fastcall EndSection();
    void __fastcall WriteEntry (LPCSTR _entry, LPCSTR _data);
    void __fastcall WriteEntry (int _entry, LPCSTR _data);
    void __fastcall WriteEntry (LPCSTR _entry, int _data);
    void __fastcall WriteEntry (int _entry, int _data);
    void __fastcall WriteColorpalette();
    void __fastcall WriteText();
    void __fastcall WriteWeaving();
    void __fastcall WriteWarp();
    void __fastcall WriteWeft();
    void __fastcall WriteNotes();
    void __fastcall WriteTieup();
    void __fastcall WriteColortable();
    void __fastcall WriteThreading();
    void __fastcall WriteWarpcolors();
    void __fastcall WriteTreadling();
    void __fastcall WriteLiftplan();
    void __fastcall WriteWeftcolors();
    void __fastcall WriteBlatteinzug();
};

__fastcall WifWriter::WifWriter (TDBWFRM* _frm)
{
    frm = _frm;
    file = NULL;
}

__fastcall WifWriter::~WifWriter()
{
    CloseFile();
}

bool __fastcall WifWriter::OpenFile (LPCSTR _filename)
{
    file = fopen (_filename, "wt");
    return file!=NULL;
}

void __fastcall WifWriter::CloseFile()
{
    if (file) {
        fclose (file);
        file = NULL;
    }
}

void __fastcall WifWriter::WriteSection (LPCSTR _section)
{
    dbw3_assert(file!=NULL);
    fprintf(file, "[%s]\n", _section);
}

void __fastcall WifWriter::EndSection()
{
    dbw3_assert(file!=NULL);
    fprintf(file, "\n");
}

void __fastcall  WifWriter::WriteEntry (LPCSTR _entry, LPCSTR _data)
{
    dbw3_assert(file!=NULL);
    fprintf(file, "%s=%s\n", _entry, _data);
}

void __fastcall WifWriter::WriteEntry (int _entry, LPCSTR _data)
{
    dbw3_assert(file!=NULL);
    fprintf(file, "%d=%s\n", _entry, _data);
}

void __fastcall  WifWriter::WriteEntry (LPCSTR _entry, int _data)
{
    dbw3_assert(file!=NULL);
    fprintf(file, "%s=%d\n", _entry, _data);
}

void __fastcall WifWriter::WriteEntry (int _entry, int _data)
{
    dbw3_assert(file!=NULL);
    fprintf(file, "%d=%d\n", _entry, _data);
}

void __fastcall WifWriter::Write (LPCSTR _filename)
{
    if (!OpenFile (_filename)) {
        Application->MessageBox (COULD_NOT_EXPORT, APP_TITLE, MB_OK);
        return;
    }

    CalcDimensions();

    WriteSection ("WIF");
    WriteEntry ("Version", "1.1");
    WriteEntry ("Date", "April 20, 1997");
    WriteEntry ("Developers", "wif@mhsoft.com");
    WriteEntry ("Source Program", "DB-WEAVE");
    WriteEntry ("Source Version", ver_string2);
    EndSection();

    WriteSection ("CONTENTS");
    WriteEntry ("COLOR PALETTE", "yes");
    WriteEntry ("TEXT", "yes");
    WriteEntry ("WEAVING", "yes");
    WriteEntry ("WARP", "yes");
    WriteEntry ("WEFT", "yes");
    WriteEntry ("NOTES", "yes");
    if (!frm->ViewSchlagpatrone->Checked) {
        WriteEntry ("TIEUP", "yes");
    }
    WriteEntry ("COLOR TABLE", "yes");
    WriteEntry ("THREADING", "yes");
    WriteEntry ("WARP COLORS", "yes");
    if (frm->ViewSchlagpatrone->Checked) {
        WriteEntry ("LIFTPLAN", "yes");
    } else {
        WriteEntry ("TREADLING", "yes");
    }
    WriteEntry ("WEFT COLORS", "yes");
    EndSection();

    WriteColorpalette();
    WriteText();
    WriteWeaving();
    WriteWarp();
    WriteWeft();
    WriteNotes();
    WriteColortable();
    WriteThreading();
    WriteWarpcolors();
    if (frm->ViewSchlagpatrone->Checked) {
        WriteLiftplan();
    } else {
        WriteTreadling();
        WriteTieup();
    }
    WriteWeftcolors();
    WriteBlatteinzug();

    CloseFile();
}

void __fastcall WifWriter::CalcDimensions()
{
    // Schäfte
    firstschaft = lastschaft = 0;
    for (int i=0; i<Data->MAXY1; i++)
        if (!frm->freieschaefte[i]) {
            firstschaft = i;
            break;
        }
    for (int i=Data->MAXY1-1; i>=0; i--)
        if (!frm->freieschaefte[i]) {
            lastschaft = i;
            break;
        }

    // Tritte
    firsttritt = lasttritt = 0;
    for (int i=0; i<Data->MAXX2; i++)
        if (!frm->freietritte[i]) {
            firsttritt = i;
            break;
        }
    for (int i=Data->MAXX2-1; i>=0; i--)
        if (!frm->freietritte[i]) {
            lasttritt = i;
            break;
        }

    // Kettfäden
    firstkettfaden = frm->kette.a;
    lastkettfaden = frm->kette.b;

    // Schussfäden
    firstschussfaden = frm->schuesse.a;
    lastschussfaden = frm->schuesse.b;
}

void __fastcall WifWriter::WriteColorpalette()
{
    WriteSection("COLOR PALETTE");
    WriteEntry ("Entries", MAX_PAL_ENTRY);
    WriteEntry ("Range", "0,255");
    EndSection();
}

void __fastcall WifWriter::WriteText()
{
    WriteSection("TEXT");
    WriteEntry ("Title", frm->filename.c_str());
    WriteEntry ("Author", Data->properties->Author());
    EndSection();
}

void __fastcall WifWriter::WriteWeaving()
{
    WriteSection("WEAVING");

    WriteEntry("Shafts", lastschaft-firstschaft+1);
    WriteEntry("Treadles", lasttritt-firsttritt+1);
    WriteEntry ("Rising Shed", !frm->sinkingshed ? "yes" : "no");

    EndSection();
}

void __fastcall WifWriter::WriteWarp()
{
    WriteSection ("WARP");

    WriteEntry ("Threads", lastkettfaden-firstkettfaden+1);
    WriteEntry ("Color", DEFAULT_COLORV);

    EndSection();
}

void __fastcall WifWriter::WriteWeft()
{
    WriteSection ("WEFT");

    WriteEntry ("Threads", lastschussfaden-firstschussfaden+1);
    WriteEntry ("Color", DEFAULT_COLORH);

    EndSection();
}

void __fastcall WifWriter::WriteNotes()
{
    WriteSection ("NOTES");
    WriteEntry ("1", Data->properties->Remarks());
    EndSection();
}

void __fastcall WifWriter::WriteTieup()
{
    WriteSection ("TIEUP");

    for (int i=firsttritt; i<=lasttritt; i++) {
        String line = "";
        for (int j=firstschaft; j<=lastschaft; j++)
            if (frm->aufknuepfung.feld.Get(i, j)>0)
                line += String(",") + IntToStr(j-firstschaft+1);
        line = line.SubString (2, line.Length()-1);
        if (line!="") WriteEntry (i-firsttritt+1, line.c_str());
    }

    EndSection();
}

void __fastcall WifWriter::WriteColortable()
{
    WriteSection ("COLOR TABLE");

    for (int i=0; i<MAX_PAL_ENTRY; i++) {
        COLORREF col = Data->palette->GetColor(i);
        char szcol[25];
        wsprintf (szcol, "%d,%d,%d", GetRValue(col), GetGValue(col), GetBValue(col));
        WriteEntry (i+1, szcol);
    }

    EndSection();
}

void __fastcall WifWriter::WriteThreading()
{
    WriteSection ("THREADING");

    for (int i=firstkettfaden; i<=lastkettfaden; i++)
        WriteEntry (i-firstkettfaden+1, frm->einzug.feld.Get(i));

    EndSection();
}

void __fastcall WifWriter::WriteWarpcolors()
{
    WriteSection ("WARP COLORS");

    for (int i=firstkettfaden; i<=lastkettfaden; i++)
        WriteEntry (i-firstkettfaden+1, frm->kettfarben.feld.Get(i)+1);

    EndSection();
}

void __fastcall WifWriter::WriteTreadling()
{
    WriteSection ("TREADLING");

    for (int j=firstschussfaden; j<=lastschussfaden; j++) {
        String tritte = "";
        for (int i=firsttritt; i<=lasttritt; i++)
            if (frm->trittfolge.feld.Get(i, j)>0)
                tritte += String(",") + IntToStr(i+1);
        tritte = tritte.SubString (2, tritte.Length()-1);
        WriteEntry (j-firstschussfaden+1, tritte.c_str());
    }

    EndSection();
}

void __fastcall WifWriter::WriteLiftplan()
{
    WriteSection ("LIFTPLAN");

    for (int j=firstschussfaden; j<=lastschussfaden; j++) {
        String tritte = "";
        for (int i=firsttritt; i<=lasttritt; i++)
            if (frm->trittfolge.feld.Get(i, j)>0)
                tritte += String(",") + IntToStr(i-firsttritt+1);
        tritte = tritte.SubString (2, tritte.Length()-1);
        WriteEntry (j-firstschussfaden+1, tritte.c_str());
    }

    EndSection();
}

void __fastcall WifWriter::WriteWeftcolors()
{
    WriteSection ("WEFT COLORS");

    for (int i=firstschussfaden; i<=lastschussfaden; i++)
        WriteEntry (i-firstschussfaden+1, frm->schussfarben.feld.Get(i)+1);

    EndSection();
}

void __fastcall WifWriter::WriteBlatteinzug()
{
    WriteSection ("PRIVATE DBWEAVE BLATTEINZUG");

    for (int i=firstkettfaden; i<=lastkettfaden; i++)
        WriteEntry (i-firstkettfaden+1, frm->blatteinzug.feld.Get(i));

    EndSection();
}

void __fastcall TDBWFRM::DateiExportieren(const String& _filename)
{
    // Daten im WIF-Format abspeichern...
    TCursor oldCursor = Cursor;
    Cursor = crHourGlass;
    WifWriter writer (this);
    writer.Write(_filename.c_str());
    Cursor = oldCursor;
}

void __fastcall TDBWFRM::DateiExportClick(TObject *Sender)
{
    Data->ExportDialog->FileName = "";

    // Voreinstellung des Filters
    if (Sender==ExportBitmap) Data->ExportDialog->FilterIndex = 1;
    else if (Sender==ExportWIF) Data->ExportDialog->FilterIndex = 2;
    else if (Sender==ExportDBW35) Data->ExportDialog->FilterIndex = 3;
    else Data->ExportDialog->FilterIndex = 1;

    if (Data->ExportDialog->Execute()) {
        if (!CheckExistence (Data->ExportDialog->FileName)) return;
        if (Data->ExportDialog->FilterIndex==2) DateiExportieren(Data->ExportDialog->FileName);
        else if (Data->ExportDialog->FilterIndex==3) {
            // Im alten (ungebrochenen) 35er Format exportieren
            String oldfilename = filename;
            filename = Data->ExportDialog->FileName;
            CloseDataFile();
            Save(false);
            CloseDataFile();
            filename = oldfilename;
        } else if (Data->ExportDialog->FilterIndex==1) {
            // Als BMP-File speichern
            DoExportBitmap (Data->ExportDialog->FileName);
        }
        ::MessageBeep(MB_OK);
    }

}


