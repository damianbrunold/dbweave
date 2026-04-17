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
#ifndef entervv_formH
#define entervv_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
/*-----------------------------------------------------------------*/
class TEnterVVForm : public TForm
{
__published:	// IDE-managed Components
    TLabel *labVV;
    TEdit *VV1;
    TLabel *labVV12;
    TEdit *VV2;
    TLabel *labVV23;
    TEdit *VV3;
    TLabel *labVV34;
    TEdit *VV4;
    TLabel *labVV45;
    TEdit *VV5;
    TLabel *labVV56;
    TEdit *VV6;
    TButton *bOk;
    TButton *bCancel;
    void __fastcall bOkClick(TObject *Sender);
    void __fastcall VV1Change(TObject *Sender);
    void __fastcall VV2Change(TObject *Sender);
    void __fastcall VV3Change(TObject *Sender);
    void __fastcall VV4Change(TObject *Sender);
    void __fastcall VV5Change(TObject *Sender);
    void __fastcall VV6Change(TObject *Sender);
private:	// User declarations
    void __fastcall LoadLanguage();
public:		// User declarations
    __fastcall TEnterVVForm(TComponent* Owner);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
