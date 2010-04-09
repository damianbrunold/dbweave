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
#include "dbw3_form.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBFileNewClick(TObject *Sender)
{
    FileNewClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBFileOpenClick(TObject *Sender)
{
    FileOpenClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBSaveClick(TObject *Sender)
{
    FileSaveClick (Sender);
}
/*-----------------------------------------------------------------*/
// Druckvorschau und Drucken sind in print.cpp!
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBEditCutClick(TObject *Sender)
{
    EditCutClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBEditCopyClick(TObject *Sender)
{
    EditCopyClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBEditPasteClick(TObject *Sender)
{
    //xxx Eventuell auch EditPasteTransparentClick...
    EditPasteClick (Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBUndoClick(TObject *Sender)
{
    EditUndoClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBRedoClick(TObject *Sender)
{
    EditRedoClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBInvertClick(TObject *Sender)
{
    EditInvertClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBMirrorHClick(TObject *Sender)
{
    EditMirrorHorzClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBMirrorVClick(TObject *Sender)
{
    EditMirrorVertClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBRotateClick(TObject *Sender)
{
    EditRotateClick(Sender);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::SBCentralsymClick(TObject *Sender)
{
    EditCentralsymClick(Sender);
}
/*-----------------------------------------------------------------*/
int __fastcall TDBWFRM::UpdateToolButton (TSpeedButton* _sb, int _x)
{
#if(0)
    if (_sb->Enabled) {
        if (!_sb->Visible) _sb->Visible = true;
        if (_sb->Left!=_x) _sb->Left = _x;
        _x += _sb->Width;
    } else {
        if (_sb->Visible) _sb->Visible = false;
    }
#else
    if (_sb->Left!=_x) _sb->Left = _x;
    _x += _sb->Width;
#endif
    return _x;
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::UpdateToolButtons()
{
    int x = ToolSeparator->Left + ToolSeparator->Width;
    x = UpdateToolButton (SBMoveUp, x);
    x = UpdateToolButton (SBMoveDown, x);
    x = UpdateToolButton (SBMoveLeft, x);
    x = UpdateToolButton (SBMoveRight, x);
    x = UpdateToolButton (SBInvert, x);
    x = UpdateToolButton (SBMirrorH, x);
    x = UpdateToolButton (SBMirrorV, x);
    x = UpdateToolButton (SBRotate, x);
    x = UpdateToolButton (SBCentralsym, x);
    UpdateToolButton (SBHighlight, x);
}
/*-----------------------------------------------------------------*/

