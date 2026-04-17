/* DB-WEAVE, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File techinfo_form.cpp
 * Copyright (C) 2000-2001 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "dbw3_form.h"
#include "techinfo_form.h"
#include "language.h"
#include "version.h"
#include "dbw3_strings.h"
#include "osversion.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
__fastcall TTechinfoForm::TTechinfoForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
    GatherData();
}
/*-----------------------------------------------------------------*/
__fastcall TTechinfoForm::~TTechinfoForm()
{
}
/*-----------------------------------------------------------------*/
void __fastcall TTechinfoForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Technical Information", "")
    LANG_C_H(this, GE, "Technische Informationen", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::HelpTechinfoClick(TObject *Sender)
{
    try {
        TTechinfoForm* pFrm = new TTechinfoForm(this);
        pFrm->ShowModal();
        delete pFrm;
    } catch (...) {
    }
}
/*-----------------------------------------------------------------*/
#define DEMOVERSIONTIMEMSG LANG_STR("%d of %d days used", "%d von %d Tagen benutzt")
#define DEMOVERSIONEXPIRED LANG_STR("Trial expired", "Probe abgelaufen")
/*-----------------------------------------------------------------*/
int __fastcall GetCols (int _bpp)
{
    int cols = 1;
    for (int i=0; i<_bpp; i++)
        cols *= 2;
    return cols;
}
/*-----------------------------------------------------------------*/
#pragma warn -8002
String GetProcessorVendor()
{
    String vendor;
    DWORD s1, s2, s3;
    __try {
        _asm {
            mov eax, 0
            CPUID
            mov s1, ebx
            mov s2, edx
            mov s3, ecx
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) { s1=s2=s3=0; }

    vendor = String(char(s1&0xff))+char((s1>>8)&0xff)+char((s1>>16)&0xff)+char((s1>>24)&0xff);
    vendor += String(char(s2&0xff))+char((s2>>8)&0xff)+char((s2>>16)&0xff)+char((s2>>24)&0xff);
    vendor += String(char(s3&0xff))+char((s3>>8)&0xff)+char((s3>>16)&0xff)+char((s3>>24)&0xff);

    return vendor;
}
/*-----------------------------------------------------------------*/
DWORD DoCPUID()
{
    DWORD retval;
    __try {
        _asm {
            mov eax, 1
            CPUID
            and eax, 03fffh
            mov retval, eax
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) { retval=0; }
    return retval;
}
/*-----------------------------------------------------------------*/
DWORD GetProcessorFamily()
{
    return (DoCPUID() >> 8) & 0xf;
}
/*-----------------------------------------------------------------*/
DWORD GetProcessorModel()
{
    return (DoCPUID() >> 4) & 0xf;
}
/*-----------------------------------------------------------------*/
DWORD GetProcessorStepping()
{
    return DoCPUID() & 0xf;
}
/*-----------------------------------------------------------------*/
void __fastcall TTechinfoForm::Add (const String& _data, bool _bold/*=false*/)
{
    if (!_bold) infos->Lines->Add (String("      ")+_data);
    else {
        infos->Lines->Add("");
        infos->SelAttributes->Style = infos->SelAttributes->Style << fsBold;
        infos->Lines->Add(String("  ")+_data);
        infos->SelAttributes->Style = infos->SelAttributes->Style >> fsBold;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TTechinfoForm::GatherData()
{
    Add(LANG_STR("DB-WEAVE version information", "DB-WEAVE Versionsdaten"), true);
    Add(String(VERSION)+ver_string);

    Add(LANG_STR("DB-WEAVE installation", "DB-WEAVE Installation"), true);
    Add(LANG_STR("Installed to: ", "Installiert in: ")+ExtractFilePath(ParamStr(0)));

    // Zusatzinfos wie Betriebssystem und Anzahl Farben
    Add (LANG_STR("Operating system", "Betriebssystem"), true);
    String line1, line2;
    String osversion = GetOsVersion();
    if (osversion.Pos("\n")==0) {
        line1 = osversion;
        line2 = "";
    } else {
        line1 = osversion.SubString(1, osversion.Pos("\n")-1);
        line2 = osversion.SubString(osversion.Pos("\n")+1, osversion.Length());
    }
    Add(line1);
    if (line2!="") { Add(line2); }

    Add (LANG_STR("Computer Equipment", "Computerausstattung"), true);
    // Prozessorinfos
    String pr;
    SYSTEM_INFO si;
    GetSystemInfo (&si);
    switch (si.wProcessorLevel) {
        case 3: pr = "80386 "; break;
        case 4: pr = "80486 "; break;
        default: {
            String vendor = GetProcessorVendor();
            int f = GetProcessorFamily();
            int m = GetProcessorModel();
            int s = GetProcessorStepping();
            if (vendor=="GenuineIntel") {
                if (f==5 && m<4) pr = "Intel Pentium";
                else if (f==5 && m>=4) pr = "Intel Pentium MMX";
                else if (f==6) {
                    if (m==1) pr = "Intel Pentium Pro";
                    else if (m==3 || m==5) pr = "Intel Pentium II";
                    else if (m==6) pr = "Intel Celeron";
                    else if (m==7 || m==8 || m==10) pr = "Intel Pentium III";
                    else pr = "Intel";
                } else pr = "Intel";
            } else if (vendor=="AuthenticAMD") {
                if (f==5 && m<=3) pr = "AMD-K5";
                else if (f==5 && (m==6 || m==7)) pr = "AMD-K6";
                else if (f==5 && m==8) pr = "AMD-K6-2";
                else if (f==5 && m==9) pr = "AMD-K6-III";
                else if (f==5) pr = "AMD-Kx";
                else if (f==6 && (m==1 || m==2 || m==4)) pr = "AMD Athlon";
                else if (f==6 && m==3) pr = "AMD Duron";
                else pr = "AMD";
            } else {
                pr = "(Unknown)";
            }
            pr += " Family "+IntToStr(f);
            pr += " Model "+IntToStr(m);
            pr += " Stepping "+IntToStr(s);
        }
    }
    Add(pr);
    MEMORYSTATUS ms;
    ms.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus (&ms);
    char buff[100];
    wsprintf (buff, LANG_STR("Total memory: %u M", "Total Speicher: %u M").c_str(), ms.dwTotalPhys/1024/1024);
    Add(buff);
    wsprintf (buff, LANG_STR("Memory load (0-100): %u", "Speicherbelastung (0-100): %u").c_str(), ms.dwMemoryLoad);
    Add(buff);

    // Anzahl Farben ermitteln
    String colors;
    int palsize = GetDeviceCaps(Canvas->Handle, SIZEPALETTE);
    DWORD bpp = GetDeviceCaps(Canvas->Handle, BITSPIXEL);
    DWORD planes = GetDeviceCaps(Canvas->Handle, PLANES);
    DWORD colx = GetCols(bpp)*planes;
    DWORD cols = GetDeviceCaps(Canvas->Handle, NUMCOLORS);
    if (palsize!=0 && palsize<=256) {
        char buff[70];
        wsprintf (buff, COLINFO_PAL, palsize, GetCols(24));
        colors = buff;
    } else if (colx!=0) {
        char buff[70];
        wsprintf (buff, COLINFO_NOPAL, colx);
        colors = buff;
    } else if (cols!=0) {
        char buff[70];
        wsprintf (buff, COLINFO_NOPAL, cols);
        colors = buff;
    } else {
        colors = "";
    }
    Add(colors+LANG_STR(" colors", " Farben"));
}
/*-----------------------------------------------------------------*/

