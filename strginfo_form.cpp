// DB-WEAVE, textile CAD/CAM software
// Copyright (c) 1998-2005 Brunold Software, Switzerland

#include <vcl.h>
#pragma hdrstop

#include "strginfo_form.h"
#include "language.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

__fastcall TStrgInfoForm::TStrgInfoForm(TComponent* Owner)
: TForm(Owner)
{
    LANG_C_H(this, EN, "Loom information", "")
    LANG_C_H(this, GE, "Webstuhl Informationen", "")
}

void __fastcall TStrgInfoForm::FormResize(TObject *Sender)
{
    infos->Width = ClientWidth;
    infos->Height = ClientHeight;
    infos->Invalidate();
    bOk->Left = ClientWidth/2 - bOk->Width/2;
    bOk->Top = ClientHeight - 10 - bOk->Height;
}

String GE_ARM_PATRONIC =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\b\\f0\\fs26 ARM Patronic\\par"
"\\b0\\fs20\\par"
"\\b Hersteller\\par"
"\\b0 ARM AG, Biglen, Schweiz\\par"
"\\par"
"\\b Informationen\\par"
"\\b0 Direkt beim Hersteller (armloom@hotmail.com)\\par"
"\\par"
"\\b Beschreibung\\par"
"\\b0 Der ARM Patronic ist ein Musterwebstuhl mit 24 Sch\\'e4ften, der mit der "
"Selecontrol-Steuerung ausgestattet ist. Dieser Treiber unterst\\'fctzt die "
"Selecontrol-Steuerung im direkten Modus. Das bedeutet, dass jeder Schuss, den "
"der Computer auf den Webstuhl \\'fcbertr\\'e4gt, gleich gewoben wird.\\par"
"}";

String EN_ARM_PATRONIC =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\b\\f0\\fs26 ARM Patronic\\par"
"\\b0\\fs20\\par"
"\\b Manufacturer\\par"
"\\b0 ARM AG, Biglen, Schweiz\\par"
"\\par"
"\\b Information\\par"
"\\b0 Directly from the manufacturer (armloom@hotmail.com)\\par"
"\\par"
"\\b Description\\par"
"\\b0 The ARM Patronic is a pattern loom with 24 harnesses, that is equipped "
"with the selecontrol unit. This driver supportes the selecontrol-unit in "
"direct mode. That means, that every pick that is transferred from the "
"computer to the loom is immediately woven.\\par"
"}";

String GE_ARM_PATRONIC2 =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\b\\f0\\fs26 ARM Patronic Indirekt\\par"
"\\b0\\fs20\\par"
"\\b Hersteller\\par"
"\\b0 ARM AG, Biglen, Schweiz\\par"
"\\par"
"\\b Informationen\\par"
"\\b0 Direkt beim Hersteller (armloom@hotmail.com)\\par"
"\\par"
"\\b Beschreibung\\par"
"\\b0 Der ARM Patronic ist ein Musterwebstuhl mit 24 Sch\\'e4ften, der mit der "
"Selecontrol-Steuerung ausgestattet ist. Dieser Treiber unterst\\'fctzt die "
"Selecontrol-Steuerung im indirekten Modus. Das bedeutet, dass die gew\\'fcnschten "
"Anzahl Sch\\'e4fte auf einmal in die Selecontrol-Steuerung \\'fcbertragen werden. "
"Danach kann der Webstuhl autonom (d.h. ohne Computer) das Muster weben.\\par"
"}";

String EN_ARM_PATRONIC2 =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\b\\f0\\fs26 ARM Patronic indirect\\par"
"\\b0\\fs20\\par"
"\\b Manufacturer\\par"
"\\b0 ARM AG, Biglen, Schweiz\\par"
"\\par"
"\\b Information\\par"
"\\b0 Directly from the manufacturer (armloom@hotmail.com)\\par"
"\\par"
"\\b Description\\par"
"\\b0 The ARM Patronic is a pattern loom with 24 harnesses, that is equipped "
"with the selecontrol-unit. This driver supports the selecontrol-unit in "
"indirect mode. That means, the desired picks are transmitted all at once "
"to the selecontrol-unit. Afterwards the loom can weave the pattern "
"autonomously (i.e. without the computer.)\\par"
"}";

String GE_DUMMY =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\b\\f0\\fs26 Kein Webstuhl\\par"
"\\b0\\fs20\\par"
"\\b Beschreibung\\par"
"\\b0 Dies ist ein Treiber, der f\\'fcr Testzwecke eingesetzt werden kann. Es "
"wird kein Webstuhl angesteuert. Das Weben wird simuliert.\\par"
"}";

String EN_DUMMY =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\b\\f0\\fs26 No loom\\par"
"\\b0\\fs20\\par"
"\\b Description\\par"
"\\b0 This is a driver for testing purposes. No loom is controlled. The "
"weaving is simulated.\\par"
"}";

String GE_ARM_DESIGNER =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\b\\f0\\fs26 ARM Designer Electronic\\par"
"\\b0\\fs20\\par"
"\\b Hersteller\\par"
"\\b0 ARM AG, Biglen, Schweiz\\par"
"\\par"
"\\b Informationen\\par"
"\\b0 Direkt beim Hersteller (armloom@hotmail.com)\\par"
"\\par"
"\\b Beschreibung\\par"
"\\b0 Der ARM Designer Electronic ist ein Handwebstuhl mit Computersteuerung. "
"Die Steuerung ist in zwei Varianten erh\\'e4ltlich: einerseits eine direkte "
"Designer-Steuerung und andererseits als Selecontrol-Modul wie bei den "
"Patronic-Musterwebst\\'fchlen. Falls Sie einen ARM Designer Electronic mit "
"Selecontrol haben, m\\'fcssen Sie den ARM Patronic Direkt/Indirekt Treiber "
"verwenden.\\par"
"}";

String EN_ARM_DESIGNER =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\b\\f0\\fs26 ARM Designer Electronic\\par"
"\\b0\\fs20\\par"
"\\b Manufacturer\\par"
"\\b0 ARM AG, Biglen, Schweiz\\par"
"\\par"
"\\b Information\\par"
"\\b0 Directly from the manufacturer (armloom@hotmail.com)\\par"
"\\par"
"\\b Description\\par"
"\\b0 The ARM Designer Electronic is a loom with computer control. "
"The controller is available in two variants: either as a direct designer "
"controller or as a selecontrol unit like the Patronic pattern loom. "
"If you have an ARM Designer Electronic with selecontrol unit, please use "
"the ARM Patronic direct/indirect driver.\\par"
"}";

String GE_VARPAPUU_PARALLEL =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\b\\f0\\fs26 Varpapuu Parallel\\par"
"\\b0\\fs20\\par"
"\\b Hersteller\\par"
"\\b0 Varpa-Set Oy, PIEKSÄMÄKI, Finnland\\par"
"\\par"
"\\b Informationen\\par"
"\\b0 Direkt beim Hersteller (varpa-set@co.inet.fi)\\par"
"\\par"
"\\b Beschreibung\\par"
"\\b0 Varpa-Set stellt die Varpapuu-Steuerung her, die sich auf jeden normalen "
"(nicht computergesteuerten) Webstuhl montieren l\\'e4sst. Damit l\\'e4sst sich "
"auch nachtr\\'e4glich ein manueller Webstuhl auf Computersteuerung "
"umr\\'fcsten.\\par"
"\\b0 Die Steuerung existiert in zwei Versionen. Die zweite Version funktioniert "
"\\'fcber den Druckeranschluss. Dieser Treiber ist f\\'fcr diese Version. "
"(Die \\'e4ltere Version funktioniert \\'fcber eine ISA-Karte.)\\par"
"}";

String EN_VARPAPUU_PARALLEL =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\b\\f0\\fs26 Varpapuu Parallel\\par"
"\\b0\\fs20\\par"
"\\b Manufacturer\\par"
"\\b0 Varpa-Set Oy, PIEKSÄMÄKI, Finnland\\par"
"\\par"
"\\b Information\\par"
"\\b0 Directly from the manufacturer (varpa-set@co.inet.fi)\\par"
"\\par"
"\\b Description\\par"
"\\b0 Varpa-Set produces the Varpapuu-control that can be mounted onto any "
"normal (non computerised) loom. In this manner a manual loom can be "
"computerised. \\par"
"\\b0 The controller exists in two versions. The second version works with the "
"parallel printer port. This driver is for this version. (The older version "
"works with a ISA-Card.)\\par"
"}";

String EN_SLIPS =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fprq2\\fcharset0 Arial;}"
"{\\f1\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\nowidctlpar\\lang1031\\b\\f0\\fs26 Generic SLIPS\\par"
"\\b0\\fs20\\par"
"\\b Manufacturer\\par"
"\\b0 ---\\par"
"\\b\\par"
" Information\\par"
"\\b0 Ask your loom manufacturer if he supports SLIPS.\\par"
"\\par"
"\\b Description\\par"
"\\b0 Generic SLIPS is an open standard for controlling looms via the serial port. "
"It was developed jointly by Bob and Jacquie Kelly (Citadel Enterprises) and "
"Ravi Nielsen (Maple Hill Software). Various loom manufacturer sell looms that "
"support SLIPS. For example Schacht Spindle and Cyrefco.\\par"
"\\pard\\lang2055\\f1\\par"
"}";

String GE_SLIPS =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fprq2\\fcharset0 Arial;}"
"{\\f1\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\nowidctlpar\\lang1031\\b\\f0\\fs26 Allgemeines SLIPS\\par"
"\\b0\\fs20\\par"
"\\b Hersteller\\par"
"\\b0 ---\\par"
"\\b\\par"
" Information\\par"
"\\b0 Fragen Sie Ihren Webstuhlhersteller ob er SLIPS unterst\\'fctzt.\\par"
"\\par"
"\\b Description\\par"
"\\b0 Allgemeines SLIPS ist ein offener Standard um Webst\\'fchle \\'fcber die serielle Schnittstelle "
"zu steuern. Er wurde gemeinsam von Bob und Jacquie Kelly (Citadel Enterprises) und Ravi Nielsen (Maple "
"Hill Software) entwickelt. Diverse Webstuhlhersteller bieten Webst\\'fchle an, die SLIPS unterst\\'fctzen. "
"Beispielsweise Schacht Spindle und Cyrefco.\\par"
"\\pard\\lang2055\\f1\\par"
"}";

String EN_LIPS =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fprq2\\fcharset0 Arial;}"
"{\\f1\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\nowidctlpar\\lang1031\\b\\f0\\fs26 Generic LIPS\\par"
"\\b0\\fs20\\par"
"\\b Manufacturer\\par"
"\\b0 ---\\par"
"\\b\\par"
" Information\\par"
"\\b0 Ask your loom manufacturer if he supports LIPS.\\par"
"\\par"
"\\b Description\\par"
"\\b0 Generic LIPS is an open standard for controlling looms via the parallel printer "
"port. It was developed jointly by Bob and Jacquie Kelly (Citadel Enterprises) and "
"Ravi Nielsen (Maple Hill Software). Various loom manufacturer sell looms that support "
"LIPS. For example Schacht Spindle and Cyrefco.\\par"
"\\pard\\lang2055\\f1\\par"
"}";

String GE_LIPS =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fprq2\\fcharset0 Arial;}"
"{\\f1\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\nowidctlpar\\lang1031\\b\\f0\\fs26 Allgemeines LIPS\\par"
"\\b0\\fs20\\par"
"\\b Hersteller\\par"
"\\b0 ---\\par"
"\\b\\par"
" Information\\par"
"\\b0 Fragen Sie Ihren Webstuhlhersteller ob er LIPS unterst\\'fctzt.\\par"
"\\par"
"\\b Description\\par"
"\\b0 Allgemeines LIPS ist ein offener Standard um Webst\\'fchle \\'fcber den parallelen "
"Druckerausgang zu steuern. Er wurde gemeinsam von Bob und Jacquie Kelly (Citadel Enterprises) "
"und Ravi Nielsen (Maple Hill Software) entwickelt. Diverse Webstuhlhersteller bieten Webst\\'fchle "
"an, die LIPS unterst\\'fctzen. Beispielsweise Schacht Spindle und Cyrefco.\\par"
"\\pard\\lang2055\\f1\\par"
"}";

String GE_AVL_CDIII =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\b\\f0\\fs26 AVL Compu-Dobby III\\par"
"\\b0\\fs20\\par"
"\\b Hersteller\\par"
"\\b0 AVL, USA\\par"
"\\par"
"\\b Informationen\\par"
"\\b0 Direkt beim Hersteller (www.avlusa.com)\\par"
"\\par"
"\\b Beschreibung\\par"
"\\b0 \\par"
"}";

String EN_AVL_CDIII =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang2055{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}}"
"\\viewkind4\\uc1\\pard\\b\\f0\\fs26 AVL Compu-Dobby III\\par"
"\\b0\\fs20\\par"
"\\b Manufacturer\\par"
"\\b0 AVL, USA\\par"
"\\par"
"\\b Information\\par"
"\\b0 Directly from the manufacturer (www.avlusa.com)\\par"
"\\par"
"\\b Description\\par"
"\\b0 \\par"
"}";

static void __fastcall Add (TStrgInfoForm* _frm, const String& _data)
{
    try {
        TMemoryStream* ms = new TMemoryStream();
        ms->Write (_data.c_str(), _data.Length());
        ms->Position = 0;
        _frm->infos->Lines->LoadFromStream (ms);
        delete ms;
    } catch(...) {
    }
}

void __fastcall ShowLoomInfo (TComponent* _owner, LOOMINTERFACE _loom)
{
    TStrgInfoForm* pFrm;
    try {
        pFrm = new TStrgInfoForm(_owner);
    } catch (...) {
        return;
    }

    switch (_loom) {
        case intrf_dummy:
            Add (pFrm, LANG_STR(EN_DUMMY, GE_DUMMY));
            break;

        case intrf_arm_patronic:
            Add (pFrm, LANG_STR(EN_ARM_PATRONIC, GE_ARM_PATRONIC));
            break;

        case intrf_arm_patronic_indirect:
            Add (pFrm, LANG_STR(EN_ARM_PATRONIC2, GE_ARM_PATRONIC2));
            break;

        case intrf_arm_designer:
            Add (pFrm, LANG_STR(EN_ARM_DESIGNER, GE_ARM_DESIGNER));
            break;

        case intrf_varpapuu_parallel:
            Add (pFrm, LANG_STR(EN_VARPAPUU_PARALLEL, GE_VARPAPUU_PARALLEL));
            break;

        case intrf_slips:
            Add (pFrm, LANG_STR(EN_SLIPS, GE_SLIPS));
            break;

        case intrf_lips:
            Add (pFrm, LANG_STR(EN_LIPS, GE_LIPS));
            break;

        case intrf_avl_cd_iii:
            Add (pFrm, LANG_STR(EN_AVL_CDIII, GE_AVL_CDIII));
            break;

        default:
            //xxxx
            break;
    }

    pFrm->ShowModal();
    delete pFrm;
}


