/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File splash_form.cpp
 * Copyright (C) 2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "splash_form.h"
#include "dbw3_form.h"
#include "dbw3_strings.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
SplashData splashdata;
/*-----------------------------------------------------------------*/
__fastcall TSplashscreen::TSplashscreen(TComponent* Owner)
: TForm(Owner)
{
    Visible = splashdata.showsplash;
    alreadydone = false;
}
/*-----------------------------------------------------------------*/
void __fastcall TSplashscreen::FormShow(TObject *Sender)
{
    if (!alreadydone) {
        DrawData();
        alreadydone = true;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSplashscreen::SplashimageClick(TObject *Sender)
{
    alreadydone = true;
    if (!splashdata.demo) Visible = false;
}
/*-----------------------------------------------------------------*/
#define SPL_BASE_X 50
#define SPL_BASE_Y 200
#define SPL_BAR_Y (SPL_BASE_Y + 70)
/*-----------------------------------------------------------------*/
#define DEMOVERSIONTIMEMSG LANG_STR("This trial version has been used for %d days. You have %d days left.", "Diese Probeversion wurde schon %d Tage benutzt. Sie haben noch %d Tage Zeit.")
#define DEMOVERSIONEXPIRED LANG_STR("The trial version has been used for more than 30 days and has expired.", "Diese Probeversion wurde mehr als 30 Tage benutzt und ist abgelaufen.")
/*-----------------------------------------------------------------*/
void __fastcall TSplashscreen::DrawData()
{
    Splashimage->Canvas->Font->Name = "Arial";
    SetBkMode (Splashimage->Canvas->Handle, TRANSPARENT);

    if (splashdata.demo) {
        // Text ausgeben...
        if (splashdata.daysmax>=splashdata.dayselapsed) {
            char buff[200];
            wsprintf (buff, DEMOVERSIONTIMEMSG.c_str(), splashdata.dayselapsed, splashdata.daysmax-splashdata.dayselapsed);
            Splashimage->Canvas->TextOut (SPL_BASE_X, SPL_BAR_Y+25, buff);
        } else {
            Splashimage->Canvas->TextOut (SPL_BASE_X, SPL_BAR_Y+25, DEMOVERSIONEXPIRED);
        }

        // Fortschrittsbalken zeichnen
        int maxx = Splashimage->Width - 2*SPL_BASE_X;
        Splashimage->Canvas->Pen->Color = clDkGray;
        Splashimage->Canvas->Brush->Color = clDkGray;
        Splashimage->Canvas->Rectangle (SPL_BASE_X, SPL_BAR_Y, SPL_BASE_X+maxx, SPL_BAR_Y+20);

        int xx = maxx*splashdata.dayselapsed/splashdata.daysmax;
        if (splashdata.dayselapsed>=splashdata.daysmax) xx = maxx;
        Splashimage->Canvas->Pen->Color = clRed;
        Splashimage->Canvas->Brush->Color = clRed;
        Splashimage->Canvas->Rectangle (SPL_BASE_X, SPL_BAR_Y, SPL_BASE_X+xx, SPL_BAR_Y+20);
    } else {
        Splashimage->Canvas->TextOut (SPL_BASE_X, SPL_BASE_Y+0, splashdata.user);
        Splashimage->Canvas->TextOut (SPL_BASE_X, SPL_BASE_Y+20, splashdata.organisation);
        Splashimage->Canvas->TextOut (SPL_BASE_X, SPL_BASE_Y+40, splashdata.serialnr);
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSplashscreen::FormKeyPress(TObject *Sender, char &Key)
{
    alreadydone = true;
    if (!splashdata.demo) Visible = false;
}
/*-----------------------------------------------------------------*/

