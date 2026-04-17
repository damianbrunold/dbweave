/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File fixeinzug_form.cpp
 * Copyright (C) 1999-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "fixeinzug_form.h"
#include "dbw3_form.h"
#include "datamodule.h"
#include "einzug.h"
#include "dbw3_strings.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TFixeinzugForm::TFixeinzugForm(TComponent* Owner, TDBWFRM* _frm, CrCursorHandler* _ch)
: TForm(Owner), frm(_frm)
{
    LoadLanguage();

    dbw3_assert(frm!=NULL);

    Width = frm->Width;
    Left = frm->Left;

    Top = frm->gewebe.pos.y0+30;

    cursorhandler = _ch;
    if (cursorhandler) cursordirection = cursorhandler->GetCursorDirection();

    // Daten kopieren
    try {
        einzug = new short[Data->MAXX1];
        memset (einzug, 0, sizeof(short)*Data->MAXX1);
        size = frm->fixsize;
        for (int i=0; i<=size; i++)
            einzug[i] = frm->fixeinzug[i];
        firstfree = frm->firstfree;
        changed = false;
    } catch (...) {
        dbw3_assert(false);
    }

    cx = cy = 0;
    scrollx = 0;
    scrolly = 0;

    RecalcDimensions();
    CalcRange();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::GrabClick(TObject *Sender)
{
    frm->UpdateEinzugFixiert();
    memset (einzug, 0, sizeof(short)*Data->MAXX1);
    size = frm->fixsize;
    for (int i=0; i<=size; i++)
        einzug[i] = frm->fixeinzug[i];
    firstfree = frm->firstfree;
    changed = true;
    CalcRange();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::RevertClick(TObject *Sender)
{
    ModalResult = mrCancel;
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::CloseClick(TObject *Sender)
{
    // Checken ob korrekter Einzug
    for (int i=0; i<size; i++)
        if (einzug[i]==0) {
            Application->MessageBox (USERDEFEZ_CONTINUOUS, APP_TITLE, MB_OK);
            ModalResult = mrNone;
            return;
        }
    // Daten speichern ...
    memcpy (frm->fixeinzug, einzug, sizeof(short)*Data->MAXX1);
    frm->firstfree = CalcFirstFree();
    frm->fixsize = size;
    // ... und rapportieren
    int k = 0;
    for (int i=size+1; i<Data->MAXX1; i++) {
        frm->fixeinzug[i] = frm->fixeinzug[k++];
        if (k>size) k = 0;
    }
    ModalResult = mrOk;
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::pbFixeinzugMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    int i = X / dx;
    int j = maxy-1 - (Y / dy);

    dbw3_assert(i>=0);
    dbw3_assert(j>=0);

    if (i>=maxx || j>=maxy) return;

    short old = einzug[scrollx+i];
    if (old==j+1) einzug[scrollx+i] = 0;
    else einzug[scrollx+i] = short(j+1);
    changed = true;
    CalcRange();
    if (old!=0) {
        pbFixeinzug->Canvas->Pen->Color = clBtnFace;
        pbFixeinzug->Canvas->Brush->Color = clBtnFace;
        pbFixeinzug->Canvas->Rectangle (i*dx+2, (maxy-old)*dy+2, (i+1)*dx-1, (maxy-old+1)*dy-1);
    }
    DrawData (pbFixeinzug, i);
    DeleteCursor();
    cx = i;
    cy = j;
    DeleteRange();
    CalcRange();
    DrawRange();
    DrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::CurDirLeft()
{
    if (cursordirection&CD_RIGHT) cursordirection &= ~CD_RIGHT;
    else cursordirection |= CD_LEFT;
    cursorhandler->SetCursorDirection (cursordirection);
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::CurDirRight()
{
    if (cursordirection&CD_LEFT) cursordirection &= ~CD_LEFT;
    else cursordirection |= CD_RIGHT;
    cursorhandler->SetCursorDirection (cursordirection);
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::CurDirUp()
{
    if (cursordirection&CD_DOWN) cursordirection &= ~CD_DOWN;
    else cursordirection |= CD_UP;
    cursorhandler->SetCursorDirection (cursordirection);
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::CurDirDown()
{
    if (cursordirection&CD_UP) cursordirection &= ~CD_UP;
    else cursordirection |= CD_DOWN;
    cursorhandler->SetCursorDirection (cursordirection);
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::CurLeft (bool _step)
{
    if (cx>0) {
        DeleteCursor();
        if (!_step) cx--;
        else {
            cx -= 4;
            if (cx<0) cx = 0;
        }
        DrawRange();
        DrawCursor();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::CurRight (bool _step)
{
    if (cx<maxx-1) {
        DeleteCursor();
        if (!_step) cx++;
        else {
            cx += 4;
            if (cx>maxx-1) cx = maxx-1;
        }
        DrawRange();
        DrawCursor();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::CurUp (bool _step)
{
    if (cy<maxy-1) {
        DeleteCursor();
        if (!_step) cy++;
        else {
            cy += 4;
            if (cy>maxy-1) cy = maxy-1;
        }
        DrawRange();
        DrawCursor();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::CurDown (bool _step)
{
    if (cy>0) {
        DeleteCursor();
        if (!_step) cy--;
        else {
            cy -= 4;
            if (cy<0) cy = 0;
        }
        DrawRange();
        DrawCursor();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::CurMove()
{
    if (frm->toptobottom) {
        if ((cursordirection&CD_DOWN) && cy>0) cy--;
        if ((cursordirection&CD_UP) && cy<maxy-1) cy++;
    } else {
        if ((cursordirection&CD_UP) && cy<maxy-1) cy++;
        if ((cursordirection&CD_DOWN) && cy>0) cy--;
    }
    if (frm->righttoleft) {
        if ((cursordirection&CD_RIGHT) && cx<maxx-1) cx++;
        if ((cursordirection&CD_LEFT) && cx>0) cx--;
    } else {
        if ((cursordirection&CD_LEFT) && cx>0) cx--;
        if ((cursordirection&CD_RIGHT) && cx<maxx-1) cx++;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
    switch (Key) {
        case VK_SPACE: {
            DeleteCursor();
            DeleteRange();
            short old = einzug[scrollx+cx];
            if (old==cy+1) einzug[scrollx+cx] = 0;
            else einzug[scrollx+cx] = short(cy+1);
            CalcRange();
            if (old!=0) {
                pbFixeinzug->Canvas->Pen->Color = clBtnFace;
                pbFixeinzug->Canvas->Brush->Color = clBtnFace;
                int x, y;
                if (frm->righttoleft) x = (maxx-cx-1)*dx;
                else x = cx*dx;
                if (frm->toptobottom) y = (old-1)*dy;
                else y = (maxy-old)*dy;
                pbFixeinzug->Canvas->Rectangle (x+2, y+2, x+dx-1, y+dy-1);
            }
            DrawData(pbFixeinzug, cx);
            DrawRange();
            CurMove();
            DrawCursor();
            changed = true;
            break;
        }
        case VK_LEFT:
            if (Shift.Contains(ssCtrl) && Shift.Contains(ssAlt)) {
                if (frm->righttoleft) CurDirRight();
                else CurDirLeft();
                break;
            }
            if (frm->righttoleft) CurRight(Shift.Contains(ssCtrl));
            else CurLeft (Shift.Contains(ssCtrl));
            break;
        case VK_RIGHT:
            if (Shift.Contains(ssCtrl) && Shift.Contains(ssAlt)) {
                if (frm->righttoleft) CurDirLeft();
                else CurDirRight();
                break;
            }
            if (frm->righttoleft) CurLeft(Shift.Contains(ssCtrl));
            else CurRight (Shift.Contains(ssCtrl));
            break;
        case VK_UP:
            if (Shift.Contains(ssCtrl) && Shift.Contains(ssAlt)) {
                if (frm->toptobottom) CurDirDown();
                else CurDirUp();
                break;
            }
            if (frm->toptobottom) CurDown (Shift.Contains(ssCtrl));
            else CurUp (Shift.Contains(ssCtrl));
            break;
        case VK_DOWN:
            if (Shift.Contains(ssCtrl) && Shift.Contains(ssAlt)) {
                if (frm->toptobottom) CurDirUp();
                else CurDirDown();
                break;
            }
            if (frm->toptobottom) CurUp (Shift.Contains(ssCtrl));
            else CurDown (Shift.Contains(ssCtrl));
            break;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::FormResize(TObject *Sender)
{
    RecalcDimensions();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::RecalcDimensions()
{
    dx = dy = frm->zoom[frm->currentzoom];

    maxx = (ClientWidth - 5 - sbVert->Width) / dx;
    maxy = (ClientHeight - 5 - sbHorz->Height) / dy;

    if (maxx>Data->MAXX1) maxx = Data->MAXX1;
    if (maxy>Data->MAXY1) maxy = Data->MAXY1;

    pbFixeinzug->Left = 0;
    pbFixeinzug->Width = maxx * dx + 1;
    pbFixeinzug->Top = 0;
    pbFixeinzug->Height = maxy * dy + 1;

    sbHorz->Left = 0;
    sbHorz->Top = pbFixeinzug->Top + pbFixeinzug->Height + 3;
    sbHorz->Width = pbFixeinzug->Width;

    sbVert->Top = 0;
    sbVert->Left = pbFixeinzug->Left + pbFixeinzug->Width + 3;
    sbVert->Height = pbFixeinzug->Height;
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::pbFixeinzugPaint(TObject *Sender)
{
    DrawGrid (pbFixeinzug);
    DrawData (pbFixeinzug);
    DrawRange();
    DrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::DrawGrid (TPaintBox* _pb)
{
    _pb->Canvas->Pen->Color = clDkGray;
    for (int i=0; i<=maxx; i++) {
        _pb->Canvas->MoveTo (i*dx, 0);
        _pb->Canvas->LineTo (i*dx, maxy*dy+1);
    }
    for (int j=0; j<=maxy; j++) {
        _pb->Canvas->MoveTo (0, j*dy);
        _pb->Canvas->LineTo (maxx*dx, j*dy);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::DrawData (TPaintBox* _pb, int _i)
{
    short j = einzug[scrollx+_i];
    if (j!=0 && j-1>=scrolly && j-1<scrolly+maxy) {
        int x, y;
        if (frm->righttoleft) x = (maxx-_i-1)*dx;
        else x = _i*dx;
        if (frm->toptobottom) y = (j-1-scrolly)*dy;
        else y = (maxy-(j-scrolly))*dy;
        PaintCell (pbFixeinzug->Canvas, DBWFRM->einzug.darstellung, x, y, x+dx, y+dy);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::DrawData (TPaintBox* _pb)
{
    for (int i=0; i<maxx; i++)
        DrawData (_pb, i);
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::_DrawCursor(TColor _col)
{
    TCanvas* pCanvas = pbFixeinzug->Canvas;
    pCanvas->Pen->Color = _col;
    int x, y;
    if (frm->righttoleft) x = (maxx-1-cx)*dx;
    else x = cx*dx;
    if (frm->toptobottom) y = cy*dy;
    else y = (maxy-1-cy)*dy;
    pCanvas->MoveTo (x, y);
    pCanvas->LineTo (x+dx, y);
    pCanvas->LineTo (x+dx, y+dy);
    pCanvas->LineTo (x, y+dy);
    pCanvas->LineTo (x, y);
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::DrawCursor()
{
    _DrawCursor (clWhite);
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::DeleteCursor()
{
    _DrawCursor (clDkGray);
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::CalcRange()
{
    mx = -1;
    my = -1;

    for (int i=0; i<Data->MAXX1; i++) {
        if (einzug[i]!=0) {
            mx = i;
            if (einzug[i]-1 > my) my = einzug[i]-1;
        }
    }

    size = mx;
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::_DrawRange (bool _paint, TPaintBox* _pb)
{
    if (mx==-1 || my==-1) return;

    _pb->Canvas->Pen->Color = _paint ? clRed : clDkGray;

    int x, y, xx, yy;
    if (frm->righttoleft) {
        x = (maxx-1-mx)*dx;
        xx = maxx*dx;
    } else {
        x = (mx+1)*dx;
        xx = 1;
    }
    if (frm->toptobottom) {
        y = (my+1)*dy;
        yy = 1;
    } else {
        y = (maxy-1-my)*dy;
        yy = maxy*dy;
    }

    _pb->Canvas->MoveTo (xx, y);
    _pb->Canvas->LineTo (x, y);
    _pb->Canvas->MoveTo (x, y);
    _pb->Canvas->LineTo (x, yy);
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::DrawRange()
{
    _DrawRange (true, pbFixeinzug);
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::DeleteRange()
{
    _DrawRange (false, pbFixeinzug);
}
/*-----------------------------------------------------------------*/
short __fastcall TFixeinzugForm::CalcFirstFree()
{
    firstfree = 0;
    for (int i=0; i<Data->MAXX1; i++)
        if (einzug[i]>=firstfree) firstfree = short(einzug[i]);
    return firstfree;
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::DeleteClick(TObject *Sender)
{
    DeleteRange();
    firstfree = 0;
    memset (einzug, 0, sizeof(short)*Data->MAXX1);
    size = 0;
    CalcRange();
    Invalidate();
}
/*-----------------------------------------------------------------*/
void __fastcall TFixeinzugForm::LoadLanguage()
{
    LANG_C_H(this, EN, "User defined threading", "")
    LANG_C_H(this, GE, "Benutzerdefinierter Einzug", "")
    LANG_C_H(MenuEinzug, EN, "&Threading", "")
    LANG_C_H(MenuEinzug, GE, "&Einzug", "")
    LANG_C_H(Grab, EN, "&Take threading from pattern", "Takes the threading from the pattern as user defined threading")
    LANG_C_H(Grab, GE, "&Einzug von Muster übernehmen", "Übernimmt den bisherigen Einzug als benutzerdefinierten Einzug")
    LANG_C_H(Delete, EN, "&Delete", "Deletes the user defined threading")
    LANG_C_H(Delete, GE, "&Löschen", "Löscht den benutzerdefinierten Einzug")
    LANG_C_H(Revert, EN, "&Revert", "Reverts any changes")
    LANG_C_H(Revert, GE, "&Verwerfen", "Verwirft alle Änderungen")
    LANG_C_H(Close, EN, "&Close", "Closes this window")
    LANG_C_H(Close, GE, "&Schliessen", "Schliesst dieses Fenster")
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::EditFixeinzug(TObject *Sender)
{
    if (fixeinzug[0]==0)
        UpdateEinzugFixiert();

    if (!EzFixiert->Checked)
        EzFixiert->Checked = true;

    cursorhandler->DisableCursor();
    try {
        TFixeinzugForm* pFrm = new TFixeinzugForm(this, this, cursorhandler);
        if (pFrm->ShowModal()==mrOk/* && pFrm->changed*/) {
            //RecalcAll();
            einzughandler->Fixiert();
            CalcRapport();
            UpdateStatusBar();
        }
        delete pFrm;
    } catch (...) {
        dbw3_assert(false);
    }
    cursorhandler->EnableCursor();
}
/*-----------------------------------------------------------------*/

