/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File blockmuster_form.cpp
 * Copyright (C) 1999-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "blockmuster_form.h"
#include "rangecolors.h"
#include "assert.h"
#include "zentralsymm.h"
#include "dbw3_strings.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
#define GBCOLOR clGray
/*-----------------------------------------------------------------*/
__fastcall TBlockmusterForm::TBlockmusterForm(TComponent* Owner, BlockUndo& _undo, PMUSTERARRAY _bindungen, int& _current, CrCursorHandler* _ch, String _caption, bool _eztr)
: TForm(Owner), undo(_undo), bindungen(_bindungen), current(_current)
{
    LoadLanguage();

    cursorhandler = _ch;
    if (cursorhandler) cursordirection = cursorhandler->GetCursorDirection();
    Caption = _caption;
    withrange = _eztr; // Die rote Grössenmarkierung braucht es nur bei Block-
                       // aber nicht bei Bereichmusterung
    if (!_eztr) {
        BlockEinzug->Visible = false;
        BlockTrittfolge->Visible = false;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::FormShow(TObject *Sender)
{
    dx = dy = 11;
    cx = cy = 0;
    CalcRange();
    Bindung0Click(this);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::DrawGrid (TPaintBox* _pb)
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
void __fastcall TBlockmusterForm::pbBindungPaint(TObject *Sender)
{
    DrawGrid (pbBindung);

    for (int i=0; i<maxx; i++)
        for (int j=0; j<maxy; j++) {
            char c = (*bindungen)[current].Get(i,j);
            if (c!=0) {
                pbBindung->Canvas->Pen->Color = current==0 ? GBCOLOR : GetRangeColor(c);
                pbBindung->Canvas->Brush->Color = pbBindung->Canvas->Pen->Color;
                pbBindung->Canvas->Rectangle (i*dx+2, (maxy-j-1)*dy+2, (i+1)*dx-1, (maxy-j)*dy-1);
            }
        }

    _DrawRange (true, pbBindung);

    DrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::pbBindungMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    int i = X / dx;
    int j = maxy-1 - (Y / dy);

    dbw3_assert(i>=0);
    dbw3_assert(j>=0);

    if (i>=maxx || j>=maxy) return;

    char c = Toggle((*bindungen)[current].Get(i, j));
    (*bindungen)[current].Set (i, j, c);

    pbBindung->Canvas->Pen->Color = c==0 ? clBtnFace : GetColor(current);
    pbBindung->Canvas->Brush->Color = c==0 ? clBtnFace : GetColor(current);
    pbBindung->Canvas->Rectangle (i*dx+2, (maxy-j-1)*dy+2, (i+1)*dx-1, (maxy-j)*dy-1);

    pbUsed->Repaint();

    DeleteCursor();
    cx = i;
    cy = j;
    DeleteRange();
    CalcRange();
    DrawRange();
    DrawCursor();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
    switch (Key) {
        case VK_ESCAPE:
            ModalResult = mrCancel;
            break;
        case VK_RETURN:
            DeleteCursor();
            if (!Shift.Contains(ssShift)) current = (current+1) % 10;
            else current = (current+9) % 10;
            // Label neu setzen
            if (current==0) description->Caption = RANGE_GRUNDBINDUNG;
            else description->Caption = String(RANGE_BINDUNG) + IntToStr(current);
            Neuzeichnen();
            break;
        case VK_SPACE: {
            char c = Toggle((*bindungen)[current].Get(cx, cy));
            (*bindungen)[current].Set (cx, cy, c);
            pbBindung->Canvas->Pen->Color = c==0 ? clBtnFace : GetColor(current);
            pbBindung->Canvas->Brush->Color = c==0 ? clBtnFace : GetColor(current);
            pbBindung->Canvas->Rectangle (cx*dx+2, (maxy-cy-1)*dy+2, (cx+1)*dx-1, (maxy-cy)*dy-1);
            DeleteCursor();
            DeleteRange();
            CalcRange();
            DrawRange();
            if ((cursordirection&CD_UP) && cy<maxy-1) cy++;
            if ((cursordirection&CD_DOWN) && cy>0) cy--;
            if ((cursordirection&CD_LEFT) && cx>0) cx--;
            if ((cursordirection&CD_RIGHT) && cx<maxx-1) cx++;
            DrawCursor();
            pbUsed->Repaint();
            undo.Snapshot();
            break;
        }
        case VK_LEFT:
            if (Shift.Contains(ssCtrl) && Shift.Contains(ssAlt)) {
                if (cursordirection&CD_RIGHT) cursordirection &= ~CD_RIGHT;
                else cursordirection |= CD_LEFT;
                cursorhandler->SetCursorDirection (cursordirection);
                break;
            }
            if (Shift.Contains(ssShift)) {
                RollLeft ((*bindungen)[current]);
                CalcRange();
                pbBindung->Repaint();
                undo.Snapshot();
                break;
            }
            if (cx>0) {
                DeleteCursor();
                if (!Shift.Contains(ssCtrl)) cx--;
                else {
                    cx -= 4;
                    if (cx<0) cx = 0;
                }
                DrawRange();
                DrawCursor();
            }
            break;
        case VK_RIGHT:
            if (Shift.Contains(ssCtrl) && Shift.Contains(ssAlt)) {
                if (cursordirection&CD_LEFT) cursordirection &= ~CD_LEFT;
                else cursordirection |= CD_RIGHT;
                cursorhandler->SetCursorDirection (cursordirection);
                break;
            }
            if (Shift.Contains(ssShift)) {
                RollRight ((*bindungen)[current]);
                CalcRange();
                pbBindung->Repaint();
                undo.Snapshot();
                break;
            }
            if (cx<maxx-1) {
                DeleteCursor();
                if (!Shift.Contains(ssCtrl)) cx++;
                else {
                    cx += 4;
                    if (cx>maxx-1) cx = maxx-1;
                }
                DrawRange();
                DrawCursor();
            }
            break;
        case VK_UP:
            if (Shift.Contains(ssCtrl) && Shift.Contains(ssAlt)) {
                if (cursordirection&CD_DOWN) cursordirection &= ~CD_DOWN;
                else cursordirection |= CD_UP;
                cursorhandler->SetCursorDirection (cursordirection);
                break;
            }
            if (Shift.Contains(ssShift)) {
                RollUp ((*bindungen)[current]);
                CalcRange();
                pbBindung->Repaint();
                undo.Snapshot();
                break;
            }
            if (cy<maxy-1) {
                DeleteCursor();
                if (!Shift.Contains(ssCtrl)) cy++;
                else {
                    cy += 4;
                    if (cy>maxy-1) cy = maxy-1;
                }
                DrawRange();
                DrawCursor();
            }
            break;
        case VK_DOWN:
            if (Shift.Contains(ssCtrl) && Shift.Contains(ssAlt)) {
                if (cursordirection&CD_UP) cursordirection &= ~CD_UP;
                else cursordirection |= CD_DOWN;
                cursorhandler->SetCursorDirection (cursordirection);
                break;
            }
            if (Shift.Contains(ssShift)) {
                RollDown ((*bindungen)[current]);
                CalcRange();
                pbBindung->Repaint();
                undo.Snapshot();
                break;
            }
            if (cy>0) {
                DeleteCursor();
                if (!Shift.Contains(ssCtrl)) cy--;
                else {
                    cy -= 4;
                    if (cy<0) cy = 0;
                }
                DrawRange();
                DrawCursor();
            }
            break;
        case '0':
        case 191:
        case 'g':
        case 'G':
            if (Shift.Contains(ssAlt)) break;
            Bindung0Click (this);
            break;
        case '1':
            if (Shift.Contains(ssAlt)) break;
            Bindung1Click (this);
            break;
        case '2':
            if (Shift.Contains(ssAlt)) break;
            Bindung2Click (this);
            break;
        case '3':
            if (Shift.Contains(ssAlt)) break;
            Bindung3Click (this);
            break;
        case '4':
            if (Shift.Contains(ssAlt)) break;
            Bindung4Click (this);
            break;
        case '5':
            if (Shift.Contains(ssAlt)) break;
            Bindung5Click (this);
            break;
        case '6':
            if (Shift.Contains(ssAlt)) break;
            Bindung6Click (this);
            break;
        case '7':
            if (Shift.Contains(ssAlt)) break;
            Bindung7Click (this);
            break;
        case '8':
            if (Shift.Contains(ssAlt)) break;
            Bindung8Click (this);
            break;
        case '9':
            if (Shift.Contains(ssAlt)) break;
            Bindung9Click (this);
            break;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::DrawCursor()
{
    TCanvas* pCanvas = pbBindung->Canvas;
    pCanvas->Pen->Color = clWhite;
    pCanvas->MoveTo (cx*dx, (maxy-1-cy)*dy);
    pCanvas->LineTo ((cx+1)*dx, (maxy-1-cy)*dy);
    pCanvas->LineTo ((cx+1)*dx, (maxy-cy)*dy);
    pCanvas->LineTo (cx*dx, (maxy-cy)*dy);
    pCanvas->LineTo (cx*dy, (maxy-1-cy)*dy);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::DeleteCursor()
{
    TCanvas* pCanvas = pbBindung->Canvas;
    pCanvas->Pen->Color = clDkGray;
    pCanvas->MoveTo (cx*dx, (maxy-1-cy)*dy);
    pCanvas->LineTo ((cx+1)*dx, (maxy-1-cy)*dy);
    pCanvas->LineTo ((cx+1)*dx, (maxy-cy)*dy);
    pCanvas->LineTo (cx*dx, (maxy-cy)*dy);
    pCanvas->LineTo (cx*dy, (maxy-1-cy)*dy);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::BlockDoitClick(TObject *Sender)
{
    CalcRange();
    ModalResult = mrOk;
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::BlockCloseClick(TObject *Sender)
{
    ModalResult = mrCancel;
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::EinzugZClick(TObject *Sender)
{
    EinzugZ->Checked = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::EinzugSClick(TObject *Sender)
{
    EinzugS->Checked = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::TrittfolgeZClick(TObject *Sender)
{
    TrittfolgeZ->Checked = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::TrittfolgeSClick(TObject *Sender)
{
    TrittfolgeS->Checked = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::CalcRange()
{
    mx = -1;
    my = -1;

    bool nonempty = false;
    for (int i=0; i<maxx; i++)
        for (int j=0; j<maxy; j++)
            for (int k=0; k<10; k++)
                if ((*bindungen)[k].Get(i,j)!=0) {
                    if (mx<i) mx = i;
                    if (my<j) my = j;
                    nonempty = true;
                }

    if (!nonempty) mx = my = -1;
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::_DrawRange (bool _paint, TPaintBox* _pb)
{
    if (!withrange) return;
    if (mx==-1 || my==-1) return;

    _pb->Canvas->Pen->Color = _paint ? clRed : clDkGray;
    _pb->Canvas->MoveTo (1, (maxy-1-my)*dy);
    _pb->Canvas->LineTo ((mx+1)*dx, (maxy-1-my)*dy);
    _pb->Canvas->MoveTo ((mx+1)*dx, (maxy-1-my)*dy);
    _pb->Canvas->LineTo ((mx+1)*dx, maxy*dy);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::DrawRange()
{
    _DrawRange (true, pbBindung);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::DeleteRange()
{
    _DrawRange (false, pbBindung);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::EditDeleteClick(TObject *Sender)
{
    if (mx==-1 || my==-1) return;
    (*bindungen)[current].Clear();
    Neuzeichnen();
    undo.Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::EditSpiegelnHClick(TObject *Sender)
{
    if (mx==-1 || my==-1) return;
    for (int i=0; i<=mx/2; i++)
        for (int j=0; j<=my; j++) {
            char set = (*bindungen)[current].Get (i,j);
            (*bindungen)[current].Set (i, j, (*bindungen)[current].Get (mx-i, j));
            (*bindungen)[current].Set (mx-i, j, set);
        }
    pbBindung->Repaint();
    undo.Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::EditSpiegelnVClick(TObject *Sender)
{
    if (mx==-1 || my==-1) return;
    for (int i=0; i<=mx; i++)
        for (int j=0; j<=my/2; j++) {
            char set = (*bindungen)[current].Get (i,j);
            (*bindungen)[current].Set (i, j, (*bindungen)[current].Get (i, my-j));
            (*bindungen)[current].Set (i, my-j, set);
        }
    pbBindung->Repaint();
    undo.Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::EditRotierenClick(TObject *Sender)
{
    if (mx==-1 || my==-1) return;
    if (mx!=my) return;
    try {
        char* pData = new char[(mx+1)*(my+1)];
        for (int i=0; i<=mx; i++)
            for (int j=0; j<=my; j++) {
                pData[j*(mx+1)+i] = (*bindungen)[current].Get (i, j);
                (*bindungen)[current].Set (i, j, 0);
            }
        for (int i=0; i<=mx; i++)
            for (int j=0; j<=my; j++)
                (*bindungen)[current].Set (j, my-i, pData[j*(mx+1)+i]);
        delete[] pData;
    } catch (...) {
    }
    pbBindung->Repaint();
    undo.Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::EditInvertierenClick(TObject *Sender)
{
    if (mx==-1 || my==-1) return;
    for (int i=0; i<=mx; i++)
        for (int j=0; j<=my; j++) {
            char c = Toggle((*bindungen)[current].Get (i, j));
            (*bindungen)[current].Set (i, j, c);
        }
    Neuzeichnen();
    undo.Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::EditCentralsymClick(TObject *Sender)
{
    if (mx==-1 || my==-1) return;
    if (mx!=my) return;

    ZentralSymmChecker symchecker(mx+1, my+1);

    // Bindung reinkopieren
    for (int i=0; i<=mx; i++)
        for (int j=0; j<=my; j++)
            symchecker.SetData (i, j, (*bindungen)[current].Get(i, j));

    if (!symchecker.IsAlreadySymmetric()) {
        // Falls Symmetrie vorhanden Bindung entsprechend ändern
        if (symchecker.SearchSymmetry()) {
            for (int i=0; i<=mx; i++)
                for (int j=0; j<=my; j++)
                    (*bindungen)[current].Set (i, j, symchecker.GetData (i, j));
        } else {
            Application->MessageBox (NOCENTRALSYMMFOUND.c_str(), APP_TITLE, MB_OK);
        }
        pbBindung->Repaint();
        undo.Snapshot();
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::EditRollUpClick(TObject *Sender)
{
    RollUp ((*bindungen)[current]);
    CalcRange();
    pbBindung->Repaint();
    undo.Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::EditRollDownClick(TObject *Sender)
{
    RollDown ((*bindungen)[current]);
    CalcRange();
    pbBindung->Repaint();
    undo.Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::EditRollLeftClick(TObject *Sender)
{
    RollLeft ((*bindungen)[current]);
    CalcRange();
    pbBindung->Repaint();
    undo.Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::EditRollRightClick(TObject *Sender)
{
    RollRight ((*bindungen)[current]);
    CalcRange();
    pbBindung->Repaint();
    undo.Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::RollUp (Muster& _b)
{
    if (mx==-1 || my==-1) return;
    try {
        char* data = new char[mx+1];
        for (int i=0; i<=mx; i++) data[i] = _b.Get (i, my);
        for (int j=my; j>0; j--)
            for (int i=0; i<=mx; i++)
                _b.Set (i, j, _b.Get (i, j-1));
        for (int i=0; i<=mx; i++) _b.Set (i, 0, data[i]);
        delete[] data;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::RollDown (Muster& _b)
{
    if (mx==-1 || my==-1) return;
    try {
        char* data = new char[mx+1];
        for (int i=0; i<=mx; i++) data[i] = _b.Get (i, 0);
        for (int j=0; j<my; j++)
            for (int i=0; i<=mx; i++)
                _b.Set (i, j, _b.Get (i, j+1));
        for (int i=0; i<=mx; i++) _b.Set (i, my, data[i]);
        delete[] data;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::RollLeft (Muster& _b)
{
    if (mx==-1 || my==-1) return;
    try {
        char* data = new char[my+1];
        for (int j=0; j<=my; j++) data[j] = _b.Get (0, j);
        for (int i=0; i<mx; i++)
            for (int j=0; j<=my; j++)
                _b.Set (i, j, _b.Get (i+1, j));
        for (int j=0; j<=mx; j++) _b.Set (mx, j, data[j]);
        delete[] data;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::RollRight (Muster& _b)
{
    if (mx==-1 || my==-1) return;
    try {
        char* data = new char[my+1];
        for (int j=0; j<=my; j++) data[j] = _b.Get (mx, j);
        for (int i=mx; i>0; i--)
            for (int j=0; j<=my; j++)
                _b.Set (i, j, _b.Get (i-1, j));
        for (int j=0; j<=mx; j++) _b.Set (0, j, data[j]);
        delete[] data;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::MusterUndoClick(TObject *Sender)
{
    if (undo.CanUndo()) {
        undo.Undo();
        CalcRange();
        switch (current) {
            case 0: Bindung0Click (this); break;
            case 1: Bindung1Click (this); break;
            case 2: Bindung2Click (this); break;
            case 3: Bindung3Click (this); break;
            case 4: Bindung4Click (this); break;
            case 5: Bindung5Click (this); break;
            case 6: Bindung6Click (this); break;
            case 7: Bindung7Click (this); break;
            case 8: Bindung8Click (this); break;
            case 9: Bindung9Click (this); break;
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::MusterRedoClick(TObject *Sender)
{
    if (undo.CanRedo()) {
        undo.Redo();
        CalcRange();
        switch (current) {
            case 0: Bindung0Click (this); break;
            case 1: Bindung1Click (this); break;
            case 2: Bindung2Click (this); break;
            case 3: Bindung3Click (this); break;
            case 4: Bindung4Click (this); break;
            case 5: Bindung5Click (this); break;
            case 6: Bindung6Click (this); break;
            case 7: Bindung7Click (this); break;
            case 8: Bindung8Click (this); break;
            case 9: Bindung9Click (this); break;
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::updatetimerTimer(TObject *Sender)
{
    MusterUndo->Enabled = undo.CanUndo();
    MusterRedo->Enabled = undo.CanRedo();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::FormActivate(TObject *Sender)
{
    updatetimer->Enabled = true;
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::FormDeactivate(TObject *Sender)
{
    updatetimer->Enabled = false;
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Bindung0Click(TObject *Sender)
{
    current = 0;
    Bindung0->Checked = true;
    description->Caption = RANGE_GRUNDBINDUNG;
    Neuzeichnen();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Bindung1Click(TObject *Sender)
{
    current = 1;
    Bindung1->Checked = true;
    description->Caption = String(RANGE_BINDUNG) + IntToStr(current);
    Neuzeichnen();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Bindung2Click(TObject *Sender)
{
    current = 2;
    Bindung2->Checked = true;
    description->Caption = String(RANGE_BINDUNG) + IntToStr(current);
    Neuzeichnen();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Bindung3Click(TObject *Sender)
{
    current = 3;
    Bindung3->Checked = true;
    description->Caption = String(RANGE_BINDUNG) + IntToStr(current);
    Neuzeichnen();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Bindung4Click(TObject *Sender)
{
    current = 4;
    Bindung4->Checked = true;
    description->Caption = String(RANGE_BINDUNG) + IntToStr(current);
    Neuzeichnen();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Bindung5Click(TObject *Sender)
{
    current = 5;
    Bindung5->Checked = true;
    description->Caption = String(RANGE_BINDUNG) + IntToStr(current);
    Neuzeichnen();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Bindung6Click(TObject *Sender)
{
    current = 6;
    Bindung6->Checked = true;
    description->Caption = String(RANGE_BINDUNG) + IntToStr(current);
    Neuzeichnen();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Bindung7Click(TObject *Sender)
{
    current = 7;
    Bindung7->Checked = true;
    description->Caption = String(RANGE_BINDUNG) + IntToStr(current);
    Neuzeichnen();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Bindung8Click(TObject *Sender)
{
    current = 8;
    Bindung8->Checked = true;
    description->Caption = String(RANGE_BINDUNG) + IntToStr(current);
    Neuzeichnen();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Bindung9Click(TObject *Sender)
{
    current = 9;
    Bindung9->Checked = true;
    description->Caption = String(RANGE_BINDUNG) + IntToStr(current);
    Neuzeichnen();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::CopyFromB0Click(TObject *Sender)
{
    Grab (0);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::CopyFromB1Click(TObject *Sender)
{
    Grab (1);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::CopyFromB2Click(TObject *Sender)
{
    Grab (2);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::CopyFromB3Click(TObject *Sender)
{
    Grab (3);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::CopyFromB4Click(TObject *Sender)
{
    Grab (4);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::CopyFromB5Click(TObject *Sender)
{
    Grab (5);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::CopyFromB6Click(TObject *Sender)
{
    Grab (6);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::CopyFromB7Click(TObject *Sender)
{
    Grab (7);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::CopyFromB8Click(TObject *Sender)
{
    Grab (8);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::CopyFromB9Click(TObject *Sender)
{
    Grab (9);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Grab(int _from)
{
    dbw3_assert (_from>=0 && _from<10);
    if (_from==current) return;
    if (mx==-1 || my==-1) return;
    for (int i=0; i<=mx; i++)
        for (int j=0; j<=my; j++) {
            char c = (*bindungen)[_from].Get (i, j);
            c = Toggle(Toggle(c));
            (*bindungen)[current].Set (i, j, c);
        }
    Neuzeichnen();
    undo.Snapshot();
}
/*-----------------------------------------------------------------*/
TColor __fastcall TBlockmusterForm::GetColor (int _s)
{
    dbw3_assert (_s>=0 && _s<10);
    if (_s==0) return GBCOLOR;
    else return GetRangeColor(_s);
}
/*-----------------------------------------------------------------*/
char __fastcall TBlockmusterForm::Toggle (char _s)
{
    if (_s==0) {
        if (current==0) return 1;
        else return char(current);
    } else
        return 0;
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Neuzeichnen()
{
    pbBindung->Repaint();
    pbUsed->Repaint();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::pbUsedPaint(TObject *Sender)
{
    int x = 0;

    if ((*bindungen)[0].IsEmpty()) pbUsed->Canvas->Font->Color = clDkGray;
    else pbUsed->Canvas->Font->Color = clRed;
    pbUsed->Canvas->TextOut (x, 0, LANG_STR("B", "G"));
    x += pbUsed->Canvas->TextWidth(LANG_STR("B", "G"));

    for (int i=1; i<10; i++) {
        if ((*bindungen)[i].IsEmpty()) pbUsed->Canvas->Font->Color = clDkGray;
        else pbUsed->Canvas->Font->Color = clRed;
        pbUsed->Canvas->TextOut (x, 0, IntToStr(i));
        x += pbUsed->Canvas->TextWidth(IntToStr(i));
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::LoadLanguage()
{
    LANG_C_H(BlockMuster, EN, "&Pattern", "")
    LANG_C_H(BlockMuster, GE, "&Muster", "")
    LANG_C_H(BlockDoit, EN, "&Apply", "Creates the substitution")
    LANG_C_H(BlockDoit, GE, "&Anwenden", "Erstellt die Substitution")
    LANG_C_H(BlockClose, EN, "&Close", "Leaves substitutuion mode")
    LANG_C_H(BlockClose, GE, "&Schliessen", "Verlässt den Substitutionsmodus")
    LANG_C_H(BlockEinzug, EN, "&Threading", "")
    LANG_C_H(BlockEinzug, GE, "&Einzug", "")
    LANG_C_H(EinzugZ, EN, "Straight &rising", "Creates a straight rising threading")
    LANG_C_H(EinzugZ, GE, "Gerade &steigend", "Erstellt einen gerade steigenden Einzug")
    LANG_C_H(EinzugS, EN, "Straight &falling", "Creates a straight falling threading")
    LANG_C_H(EinzugS, GE, "Gerade &fallend", "Erstellt einen gerade fallenden Einzug")
    LANG_C_H(BlockTrittfolge, EN, "Tre&adling", "")
    LANG_C_H(BlockTrittfolge, GE, "&Trittfolge", "")
    LANG_C_H(TrittfolgeZ, EN, "Straight &rising", "Creates a straight rising treadling")
    LANG_C_H(TrittfolgeZ, GE, "Gerade &steigend", "Erstellt eine gerade steigende Trittfolge")
    LANG_C_H(TrittfolgeS, EN, "Straight &falling", "Creates a straight falling treadling")
    LANG_C_H(TrittfolgeS, GE, "Gerade &fallend", "Erstellt eine gerade fallende Trittfolge")
    LANG_C_H(MenuBindung1, EN, "&Edit", "")
    LANG_C_H(MenuBindung1, GE, "&Bearbeiten", "")
    LANG_C_H(EditDelete, EN, "&Delete", "Deletes the pattern")
    LANG_C_H(EditDelete, GE, "&Löschen", "Löscht das Muster")
    LANG_C_H(EditSpiegelnH, EN, "Mirror &horizontal", "Mirrors the pattern in horizontal direction")
    LANG_C_H(EditSpiegelnH, GE, "Spiegeln &horizontal", "Spiegelt das Muster in horizontaler Richtung")
    LANG_C_H(EditSpiegelnV, EN, "Mirror &vertical", "Mirrors the pattern in vertical direction")
    LANG_C_H(EditSpiegelnV, GE, "Spiegeln &vertikal", "Spiegelt das Muster in vertikaler Richtung")
    LANG_C_H(EditRotieren, EN, "&Rotate", "Rotates the square pattern in clockwise direction")
    LANG_C_H(EditRotieren, GE, "&Rotieren", "Rotiert das quadratische Muster im Uhrzeigersinn")
    LANG_C_H(EditInvertieren, EN, "&Invert", "Inverts the pattern")
    LANG_C_H(EditInvertieren, GE, "&Invertieren", "Invertiert das Muster")
    LANG_C_H(EditCentralsym, EN, "&Make central symmetric", "Tries to make the pattern central symmetric")
    LANG_C_H(EditCentralsym, GE, "&Zentralsymmetrisch", "Versucht das Muster zentralsymmetrisch zu machen")
    LANG_C_H(Muster1, EN, "&Binding", "")
    LANG_C_H(Muster1, GE, "&Bindung", "")
    LANG_C_H(Leinwand, EN, "Tabby", "")
    LANG_C_H(Leinwand, GE, "Leinwand", "")
    LANG_C_H(Koeper22, EN, "Twill 2/2", "")
    LANG_C_H(Koeper22, GE, "Köper 2/2", "")
    LANG_C_H(Koeper33, EN, "Twill 3/3", "")
    LANG_C_H(Koeper33, GE, "Köper 3/3", "")
    LANG_C_H(Koeper44, EN, "Twill 4/4", "")
    LANG_C_H(Koeper44, GE, "Köper 4/4", "")
    LANG_C_H(Kettkoeper21, EN, "Twill 2/1", "")
    LANG_C_H(Kettkoeper21, GE, "Köper 2/1", "")
    LANG_C_H(Kettkoeper31, EN, "Twill 3/1", "")
    LANG_C_H(Kettkoeper31, GE, "Köper 3/1", "")
    LANG_C_H(Kettkoeper32, EN, "Twill 3/2", "")
    LANG_C_H(Kettkoeper32, GE, "Köper 3/2", "")
    LANG_C_H(Kettkoeper41, EN, "Twill 4/1", "")
    LANG_C_H(Kettkoeper41, GE, "Köper 4/1", "")
    LANG_C_H(Kettkoeper51, EN, "Twill 5/1", "")
    LANG_C_H(Kettkoeper51, GE, "Köper 5/1", "")
    LANG_C_H(Kettkoeper52, EN, "Twill 5/2", "")
    LANG_C_H(Kettkoeper52, GE, "Köper 5/2", "")
    LANG_C_H(Atlas5, EN, "Satin 5x5", "")
    LANG_C_H(Atlas5, GE, "Atlas 5x5", "")
    LANG_C_H(Atlas7, EN, "Satin 7x7", "")
    LANG_C_H(Atlas7, GE, "Atlas 7x7", "")
    LANG_C_H(Atlas9, EN, "Satin 9x9", "")
    LANG_C_H(Atlas9, GE, "Atlas 9x9", "")
    LANG_C_H(Panama21, EN, "Extended tabby 2/1", "")
    LANG_C_H(Panama21, GE, "Panama 2/1", "")
    LANG_C_H(Panama22, EN, "Extended tabby 2/2", "")
    LANG_C_H(Panama22, GE, "Panama 2/2", "")
    LANG_C_H(LeftRoll, EN, "&Roll", "")
    LANG_C_H(LeftRoll, GE, "&Rollen", "")
    LANG_C_H(EditRollUp, EN, "&Up", "Rolls the pattern up")
    LANG_C_H(EditRollUp, GE, "Nach &oben", "Rollt das Muster nach oben")
    LANG_C_H(EditRollDown, EN, "&Down", "Rolls the pattern down")
    LANG_C_H(EditRollDown, GE, "Nach &unten", "Rollt das Muster nach unten")
    LANG_C_H(EditRollLeft, EN, "&Left", "Rolls the pattern left")
    LANG_C_H(EditRollLeft, GE, "Nach &links", "Rollt das Muster nach links")
    LANG_C_H(EditRollRight, EN, "&Right", "Rolls the pattern right")
    LANG_C_H(EditRollRight, GE, "Nach &rechts", "Rollt das Muster nach rechts")
    LANG_C_H(MusterUndo, EN, "&Undo", "Undoes the last change")
    LANG_C_H(MusterUndo, GE, "&Rückgängig", "Macht die letzte Änderung rückgängig")
    LANG_C_H(MusterRedo, EN, "&Redo", "Redoes the last undone change")
    LANG_C_H(MusterRedo, GE, "&Wiederholen", "Wiederholt die letzte rückgängig gemachte Änderung")
    LANG_C_H(Kettkoeper42, EN, "Twill 4/2", "")
    LANG_C_H(Kettkoeper42, GE, "Köper 4/2", "")
    LANG_C_H(Kettkoeper43, EN, "Twill 4/3", "")
    LANG_C_H(Kettkoeper43, GE, "Köper 4/3", "")
    LANG_C_H(Kettkoeper53, EN, "Twill 5/3", "")
    LANG_C_H(Kettkoeper53, GE, "Köper 5/3", "")
    LANG_C_H(description, EN, "Base pattern", "")
    LANG_C_H(description, GE, "Muster für den Grund", "")
    LANG_C_H(Bindung0, EN, "&Base pattern", "")
    LANG_C_H(Bindung0, GE, "&Grundmuster", "")
    LANG_C_H(Bindung1, EN, "Pattern &1", "")
    LANG_C_H(Bindung1, GE, "Muster &1", "")
    LANG_C_H(Bindung2, EN, "Pattern &2", "")
    LANG_C_H(Bindung2, GE, "Muster &2", "")
    LANG_C_H(Bindung3, EN, "Pattern &3", "")
    LANG_C_H(Bindung3, GE, "Muster &3", "")
    LANG_C_H(Bindung4, EN, "Pattern &4", "")
    LANG_C_H(Bindung4, GE, "Muster &4", "")
    LANG_C_H(Bindung5, EN, "Pattern &5", "")
    LANG_C_H(Bindung5, GE, "Muster &5", "")
    LANG_C_H(Bindung6, EN, "Pattern &6", "")
    LANG_C_H(Bindung6, GE, "Muster &6", "")
    LANG_C_H(Bindung7, EN, "Pattern &7", "")
    LANG_C_H(Bindung7, GE, "Muster &7", "")
    LANG_C_H(Bindung8, EN, "Pattern &8", "")
    LANG_C_H(Bindung8, GE, "Muster &8", "")
    LANG_C_H(Bindung9, EN, "Pattern &9", "")
    LANG_C_H(Bindung9, GE, "Muster &9", "")
    LANG_C_H(CopyFrom, EN, "&Copy from", "")
    LANG_C_H(CopyFrom, GE, "&Kopieren von", "")
    LANG_C_H(CopyFromB0, EN, "&Base pattern", "")
    LANG_C_H(CopyFromB0, GE, "&Grundmuster", "")
    LANG_C_H(CopyFromB1, EN, "Pattern &1", "")
    LANG_C_H(CopyFromB1, GE, "Muster &1", "")
    LANG_C_H(CopyFromB9, EN, "Pattern &9", "")
    LANG_C_H(CopyFromB9, GE, "Muster &9", "")
    LANG_C_H(CopyFromB8, EN, "Pattern &8", "")
    LANG_C_H(CopyFromB8, GE, "Muster &8", "")
    LANG_C_H(CopyFromB7, EN, "Pattern &7", "")
    LANG_C_H(CopyFromB7, GE, "Muster &7", "")
    LANG_C_H(CopyFromB6, EN, "Pattern &6", "")
    LANG_C_H(CopyFromB6, GE, "Muster &6", "")
    LANG_C_H(CopyFromB5, EN, "Pattern &5", "")
    LANG_C_H(CopyFromB5, GE, "Muster &5", "")
    LANG_C_H(CopyFromB4, EN, "Pattern &4", "")
    LANG_C_H(CopyFromB4, GE, "Muster &4", "")
    LANG_C_H(CopyFromB3, EN, "Pattern &3", "")
    LANG_C_H(CopyFromB3, GE, "Muster &3", "")
    LANG_C_H(CopyFromB2, EN, "Pattern &2", "")
    LANG_C_H(CopyFromB2, GE, "Muster &2", "")
    LANG_C_H(belegte, EN, "Used patterns", "")
    LANG_C_H(belegte, GE, "Belegte Muster", "")
}
/*-----------------------------------------------------------------*/

