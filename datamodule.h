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

#ifndef datamoduleH
#define datamoduleH

#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Dialogs.hpp>
#include <ExtDlgs.hpp>

// Forward-Deklarationen
class FileProperties;
class Palette;

class TData : public TDataModule
{
__published:    // IDE-managed Components
    TPrinterSetupDialog *PrinterSetupDialog;
    TPrintDialog *PrintDialog;
    TOpenDialog *OpenDialog;
    TSaveDialog *SaveDialog;
    TOpenPictureDialog *ImportBmpDialog;
    TSaveDialog *ExportDialog;
	TOpenDialog *ImportDialog;
private:    // User declarations

public:
    // Ausmasse der Felder.
    volatile int MAXX1, MAXY1;
    volatile int MAXX2, MAXY2;

    FileProperties* properties;
	Palette* palette;
    unsigned char color;
    unsigned char defcolorh;
    unsigned char defcolorv;

public:        // User declarations
    __fastcall TData(TComponent* Owner);
    virtual __fastcall ~TData();
    void __fastcall ReloadLanguage();
};

extern PACKAGE TData *Data;

#endif

