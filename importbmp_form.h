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
#ifndef importbmp_formH
#define importbmp_formH
/*-----------------------------------------------------------------*/
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
/*-----------------------------------------------------------------*/
class TImportBmpForm : public TForm
{
__published:	// IDE-managed Components
    TImage *Bitmap;
    TLabel *lBitmap;
    TLabel *lPreview;
    TLabel *lResolution;
    TEdit *Resolution;
    TButton *bOk;
    TButton *bCancel;
    TPaintBox *Preview;
    TTimer *lazytimer;
    void __fastcall ResolutionChange(TObject *Sender);
    void __fastcall PreviewPaint(TObject *Sender);
    void __fastcall lazytimerTimer(TObject *Sender);
    void __fastcall FormActivate(TObject *Sender);
    void __fastcall FormDeactivate(TObject *Sender);
    void __fastcall bOkClick(TObject *Sender);
    
private:	// User declarations
    TPicture* Picture;
    bool needrepaint;
    TColor colors[9];
    TColor bkground;
    int used;
    void __fastcall BuildColorTable();
    TColor __fastcall GetRangeCol (TColor _col);
    char __fastcall GetRange (TColor _col);
    void __fastcall LoadLanguage();
public:		// User declarations
    int x, y;
    char* data;
    __fastcall TImportBmpForm(TComponent* Owner, String _filename);
    virtual __fastcall ~TImportBmpForm();
};
/*-----------------------------------------------------------------*/
#endif
