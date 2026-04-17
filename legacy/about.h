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
#ifndef about_formH
#define about_formH
/*-----------------------------------------------------------------*/
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\ComCtrls.hpp>
#include <Graphics.hpp>
/*-----------------------------------------------------------------*/
class TAboutBox : public TForm
{
__published:    // IDE-managed Components
    TButton* OKButton;
    TImage *Bkground;
    TButton *bTechinfo;
    TPaintBox *pbInfos;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall BkgroundDblClick(TObject *Sender);
    void __fastcall bTechinfoClick(TObject *Sender);
    void __fastcall pbInfosPaint(TObject *Sender);

private:    // User declarations
    void __fastcall LoadLanguage();
public:        // User declarations
    __fastcall TAboutBox (TComponent* Owner);
    __fastcall ~TAboutBox();
};
/*-----------------------------------------------------------------*/
extern PACKAGE TAboutBox *AboutBox;
/*-----------------------------------------------------------------*/
#endif