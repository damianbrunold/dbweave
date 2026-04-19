/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope:
      * Physical2Logical + HitCheck pixel-to-cell lookup.
      * handleCanvasMousePress / Move / Release -- legacy semantics:
          GEWEBE / EINZUG / AUFKNUEPFUNG / TRITTFOLGE: drag builds a
              rubber-band selection; Set* is only driven directly on
              the initial click cell for parity with legacy feedback.
              A Range1..9 digit key applies currentrange across the
              selection (ApplyRangeToSelection).
          BLATTEINZUG / KETTFARBEN / SCHUSSFARBEN: drag paints every
              new cell crossed (lastfarbei/j / lastblatteinzugi
              deduplication matches legacy).
      * Deferred: hlinehorz/vert grab-bar drag, tool palette drawing
        modes (TOOL_LINE / TOOL_RECT / ...), divider-drag to resize
        einzug/trittfolge strips, right-click context menu.
*/

#include "mainwindow.h"
#include "cursor.h"
#include "hilfslinien.h"
#include "enums.h"

static bool HitCheck(const GRIDPOS& _grid, int _x, int _y, int _scrollx, int _scrolly, int& _i,
                     int& _j, int _gw, int _gh, bool _righttoleft = false,
                     bool _toptobottom = false)
{
    (void)_scrollx;
    (void)_scrolly;
    if (_gw <= 0 || _gh <= 0)
        return false;
    if (_x <= _grid.x0 || _x >= _grid.x0 + _grid.width)
        return false;
    if (_y <= _grid.y0 || _y >= _grid.y0 + _grid.height)
        return false;
    if (_righttoleft)
        _i = (_grid.x0 + _grid.width - _x) / _gw;
    else
        _i = (_x - _grid.x0) / _gw;
    if (_toptobottom)
        _j = (_y - _grid.y0) / _gh;
    else
        _j = (_grid.y0 + _grid.height - _y) / _gh;
    return true;
}

void TDBWFRM::Physical2Logical(int _x, int _y, FELD& _feld, int& _i, int& _j)
{
    const bool rtl = righttoleft;
    const bool ttb = toptobottom;
    _feld = INVALID;
    _i = _j = 0;

    if (HitCheck(blatteinzug.pos, _x, _y, scroll_x1, 0, _i, _j, blatteinzug.gw, blatteinzug.gh,
                 rtl))
        _feld = BLATTEINZUG;
    else if (HitCheck(kettfarben.pos, _x, _y, scroll_x1, 0, _i, _j, kettfarben.gw, kettfarben.gh,
                      rtl))
        _feld = KETTFARBEN;
    else if (HitCheck(einzug.pos, _x, _y, scroll_x1, scroll_y1, _i, _j, einzug.gw, einzug.gh, rtl,
                      ttb))
        _feld = EINZUG;
    else if (HitCheck(gewebe.pos, _x, _y, scroll_x1, scroll_y2, _i, _j, gewebe.gw, gewebe.gh, rtl))
        _feld = GEWEBE;
    else if (HitCheck(aufknuepfung.pos, _x, _y, scroll_x2, scroll_y1, _i, _j, aufknuepfung.gw,
                      aufknuepfung.gh, false, ttb))
        _feld = AUFKNUEPFUNG;
    else if (HitCheck(trittfolge.pos, _x, _y, scroll_x2, scroll_y2, _i, _j, trittfolge.gw,
                      trittfolge.gh))
        _feld = TRITTFOLGE;
    else if (HitCheck(schussfarben.pos, _x, _y, 0, scroll_y2, _i, _j, schussfarben.gw,
                      schussfarben.gh))
        _feld = SCHUSSFARBEN;
    /*  Hilfslinien grab bars. The horizontal bars only care about
        the x coordinate (for the cell column), the vertical bars
        only about y. Use the bar's gw/gh which matches the grid
        pitch of the adjacent field.                              */
    else if (HitCheck(hlinehorz1, _x, _y, scroll_x1, 0, _i, _j, hlinehorz1.gw, hlinehorz1.gh,
                      rtl))
        _feld = HLINEHORZ1;
    else if (HitCheck(hlinehorz2, _x, _y, scroll_x2, 0, _i, _j, hlinehorz2.gw, hlinehorz2.gh))
        _feld = HLINEHORZ2;
    else if (HitCheck(hlinevert1, _x, _y, 0, scroll_y1, _i, _j, hlinevert1.gw, hlinevert1.gh,
                      false, ttb))
        _feld = HLINEVERT1;
    else if (HitCheck(hlinevert2, _x, _y, 0, scroll_y2, _i, _j, hlinevert2.gw, hlinevert2.gh))
        _feld = HLINEVERT2;
}

/*  Data-coord cell indices for the hit field. Scroll offset differs
    per field; centralise it here.                                 */
static void dataCoords(TDBWFRM* frm, FELD f, int vp_i, int vp_j, int& di, int& dj)
{
    switch (f) {
    case GEWEBE:
        di = vp_i + frm->scroll_x1;
        dj = vp_j + frm->scroll_y2;
        break;
    case EINZUG:
        di = vp_i + frm->scroll_x1;
        dj = vp_j + frm->scroll_y1;
        break;
    case AUFKNUEPFUNG:
        di = vp_i + frm->scroll_x2;
        dj = vp_j + frm->scroll_y1;
        break;
    case TRITTFOLGE:
        di = vp_i + frm->scroll_x2;
        dj = vp_j + frm->scroll_y2;
        break;
    case BLATTEINZUG:
    case KETTFARBEN:
        di = vp_i + frm->scroll_x1;
        dj = 0;
        break;
    case SCHUSSFARBEN:
        di = 0;
        dj = vp_j + frm->scroll_y2;
        break;
    default:
        di = vp_i;
        dj = vp_j;
        break;
    }
}

void TDBWFRM::handleCanvasMousePress(int _x, int _y, bool _shift, bool _ctrl)
{
    FELD f;
    int i, j;
    Physical2Logical(_x, _y, f, i, j);
    if (f == INVALID)
        return;

    mousedown = true;
    md_feld = f;
    lastfarbei = -1;
    lastfarbej = -1;
    lastblatteinzugi = -1;
    bSelectionCleared = selection.Valid();
    md_ctrl = _ctrl;

    int di, dj;
    dataCoords(this, f, i, j, di, dj);
    md = PT(di, dj);

    if (cursorhandler)
        cursorhandler->SetCursor(f, di, dj, /*_clearselection=*/true);

    switch (f) {
    case GEWEBE:
        /*  Non-POINT drawing tool: capture the anchor cell; the
            paintEvent will draw a live preview between tool_i0/j0
            and tool_i1/j1, and the release will rasterise the
            shape into gewebe. No rubber-band selection in this
            mode. */
        if (tool != TOOL_POINT) {
            tool_active = true;
            tool_i0 = tool_i1 = di;
            tool_j0 = tool_j1 = dj;
            ClearSelection();
            refresh();
            return;
        }
        /*  fall through */
    case EINZUG:
    case AUFKNUEPFUNG:
    case TRITTFOLGE:
        /*  Start a fresh rubber-band selection at the clicked cell.
            The toggle (if the user doesn't drag) happens on the
            matching release event, matching legacy FormMouseUp.   */
        ClearSelection();
        ResizeSelection(di, dj, f, _shift);
        break;

    case KETTFARBEN:
        SetKettfarben(i);
        lastfarbei = i;
        break;
    case SCHUSSFARBEN:
        SetSchussfarben(j);
        lastfarbej = j;
        break;
    case BLATTEINZUG:
        SetBlatteinzug(i);
        lastblatteinzugi = i;
        break;
    case HLINEHORZ1:
        /*  Horizontal bar above einzug/gewebe: clicks toggle a
            VERTICAL guide line at column (i + scroll_x1) on the LEFT
            side.                                                    */
        if (hlines.GetLine(HL_VERT, HL_LEFT, i + scroll_x1))
            hlines.Delete(hlines.GetLine(HL_VERT, HL_LEFT, i + scroll_x1));
        else
            hlines.Add(HL_VERT, HL_LEFT, i + scroll_x1);
        SetModified();
        break;
    case HLINEHORZ2:
        if (hlines.GetLine(HL_VERT, HL_RIGHT, i + scroll_x2))
            hlines.Delete(hlines.GetLine(HL_VERT, HL_RIGHT, i + scroll_x2));
        else
            hlines.Add(HL_VERT, HL_RIGHT, i + scroll_x2);
        SetModified();
        break;
    case HLINEVERT1:
        if (hlines.GetLine(HL_HORZ, HL_TOP, j + scroll_y1))
            hlines.Delete(hlines.GetLine(HL_HORZ, HL_TOP, j + scroll_y1));
        else
            hlines.Add(HL_HORZ, HL_TOP, j + scroll_y1);
        SetModified();
        break;
    case HLINEVERT2:
        if (hlines.GetLine(HL_HORZ, HL_BOTTOM, j + scroll_y2))
            hlines.Delete(hlines.GetLine(HL_HORZ, HL_BOTTOM, j + scroll_y2));
        else
            hlines.Add(HL_HORZ, HL_BOTTOM, j + scroll_y2);
        SetModified();
        break;
    default:
        break;
    }
    refresh();
}

void TDBWFRM::handleCanvasMouseMove(int _x, int _y, bool _shift)
{
    if (!mousedown)
        return;

    FELD f;
    int i, j;
    Physical2Logical(_x, _y, f, i, j);
    if (f != md_feld)
        return;

    int di, dj;
    dataCoords(this, f, i, j, di, dj);

    switch (f) {
    case GEWEBE:
        if (tool_active) {
            /*  Update preview cell; PatternCanvas paints the
                overlay. */
            tool_i1 = di;
            tool_j1 = dj;
            refresh();
            return;
        }
        /*  fall through */
    case EINZUG:
    case AUFKNUEPFUNG:
    case TRITTFOLGE:
        if (cursorhandler)
            cursorhandler->SetCursor(f, di, dj, /*_clearselection=*/false);
        ResizeSelection(di, dj, f, _shift);
        break;

    case KETTFARBEN:
        if (i != lastfarbei) {
            SetKettfarben(i);
            lastfarbei = i;
            refresh();
        }
        break;
    case SCHUSSFARBEN:
        if (j != lastfarbej) {
            SetSchussfarben(j);
            lastfarbej = j;
            refresh();
        }
        break;
    case BLATTEINZUG:
        if (i != lastblatteinzugi) {
            SetBlatteinzug(i);
            lastblatteinzugi = i;
            refresh();
        }
        break;
    default:
        break;
    }
}

void TDBWFRM::handleCanvasMouseRelease()
{
    /*  Tool-drag release: rasterise the shape between anchor and
        current cell via DrawTool, clear the preview, and bail out
        before the default click-without-drag path. Data-coord
        anchors are translated back to viewport-relative indices
        because DrawTool* add scroll_x1/scroll_y2 themselves. */
    if (tool_active) {
        const int vi0 = tool_i0 - scroll_x1;
        const int vj0 = tool_j0 - scroll_y2;
        const int vi1 = tool_i1 - scroll_x1;
        const int vj1 = tool_j1 - scroll_y2;
        tool_active = false;
        DrawTool(vi0, vj0, vi1, vj1);
        mousedown = false;
        md_feld = INVALID;
        bSelectionCleared = false;
        md_ctrl = false;
        return;
    }

    /*  Click-without-drag: the press installed a transient 1x1
        selection at the click cell. On release, always drop that
        transient selection (otherwise the next click sees a
        leftover "prior" selection and bSelectionCleared traps it
        forever); then, if no drag happened and nothing is
        suppressing the toggle, dispatch the per-field Set* op.
        Matches legacy FormMouseUp (mousehandling.cpp:503). */
    if (mousedown && md_feld != INVALID) {
        RANGE sel = selection;
        sel.Normalize();
        const bool sameCell = sel.Valid() && sel.feld == md_feld && sel.begin.i == sel.end.i
                              && sel.begin.j == sel.end.j && sel.begin.i == md.i
                              && sel.begin.j == md.j;
        if (sameCell) {
            ClearSelection();
            if (!bSelectionCleared && !md_ctrl) {
                const int r = currentrange;
                switch (md_feld) {
                case GEWEBE:
                    SetGewebe(md.i - scroll_x1, md.j - scroll_y2, /*_set=*/false, r);
                    break;
                case EINZUG:
                    SetEinzug(md.i - scroll_x1, md.j - scroll_y1);
                    break;
                case AUFKNUEPFUNG:
                    SetAufknuepfung(md.i - scroll_x2, md.j - scroll_y1, /*_set=*/false, r);
                    break;
                case TRITTFOLGE:
                    SetTrittfolge(md.i - scroll_x2, md.j - scroll_y2, /*_set=*/false, r);
                    break;
                default:
                    break;
                }
            }
        }
    }

    mousedown = false;
    md_feld = INVALID;
    lastfarbei = -1;
    lastfarbej = -1;
    lastblatteinzugi = -1;
    bSelectionCleared = false;
    md_ctrl = false;
}
