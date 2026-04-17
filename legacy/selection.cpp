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
#pragma hdrstop

#include "assert.h"
#include "dbw3_form.h"
#include "datamodule.h"
#include "felddef.h"
#include "undoredo.h"
#include "zentralsymm.h"
#include "dbw3_strings.h"

// Zu beachten: Die Werte in der Form-Variablen selection sind
// immer absolut. Das heisst, eine Selektion kann auch ausserhalb
// des aktuell dargestellten Bereiches liegen.

bool __fastcall RANGE::Valid()
{
    return begin.i>=0 && begin.j>=0 && end.i>-1 && end.j>-1 && feld!=INVALID;
}

void __fastcall RANGE::Normalize()
{
    int minx = min (begin.i, end.i);
    int miny = min (begin.j, end.j);
    int maxx = max (begin.i, end.i);
    int maxy = max (begin.j, end.j);

    begin = PT (minx, miny);
	end   = PT (maxx, maxy);
}

void __fastcall TDBWFRM::CalcSelectionRect (int& _x, int& _y, int& _xx, int& _yy)
{
    switch (selection.feld) {
        case EINZUG:
            if (toptobottom) {
                _y  = einzug.pos.y0 + (selection.end.j-scroll_y1+1)*einzug.gh;
                _yy = einzug.pos.y0 + (selection.begin.j-scroll_y1)*einzug.gh;
            } else {
                _y  = einzug.pos.y0 + einzug.pos.height - (selection.end.j-scroll_y1+1)*einzug.gh;
                _yy = einzug.pos.y0 + einzug.pos.height - (selection.begin.j-scroll_y1)*einzug.gh;
            }
            if (righttoleft) {
                _x  = einzug.pos.x0 + einzug.pos.width - (selection.begin.i-scroll_x1)*einzug.gw;
                _xx = einzug.pos.x0 + einzug.pos.width - (selection.end.i-scroll_x1+1)*einzug.gw;
            } else {
                _x  = einzug.pos.x0 + (selection.begin.i-scroll_x1)*einzug.gw;
                _xx = einzug.pos.x0 + (selection.end.i-scroll_x1+1)*einzug.gw;
            }
            break;

        case AUFKNUEPFUNG:
            if (toptobottom) {
                _y  = aufknuepfung.pos.y0 + (selection.end.j-scroll_y1+1)*aufknuepfung.gh;
                _yy = aufknuepfung.pos.y0 + (selection.begin.j-scroll_y1)*aufknuepfung.gh;
            } else {
                _y  = aufknuepfung.pos.y0 + aufknuepfung.pos.height - (selection.end.j-scroll_y1+1)*aufknuepfung.gh;
                _yy = aufknuepfung.pos.y0 + aufknuepfung.pos.height - (selection.begin.j-scroll_y1)*aufknuepfung.gh;
            }
            _x  = aufknuepfung.pos.x0 + (selection.begin.i-scroll_x2)*aufknuepfung.gw;
            _xx = aufknuepfung.pos.x0 + (selection.end.i-scroll_x2+1)*aufknuepfung.gw;
            break;

        case TRITTFOLGE:
            _x  = trittfolge.pos.x0 + (selection.begin.i-scroll_x2)*trittfolge.gw;
            _y  = trittfolge.pos.y0 + trittfolge.pos.height - (selection.end.j-scroll_y2+1)*trittfolge.gh;
            _xx = trittfolge.pos.x0 + (selection.end.i-scroll_x2+1)*trittfolge.gw;
            _yy = trittfolge.pos.y0 + trittfolge.pos.height - (selection.begin.j-scroll_y2)*trittfolge.gh;
            break;

        case GEWEBE:
            if (righttoleft) {
                _x  = gewebe.pos.x0+gewebe.pos.width - (selection.begin.i-scroll_x1)*gewebe.gw;
                _y  = gewebe.pos.y0 + gewebe.pos.height - (selection.end.j-scroll_y2+1)*gewebe.gh;
                _xx = gewebe.pos.x0+gewebe.pos.width - (selection.end.i-scroll_x1+1)*gewebe.gw;
                _yy = gewebe.pos.y0 + gewebe.pos.height - (selection.begin.j-scroll_y2)*gewebe.gh;
            } else {
                _x  = gewebe.pos.x0 + (selection.begin.i-scroll_x1)*gewebe.gw;
                _y  = gewebe.pos.y0 + gewebe.pos.height - (selection.end.j-scroll_y2+1)*gewebe.gh;
                _xx = gewebe.pos.x0 + (selection.end.i-scroll_x1+1)*gewebe.gw;
                _yy = gewebe.pos.y0 + gewebe.pos.height - (selection.begin.j-scroll_y2)*gewebe.gh;
            }
            break;

        case BLATTEINZUG:
            _y  = blatteinzug.pos.y0;
            _yy = blatteinzug.pos.y0 + blatteinzug.pos.height;
            if (righttoleft) {
                _x  = blatteinzug.pos.x0 + blatteinzug.pos.width - (selection.begin.i-scroll_x1)*blatteinzug.gw;
                _xx = blatteinzug.pos.x0 + blatteinzug.pos.width - (selection.end.i-scroll_x1+1)*blatteinzug.gw;
            } else {
                _x  = blatteinzug.pos.x0 + (selection.begin.i-scroll_x1)*blatteinzug.gw;
                _xx = blatteinzug.pos.x0 + (selection.end.i-scroll_x1+1)*blatteinzug.gw;
            }
            break;

        case KETTFARBEN:
            _y  = kettfarben.pos.y0;
            _yy = kettfarben.pos.y0 + kettfarben.pos.height;
            if (righttoleft) {
                _x  = kettfarben.pos.x0 + kettfarben.pos.width - (selection.begin.i-scroll_x1)*kettfarben.gw;
                _xx = kettfarben.pos.x0 + kettfarben.pos.width - (selection.end.i-scroll_x1+1)*kettfarben.gw;
            } else {
                _x  = kettfarben.pos.x0 + (selection.begin.i-scroll_x1)*kettfarben.gw;
                _xx = kettfarben.pos.x0 + (selection.end.i-scroll_x1+1)*kettfarben.gw;
            }
            break;

        case SCHUSSFARBEN:
            _x  = schussfarben.pos.x0;
            _y  = schussfarben.pos.y0 + schussfarben.pos.height - (selection.end.j-scroll_y2+1)*schussfarben.gh;
            _xx = schussfarben.pos.x0 + schussfarben.pos.width;
            _yy = schussfarben.pos.y0 + schussfarben.pos.height - (selection.begin.j-scroll_y2)*schussfarben.gh;
            break;

        default:
            _x = _y = _xx = _yy = 0;
    }
}

void __fastcall TDBWFRM::ClearSelection (bool _cleardata/*=true*/)
{
    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid()) {
        DeleteCursor();

        // Zuerst Rahmen neuzeichnen!
        Canvas->Pen->Color = clBtnShadow;
        int x, y, xx, yy;
        CalcSelectionRect (x, y, xx, yy);
        Canvas->MoveTo (x, y);
        Canvas->LineTo (xx, y);
        Canvas->LineTo (xx, yy);
        Canvas->LineTo (x, yy);
        Canvas->LineTo (x, y);

        for (int i=selection.begin.i; i<=selection.end.i; i++) {
            switch (selection.feld) {
                case GEWEBE:
                    if (righttoleft) {
                        int j1 = selection.begin.j - scroll_y2;
                        int j2 = selection.end.j - scroll_y2;
                        int ii = selection.end.i - (i-selection.begin.i) - scroll_x1;
                        if (j1-1>=0) DrawGewebeRahmen (ii, j1-1);
                        DrawGewebeRahmen (ii, j2);
                        if (j1-1>=0) DrawGewebe (ii, j1-1);
                        DrawGewebe (ii, j2);
                        if (j2-1>0) DrawGewebe (ii, j2-1);
                    } else {
                        if (selection.begin.j>scroll_y2)
                            DrawGewebeRahmen (i-scroll_x1, selection.begin.j-scroll_y2-1);
                        DrawGewebeRahmen (i-scroll_x1, selection.end.j-scroll_y2);
                        DrawGewebe (i-scroll_x1, selection.begin.j-scroll_y2);
                        if (selection.begin.j>scroll_y2)
                            DrawGewebe (i-scroll_x1, selection.begin.j-scroll_y2-1);
                        if (selection.end.j-scroll_y2+1<gewebe.pos.height/gewebe.gh)
                            DrawGewebe (i-scroll_x1, selection.end.j-scroll_y2+1);
                        DrawGewebe (i-scroll_x1, selection.end.j-scroll_y2);
                    }
                    break;
                case EINZUG:
                    if (toptobottom) {
                        if (selection.begin.j-scroll_y1<einzug.pos.height/einzug.gh)
                            DrawEinzugRahmen (i-scroll_x1, selection.begin.j-scroll_y1);
                        DrawEinzugRahmen (i-scroll_x1, selection.end.j-scroll_y1);
                    } else {
                        if (selection.begin.j-scroll_y1+1<einzug.pos.height/einzug.gh)
                            DrawEinzugRahmen (i-scroll_x1, selection.begin.j-scroll_y1+1);
                        DrawEinzugRahmen (i-scroll_x1, selection.end.j-scroll_y1);
                    }
                    break;
                case AUFKNUEPFUNG:
                    if (toptobottom) {
                        DrawAufknuepfungRahmen (i-scroll_x2, selection.begin.j-scroll_y1);
                        DrawAufknuepfungRahmen (i-scroll_x2, selection.end.j-scroll_y1);
                    } else {
                        if (selection.begin.j-scroll_y1+1<aufknuepfung.pos.height/aufknuepfung.gh)
                            DrawAufknuepfungRahmen (i-scroll_x2, selection.begin.j-scroll_y1+1);
                        DrawAufknuepfungRahmen (i-scroll_x2, selection.end.j-scroll_y1);
                    }
                    break;
                case TRITTFOLGE:
                    if (selection.begin.j-scroll_y2+1<trittfolge.pos.height/trittfolge.gh)
                        DrawTrittfolgeRahmen (i-scroll_x2, selection.begin.j-scroll_y2+1);
                    DrawTrittfolgeRahmen (i-scroll_x2, selection.begin.j-scroll_y2);
                    DrawTrittfolgeRahmen (i-scroll_x2, selection.end.j-scroll_y2);
                    break;
            }
        }

        for (int j=selection.end.j; j>=selection.begin.j; j--) {
            switch (selection.feld) {
                case GEWEBE:
                    if (righttoleft) {
                        int i1 = selection.begin.i - scroll_x1;
                        int i2 = selection.end.i - scroll_x1;
                        DrawGewebeRahmen (i1, j-scroll_y2);
                        DrawGewebeRahmen (i2, j-scroll_y2);
                        if (i1-1>=0) DrawGewebe (i1-1, j-scroll_y2);
                        DrawGewebe (i1, j-scroll_y2);
                        if (i2-1>=0) DrawGewebe (i2-1, j-scroll_y2);
                        DrawGewebe (i2, j-scroll_y2);
                    } else {
                        DrawGewebeRahmen (selection.begin.i-scroll_x1, j-scroll_y2);
                        DrawGewebe (selection.begin.i-scroll_x1, j-scroll_y2);
                        if (selection.begin.i-scroll_x1+1<gewebe.pos.width/gewebe.gw)
                            DrawGewebe (selection.begin.i-scroll_x1+1, j-scroll_y2);
                        if (selection.end.i-scroll_x1+1<gewebe.pos.width/gewebe.gw)
                            DrawGewebeRahmen (selection.end.i-scroll_x1+1, j-scroll_y2);
                        if (selection.end.i-scroll_x1+1<gewebe.pos.width/gewebe.gw)
                            DrawGewebe (selection.end.i-scroll_x1+1, j-scroll_y2);
                        if (selection.end.i-scroll_x1+2<gewebe.pos.width/gewebe.gw)
                            DrawGewebe (selection.end.i-scroll_x1+2, j-scroll_y2);
                    }
                    break;
                case EINZUG:
                    if (toptobottom) {
                        if (righttoleft) {
                            int i1 = selection.begin.i-scroll_x1;
                            int i2 = selection.end.i-scroll_x1;
                            if (i1-1>=0) DrawEinzugRahmen (i1-1, j-scroll_y1);
                            DrawEinzugRahmen (i2, j-scroll_y1);
                        } else {
                            int maxi = einzug.pos.width/einzug.gw;
                            int i1 = selection.begin.i-scroll_x1;
                            int i2 = selection.end.i-scroll_x1;
                            DrawEinzugRahmen (i1, j-scroll_y1);
                            if (i2+1<maxi) DrawEinzugRahmen (i2+1, j-scroll_y1);
                        }
                    } else {
                        if (righttoleft) {
                            int i1 = selection.begin.i-scroll_x1;
                            int i2 = selection.end.i-scroll_x1;
                            if (i1-1>=0) DrawEinzugRahmen (i1-1, j-scroll_y1);
                            DrawEinzugRahmen (i2, j-scroll_y1);
                        } else {
                            int maxi = einzug.pos.width/einzug.gw;
                            int i1 = selection.begin.i-scroll_x1;
                            int i2 = selection.end.i-scroll_x1;
                            DrawEinzugRahmen (i1, j-scroll_y1);
                            if (i2+1<maxi) DrawEinzugRahmen (i2+1, j-scroll_y1);
                        }
                    }
                    break;
                case AUFKNUEPFUNG:
                    DrawAufknuepfungRahmen (selection.begin.i-scroll_x2, j-scroll_y1);
                    if (selection.end.i-scroll_x2+1<aufknuepfung.pos.width/aufknuepfung.gw)
                        DrawAufknuepfungRahmen (selection.end.i-scroll_x2+1, j-scroll_y1);
                    break;
                case TRITTFOLGE:
                    DrawTrittfolgeRahmen (selection.begin.i-scroll_x2, j-scroll_y2);
                    if (selection.end.i-scroll_x2+1<trittfolge.pos.width/trittfolge.gw)
                        DrawTrittfolgeRahmen (selection.end.i-scroll_x2+1, j-scroll_y2);
                    break;
            }
        }

        // Würg...
        if (righttoleft && selection.feld==GEWEBE) {
            int i = selection.begin.i - 1;
            int j = selection.begin.j - 1;
            if (i>=0 && j>=0) DrawGewebe (i, j);
        }

        DrawCursor();
    }

    selection = savesel;

    // Nun Selektion löschen
    if (_cleardata) selection.Clear();
}

void __fastcall TDBWFRM::DrawSelection()
{
    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid()) {
        if (selection.feld!=GEWEBE || !GewebeNone->Checked) {
            DeleteCursor();

            int x, y, xx, yy;
            CalcSelectionRect (x, y, xx, yy);

            Canvas->Pen->Color = clWhite;
            Canvas->MoveTo (x, y);
            Canvas->LineTo (xx, y);
            Canvas->LineTo (xx, yy);
            Canvas->LineTo (x, yy);
            Canvas->LineTo (x, y);

            UpdateStatusBar();

            DrawCursor();
        }
    }

    selection = savesel;
}

void __fastcall TDBWFRM::ResizeSelection (int _i, int _j, FELD _feld, bool _square)
{
    if (!selection.Valid()) {
        selection.begin = selection.end = PT(_i, _j);
        selection.feld = _feld;
        DrawSelection();
    } else if (_feld==selection.feld) {
        if (_square) {
            int di = abs(_i-selection.begin.i);
            int dj = abs(_j-selection.begin.j);
            int d = di-dj;
            if (d>0) {
                if (_i>selection.begin.i) _i -= d;
                else _i += d;
            } else if (d<0) {
                if (_j>selection.begin.j) _j += d;
                else _j -= d;
            }
        }
        if (selection.end.i!=_i || selection.end.j!=_j) {
            ClearSelection (false);
            selection.end = PT(_i, _j);
            DrawSelection();
        }
    }
}

void __fastcall TDBWFRM::CutSelection()
{
    RANGE savesel = selection;
	if (CopySelection (false)) {
        selection = savesel;
        EditDeleteClick(this);
        ClearSelection();
    }
}

void __fastcall TDBWFRM::GetSelectionLine (const RANGE& _selection, int _j, char* _buff)
{
    switch (selection.feld) {
        case GEWEBE:
            for (int i=_selection.begin.i; i<=_selection.end.i; i++)
                *_buff++ = (char)('k' + gewebe.feld.Get (i, _j));
            break;

        case TRITTFOLGE:
            for (int i=_selection.begin.i; i<=_selection.end.i; i++)
                *_buff++ = (char)('k' + trittfolge.feld.Get (i, _j));
            break;

        case AUFKNUEPFUNG:
            for (int i=_selection.begin.i; i<=_selection.end.i; i++)
                *_buff++ = (char)('k' + aufknuepfung.feld.Get (i, _j));
            break;

        case EINZUG:
            for (int i=_selection.begin.i; i<=_selection.end.i; i++)
                *_buff++ = (char)(einzug.feld.Get(i)==_j+1 ? 'l' : 'k');
            break;

        case BLATTEINZUG:
            for (int i=_selection.begin.i; i<=_selection.end.i; i++)
                *_buff++ = (char)(blatteinzug.feld.Get(i) ? 'l' : 'k');
            break;
    }
    *_buff = '\0';
}

bool __fastcall TDBWFRM::CopySelection (bool _movecursor/*=true*/)
{
    bool success = false;
    RANGE savesel = selection;
    selection.Normalize();

	if (selection.Valid()) {
        HGLOBAL hMem = GlobalAlloc (GMEM_DDESHARE|GMEM_MOVEABLE,
                            20+(selection.end.i-selection.begin.i+3)*(selection.end.j-selection.begin.j+1)+1);
        if (hMem) {
            try {
                //xxxx Könnte effizienter gemacht werden!
                char* buff = new char[selection.end.i-selection.begin.i+12];
                char* ptr = (char*)GlobalLock (hMem);
                lstrcpy (ptr, "dbw\r\n");
                wsprintf (buff, "%04d %04d\r\n", selection.end.i-selection.begin.i+1, selection.end.j-selection.begin.j+1);
                lstrcat (ptr, buff);
                for (int j=selection.begin.j; j<=selection.end.j; j++) {
                    buff[0] = 0;
                    GetSelectionLine (selection, selection.end.j-(j-selection.begin.j), buff);
                    lstrcat (ptr, buff);
                    lstrcat (ptr, "\r\n");
                }
                delete[] buff;
                GlobalUnlock (hMem);
                if (::OpenClipboard(Handle)) {
                    success = ::EmptyClipboard() && ::SetClipboardData (CF_TEXT, hMem)!=NULL;
                    ::CloseClipboard();
                }
            } catch (...) {
            }
        }
	}

    if (_movecursor) {
        if (selection.feld==TRITTFOLGE) SetCursor (selection.begin.i, selection.end.j+1, true);
        else SetCursor (selection.end.i+1, selection.begin.j, true);
    } else selection = savesel;
    return success;
}

void __fastcall TDBWFRM::PasteSelection (bool _transparent)
{
    PT start;
    if (::OpenClipboard(NULL)) {
        HGLOBAL hMem = ::GetClipboardData (CF_TEXT);
        if (hMem) {
            char* ptr = (char*)GlobalLock(hMem);
            if (strstr(ptr, "dbw")==ptr) {
                int x = atoi(ptr+5);
                int y = atoi(ptr+5+5);
                ptr += 16;
                switch (kbd_field) {
                    case GEWEBE:
                        for (int j=y-1; j>=0; j--) {
                            int jj = scroll_y2+gewebe.kbd.j+j;
                            if (jj<Data->MAXY2) {
                                for (int i=0; i<x; i++) {
                                    int ii = scroll_x1+gewebe.kbd.i+i;
                                    if (ii>=Data->MAXX1) break;
                                    if (ptr[i]!='k' || !_transparent)
                                        gewebe.feld.Set (ii, jj, (char)(ptr[i]-'k'));
                                }
                            }
                            ptr += x+2;
                        }
                        RecalcAll();
                        start = PT(scroll_x1+gewebe.kbd.i, scroll_y2+gewebe.kbd.j);
                        SetCursor (scroll_x1+gewebe.kbd.i+x, scroll_y2+gewebe.kbd.j, true);
                        Invalidate();
                        selection.begin = start;
                        selection.end.i = selection.begin.i + x - 1;
                        selection.end.j = selection.begin.j + y - 1;
                        if (selection.end.i>=scroll_x1+gewebe.pos.width/gewebe.gw) selection.end.i = scroll_x1 + gewebe.pos.width/gewebe.gw - 1;
                        if (selection.end.j>=scroll_y2+gewebe.pos.height/gewebe.gh) selection.end.j = scroll_y2 + gewebe.pos.height/gewebe.gh - 1;
                        selection.feld = GEWEBE;
                        break;

                    case EINZUG:
                        for (int j=y-1; j>=0; j--) {
                            int jj = scroll_y1+einzug.kbd.j+j;
                            if (jj<Data->MAXY2) {
                                for (int i=0; i<x; i++) {
                                    int ii = scroll_x1+einzug.kbd.i+i;
                                    if (ii>=Data->MAXX1) break;
                                    if (ptr[i]=='l')
                                        einzug.feld.Set (ii, (char)(ptr[i]=='l' ? jj+1 : 0));
                                }
                            }
                            ptr += x+2;
                        }
                        RecalcGewebe();
                        start = PT (scroll_x1+einzug.kbd.i, scroll_y1+einzug.kbd.j);
                        SetCursor (scroll_x1+einzug.kbd.i+x, scroll_y1+einzug.kbd.j, true);
                        Invalidate();
                        selection.begin = start;
                        selection.end.i = selection.begin.i + x - 1;
                        selection.end.j = selection.begin.j + y - 1;
                        if (selection.end.i>=scroll_x1+einzug.pos.width/einzug.gw) selection.end.i = scroll_x1 + einzug.pos.width/einzug.gw - 1;
                        if (selection.end.j>=scroll_y1+einzug.pos.height/einzug.gh) selection.end.j = scroll_y1 + einzug.pos.height/einzug.gh - 1;
                        selection.feld = EINZUG;
                        break;

                    case TRITTFOLGE:
                        for (int j=y-1; j>=0; j--) {
                            int jj = scroll_y2+trittfolge.kbd.j+j;
                            if (jj<Data->MAXY2) {
                                for (int i=0; i<x; i++) {
                                    int ii = scroll_x2+trittfolge.kbd.i+i;
                                    if (ii>=Data->MAXX2) break;
                                    if (ptr[i]!='k' || !_transparent)
                                        trittfolge.feld.Set (ii, jj, (char)(ptr[i]-'k'));
                                }
                                RecalcTrittfolgeEmpty(jj);
                            }
                            ptr += x+2;
                        }
                        RecalcGewebe();
                        start = PT(scroll_x2+trittfolge.kbd.i, scroll_y2+trittfolge.kbd.j);
                        SetCursor (scroll_x2+trittfolge.kbd.i, scroll_y2+trittfolge.kbd.j+y, true);
                        Invalidate();
                        selection.begin = start;
                        selection.end.i = selection.begin.i + x - 1;
                        selection.end.j = selection.begin.j + y - 1;
                        if (selection.end.i>=scroll_x2+trittfolge.pos.width/trittfolge.gw) selection.end.i = scroll_x2 + trittfolge.pos.width/trittfolge.gw - 1;
                        if (selection.end.j>=scroll_y2+trittfolge.pos.height/trittfolge.gh) selection.end.j = scroll_y2 + trittfolge.pos.height/trittfolge.gh - 1;
                        selection.feld = TRITTFOLGE;
                        break;

                    case AUFKNUEPFUNG:
                        for (int j=y-1; j>=0; j--) {
                            int jj = scroll_y1+aufknuepfung.kbd.j+j;
                            if (jj<Data->MAXY1) {
                                for (int i=0; i<x; i++) {
                                    int ii = scroll_x2+aufknuepfung.kbd.i+i;
                                    if (ii>=Data->MAXX2) break;
                                    if (ptr[i]!='k' || !_transparent)
                                        aufknuepfung.feld.Set (ii, jj, (char)(ptr[i]-'k'));
                                }
                            }
                            ptr += x+2;
                        }
                        RecalcGewebe();
                        start = PT(scroll_x2+aufknuepfung.kbd.i, scroll_y1+aufknuepfung.kbd.j);
                        SetCursor (scroll_x2+aufknuepfung.kbd.i+x, scroll_y1+aufknuepfung.kbd.j, true);
                        Invalidate();
                        selection.begin = start;
                        selection.end.i = selection.begin.i + x - 1;
                        selection.end.j = selection.begin.j + y - 1;
                        if (selection.end.i>=scroll_x2+aufknuepfung.pos.width/aufknuepfung.gw) selection.end.i = scroll_x2 + aufknuepfung.pos.width/aufknuepfung.gw - 1;
                        if (selection.end.j>=scroll_y1+aufknuepfung.pos.height/aufknuepfung.gh) selection.end.j = scroll_y1 + aufknuepfung.pos.height/aufknuepfung.gh - 1;
                        selection.feld = AUFKNUEPFUNG;
                        break;
                }
                CalcRangeSchuesse();
                CalcRangeKette();
                UpdateRapport();
                SetModified();
            }
            GlobalUnlock (hMem);
        }
        ::CloseClipboard();
    }
}

void __fastcall TDBWFRM::EditCutClick(TObject *Sender)
{
    CutSelection();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}

void __fastcall TDBWFRM::EditCopyClick(TObject *Sender)
{
    CopySelection();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}

void __fastcall TDBWFRM::EditPasteClick(TObject *Sender)
{
    PasteSelection (false);

    dbw3_assert (undo!=0);
    undo->Snapshot();
}

void __fastcall TDBWFRM::EditPasteTransparentClick(TObject *Sender)
{
    PasteSelection (true);

    dbw3_assert (undo!=0);
    undo->Snapshot();
}

void __fastcall TDBWFRM::EditInvertClick(TObject *Sender)
{
    int i, j;
    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid()) {
		switch (selection.feld) {
			case GEWEBE:
				for (i=selection.begin.i; i<=selection.end.i; i++) {
                	for (j=selection.begin.j; j<=selection.end.j; j++) {
                        char s = gewebe.feld.Get (i, j);
                        gewebe.feld.Set (i, j, char(s==0 ? currentrange : -s));
					}
				}
				RecalcAll();
				break;
			case AUFKNUEPFUNG:
				for (i=selection.begin.i; i<=selection.end.i; i++) {
                	for (j=selection.begin.j; j<=selection.end.j; j++) {
                        char s = aufknuepfung.feld.Get(i, j);
						aufknuepfung.feld.Set (i, j, char(s==0 ? currentrange : -s));
					}
				}
				RecalcGewebe();
				break;
			case TRITTFOLGE:
                if (ViewSchlagpatrone->Checked || !trittfolge.einzeltritt) {
                    for (j=selection.begin.j; j<=selection.end.j; j++) {
                        for (i=selection.begin.i; i<=selection.end.i; i++) {
                            char s = trittfolge.feld.Get(i, j);
                            trittfolge.feld.Set (i, j, char(s==0 ? currentrange : -s));
                        }
                    }
                    UpdateIsEmpty (selection.begin.j, selection.end.j);
                    RecalcGewebe();
                } else
                    ::MessageBeep (-1);
				break;
			default:
				::MessageBeep (-1);
				break;
		}

        CalcRangeSchuesse();
        CalcRangeKette();
        UpdateRapport();
    }

    selection = savesel;
	Invalidate();
    SetModified();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}

void __fastcall TDBWFRM::EditMirrorHorzClick(TObject *Sender)
{
    int i, j;
    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid()) {
		switch (selection.feld) {
			case GEWEBE:
				for (i=selection.begin.i; i<=(selection.begin.i+selection.end.i)/2; i++) {
                	for (j=selection.begin.j; j<=selection.end.j; j++) {
						char temp = gewebe.feld.Get (i, j);
						gewebe.feld.Set (i, j, gewebe.feld.Get(selection.end.i-i+selection.begin.i, j));
						gewebe.feld.Set (selection.end.i-i+selection.begin.i, j, temp);
					}
				}
				RecalcAll();
				break;
			case AUFKNUEPFUNG:
				for (i=selection.begin.i; i<=(selection.begin.i+selection.end.i)/2; i++) {
                	for (j=selection.begin.j; j<=selection.end.j; j++) {
						char temp = aufknuepfung.feld.Get (i, j);
						aufknuepfung.feld.Set (i, j, aufknuepfung.feld.Get(selection.end.i-i+selection.begin.i, j));
						aufknuepfung.feld.Set (selection.end.i-i+selection.begin.i, j, temp);
					}
				}
				RecalcGewebe();
				break;
			case EINZUG:
				for (i=selection.begin.i; i<=(selection.begin.i+selection.end.i)/2; i++) {
					short temp = einzug.feld.Get (i);
					einzug.feld.Set (i, einzug.feld.Get (selection.end.i-i+selection.begin.i));
					einzug.feld.Set (selection.end.i-i+selection.begin.i, temp);
				}
				//xxxx selection.cpp(EditMirrorHorzClick): Statt RecalcGewebe Kettfäden umkopieren
				RecalcGewebe();
				break;
			case TRITTFOLGE:
				for (i=selection.begin.i; i<=(selection.begin.i+selection.end.i)/2; i++) {
                	for (j=selection.begin.j; j<=selection.end.j; j++) {
						char temp = trittfolge.feld.Get (i, j);
						trittfolge.feld.Set (i, j, trittfolge.feld.Get(selection.end.i-i+selection.begin.i, j));
						trittfolge.feld.Set (selection.end.i-i+selection.begin.i, j, temp);
					}
   				}
                UpdateIsEmpty (selection.begin.j, selection.end.j);
				RecalcGewebe();
				break;
			default:
				::MessageBeep (-1);
				break;
		}

        CalcRangeSchuesse();
        CalcRangeKette();
        UpdateRapport();
    }

    selection = savesel;
	Invalidate();
    SetModified();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}

void __fastcall TDBWFRM::EditMirrorVertClick(TObject *Sender)
{
    int i, j;
    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid()) {
		switch (selection.feld) {
			case GEWEBE:
				for (i=selection.begin.i; i<=selection.end.i; i++) {
                	for (j=selection.begin.j; j<=(selection.begin.j+selection.end.j)/2; j++) {
						char temp = gewebe.feld.Get (i, j);
						gewebe.feld.Set (i, j, gewebe.feld.Get(i, selection.end.j-j+selection.begin.j));
						gewebe.feld.Set (i, selection.end.j-j+selection.begin.j, temp);
					}
				}
				RecalcAll();
				break;
			case AUFKNUEPFUNG:
				for (i=selection.begin.i; i<=selection.end.i; i++) {
                	for (j=selection.begin.j; j<=(selection.begin.j+selection.end.j)/2; j++) {
						char temp = aufknuepfung.feld.Get (i, j);
						aufknuepfung.feld.Set (i, j, aufknuepfung.feld.Get(i, selection.end.j-j+selection.begin.j));
						aufknuepfung.feld.Set (i, selection.end.j-j+selection.begin.j, temp);
					}
				}
				RecalcGewebe();
				break;
			case EINZUG:
				for (i=selection.begin.i; i<=selection.end.i; i++) {
					einzug.feld.Set (i, (short)(selection.end.j-einzug.feld.Get(i)+1+selection.begin.j+1));
				}
				//xxxx selection.cpp(EditMirrorVertClick): Statt RecalcGewebe Kettfäden neu berechnen
				RecalcGewebe();
				break;
			case TRITTFOLGE:
				for (i=selection.begin.i; i<=selection.end.i; i++) {
                	for (j=selection.begin.j; j<=(selection.begin.j+selection.end.j)/2; j++) {
						char temp = trittfolge.feld.Get (i, j);
						trittfolge.feld.Set (i, j, trittfolge.feld.Get(i, selection.end.j-j+selection.begin.j));
						trittfolge.feld.Set (i, selection.end.j-j+selection.begin.j, temp);
					}
				}
                UpdateIsEmpty (selection.begin.j, selection.end.j);
				RecalcGewebe();
				break;
			default:
				::MessageBeep (-1);
				break;
		}

        CalcRangeSchuesse();
        CalcRangeKette();
        UpdateRapport();
    }

    selection = savesel;
	Invalidate();
    SetModified();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}

void __fastcall TDBWFRM::EditRotateClick(TObject *Sender)
{
    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid()) {
		// Selektion muss quadratisch sein!
		if ((selection.end.i-selection.begin.i)!=(selection.end.j-selection.begin.j)) {
            ::MessageBeep (-1);
            goto g_exit;
        }
        try {
            int dx = selection.end.i-selection.begin.i+1;
            int dy = selection.end.j-selection.begin.j+1;
            char* buff = new char[dx*dy];
            switch (selection.feld) {
                case GEWEBE:
                    // Daten in Buffer kopieren
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        for (int j=selection.begin.j; j<=selection.end.j; j++) {
                            int ii = i - selection.begin.i;
                            int jj = j - selection.begin.j;
                            buff[ii+jj*dx] = gewebe.feld.Get(i, j);
                        }
                    // Daten rotiert zurückschreiben
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        for (int j=selection.begin.j; j<=selection.end.j; j++) {
                            int ii = i - selection.begin.i;
                            int jj = j - selection.begin.j;
                            gewebe.feld.Set (selection.begin.i+(j-selection.begin.j), selection.begin.j+(selection.end.i-i), buff[ii+jj*dx]);
                        }
                    // Neuberechnen und -zeichnen
                    RecalcAll();
                    Invalidate();
                    break;
                case AUFKNUEPFUNG:
                    // Daten in Buffer kopieren
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        for (int j=selection.begin.j; j<=selection.end.j; j++) {
                            int ii = i - selection.begin.i;
                            int jj = j - selection.begin.j;
                            buff[ii+jj*dx] = aufknuepfung.feld.Get(i, j);
                        }
                    // Daten rotiert zurückschreiben
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        for (int j=selection.begin.j; j<=selection.end.j; j++) {
                            int ii = i - selection.begin.i;
                            int jj = j - selection.begin.j;
                            aufknuepfung.feld.Set (selection.begin.i+(j-selection.begin.j), selection.begin.j+(selection.end.i-i), buff[ii+jj*dx]);
                        }
                    // Neuberechnen und -zeichnen
                    RecalcGewebe();
                    Invalidate();
                    break;
                case TRITTFOLGE:
                    if (!ViewSchlagpatrone->Checked && trittfolge.einzeltritt)
                        break;
                    // Daten in Buffer kopieren
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        for (int j=selection.begin.j; j<=selection.end.j; j++) {
                            int ii = i - selection.begin.i;
                            int jj = j - selection.begin.j;
                            buff[ii+jj*dx] = trittfolge.feld.Get(i, j);
                        }
                    // Daten rotiert zurückschreiben
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        for (int j=selection.begin.j; j<=selection.end.j; j++) {
                            int ii = i - selection.begin.i;
                            int jj = j - selection.begin.j;
                            trittfolge.feld.Set (selection.begin.i+(j-selection.begin.j), selection.begin.j+(selection.end.i-i), buff[ii+jj*dx]);
                        }
                    // Neuberechnen und -zeichnen
                    UpdateIsEmpty (selection.begin.j, selection.end.j);
                    RecalcGewebe();
                    Invalidate();
                    break;
            }
            delete[] buff;
        } catch (...) {
        }

        CalcRangeSchuesse();
        CalcRangeKette();
        UpdateRapport();
	}
g_exit:
    selection = savesel;
	Invalidate();
    SetModified();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}

void __fastcall TDBWFRM::EditDeleteClick(TObject *Sender)
{
	int i, j;
    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid()) {
		switch (selection.feld) {
			case GEWEBE:
				for (i=selection.begin.i; i<=selection.end.i; i++) {
                	for (j=selection.begin.j; j<=selection.end.j; j++) {
						gewebe.feld.Set (i, j, 0);
					}
				}
				RecalcAll();
				break;
			case EINZUG:
				for (i=selection.begin.i; i<=selection.end.i; i++) {
					einzug.feld.Set (i, 0);
				}
				RecalcGewebe();
				break;
			case TRITTFOLGE:
				for (i=selection.begin.i; i<=selection.end.i; i++) {
                	for (j=selection.begin.j; j<=selection.end.j; j++) {
						trittfolge.feld.Set (i, j, 0);
					}
				}
                UpdateIsEmpty (selection.begin.j, selection.end.j);
				RecalcGewebe();
				break;
			case AUFKNUEPFUNG:
				for (i=selection.begin.i; i<=selection.end.i; i++) {
                	for (j=selection.begin.j; j<=selection.end.j; j++) {
						aufknuepfung.feld.Set (i, j, 0);
					}
				}
				RecalcGewebe();
				break;
			default:
				::MessageBeep (-1);
				break;
		}

        CalcRangeSchuesse();
        CalcRangeKette();
        UpdateRapport();
    }

    selection = savesel;
	Invalidate();
    SetModified();

    dbw3_assert (undo!=0);
    undo->Snapshot();
}

void __fastcall TDBWFRM::RollUpClick(TObject *Sender)
{
    RANGE savesel = selection;
    selection.Normalize();

    try {
        if (selection.Valid()) {
            char* data = new char[selection.end.i-selection.begin.i+1];
            switch (selection.feld) {
                case GEWEBE:
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        data[i-selection.begin.i] = gewebe.feld.Get (i, selection.end.j);
                    for (int j=selection.end.j; j>selection.begin.j; j--)
                        for (int i=selection.begin.i; i<=selection.end.i; i++)
                            gewebe.feld.Set (i, j, gewebe.feld.Get (i, j-1));
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        gewebe.feld.Set (i, selection.begin.j, data[i-selection.begin.i]);
                    RecalcAll();
                    break;
                case EINZUG: {
                    int dy = abs(selection.end.j-selection.begin.j+1);
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        if (einzug.feld.Get(i))
                            einzug.feld.Set (i, short(selection.begin.j+1+((einzug.feld.Get(i)-1-selection.begin.j+1)%dy)));
                    RecalcGewebe();
                    break;
                }
                case TRITTFOLGE:
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        data[i-selection.begin.i] = trittfolge.feld.Get (i, selection.end.j);
                    for (int j=selection.end.j; j>selection.begin.j; j--)
                        for (int i=selection.begin.i; i<=selection.end.i; i++)
                            trittfolge.feld.Set (i, j, trittfolge.feld.Get (i, j-1));
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        trittfolge.feld.Set (i, selection.begin.j, data[i-selection.begin.i]);
                    UpdateIsEmpty (selection.begin.j, selection.end.j);
                    RecalcGewebe();
                    break;
                case AUFKNUEPFUNG:
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        data[i-selection.begin.i] = aufknuepfung.feld.Get (i, selection.end.j);
                    for (int j=selection.end.j; j>selection.begin.j; j--)
                        for (int i=selection.begin.i; i<=selection.end.i; i++)
                            aufknuepfung.feld.Set (i, j, aufknuepfung.feld.Get (i, j-1));
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        aufknuepfung.feld.Set (i, selection.begin.j, data[i-selection.begin.i]);
                    RecalcGewebe();
                    break;
                default:
                    ::MessageBeep (-1);
                    break;
            }

            CalcRangeSchuesse();
            CalcRangeKette();
            UpdateRapport();
            delete[] data;

            Invalidate();
            SetModified();

            dbw3_assert (undo!=0);
            undo->Snapshot();
        }
    } catch (...) {
    }

    selection = savesel;
}

void __fastcall TDBWFRM::RollDownClick(TObject *Sender)
{
    RANGE savesel = selection;
    selection.Normalize();

    try {
        if (selection.Valid()) {
            char* data = new char[selection.end.i-selection.begin.i+1];
            switch (selection.feld) {
                case GEWEBE:
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        data[i-selection.begin.i] = gewebe.feld.Get (i, selection.begin.j);
                    for (int j=selection.begin.j; j<selection.end.j; j++)
                        for (int i=selection.begin.i; i<=selection.end.i; i++)
                            gewebe.feld.Set (i, j, gewebe.feld.Get (i, j+1));
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        gewebe.feld.Set (i, selection.end.j, data[i-selection.begin.i]);
                    RecalcAll();
                    break;
                case EINZUG: {
                    int dy = abs(selection.end.j-selection.begin.j+1);
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        if (einzug.feld.Get(i))
                            einzug.feld.Set (i, short(selection.begin.j+1+((einzug.feld.Get(i)-1-selection.begin.j-1+dy)%dy)));
                    RecalcGewebe();
                    break;
                }
                case TRITTFOLGE:
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        data[i-selection.begin.i] = trittfolge.feld.Get (i, selection.begin.j);
                    for (int j=selection.begin.j; j<selection.end.j; j++)
                        for (int i=selection.begin.i; i<=selection.end.i; i++)
                            trittfolge.feld.Set (i, j, trittfolge.feld.Get (i, j+1));
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        trittfolge.feld.Set (i, selection.end.j, data[i-selection.begin.i]);
                    UpdateIsEmpty (selection.begin.j, selection.end.j);
                    RecalcGewebe();
                    break;
                case AUFKNUEPFUNG:
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        data[i-selection.begin.i] = aufknuepfung.feld.Get (i, selection.begin.j);
                    for (int j=selection.begin.j; j<selection.end.j; j++)
                        for (int i=selection.begin.i; i<=selection.end.i; i++)
                            aufknuepfung.feld.Set (i, j, aufknuepfung.feld.Get (i, j+1));
                    for (int i=selection.begin.i; i<=selection.end.i; i++)
                        aufknuepfung.feld.Set (i, selection.end.j, data[i-selection.begin.i]);
                    RecalcGewebe();
                    break;
                default:
                    ::MessageBeep (-1);
                    break;
            }

            CalcRangeSchuesse();
            CalcRangeKette();
            UpdateRapport();
            delete[] data;

            Invalidate();
            SetModified();

            dbw3_assert (undo!=0);
            undo->Snapshot();
        }
    } catch (...) {
    }

    selection = savesel;
}

void __fastcall TDBWFRM::RollLeftClick(TObject *Sender)
{
    RANGE savesel = selection;
    selection.Normalize();

    try {
        if (selection.Valid()) {
            char* data = new char[selection.end.j-selection.begin.j+1];
            switch (selection.feld) {
                case GEWEBE:
                    for (int j=selection.begin.j; j<=selection.end.j; j++)
                        data[j-selection.begin.j] = gewebe.feld.Get (selection.begin.i, j);
                    for (int i=selection.begin.i; i<selection.end.i; i++)
                        for (int j=selection.begin.j; j<=selection.end.j; j++)
                            gewebe.feld.Set (i, j, gewebe.feld.Get (i+1, j));
                    for (int j=selection.begin.j; j<=selection.end.j; j++)
                        gewebe.feld.Set (selection.end.i, j, data[j-selection.begin.j]);
                    RecalcAll();
                    break;
                case EINZUG: {
                    short i0 = einzug.feld.Get (selection.begin.i);
                    for (int i=selection.begin.i; i<selection.end.i; i++)
                        einzug.feld.Set (i, einzug.feld.Get (i+1));
                    einzug.feld.Set (selection.end.i, i0);
                    RecalcGewebe();
                    break;
                }
                case TRITTFOLGE:
                    for (int j=selection.begin.j; j<=selection.end.j; j++)
                        data[j-selection.begin.j] = trittfolge.feld.Get (selection.begin.i, j);
                    for (int i=selection.begin.i; i<selection.end.i; i++)
                        for (int j=selection.begin.j; j<=selection.end.j; j++)
                            trittfolge.feld.Set (i, j, trittfolge.feld.Get (i+1, j));
                    for (int j=selection.begin.j; j<=selection.end.j; j++)
                        trittfolge.feld.Set (selection.end.i, j, data[j-selection.begin.j]);
                    UpdateIsEmpty (selection.begin.j, selection.end.j);
                    RecalcGewebe();
                    break;
                case AUFKNUEPFUNG:
                    for (int j=selection.begin.j; j<=selection.end.j; j++)
                        data[j-selection.begin.j] = aufknuepfung.feld.Get (selection.begin.i, j);
                    for (int i=selection.begin.i; i<selection.end.i; i++)
                        for (int j=selection.begin.j; j<=selection.end.j; j++)
                            aufknuepfung.feld.Set (i, j, aufknuepfung.feld.Get (i+1, j));
                    for (int j=selection.begin.j; j<=selection.end.j; j++)
                        aufknuepfung.feld.Set (selection.end.i, j, data[j-selection.begin.j]);
                    RecalcGewebe();
                    break;
                default:
                    ::MessageBeep (-1);
                    break;
            }

            CalcRangeSchuesse();
            CalcRangeKette();
            UpdateRapport();
            delete[] data;

            Invalidate();
            SetModified();

            dbw3_assert (undo!=0);
            undo->Snapshot();
        }
    } catch (...) {
    }

    selection = savesel;
}

void __fastcall TDBWFRM::RollRightClick(TObject *Sender)
{
    RANGE savesel = selection;
    selection.Normalize();

    try {
        if (selection.Valid()) {
            char* data = new char[selection.end.j-selection.begin.j+1];
            switch (selection.feld) {
                case GEWEBE:
                    for (int j=selection.begin.j; j<=selection.end.j; j++)
                        data[j-selection.begin.j] = gewebe.feld.Get (selection.end.i, j);
                    for (int i=selection.end.i; i>selection.begin.i; i--)
                        for (int j=selection.begin.j; j<=selection.end.j; j++)
                            gewebe.feld.Set (i, j, gewebe.feld.Get (i-1, j));
                    for (int j=selection.begin.j; j<=selection.end.j; j++)
                        gewebe.feld.Set (selection.begin.i, j, data[j-selection.begin.j]);
                    RecalcAll();
                    break;
                case EINZUG: {
                    short i0 = einzug.feld.Get (selection.end.i);
                    for (int i=selection.end.i; i>selection.begin.i; i--)
                        einzug.feld.Set (i, einzug.feld.Get (i-1));
                    einzug.feld.Set (selection.begin.i, i0);
                    RecalcGewebe();
                    break;
                }
                case TRITTFOLGE:
                    for (int j=selection.begin.j; j<=selection.end.j; j++)
                        data[j-selection.begin.j] = trittfolge.feld.Get (selection.end.i, j);
                    for (int i=selection.end.i; i>selection.begin.i; i--)
                        for (int j=selection.begin.j; j<=selection.end.j; j++)
                            trittfolge.feld.Set (i, j, trittfolge.feld.Get (i-1, j));
                    for (int j=selection.begin.j; j<=selection.end.j; j++)
                        trittfolge.feld.Set (selection.begin.i, j, data[j-selection.begin.j]);
                    UpdateIsEmpty (selection.begin.j, selection.end.j);
                    RecalcGewebe();
                    break;
                case AUFKNUEPFUNG:
                    for (int j=selection.begin.j; j<=selection.end.j; j++)
                        data[j-selection.begin.j] = aufknuepfung.feld.Get (selection.end.i, j);
                    for (int i=selection.end.i; i>selection.begin.i; i--)
                        for (int j=selection.begin.j; j<=selection.end.j; j++)
                            aufknuepfung.feld.Set (i, j, aufknuepfung.feld.Get (i-1, j));
                    for (int j=selection.begin.j; j<=selection.end.j; j++)
                        aufknuepfung.feld.Set (selection.begin.i, j, data[j-selection.begin.j]);
                    RecalcGewebe();
                    break;
                default:
                    ::MessageBeep (-1);
                    break;
            }

            CalcRangeSchuesse();
            CalcRangeKette();
            UpdateRapport();
            delete[] data;

            SetModified();
            Invalidate();

            dbw3_assert (undo!=0);
            undo->Snapshot();
        }
    } catch (...) {
    }

    selection = savesel;
}

void __fastcall TDBWFRM::EditCentralsymClick(TObject *Sender)
{
    RANGE savesel = selection;
    selection.Normalize();

    if (selection.Valid() &&
        (selection.feld==GEWEBE || selection.feld==AUFKNUEPFUNG || (selection.feld==TRITTFOLGE && (ViewSchlagpatrone->Checked || !trittfolge.einzeltritt)))) {
        int sizex = selection.end.i-selection.begin.i+1;
        int sizey = selection.end.j-selection.begin.j+1;
        int x = selection.begin.i;
        int y = selection.begin.j;

        ZentralSymmChecker symchecker(sizex, sizey);

        // Selektion reinkopieren
        if (selection.feld==GEWEBE) {
            for (int i=0; i<sizex; i++)
                for (int j=0; j<sizey; j++)
                    symchecker.SetData (i, j, gewebe.feld.Get(x+i, y+j));
        } else if (selection.feld==AUFKNUEPFUNG) {
            for (int i=0; i<sizex; i++)
                for (int j=0; j<sizey; j++)
                    symchecker.SetData (i, j, aufknuepfung.feld.Get(x+i, y+j));
        } else if (selection.feld==TRITTFOLGE) {
            for (int i=0; i<sizex; i++)
                for (int j=0; j<sizey; j++)
                    symchecker.SetData (i, j, trittfolge.feld.Get(x+i, y+j));
        }

        if (!symchecker.IsAlreadySymmetric()) {
            // Falls Symmetrie vorhanden Selektion entsprechend ändern
            if (symchecker.SearchSymmetry()) {
                if (selection.feld==GEWEBE) {
                    for (int i=0; i<sizex; i++)
                        for (int j=0; j<sizey; j++)
                            gewebe.feld.Set (x+i, y+j, symchecker.GetData(i, j));
                } else if (selection.feld==AUFKNUEPFUNG) {
                    for (int i=0; i<sizex; i++)
                        for (int j=0; j<sizey; j++)
                            aufknuepfung.feld.Set (x+i, y+j, symchecker.GetData(i, j));
                } else if (selection.feld==TRITTFOLGE) {
                    for (int i=0; i<sizex; i++)
                        for (int j=0; j<sizey; j++)
                            trittfolge.feld.Set (x+i, y+j, symchecker.GetData(i, j));
                    UpdateIsEmpty (selection.begin.j, selection.end.j);
                }
            } else {
                Application->MessageBox (NOCENTRALSYMMFOUND.c_str(), APP_TITLE, MB_OK);
            }

            if (selection.feld==GEWEBE) RecalcAll();
            else RecalcGewebe();
            CalcRangeSchuesse();
            CalcRangeKette();
            UpdateRapport();

            Invalidate();
            SetModified();
            dbw3_assert(undo);
            undo->Snapshot();
        }
    }

    selection = savesel;
}


