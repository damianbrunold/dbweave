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
#include "assert.h"
#include "dbw3_form.h"
#include "datamodule.h"
#include "farbpalette_form.h"
#include "toolpalette_form.h"
#include "felddef.h"
#include "cursor.h"
#include "language.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawDivDragH (int _x)
{
    TPenMode oldmode = Canvas->Pen->Mode;
    Canvas->Pen->Mode = pmNot;
    Canvas->MoveTo (_x, 0);
    Canvas->LineTo (_x, ClientHeight);
    Canvas->Pen->Mode = oldmode;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DrawDivDragV (int _y)
{
    TPenMode oldmode = Canvas->Pen->Mode;
    Canvas->Pen->Mode = pmNot;
    Canvas->MoveTo (0, _y);
    Canvas->LineTo (ClientWidth, _y);
    Canvas->Pen->Mode = oldmode;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::DivDragStatus()
{
    // Statusbar mit aktuellen Infos bestücken...
    String line;

    if (hDragging) {
        int i = (trittfolge.pos.x0+trittfolge.pos.width-lasthdrag)/trittfolge.gw;
        if (i>Data->MAXX2) i = Data->MAXX2;
        if (i<=0) {
            if (ViewSchlagpatrone->Checked) line = LANG_STR("Pegplan invisible", "Schlagpatrone unsichtbar");
            else line = LANG_STR("Treadling invisible", "Trittfolge unsichtbar");
        } else {
            line = LANG_STR("No of visible treadles: ", "Anzahl sichtbarer Tritte: ")+IntToStr(i);
        }
    }

    if (vDragging) {
        if (hDragging) line += ",  ";
        int j;
        if (einzugunten) j = (einzug.pos.y0+einzug.pos.height-lastvdrag)/einzug.gh;
        else j = (lastvdrag-einzug.pos.y0)/einzug.gh;
        if (j>Data->MAXY1) j = Data->MAXY1;
        if (j<=0) {
            line += LANG_STR("Threading invisible", "Einzug unsichtbar");
        } else {
            line += LANG_STR("No of visible harnesses: ", "Anzahl sichtbarer Schäfte: ")+IntToStr(j);
        }
    }

    Statusbar->Panels->Items[0]->Text = line;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (Button==mbLeft) {
        // Dividerdrag abklären
        hDragging = vDragging = false;
        if (einzugunten) {
            if (Y>=gewebe.pos.y0+gewebe.pos.height && Y<=blatteinzug.pos.y0) vDragging = true;
            else if (Y>=blatteinzug.pos.y0+blatteinzug.pos.height && Y<=einzug.pos.y0) vDragging = true;
            else if (Y>=blatteinzug.pos.y0 && Y<=blatteinzug.pos.y0+blatteinzug.pos.height && X>=blatteinzug.pos.x0+blatteinzug.pos.width) vDragging = true;
            if (!ViewEinzug->Checked && Y>=blatteinzug.pos.y0+blatteinzug.pos.height && Y<=kettfarben.pos.y0) vDragging = true;
        } else {
            if (Y>=einzug.pos.y0+einzug.pos.height && Y<=blatteinzug.pos.y0) vDragging = true;
            else if (Y>=blatteinzug.pos.y0+blatteinzug.pos.height && Y<=gewebe.pos.y0) vDragging = true;
            else if (Y>=blatteinzug.pos.y0 && Y<=blatteinzug.pos.y0+blatteinzug.pos.height && X>=blatteinzug.pos.x0+blatteinzug.pos.width) vDragging = true;
            if (!ViewEinzug->Checked && Y>=kettfarben.pos.y0+kettfarben.pos.height && Y<=blatteinzug.pos.y0) vDragging = true;
        }
        if (X>=gewebe.pos.x0+gewebe.pos.width && X<=trittfolge.pos.x0) hDragging = true;
        if (!ViewTrittfolge->Checked && X>=gewebe.pos.x0+gewebe.pos.width && X<=schussfarben.pos.x0) hDragging = true;

        if (hDragging || vDragging) {
            if (hDragging) { DrawDivDragH(X); lasthdrag = X; }
            if (vDragging) { DrawDivDragV(Y); lastvdrag = Y; }
            DivDragStatus();
            return;
        }

        // Bei Mausklick mit Shift oder Control
        // wird die Farbe selektiert!
        if (Shift.Contains (ssShift) || Shift.Contains (ssCtrl)) {
            FELD f;
            int i, j;
            Physical2Logical (X, Y, f, i, j);
            if (f==KETTFARBEN) {
                Data->color = kettfarben.feld.Get (i+scroll_x1);
                if (FarbPalette->Visible) FarbPalette->Invalidate();
                Statusbar->Invalidate();
                return;
            } else if (f==SCHUSSFARBEN) {
                Data->color = schussfarben.feld.Get (j+scroll_y2);
                if (FarbPalette->Visible) FarbPalette->Invalidate();
                Statusbar->Invalidate();
                return;
            }
        }

        // Eine eventuell bestehende Selektion löschen
        bSelectionCleared = selection.Valid();
        ClearSelection();

        dbw3_assert (!mousedown);

        md_pt.i = X;
        md_pt.j = Y;
        mousedown = true;
        MouseCapture = true;

        lastfarbei = lastfarbej = lastblatteinzugi = -1;

        Physical2Logical (X, Y, md_feld, md.i, md.j);
        if (md_feld==AUFKNUEPFUNG && ViewSchlagpatrone->Checked)
            return;
        // Cursor heranholen
        if (md_feld!=GEWEBE || tool==TOOL_NORMAL)
            switch (md_feld) {
                case KETTFARBEN: cursorhandler->SetCursor (md_feld, md.i+scroll_x1, md.j, false); break;
                case EINZUG: cursorhandler->SetCursor (md_feld, md.i+scroll_x1, md.j+scroll_y1, false); break;
                case BLATTEINZUG: cursorhandler->SetCursor (md_feld, md.i+scroll_x1, md.j, false); break;
                case GEWEBE: cursorhandler->SetCursor (md_feld, md.i+scroll_x1, md.j+scroll_y2, false); break;
                case AUFKNUEPFUNG: cursorhandler->SetCursor (md_feld, md.i+scroll_x2, md.j+scroll_y1, false); break;
                case TRITTFOLGE: cursorhandler->SetCursor (md_feld, md.i+scroll_x2, md.j+scroll_y2, false); break;
                case SCHUSSFARBEN: cursorhandler->SetCursor (md_feld, md.i+scroll_x2, md.j+scroll_y2, false); break;
            };
        if (md_feld==GEWEBE) {
            if (tool==TOOL_NORMAL)
                ResizeSelection (md.i+scroll_x1, md.j+scroll_y2, md_feld, Shift.Contains (ssCtrl));
            else {
                DrawDragTool (md.i, md.j, md.i, md.j);
                dragto = md;
            }
        } else if (md_feld==EINZUG) {
            ResizeSelection (md.i+scroll_x1, md.j+scroll_y1, md_feld, Shift.Contains (ssCtrl));
        } else if (md_feld==AUFKNUEPFUNG) {
            ResizeSelection (md.i+scroll_x2, md.j+scroll_y1, md_feld, Shift.Contains (ssCtrl));
        } else if (md_feld==TRITTFOLGE) {
            ResizeSelection (md.i+scroll_x2, md.j+scroll_y2, md_feld, Shift.Contains (ssCtrl));
        } else if (md_feld==KETTFARBEN) {
            SetKettfarben (md.i);
            lastfarbei = md.i;
        } else if (md_feld==SCHUSSFARBEN) {
            SetSchussfarben (md.j);
            lastfarbej = md.j;
        } else if (md_feld==BLATTEINZUG) {
            SetBlatteinzug (md.i);
            lastblatteinzugi = md.i;
        } else if (md_feld==HLINEHORZ1) {
            hline = hlines.GetLine (HL_VERT, HL_LEFT, md.i+scroll_x1);
            if (hline) hlinepredrag = true;
        } else if (md_feld==HLINEHORZ2) {
            hline = hlines.GetLine (HL_VERT, HL_RIGHT, md.i+scroll_x2);
            if (hline) hlinepredrag = true;
        } else if (md_feld==HLINEVERT1) {
            hline = hlines.GetLine (HL_HORZ, HL_TOP, md.j+scroll_y1);
            if (hline) hlinepredrag = true;
        } else if (md_feld==HLINEVERT2) {
            hline = hlines.GetLine (HL_HORZ, HL_BOTTOM, md.j+scroll_y2);
            if (hline) hlinepredrag = true;
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FormMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
    if (hDragging || vDragging) {
        if (hDragging) { DrawDivDragH(lasthdrag); DrawDivDragH(X); lasthdrag = X; }
        if (vDragging) { DrawDivDragV(lastvdrag); DrawDivDragV(Y); lastvdrag = Y; }
        DivDragStatus();
        return;
    }

    if (mousedown) {
        FELD f;
        int  i, j;
        Physical2Logical (X, Y, f, i, j);
        if (f==AUFKNUEPFUNG && ViewSchlagpatrone->Checked)
            return;
        // Cursor heranholen
        if (md_feld!=GEWEBE || tool==TOOL_NORMAL)
            switch (f) {
                case KETTFARBEN: cursorhandler->SetCursor (f, i+scroll_x1, j, false); break;
                case EINZUG: cursorhandler->SetCursor (f, i+scroll_x1, j+scroll_y1, false); break;
                case BLATTEINZUG: cursorhandler->SetCursor (f, i+scroll_x1, j, false); break;
                case GEWEBE: cursorhandler->SetCursor (f, i+scroll_x1, j+scroll_y2, false); break;
                case AUFKNUEPFUNG: cursorhandler->SetCursor (f, i+scroll_x2, j+scroll_y1, false); break;
                case TRITTFOLGE: cursorhandler->SetCursor (f, i+scroll_x2, j+scroll_y2, false); break;
                case SCHUSSFARBEN: cursorhandler->SetCursor (f, i+scroll_x2, j+scroll_y2, false); break;
            };
        if (md_feld==f) {
            if (md_feld==GEWEBE) {
                if (tool==TOOL_NORMAL)
                    ResizeSelection (i+scroll_x1, j+scroll_y2, f, Shift.Contains (ssCtrl));
                else {
                    DeleteDragTool (md.i, md.j, dragto.i, dragto.j);
                    dragto.i = i;
                    dragto.j = j;
                    if (ToolpaletteForm->toolConstrained->Down || Shift.Contains (ssCtrl)) {
                        int dx = abs(dragto.i-md.i);
                        int dy = abs(dragto.j-md.j);
                        if (tool==TOOL_LINE && 2*dy<dx) {
                            dragto.j = md.j;
                        } else if (tool==TOOL_LINE && 2*dx<dy) {
                            dragto.i = md.i;
                        } else {
                            int d = min(dx, dy);
                            if (dragto.i-md.i > d) dragto.i = md.i+d;
                            else if (md.i-dragto.i > d) dragto.i = md.i-d;
                            if (dragto.j-md.j > d) dragto.j = md.j+d;
                            else if (md.j-dragto.j > d) dragto.j = md.j-d;
                        }
                    }
                    DrawDragTool (md.i, md.j, dragto.i, dragto.j);
                }
                return;
            } else if (md_feld==EINZUG) {
                ResizeSelection (i+scroll_x1, j+scroll_y1, f, Shift.Contains (ssCtrl));
                return;
            } else if (md_feld==AUFKNUEPFUNG) {
                ResizeSelection (i+scroll_x2, j+scroll_y1, f, Shift.Contains (ssCtrl));
                return;
            } else if (md_feld==TRITTFOLGE) {
                ResizeSelection (i+scroll_x2, j+scroll_y2, f, Shift.Contains (ssCtrl));
                return;
            } else if (f==KETTFARBEN && lastfarbei!=i) {
                SetKettfarben (i);
                lastfarbei = i;
                return;
            } else if (f==SCHUSSFARBEN && lastfarbej!=j) {
                SetSchussfarben (j);
                lastfarbej = j;
                return;
            } else if (f==BLATTEINZUG && lastblatteinzugi!=i) {
                SetBlatteinzug (i);
                lastblatteinzugi = i;
                return;
            }
        }

        // Hilfslinien
        switch (f) {
            case HLINEHORZ1:
                if (hlinepredrag || hlinedrag) {
                    HLTYP typ = hline->typ;
                    HLFELD feld = hline->feld;
                    int pos = hline->pos;
                    DeleteHilfslinie (hline);
                    hline->typ = HL_VERT;
                    hline->feld = HL_LEFT;
                    if (pos!=i+scroll_x1) {
                        hlinepredrag = false;
                        hlinedrag = true;
                    }
                    hline->pos = i+scroll_x1;
                    DrawHilfslinie (hline);
                    Hilfslinie* hl = hlines.GetLine (typ, feld, pos);
                    if (hl) DrawHilfslinie (hl);
                }
                break;

            case HLINEHORZ2:
                if (hlinepredrag || hlinedrag) {
                    HLTYP typ = hline->typ;
                    HLFELD feld = hline->feld;
                    int pos = hline->pos;
                    DeleteHilfslinie (hline);
                    hline->typ = HL_VERT;
                    hline->feld = HL_RIGHT;
                    if (pos!=i+scroll_x2) {
                        hlinepredrag = false;
                        hlinedrag = true;
                    }
                    hline->pos = i+scroll_x2;
                    DrawHilfslinie (hline);
                    Hilfslinie* hl = hlines.GetLine (typ, feld, pos);
                    if (hl) DrawHilfslinie (hl);
                }
                break;

            case HLINEVERT1:
                if (hlinepredrag || hlinedrag) {
                    HLTYP typ = hline->typ;
                    HLFELD feld = hline->feld;
                    int pos = hline->pos;
                    DeleteHilfslinie (hline);
                    hline->typ = HL_HORZ;
                    hline->feld = HL_TOP;
                    if (pos!=j+scroll_y1) {
                        hlinepredrag = false;
                        hlinedrag = true;
                    }
                    hline->pos = j+scroll_y1;
                    DrawHilfslinie (hline);
                    Hilfslinie* hl = hlines.GetLine (typ, feld, pos);
                    if (hl) DrawHilfslinie (hl);
                }
                break;

            case HLINEVERT2:
                if (hlinepredrag || hlinedrag) {
                    HLTYP typ = hline->typ;
                    HLFELD feld = hline->feld;
                    int pos = hline->pos;
                    DeleteHilfslinie (hline);
                    hline->typ = HL_HORZ;
                    hline->feld = HL_BOTTOM;
                    if (pos!=j+scroll_y2) {
                        hlinepredrag = false;
                        hlinedrag = true;
                    }
                    hline->pos = j+scroll_y2;
                    DrawHilfslinie (hline);
                    Hilfslinie* hl = hlines.GetLine (typ, feld, pos);
                    if (hl) DrawHilfslinie (hl);
                }
                break;
        }
    } else {
        // Tips in Statusbar anzeigen
        FELD f;
        int  i, j;
        Physical2Logical (X, Y, f, i, j);
        if (f==SCHUSSFARBEN) {
            Statusbar->Panels->Items[0]->Text = String(LANG_STR("Weft thread ", "Schuss "))+IntToStr(j+scroll_y2+1)+LANG_STR(" has color ", " hat Farbe ")+IntToStr(schussfarben.feld.Get(j+scroll_y2));
        } else if (f==KETTFARBEN) {
            Statusbar->Panels->Items[0]->Text = String(LANG_STR("Warp thread ", "Kettfaden "))+IntToStr(i+scroll_x1+1)+LANG_STR(" has color ", " hat Farbe ")+IntToStr(kettfarben.feld.Get(i+scroll_x1));
        } else {
            UpdateStatusBar();
        }

        // Cursor für DividerDrag ändern
        if (Cursor!=crHourGlass) {
            bool dragNS = false;
            bool dragWE = false;

            // NS abklären
            if (einzugunten) {
                if (Y>=gewebe.pos.y0+gewebe.pos.height && Y<=blatteinzug.pos.y0) dragNS = true;
                else if (Y>=blatteinzug.pos.y0+blatteinzug.pos.height && Y<=einzug.pos.y0) dragNS = true;
                else if (Y>=blatteinzug.pos.y0 && Y<=blatteinzug.pos.y0+blatteinzug.pos.height && X>=blatteinzug.pos.x0+blatteinzug.pos.width) dragNS = true;
                if (!ViewEinzug->Checked && Y>=blatteinzug.pos.y0+blatteinzug.pos.height && Y<=kettfarben.pos.y0) dragNS = true;
            } else {
                if (Y>=einzug.pos.y0+einzug.pos.height && Y<=blatteinzug.pos.y0) dragNS = true;
                else if (Y>=blatteinzug.pos.y0+blatteinzug.pos.height && Y<=gewebe.pos.y0) dragNS = true;
                else if (Y>=blatteinzug.pos.y0 && Y<=blatteinzug.pos.y0+blatteinzug.pos.height && X>=blatteinzug.pos.x0+blatteinzug.pos.width) dragNS = true;
                if (!ViewEinzug->Checked && Y>=kettfarben.pos.y0+kettfarben.pos.height && Y<=blatteinzug.pos.y0) dragNS = true;
            }

            // WE abklären
            if (X>=gewebe.pos.x0+gewebe.pos.width && X<=trittfolge.pos.x0) dragWE = true;
            if (!ViewTrittfolge->Checked && X>=gewebe.pos.x0+gewebe.pos.width && X<=schussfarben.pos.x0) dragWE = true;

            if (dragNS && dragWE) {
                if (einzugunten) Cursor = crSizeNWSE;
                else Cursor = crSizeNESW;
            } else if (dragNS) Cursor = crSizeNS;
            else if (dragWE) Cursor = crSizeWE;
            else Cursor = crDefault;
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (Button==mbLeft) {
        if (hDragging || vDragging) {
            if (hDragging) {
                DrawDivDragH(lasthdrag);
                int i = (trittfolge.pos.x0+trittfolge.pos.width - X) / trittfolge.gw;
                if (i<=0 && ViewTrittfolge->Checked) {
                    // Trittfolge unsichtbar machen
                    wvisible = i = 12>Data->MAXX2 ? Data->MAXX2 : 12;
                    ToggleTrittfolge();
                    RecalcDimensions();
                    Invalidate();
                }
                if (i>Data->MAXX2) i = Data->MAXX2;
                if (i!=wvisible) {
                    if (!ViewTrittfolge->Checked) ToggleTrittfolge();
                    wvisible = i;
                    RecalcDimensions();
                    Invalidate();
                    Statusbar->Invalidate();
                }
            }
            if (vDragging) {
                DrawDivDragV(lastvdrag);
                if (einzugunten) {
                    int j = (einzug.pos.y0+einzug.pos.height - Y) / einzug.gh;
                    if (j<=0 && ViewEinzug->Checked) {
                        // Einzug unsichtbar machen
                        hvisible = j = 12>Data->MAXY1 ? Data->MAXY1 : 12;
                        ToggleEinzug();
                        RecalcDimensions();
                        Invalidate();
                    }
                    if (j>Data->MAXY1) j = Data->MAXY1;
                    if (j!=hvisible) {
                        if (!ViewEinzug->Checked) ToggleEinzug();
                        hvisible = j;
                        RecalcDimensions();
                        Invalidate();
                    }
                } else {
                    int j = (Y - einzug.pos.y0) / einzug.gh;
                    if (j<=0 && ViewEinzug->Checked) {
                        // Einzug unsichtbar machen
                        hvisible = j = 12>Data->MAXY1 ? Data->MAXY1 : 12;
                        ToggleEinzug();
                        RecalcDimensions();
                        Invalidate();
                    }
                    if (j>Data->MAXY1) j = Data->MAXY1;
                    if (j!=hvisible) {
                        if (!ViewEinzug->Checked) ToggleEinzug();
                        hvisible = j;
                        RecalcDimensions();
                        Invalidate();
                    }
                }
            }
            Cursor = crDefault;
            hDragging = vDragging = false;
            UpdateStatusBar();
            return;
        }

        if (!mousedown) return; //xxxx
        MouseCapture = false;
        mousedown = false;
        Cursor = crDefault;
        bool ctrl = Shift.Contains(ssCtrl);

        FELD f;
        int  i, j;
        Physical2Logical (X, Y, f, i, j);
        if (md_feld==AUFKNUEPFUNG && ViewSchlagpatrone->Checked)
            return;

        // Cursor heranholen
        switch (f) {
            case KETTFARBEN: cursorhandler->SetCursor (f, i+scroll_x1, j, false); break;
            case EINZUG: cursorhandler->SetCursor (f, i+scroll_x1, j+scroll_y1, false); break;
            case BLATTEINZUG: cursorhandler->SetCursor (f, i+scroll_x1, j, false); break;
            case GEWEBE: cursorhandler->SetCursor (f, i+scroll_x1, j+scroll_y2, false); break;
            case AUFKNUEPFUNG: cursorhandler->SetCursor (f, i+scroll_x2, j+scroll_y1, false); break;
            case TRITTFOLGE: cursorhandler->SetCursor (f, i+scroll_x2, j+scroll_y2, false); break;
            case SCHUSSFARBEN: cursorhandler->SetCursor (f, i+scroll_x2, j+scroll_y2, false); break;
        };

        if (f==GEWEBE && md_feld==GEWEBE && tool!=TOOL_NORMAL) {
            DeleteDragTool (md.i, md.j, dragto.i, dragto.j);
            dragto.i = i;
            dragto.j = j;
            if (ToolpaletteForm->toolConstrained->Down || Shift.Contains (ssCtrl)) {
                int dx = abs(dragto.i-md.i);
                int dy = abs(dragto.j-md.j);
                if (tool==TOOL_LINE && 2*dy<dx) {
                    dragto.j = md.j;
                } else if (tool==TOOL_LINE && 2*dx<dy) {
                    dragto.i = md.i;
                } else {
                    int d = min(dx, dy);
                    if (dragto.i-md.i > d) dragto.i = md.i+d;
                    else if (md.i-dragto.i > d) dragto.i = md.i-d;
                    if (dragto.j-md.j > d) dragto.j = md.j+d;
                    else if (md.j-dragto.j > d) dragto.j = md.j-d;
                }
            }
            if (md.i!=dragto.i || md.j!=dragto.j) {
                DrawTool (md.i, md.j, dragto.i, dragto.j);
                return;
            }
        }

        if (md_feld==f && md.i==i && md.j==j) {
            // Selektion löschen
            ClearSelection();

            // Wenn vor dem Mousedown eine Selektion bestanden
            // hat, wurde diese durch den Mausklick gelöscht.
            // Deshalb muss in dem Fall KEIN Feld getoggelt werden,
            // denn der User wollte nur die Selektion löschen...
            if (bSelectionCleared) {
                return;
            }

            // Feld toggeln
            switch (f) {
                case BLATTEINZUG:
                    cursorhandler->SetCursor (f, i+scroll_x1, j, false);
                    if (lastblatteinzugi!=i && !ctrl) SetBlatteinzug (i);
                    return;

                case KETTFARBEN:
                    cursorhandler->SetCursor (f, i+scroll_x1, j, false);
                    if (lastfarbei!=i && !ctrl) SetKettfarben (i);
                    return;

                case EINZUG:
                    cursorhandler->SetCursor (f, i+scroll_x1, j+scroll_y1, false);
                    if (!ctrl) SetEinzug (i, j);
                    cursorhandler->CheckLocked();
                    return;

                case GEWEBE:
                    cursorhandler->SetCursor (f, i+scroll_x1, j+scroll_y2, false);
                    if (!OptionsLockGewebe->Checked) {
                        if (!ctrl) SetGewebe (i, j, false, currentrange);
                        cursorhandler->CheckLocked();
                    } else
                        MessageBeep (MB_OK);
                    return;

                case AUFKNUEPFUNG:
                    cursorhandler->SetCursor (f, i+scroll_x2, j+scroll_y1, false);
                    if (!ctrl) SetAufknuepfung (i, j, false, currentrange);
                    return;

                case TRITTFOLGE:
                    cursorhandler->SetCursor (f, i+scroll_x2, j+scroll_y2, false);
                    if (!ctrl) SetTrittfolge (i, j, false, currentrange);
                    cursorhandler->CheckLocked();
                    return;

                case SCHUSSFARBEN:
                    cursorhandler->SetCursor (f, i, j+scroll_y2, false);
                    if (lastfarbej!=j && !ctrl) SetSchussfarben (j);
                    return;
            }
        }

        // Hilfslinien
        switch (f) {
            case HLINEHORZ1:
                if (!hlinedrag) {
                    Hilfslinie* pline = hlines.GetLine(HL_VERT, HL_LEFT, i+scroll_x1);
                    if (pline) {
                        DeleteHilfslinie (pline);
                        hlines.Delete (pline);
                    } else {
                        hlines.Add (HL_VERT, HL_LEFT, i+scroll_x1);
                        DrawHilfslinien();
                    }
                    SetModified();
                } else {
                    HLTYP typ = hline->typ;
                    HLFELD feld = hline->feld;
                    int pos = hline->pos;
                    DeleteHilfslinie (hline);
                    hline->typ = HL_VERT;
                    hline->feld = HL_LEFT;
                    hline->pos = i+scroll_x1;
                    DrawHilfslinie (hline);
                    Hilfslinie* hl = hlines.GetLine (typ, feld, pos);
                    if (hl) DrawHilfslinie (hl);
                }
                break;

            case HLINEHORZ2:
                if (!hlinedrag) {
                    Hilfslinie* pline = hlines.GetLine(HL_VERT, HL_RIGHT, i+scroll_x2);
                    if (pline) {
                        DeleteHilfslinie (pline);
                        hlines.Delete (pline);
                    } else {
                        hlines.Add (HL_VERT, HL_RIGHT, i+scroll_x2);
                        DrawHilfslinien();
                    }
                    SetModified();
                } else {
                    HLTYP typ = hline->typ;
                    HLFELD feld = hline->feld;
                    int pos = hline->pos;
                    DeleteHilfslinie (hline);
                    hline->typ = HL_VERT;
                    hline->feld = HL_RIGHT;
                    hline->pos = i+scroll_x2;
                    DrawHilfslinie (hline);
                    Hilfslinie* hl = hlines.GetLine (typ, feld, pos);
                    if (hl) DrawHilfslinie (hl);
                }
                break;

            case HLINEVERT1:
                if (!hlinedrag) {
                    Hilfslinie* pline = hlines.GetLine (HL_HORZ, HL_TOP, j+scroll_y1);
                    if (pline) {
                        DeleteHilfslinie (pline);
                        hlines.Delete (pline);
                    } else {
                        hlines.Add (HL_HORZ, HL_TOP, j+scroll_y1);
                        DrawHilfslinien();
                    }
                    SetModified();
                } else {
                    HLTYP typ = hline->typ;
                    HLFELD feld = hline->feld;
                    int pos = hline->pos;
                    DeleteHilfslinie (hline);
                    hline->typ = HL_HORZ;
                    hline->feld = HL_TOP;
                    hline->pos = j+scroll_y1;
                    DrawHilfslinie (hline);
                    Hilfslinie* hl = hlines.GetLine (typ, feld, pos);
                    if (hl) DrawHilfslinie (hl);
                }
                break;

            case HLINEVERT2:
                if (!hlinedrag) {
                    Hilfslinie* pline = hlines.GetLine (HL_HORZ, HL_BOTTOM, j+scroll_y2);
                    if (pline) {
                        DeleteHilfslinie (pline);
                        hlines.Delete (pline);
                    } else {
                        hlines.Add (HL_HORZ, HL_BOTTOM, j+scroll_y2);
                        DrawHilfslinien();
                    }
                    SetModified();
                } else {
                    HLTYP typ = hline->typ;
                    HLFELD feld = hline->feld;
                    int pos = hline->pos;
                    DeleteHilfslinie (hline);
                    hline->typ = HL_HORZ;
                    hline->feld = HL_BOTTOM;
                    hline->pos = j+scroll_y2;
                    DrawHilfslinie (hline);
                    Hilfslinie* hl = hlines.GetLine (typ, feld, pos);
                    if (hl) DrawHilfslinie (hl);
                }
                break;
            default:
                if ((hlinedrag || hlinepredrag) && hline) {
                    HLTYP typ = hline->typ;
                    HLFELD feld = hline->feld;
                    int pos = hline->pos;
                    DeleteHilfslinie (hline);
                    hlines.Delete (hline);
                    Hilfslinie* hl = hlines.GetLine (typ, feld, pos);
                    if (hl) DrawHilfslinie (hl);
                }
        }
        hlinedrag = false;
        hlinepredrag = false;
        hline = NULL;
    }
}
/*-----------------------------------------------------------------*/
bool __fastcall TDBWFRM::HitCheck (const GRIDPOS& _grid, int _x, int _y,
                                    int _scrollx, int _scrolly, int& _i, int& _j,
                                    int _gw, int _gh, bool _righttoleft/*=false*/,
                                    bool _toptobottom/*=false*/)
{
    if (_x>_grid.x0 && _x<_grid.x0+_grid.width &&
        _y>_grid.y0 && _y<_grid.y0+_grid.height)
    {
        if (_righttoleft) _i = (_grid.x0+_grid.width-_x)/_gw;
        else _i = (_x-_grid.x0)/_gw;
        if (_toptobottom) _j = (_y-_grid.y0)/_gh;
        else _j = (_grid.y0+_grid.height-_y)/_gh;
        return true;
    }
    return false;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::Physical2Logical (int _x, int _y, FELD& _feld, int& _i, int& _j)
{
    bool rtl = righttoleft;
    bool ttb = toptobottom;
    if (HitCheck (blatteinzug.pos, _x, _y, scroll_x1, 0, _i, _j, blatteinzug.gw, blatteinzug.gh, rtl)) _feld = BLATTEINZUG;
    else if (HitCheck (kettfarben.pos, _x, _y, scroll_x1, 0, _i, _j, kettfarben.gw, kettfarben.gh, rtl)) _feld = KETTFARBEN;
    else if (HitCheck (einzug.pos, _x, _y, scroll_x1, scroll_y1, _i, _j, einzug.gw, einzug.gh, rtl, ttb)) _feld = EINZUG;
    else if (HitCheck (gewebe.pos, _x, _y, scroll_x1, scroll_y2, _i, _j, gewebe.gw, gewebe.gh, rtl)) _feld = GEWEBE;
    else if (HitCheck (aufknuepfung.pos, _x, _y, scroll_x2, scroll_y1, _i, _j, aufknuepfung.gw, aufknuepfung.gh, false, ttb)) _feld = AUFKNUEPFUNG;
    else if (HitCheck (trittfolge.pos, _x, _y, scroll_x2, scroll_y2, _i, _j, trittfolge.gw, trittfolge.gh)) _feld = TRITTFOLGE;
    else if (HitCheck (schussfarben.pos, _x, _y, 0, scroll_y2, _i, _j, schussfarben.gw, schussfarben.gh)) _feld = SCHUSSFARBEN;
    else if (HitCheck (hlinehorz2, _x+hlinehorz2.gw/2, _y, scroll_x2, 0, _i, _j, hlinehorz2.gw, hlinehorz2.gh)) _feld = HLINEHORZ2;
    else if (HitCheck (hlinevert2, _x, _y-hlinevert2.gh/2, 0, scroll_y2, _i, _j, hlinevert2.gw, hlinevert2.gh)) _feld = HLINEVERT2;
    else if (!ttb) {
        if (HitCheck (hlinevert1, _x, _y-hlinevert1.gh/2, 0, scroll_y1, _i, _j, hlinevert1.gw, hlinevert1.gh)) _feld = HLINEVERT1;
        else if (!rtl) {
            if (HitCheck (hlinehorz1, _x+hlinehorz1.gw/2, _y, scroll_x1, 0, _i, _j, hlinehorz1.gw, hlinehorz1.gh, rtl)) _feld = HLINEHORZ1;
            else _feld = INVALID;
        } else {
            if (HitCheck (hlinehorz1, _x-hlinehorz1.gw/2, _y, scroll_x1, 0, _i, _j, hlinehorz1.gw, hlinehorz1.gh, rtl)) _feld = HLINEHORZ1;
            else _feld = INVALID;
        }
    } else {
        if (HitCheck (hlinevert1, _x, _y+hlinevert1.gh/2, 0, scroll_y1, _i, _j, hlinevert1.gw, hlinevert1.gh, rtl, ttb)) _feld = HLINEVERT1;
        else if (!rtl) {
            if (HitCheck (hlinehorz1, _x+hlinehorz1.gw/2, _y, scroll_x1, 0, _i, _j, hlinehorz1.gw, hlinehorz1.gh, rtl)) _feld = HLINEHORZ1;
            else _feld = INVALID;
        } else {
            if (HitCheck (hlinehorz1, _x-hlinehorz1.gw/2, _y, scroll_x1, 0, _i, _j, hlinehorz1.gw, hlinehorz1.gh, rtl)) _feld = HLINEHORZ1;
            else _feld = INVALID;
        }
    }
}
/*-----------------------------------------------------------------*/
