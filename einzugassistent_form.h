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
#ifndef einzugassistent_formH
#define einzugassistent_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
/*-----------------------------------------------------------------*/
class TDBWFRM;
/*-----------------------------------------------------------------*/
class TEinzugassistentForm : public TForm
{
__published:	// IDE-managed Components
    TButton *bOK;
    TButton *bCancel;
    TPageControl *einzugarten;
    TTabSheet *pGeradedurch;
    TLabel *lSchaefte;
    TLabel *lFirstKettfaden;
    TLabel *lFirstSchaft;
    TEdit *Schaefte1;
    TGroupBox *gbOrientation;
    TRadioButton *ZGrat1;
    TRadioButton *SGrat1;
    TEdit *FirstKettfaden1;
    TUpDown *upFirstKettfaden1;
    TUpDown *upSchaefte1;
    TEdit *FirstSchaft1;
    TUpDown *upFirstSchaft1;
    TTabSheet *pAbgesetzt;
    TLabel *lErsterKettfaden2;
    TLabel *lErsterSchaft2;
    TLabel *lSchaefte2;
    TLabel *lGratlen;
    TLabel *lVersatz;
    TEdit *FirstKettfaden2;
    TUpDown *upFirstKettfaden2;
    TEdit *FirstSchaft2;
    TUpDown *upFirstSchaft2;
    TEdit *Schaefte2;
    TUpDown *upSchaefte2;
    TEdit *Gratlen;
    TUpDown *upGratlen;
    TEdit *Versatz;
    TUpDown *upVersatz;
    void __fastcall bOKClick(TObject *Sender);
    
private:	// User declarations
    TDBWFRM* frm;
    void __fastcall Recalc();
    void __fastcall CreateGerade (int _firstkettfaden, int _firstschaft, int _schaefte, bool _steigend);
    void __fastcall CreateAbgesetzt (int _firstkettfaden, int _firstschaft, int _schaefte, int _gratlen, int _versatz);
public:		// User declarations
    __fastcall TEinzugassistentForm(TDBWFRM* _frm, TComponent* Owner);
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/

