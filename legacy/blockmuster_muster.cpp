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
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "blockmuster_form.h"
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::MusterKoeper (int _current, int _h, int _s)
{
    Muster& m = (*bindungen)[_current];

    m.Clear();

    int n = _h + _s;
    if (_h<=_s) {
        for (int i=0; i<n; i++)
            for (int j=i; j<i+_h; j++)
                m.Set (i, j%n, char(current!=0 ? current : 1));
    } else {
        for (int i=0; i<n; i++)
            for (int j=i+_s; j<i+n; j++)
                m.Set (i, j%n, char(current!=0 ? current : 1));
    }

    CalcRange();
    Neuzeichnen();
    undo.Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::MusterAtlas (int _current, int _n)
{
    Muster& m = (*bindungen)[_current];
    char s = char(current!=0 ? current : 1);

    m.Clear();
    switch (_n) {
        case 5:
            m.Set (0, 0, s); m.Set (1, 2, s); m.Set (2, 4, s);
            m.Set (3, 1, s); m.Set (4, 3, s);
            break;
        case 7:
            m.Set (0, 2, s); m.Set (1, 6, s); m.Set (2, 3, s);
            m.Set (3, 0, s); m.Set (4, 4, s); m.Set (5, 1, s);
            m.Set (6, 5, s);
            break;
        case 9:
            m.Set (0, 0, s); m.Set (1, 2, s); m.Set (2, 4, s);
            m.Set (3, 6, s); m.Set (4, 8, s); m.Set (5, 1, s);
            m.Set (6, 3, s); m.Set (7, 5, s); m.Set (8, 7, s);
            break;
    }

    CalcRange();
    Neuzeichnen();
    undo.Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::MusterPanama (int _current, int _h, int _s)
{
    Muster& m = (*bindungen)[_current];

    m.Clear();

    for (int i=0; i<_h; i++)
        for (int j=0; j<_h; j++)
            m.Set (i, j, char(current!=0 ? current : 1));

    for (int i=_h; i<_h+_s; i++)
        for (int j=_h; j<_h+_s; j++)
            m.Set (i, j, char(current!=0 ? current : 1));

    CalcRange();
    Neuzeichnen();
    undo.Snapshot();
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::LeinwandClick(TObject *Sender)
{
    MusterKoeper (current, 1, 1);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Koeper22Click(TObject *Sender)
{
    MusterKoeper (current, 2, 2);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Koeper33Click(TObject *Sender)
{
    MusterKoeper (current, 3, 3);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Koeper44Click(TObject *Sender)
{
    MusterKoeper (current, 4, 4);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Kettkoeper21Click(TObject *Sender)
{
    MusterKoeper (current, 2, 1);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Kettkoeper31Click(TObject *Sender)
{
    MusterKoeper (current, 3, 1);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Kettkoeper41Click(TObject *Sender)
{
    MusterKoeper (current, 4, 1);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Kettkoeper51Click(TObject *Sender)
{
    MusterKoeper (current, 5, 1);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Kettkoeper32Click(TObject *Sender)
{
    MusterKoeper (current, 3, 2);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Kettkoeper42Click(TObject *Sender)
{
    MusterKoeper (current, 4, 2);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Kettkoeper52Click(TObject *Sender)
{
    MusterKoeper (current, 5, 2);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Kettkoeper43Click(TObject *Sender)
{
    MusterKoeper (current, 4, 3);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Kettkoeper53Click(TObject *Sender)
{
    MusterKoeper (current, 5, 3);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Atlas5Click(TObject *Sender)
{
    MusterAtlas (current, 5);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Atlas7Click(TObject *Sender)
{
    MusterAtlas (current, 7);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Atlas9Click(TObject *Sender)
{
    MusterAtlas (current, 9);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Panama21Click(TObject *Sender)
{
    MusterPanama (current, 2, 1);
}
/*-----------------------------------------------------------------*/
void __fastcall TBlockmusterForm::Panama22Click(TObject *Sender)
{
    MusterPanama (current, 2, 2);
}
/*-----------------------------------------------------------------*/

