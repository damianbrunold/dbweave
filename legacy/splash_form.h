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
#ifndef splash_formH
#define splash_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
/*-----------------------------------------------------------------*/
struct SplashData
{
    bool showsplash;
    bool demo;
    AnsiString user;
    AnsiString organisation;
    AnsiString serialnr;
    int dayselapsed;
    int daysmax;
};
/*-----------------------------------------------------------------*/
extern SplashData splashdata;
/*-----------------------------------------------------------------*/
class TSplashscreen : public TForm
{
__published:	// IDE-managed Components
    TImage *Splashimage;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall SplashimageClick(TObject *Sender);
    void __fastcall FormKeyPress(TObject *Sender, char &Key);
private:	// User declarations
    bool alreadydone;
    void __fastcall DrawData();
public:		// User declarations
    __fastcall TSplashscreen(TComponent* Owner);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
