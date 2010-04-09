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
#include <mem.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "felddef.h"
#include "dbw3_strings.h"
#include "farbpalette_form.h"
#include "datamodule.h"
#include "properties.h"
#include "palette.h"
#include "fileformat.h"
#include "version.h"
#include "steuerung_form.h"
/*-----------------------------------------------------------------*/
#define OLDFILEFORMATVERSION "0001"
#define FILEFORMATVERSION "0002"
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::Save (bool _format37/*=true*/)
{
    if (filename.IsEmpty()) return false;

	try {

        // Datei erstellen
        if (!file->IsOpen()) {
            file->Open (filename.c_str(), FfOpenRead|FfOpenWrite|FfOpenOverwrite);
        } else {
            file->SeekBegin();
        }

        // Datei muss offen sein!
        if (!file->IsOpen()) return false;

        // FileWriter erstellen
        FfWriter writer;
        writer.BreakFields(_format37);
        writer.Assign (file);

        // Daten speichern
        bool result = SaveSignatur(&writer) &&
                      SaveVersion(&writer, _format37) &&
                      SaveFileinfo(&writer) &&
                      SaveData(&writer, _format37) &&
        			  SaveView(&writer, _format37) &&
                      SavePrint(&writer, _format37);

		file->SetEndOfFile();

		return result;

	} catch (...) {
		//xxx
		return false;
	}
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::SaveSignatur (FfWriter* _writer)
{
    dbw3_assert (_writer);
    _writer->WriteSignature();
    return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::SaveVersion (FfWriter* _writer, bool _format37)
{
    _writer->BeginSection ("version", "Dateiformat und Applikationsversion");
    _writer->WriteField ("fmt", _format37 ? FILEFORMATVERSION : OLDFILEFORMATVERSION);
    _writer->WriteField ("ver", ver_string);
    _writer->EndSection();
    return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::SaveFileinfo (FfWriter* _writer)
{
    dbw3_assert (_writer);
    Data->properties->Save (_writer);
    return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::SaveData (FfWriter* _writer, bool _format37)
{
    dbw3_assert (_writer);
    _writer->BeginSection ("data", "Daten");
        _writer->BeginSection ("size");
            _writer->WriteFieldInt ("maxx1", Data->MAXX1);
            _writer->WriteFieldInt ("maxy1", Data->MAXY1);
            _writer->WriteFieldInt ("maxx2", Data->MAXX2);
            _writer->WriteFieldInt ("maxy2", Data->MAXY2);
        _writer->EndSection();
        _writer->BeginSection ("fields");
            einzug.feld.Write ("einzug", _writer);
            aufknuepfung.feld.Write ("aufknuepfung", _writer);
            _writer->BeginSection ("trittfolge");
                trittfolge.feld.Write ("trittfolge", _writer);
                trittfolge.isempty.Write ("isempty", _writer);
            _writer->EndSection();
            kettfarben.feld.Write ("kettfarben", _writer);
            schussfarben.feld.Write ("schussfarben", _writer);
            blatteinzug.feld.Write ("blatteinzug", _writer);
            _writer->BeginSection ("fixeinzug");
                _writer->WriteFieldBinary ("fixeinzug", fixeinzug, sizeof(short)*Data->MAXX1);
                _writer->WriteFieldInt ("fixsize", fixsize);
                _writer->WriteFieldInt ("firstfree", firstfree);
            _writer->EndSection();
        _writer->EndSection();
        _writer->BeginSection ("webstuhl");
            int maxkl = _format37 ? 9 : 5;
            for (int i=0; i<maxkl; i++) {
                _writer->BeginSection (("klammer"+IntToStr(i)).c_str());
                    _writer->WriteFieldInt ("first", klammern[i].first);
                    _writer->WriteFieldInt ("last", klammern[i].last);
                    _writer->WriteFieldInt ("repetitions", klammern[i].repetitions);
                _writer->EndSection();
            }
            _writer->BeginSection ("current");
                _writer->WriteFieldInt ("position", STRGFRM->weave_position);
                _writer->WriteFieldInt ("klammer", STRGFRM->weave_klammer);
                _writer->WriteFieldInt ("repetition", STRGFRM->weave_repetition);
            _writer->EndSection();
            if (_format37) {
                _writer->BeginSection ("last");
                    _writer->WriteFieldInt ("position", STRGFRM->last_position);
                    _writer->WriteFieldInt ("klammer", STRGFRM->last_klammer);
                    _writer->WriteFieldInt ("repetition", STRGFRM->last_repetition);
                _writer->EndSection();
                _writer->BeginSection ("divers");
                    _writer->WriteFieldInt ("schussselected", STRGFRM->schussselected);
                    _writer->WriteFieldInt ("scrolly", STRGFRM->scrolly);
                    _writer->WriteFieldInt ("firstschuss", STRGFRM->firstschuss);
                    _writer->WriteFieldInt ("weaving", weaving);
                _writer->EndSection();
            }
        _writer->EndSection();
        Data->palette->Save (_writer, _format37);
        _writer->BeginSection ("blockmuster");
            _writer->WriteFieldBinary ("bindung0", (void*)blockmuster[0].Data(), blockmuster[0].DataSize());
            _writer->WriteFieldBinary ("bindung1", (void*)blockmuster[1].Data(), blockmuster[1].DataSize());
            _writer->WriteFieldBinary ("bindung2", (void*)blockmuster[2].Data(), blockmuster[2].DataSize());
            _writer->WriteFieldBinary ("bindung3", (void*)blockmuster[3].Data(), blockmuster[3].DataSize());
            _writer->WriteFieldBinary ("bindung4", (void*)blockmuster[4].Data(), blockmuster[4].DataSize());
            _writer->WriteFieldBinary ("bindung5", (void*)blockmuster[5].Data(), blockmuster[5].DataSize());
            _writer->WriteFieldBinary ("bindung6", (void*)blockmuster[6].Data(), blockmuster[6].DataSize());
            _writer->WriteFieldBinary ("bindung7", (void*)blockmuster[7].Data(), blockmuster[7].DataSize());
            _writer->WriteFieldBinary ("bindung8", (void*)blockmuster[8].Data(), blockmuster[8].DataSize());
            _writer->WriteFieldBinary ("bindung9", (void*)blockmuster[9].Data(), blockmuster[9].DataSize());
            _writer->WriteFieldInt ("einzugz", (int)einzugZ);
            _writer->WriteFieldInt ("trittfolgez", (int)trittfolgeZ);
        _writer->EndSection();
        _writer->BeginSection ("bereichmuster");
            _writer->WriteFieldBinary ("bindung0", (void*)bereichmuster[0].Data(), bereichmuster[0].DataSize());
            _writer->WriteFieldBinary ("bindung1", (void*)bereichmuster[1].Data(), bereichmuster[1].DataSize());
            _writer->WriteFieldBinary ("bindung2", (void*)bereichmuster[2].Data(), bereichmuster[2].DataSize());
            _writer->WriteFieldBinary ("bindung3", (void*)bereichmuster[3].Data(), bereichmuster[3].DataSize());
            _writer->WriteFieldBinary ("bindung4", (void*)bereichmuster[4].Data(), bereichmuster[4].DataSize());
            _writer->WriteFieldBinary ("bindung5", (void*)bereichmuster[5].Data(), bereichmuster[5].DataSize());
            _writer->WriteFieldBinary ("bindung6", (void*)bereichmuster[6].Data(), bereichmuster[6].DataSize());
            _writer->WriteFieldBinary ("bindung7", (void*)bereichmuster[7].Data(), bereichmuster[7].DataSize());
            _writer->WriteFieldBinary ("bindung8", (void*)bereichmuster[8].Data(), bereichmuster[8].DataSize());
            _writer->WriteFieldBinary ("bindung9", (void*)bereichmuster[9].Data(), bereichmuster[9].DataSize());
        _writer->EndSection();
        _writer->BeginSection ("hilfslinien");
            _writer->WriteFieldInt ("count", (int)hlines.GetCount());
            _writer->WriteFieldBinary ("list", (void*)hlines.Data(), hlines.DataSize());
        _writer->EndSection();
    _writer->EndSection();

    return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::SaveView (FfWriter* _writer, bool _format37)
{
    dbw3_assert (_writer);
    _writer->BeginSection ("view", "Ansicht");
        _writer->BeginSection ("general");
            _writer->WriteFieldInt ("strongcolor", strongclr);
            _writer->WriteFieldDouble ("faktor_kette", faktor_kette);
            _writer->WriteFieldDouble ("faktor_schuss", faktor_schuss);
            _writer->WriteFieldInt ("hebung", !sinkingshed);
            _writer->WriteFieldInt ("color", Data->color);
            _writer->WriteFieldInt ("zoom", currentzoom);
            _writer->WriteFieldInt ("viewpalette", ViewFarbpalette->Checked);
            _writer->WriteFieldInt ("viewpegplan", ViewSchlagpatrone->Checked);
            _writer->WriteFieldInt ("viewrapport", RappViewRapport->Checked);
            _writer->WriteFieldInt ("viewhlines", ViewHlines->Checked);
            if (_format37) {
                _writer->WriteFieldInt ("righttoleft", righttoleft);
                _writer->WriteFieldInt ("toptobottom", toptobottom);
            }
        _writer->EndSection();

        _writer->BeginSection ("einzug");
            _writer->WriteFieldInt ("visible", ViewEinzug->Checked);
            if (_format37) {
                _writer->WriteFieldInt ("down", einzugunten);
            }
            _writer->WriteFieldInt ("viewtype", einzug.darstellung);
            _writer->WriteFieldInt ("stronglinex", einzug.pos.strongline_x);
            _writer->WriteFieldInt ("strongliney", einzug.pos.strongline_y);
			_writer->WriteFieldInt ("hvisible", hvisible);
            int ezstyle;
            if (EzFixiert->Checked) ezstyle=0;
            else if (EzMinimalZ->Checked) ezstyle=1;
            else if (EzMinimalS->Checked) ezstyle=2;
            else if (EzGeradeZ->Checked) ezstyle=3;
            else if (EzGeradeS->Checked) ezstyle=4;
            else if (EzBelassen->Checked) ezstyle=5;
            else if (EzChorig2->Checked) ezstyle=6;
            else if (EzChorig3->Checked) ezstyle=7;
            _writer->WriteFieldInt ("style", ezstyle);
        _writer->EndSection();

        _writer->BeginSection ("aufknuepfung");
            _writer->WriteFieldInt ("viewtype", aufknuepfung.darstellung);
            _writer->WriteFieldInt ("stronglinex", aufknuepfung.pos.strongline_x);
            _writer->WriteFieldInt ("strongliney", aufknuepfung.pos.strongline_y);
        _writer->EndSection();

        _writer->BeginSection ("trittfolge");
            _writer->WriteFieldInt ("visible", ViewTrittfolge->Checked);
            _writer->WriteFieldInt ("viewtype", trittfolge.darstellung);
            _writer->WriteFieldInt ("stronglinex", trittfolge.pos.strongline_x);
            _writer->WriteFieldInt ("strongliney", trittfolge.pos.strongline_y);
            _writer->WriteFieldInt ("single", trittfolge.einzeltritt);
			_writer->WriteFieldInt ("wvisible", wvisible);
            int tfstyle;
            if (TfBelassen->Checked) tfstyle=0;
            else if (TfMinimalZ->Checked) tfstyle=1;
            else if (TfMinimalS->Checked) tfstyle=2;
            else if (TfGeradeZ->Checked) tfstyle=3;
            else if (TfGeradeS->Checked) tfstyle=4;
            else if (TfGesprungen->Checked) tfstyle=5;
            _writer->WriteFieldInt ("style", tfstyle);
        _writer->EndSection();

		_writer->BeginSection ("schlagpatrone");
			_writer->WriteFieldInt ("viewtype", schlagpatronendarstellung);
        _writer->EndSection();

        _writer->BeginSection ("gewebe");
            int g = 0;
            if (GewebeNormal->Checked) g = 0;
            else if (GewebeFarbeffekt->Checked) g = 1;
            else if (GewebeSimulation->Checked) g = 2;
			else if (GewebeNone->Checked) g = 3;
            _writer->WriteFieldInt ("state", g);
            _writer->WriteFieldInt ("locked", OptionsLockGewebe->Checked);
            _writer->WriteFieldInt ("stronglinex", gewebe.pos.strongline_x);
            _writer->WriteFieldInt ("strongliney", gewebe.pos.strongline_y);
            if (_format37) {
                _writer->WriteFieldInt ("withgrid", fewithraster);
            }
        _writer->EndSection();

        _writer->BeginSection ("blatteinzug");
            _writer->WriteFieldInt ("visible", ViewBlatteinzug->Checked);
        _writer->EndSection();

        _writer->BeginSection ("kettfarben");
            _writer->WriteFieldInt ("visible", ViewFarbe->Checked);
        _writer->EndSection();

        _writer->BeginSection ("schussfarben");
            _writer->WriteFieldInt ("visible", ViewFarbe->Checked);
        _writer->EndSection();

		_writer->BeginSection ("pagesetup");
			_writer->WriteFieldInt ("topmargin", borders.range.top);
			_writer->WriteFieldInt ("bottommargin", borders.range.bottom);
			_writer->WriteFieldInt ("leftmargin", borders.range.left);
			_writer->WriteFieldInt ("rightmargin", borders.range.right);
			_writer->WriteFieldInt ("headerheight", header.height);
			_writer->WriteField ("headertext", header.text.c_str());
			_writer->WriteFieldInt ("footerheight", footer.height);
			_writer->WriteField ("footertext", footer.text.c_str());
		_writer->EndSection();
    _writer->EndSection();
    return true;
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::SavePrint (FfWriter* _writer, bool _format37)
{
    (void)_format37;
    dbw3_assert (_writer);
    _writer->BeginSection ("printsettings", "Druckeinstellungen");
        _writer->BeginSection ("printrange");
            _writer->WriteFieldInt ("kettevon", printkette.a);
            _writer->WriteFieldInt ("kettebis", printkette.b);
            _writer->WriteFieldInt ("schuessevon", printschuesse.a);
            _writer->WriteFieldInt ("schuessebis", printschuesse.b);
            _writer->WriteFieldInt ("schaeftevon", printschaefte.a);
            _writer->WriteFieldInt ("schaeftebis", printschaefte.b);
            _writer->WriteFieldInt ("trittevon", printtritte.a);
            _writer->WriteFieldInt ("trittebis", printtritte.b);
        _writer->EndSection();
    _writer->EndSection();
    return true;
}
/*-----------------------------------------------------------------*/

