/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File about.cpp
 * Copyright (C) 1998-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "about.h"
#include "dbw3_form.h"
#include "version.h"
#include "dbw3_strings.h"
#include "language.h"
#include "techinfo_form.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
TAboutBox *AboutBox;
/*-----------------------------------------------------------------*/
__fastcall TAboutBox::TAboutBox(TComponent* AOwner)
: TForm(AOwner)
{
    LoadLanguage();
}
/*-----------------------------------------------------------------*/
__fastcall TAboutBox::~TAboutBox()
{
}
/*-----------------------------------------------------------------*/
void __fastcall TAboutBox::BkgroundDblClick(TObject *Sender)
{
    // Secret?!
    if ((GetKeyState (VK_SHIFT) & 0x8000) &&
        (GetKeyState (VK_CONTROL) & 0x8000) &&
        (GetKeyState (VK_MENU) & 0x8000))
    {
        Application->MessageBox ("Programmed by Damian Brunold.\n"
                    "Many have contributed to this\n"
                    "product by discussing matters\n"
                    "with me or by suggesting new\n"
                    "features and by finding bugs.\n"
                    "Thanks to all of you folks...", APP_TITLE, MB_OK);
    }
}
/*-----------------------------------------------------------------*/
#define DEMOVERSIONTIMEMSG LANG_STR("%d of %d days used", "%d von %d Tagen benutzt")
#define DEMOVERSIONEXPIRED LANG_STR("Trial expired", "Probe abgelaufen")
/*-----------------------------------------------------------------*/
void __fastcall TAboutBox::FormShow(TObject *Sender)
{
    // Buttons positionieren
    OKButton->Left = ClientWidth - 20 - OKButton->Width;
    OKButton->Top = ClientHeight - OKButton->Height*3/2;
    bTechinfo->Left = OKButton->Left - 20 - bTechinfo->Width;
    bTechinfo->Top = OKButton->Top;

    // Paintbox ausrichten
    pbInfos->Width = ClientWidth;
    pbInfos->Top = Bkground->Top+Bkground->Height+1;
    pbInfos->Height = ClientHeight - pbInfos->Top;
}
/*-----------------------------------------------------------------*/
void __fastcall TAboutBox::LoadLanguage()
{
    LANG_C_H(this, EN, "About DB-WEAVE", "")
    LANG_C_H(this, GE, "Über DB-WEAVE", "")
    LANG_C_H(bTechinfo, EN, "&Technical Info...", "")
    LANG_C_H(bTechinfo, GE, "&Technische Info...", "")
    LANG_C_H(OKButton, EN, "OK", "")
    LANG_C_H(OKButton, GE, "OK", "")
}
/*-----------------------------------------------------------------*/
void __fastcall TAboutBox::bTechinfoClick(TObject *Sender)
{
    Hide();
    try {
        TTechinfoForm* pFrm = new TTechinfoForm(this);
        pFrm->ShowModal();
        delete pFrm;
    } catch (...) {
    }
    Show();
}
/*-----------------------------------------------------------------*/
void __fastcall TAboutBox::pbInfosPaint(TObject *Sender)
{
    // Infos ausgeben
    TCanvas* canvas = pbInfos->Canvas;

    // Hintergrund passend zum Bild
    canvas->Brush->Color = TColor(RGB(240,240,230));
    canvas->Pen->Color = canvas->Brush->Color;
    canvas->Rectangle (0, 0, pbInfos->Width, pbInfos->Height);

    canvas->Font->Color = clBlack;
    int dy = canvas->Font->Height;
    if (dy<0) {
        dy = -dy;
        if (PixelsPerInch==96) dy += 3;
        else if (PixelsPerInch>96) dy += int(3.0*PixelsPerInch/96); 
    }
    int x0 = 35;
    int y0 = 20;

    // Infos über DB-WEAVE ausgeben
    if (active_language==GE) {
        canvas->Font->Style = canvas->Font->Style << fsBold;
        canvas->TextOut (x0, y0, "Was DB-WEAVE ist"); y0 += dy*3/2;
        canvas->Font->Style = canvas->Font->Style >> fsBold;
        canvas->TextOut (x0, y0, "DB-WEAVE ist eine Software um Gewebe"); y0+=dy;
        canvas->TextOut (x0, y0, "zu entwickeln und auf einem Schaftweb-"); y0+=dy;
        canvas->TextOut (x0, y0, "stuhl zu weben. Sie können Muster ent-"); y0+=dy;
        canvas->TextOut (x0, y0, "werfen und diese nachher manuell weben."); y0+=dy;
        canvas->TextOut (x0, y0, "Oder Sie können die Muster direkt auf"); y0+=dy;
        canvas->TextOut (x0, y0, "einem computergesteuerten Webstuhl weben."); y0+=2*dy;
        canvas->Font->Style = canvas->Font->Style << fsBold;
        canvas->TextOut (x0, y0, "DB-WEAVE benutzen"); y0 += dy*3/2;
        canvas->Font->Style = canvas->Font->Style >> fsBold;
        canvas->TextOut (x0, y0, "Da DB-WEAVE ein normales Windows-Programm"); y0+=dy;
        canvas->TextOut (x0, y0, "ist, können Sie es ähnlich bedienen wie"); y0+=dy;
        canvas->TextOut (x0, y0, "andere Windows-Software. Wenn Sie zum"); y0+=dy;
        canvas->TextOut (x0, y0, "Beispiel Microsoft Word oder auch CorelDraw"); y0+=dy;
        canvas->TextOut (x0, y0, "kennen, so können Sie schnell mit DB-WEAVE"); y0+=dy;
        canvas->TextOut (x0, y0, "umgehen.");
    } else { // Default ist Englisch
        canvas->Font->Style = canvas->Font->Style << fsBold;
        canvas->TextOut (x0, y0, "What DB-WEAVE is"); y0 += dy*3/2;
        canvas->Font->Style = canvas->Font->Style >> fsBold;
        canvas->TextOut (x0, y0, "DB-WEAVE is a software for designing"); y0+=dy;
        canvas->TextOut (x0, y0, "and weaving fabrics with a dobby"); y0+=dy;
        canvas->TextOut (x0, y0, "loom. You can design your patterns and"); y0+=dy;
        canvas->TextOut (x0, y0, "weave then afterwards manually. Or you"); y0+=dy;
        canvas->TextOut (x0, y0, "can directly weave your pattern on a"); y0+=dy;
        canvas->TextOut (x0, y0, "computer-controlled loom."); y0+=2*dy;
        canvas->Font->Style = canvas->Font->Style << fsBold;
        canvas->TextOut (x0, y0, "Using DB-WEAVE"); y0 += dy*3/2;
        canvas->Font->Style = canvas->Font->Style >> fsBold;
        canvas->TextOut (x0, y0, "As DB-WEAVE is a standard Windows"); y0+=dy;
        canvas->TextOut (x0, y0, "program, the user interface is similar to"); y0+=dy;
        canvas->TextOut (x0, y0, "that of other Windows-based software. If"); y0+=dy;
        canvas->TextOut (x0, y0, "you know Microsoft Word or Excel or"); y0+=dy;
        canvas->TextOut (x0, y0, "CorelDraw, you can easily use DB-WEAVE.");
    }

    x0 = ClientWidth/2+10;
    y0 = 20;

    // Infos über DB-WEAVE ausgeben
    if (active_language==GE) {
        canvas->TextOut (x0, y0, "In jedem Fall empfehlen wir Ihnen, die"); y0+=dy;
        canvas->TextOut (x0, y0, "Fähigkeiten des Programmes mittels Versuch"); y0+=dy;
        canvas->TextOut (x0, y0, "und Irrtum kennenzulernen. Da DB-WEAVE eine"); y0+=dy;
        canvas->TextOut (x0, y0, "100 Schritte umfassende Rückgängig-Funktion"); y0+=dy;
        canvas->TextOut (x0, y0, "beinhaltet, können Sie Änderungen einfach"); y0+=dy;
        canvas->TextOut (x0, y0, "zurücknehmen."); y0+=2*dy;
        canvas->Font->Style = canvas->Font->Style << fsBold;
        canvas->TextOut (x0, y0, "Weitere Informationen"); y0 += dy*3/2;
        canvas->Font->Style = canvas->Font->Style >> fsBold;
        canvas->TextOut (x0, y0, "DB-WEAVE versucht alle relevanten Informationen"); y0+=dy;
        canvas->TextOut (x0, y0, "in der Statuszeile am unteren Ende des Programm-"); y0+=dy;
        canvas->TextOut (x0, y0, "fensters darzustellen. Wenn Sie weitere Infos"); y0+=dy;
        canvas->TextOut (x0, y0, "benötigen, so werfen Sie bitte einen Blick in das"); y0+=dy;
        canvas->TextOut (x0, y0, "Handbuch. Wenn Sie Internetzugang haben, so"); y0+=dy;
        canvas->TextOut (x0, y0, "finden Sie mehr Informationen auf unserer Website"); y0+=dy;
        canvas->TextOut (x0, y0, "unter http://www.brunoldsoftware.ch.");
    } else { // Default ist Englisch
        canvas->TextOut (x0, y0, "In any case, we recommend to explore"); y0+=dy;
        canvas->TextOut (x0, y0, "the programs capabilities by trial and"); y0+=dy;
        canvas->TextOut (x0, y0, "error. Since DB-WEAVE has a 100 step"); y0+=dy;
        canvas->TextOut (x0, y0, "undo functionality you can easily correct"); y0+=dy;
        canvas->TextOut (x0, y0, "any unwanted changes."); y0+=2*dy;
        canvas->Font->Style = canvas->Font->Style << fsBold;
        canvas->TextOut (x0, y0, "Further information"); y0 += dy*3/2;
        canvas->Font->Style = canvas->Font->Style >> fsBold;
        canvas->TextOut (x0, y0, "DB-WEAVE tries to display any relevant"); y0+=dy;
        canvas->TextOut (x0, y0, "information in the status bar at the bottom"); y0+=dy;
        canvas->TextOut (x0, y0, "of its window. If you need further information"); y0+=dy;
        canvas->TextOut (x0, y0, "please consider the manual. If you have"); y0+=dy;
        canvas->TextOut (x0, y0, "access to the internet, you can find more"); y0+=dy;
        canvas->TextOut (x0, y0, "information on our website at"); y0+=dy;
        canvas->TextOut (x0, y0, "http://www.brunoldsoftware.ch.");
    }
}
/*-----------------------------------------------------------------*/

