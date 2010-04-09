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
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "steuerung_form.h"
#include "dbw3_form.h"
#include "dbw3_strings.h"
#include "steuerung.h"
#include "assert.h"
#include "datamodule.h"
#include "strgpatronicindparms_form.h"
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::WeaveStartClick(TObject *Sender)
{
    if (!IsValidWeavePosition()) {
        if (Application->MessageBox (WEB_INVALIDPOS, APP_TITLE, MB_YESNO)==IDNO)
            return;
        ClearPositionSelection();
        _ResetCurrentPos();
        DrawPositionSelection();
        UpdateStatusbar();
    }

    if (klammern[weave_klammer].repetitions==0) {
        Application->MessageBox (WEB_NOKLAMMERN, APP_TITLE, MB_OK);
        return;
    }

    dbw3_assert(controller);
    if (controller) {
        INITDATA data;
        data.port = port;
        data.lpt = lpt;
        data.delay = delay;
        bool result = controller->Initialize (data);
        if (!result) {
            Application->MessageBox (WEB_CANNOTSEND, APP_TITLE, MB_OK);
            return;
        }
    }

    stopit = false;
    tempquit = false;
    weaving = true;
    firstschuss = true;

    WeaveStart->Enabled = false;
    SBStart->Enabled = false;
    StartWeben->Enabled = false;
    WeaveStop->Enabled = true;
    SBStop->Enabled = true;
    StopWeben->Enabled = true;
    OptionsLoom->Enabled = false;
    SBOptions->Enabled = false;
    PostMessage (Handle, WM_SWITCHSHORTCUTS, 0, 0L);

    // Screensaver disablen
    SystemParametersInfo (SPI_SETSCREENSAVEACTIVE, FALSE, 0, SPIF_SENDWININICHANGE);

    if (intrf==intrf_arm_patronic_indirect) {
        int pos = 0;
        int max = 220;
        try {
            TStrgPatronicIndParmsForm* frm = new TStrgPatronicIndParmsForm(this);
            if (rapporty<220) frm->upCount->Position = short(rapporty); //xxx
            if (frm->ShowModal()==mrCancel)
                goto g_exit;
            pos = frm->upSaveFrom->Position;
            max = frm->upCount->Position;
            if (pos+max > 220) {
                char buff[200];
                wsprintf (buff, STRG_MAXLIMIT, pos, 220-pos-1);
                if (Application->MessageBox (buff, APP_TITLE, MB_YESNO)==IDNO)
                    goto g_exit;
                max = 220-pos-1;
            }
        } catch (...) {
        }
        controller->SetSpecialData (pos);
        maxweave = max;
    }

    Weben();

g_exit:
    try {
        dbw3_assert(controller);
        controller->Terminate();
    } catch(...) {
        dbw3_assert (comport);
        if (comport->Active) comport->Active = false;
    }

    stopit = true;

    WeaveStop->Enabled = false;
    SBStop->Enabled = false;
    StopWeben->Enabled = false;
    WeaveStart->Enabled = true;
    SBStart->Enabled = true;
    StartWeben->Enabled = true;
    OptionsLoom->Enabled = true;
    SBOptions->Enabled = true;
    PostMessage (Handle, WM_SWITCHSHORTCUTS, 0, 0L);

    // Screensaver enablen
    SystemParametersInfo (SPI_SETSCREENSAVEACTIVE, TRUE, 0, SPIF_SENDWININICHANGE);

    if (tempquit) ModalResult = mrOk;
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::WeaveStopClick(TObject *Sender)
{
    dbw3_assert(controller);
    if (controller && !controller->IsAborted()) {
        try {
            controller->Abort();
        } catch (...) {
        }
        stopit = true;
        weaving = false;
        tempquit = false;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::WeaveTempQuit()
{
    dbw3_assert(controller);
    if (controller && !controller->IsAborted()) {
        try {
            controller->Abort();
        } catch (...) {
        }
        stopit = true;
        weaving = true;
        tempquit = true;
    }
}
/*-----------------------------------------------------------------*/
bool __fastcall TSTRGFRM::AtBegin()
{
    // Anfangsposition ermitteln
    int klammer;
    int position = 0;
    int repetition = 1;
    for (klammer=0; klammer<MAXKLAMMERN; klammer++)
        if (klammern[klammer].repetitions>0) {
            position = klammern[klammer].first;
            repetition = 1;
            break;
        }
    // Mit aktueller Position vergleichen
    return weave_klammer==klammer &&
           weave_position==position &&
           weave_repetition==repetition;
}
/*-----------------------------------------------------------------*/
DWORD __fastcall TSTRGFRM::GetSchaftDaten(int _pos)
{
    DWORD data = 0;
    if (intrf==intrf_arm_patronic || intrf==intrf_arm_patronic_indirect) {
        int maxi = min (trittfolge->feld.SizeX(), 24);
        int maxj = min (maxi, aufknuepfung->feld.SizeY());
        if (schlagpatrone) {
            for (signed int i=0; i<maxi; i++)
                if (trittfolge->feld.Get(i, weave_position)>0) {
                    if (!reverse) data = data | (1 << (23-i));
                    else data = data | (1 << i);
                }
        } else {
            for (signed int i=0; i<trittfolge->feld.SizeX(); i++)
                if (trittfolge->feld.Get(i, weave_position)>0)
                    for (int k=0; k<maxj; k++)
                        if (aufknuepfung->feld.Get(i, k)>0) {
                            if (!reverse) data = data | (1 << (23-k));
                            else data = data | (1 << k);
                        }
        }
    } else if (intrf==intrf_arm_designer) {
        int maxi = min (trittfolge->feld.SizeX(), MaxSchaefte());
        int maxj = min (maxi, aufknuepfung->feld.SizeY());
        if (schlagpatrone) {
            for (signed int i=0; i<maxi; i++)
                if (trittfolge->feld.Get(i, weave_position)>0) {
                    if (!reverse) data = data | (1 << (maxi-1-i));
                    else data = data | (1 << i);
                }
        } else {
            for (signed int i=0; i<trittfolge->feld.SizeX(); i++)
                if (trittfolge->feld.Get(i, weave_position)>0)
                    for (int k=0; k<maxj; k++)
                        if (aufknuepfung->feld.Get(i, k)>0) {
                            if (!reverse) data = data | (1 << (maxi-1-k));
                            else data = data | (1 << k);
                        }
        }
    } else if (intrf==intrf_varpapuu_parallel) {
        int maxi = min (trittfolge->feld.SizeX(), MaxSchaefte());
        int maxj = min (maxi, aufknuepfung->feld.SizeY());
        if (schlagpatrone) {
            for (signed int i=0; i<maxi; i++)
                if (trittfolge->feld.Get(i, weave_position)>0) {
                    if (!reverse) data = data | (1 << i);
                    else data = data | (1 << (maxi-1-i));
                }
        } else {
            for (signed int i=0; i<trittfolge->feld.SizeX(); i++)
                if (trittfolge->feld.Get(i, weave_position)>0)
                    for (int k=0; k<maxj; k++)
                        if (aufknuepfung->feld.Get(i, k)>0) {
                            if (!reverse) data = data | (1 << k);
                            else data = data | (1 << (maxi-1-k));
                        }
        }
    } else { // Default
        int maxi = min (trittfolge->feld.SizeX(), MaxSchaefte());
        int maxj = min (maxi, aufknuepfung->feld.SizeY());
        if (schlagpatrone) {
            for (signed int i=0; i<maxi; i++)
                if (trittfolge->feld.Get(i, weave_position)>0) {
                    if (!reverse) data = data | (1 << (maxi-1-i));
                    else data = data | (1 << i);
                }
        } else {
            for (signed int i=0; i<trittfolge->feld.SizeX(); i++)
                if (trittfolge->feld.Get(i, weave_position)>0)
                    for (int k=0; k<maxj; k++)
                        if (aufknuepfung->feld.Get(i, k)>0) {
                            if (!reverse) data = data | (1 << (maxi-1-k));
                            else data = data | (1 << k);
                        }
        }
    }
    return data;
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::Weben()
{
    // Weben bis die Balken krachen...
    dbw3_assert (trittfolge);
    dbw3_assert (aufknuepfung);
    int count = 0;
    while(!stopit) {
        DWORD data = GetSchaftDaten (weave_position);
        bool success;
        WEAVE_STATUS stat;
        ClearPositionSelection();
        UpdateLastPosition();
        DrawPositionSelection();
        try {
            stat = controller->WeaveSchuss (data);
            success = stat==WEAVE_SUCCESS_NEXT || stat==WEAVE_SUCCESS_PREV;
        } catch (...) {
        }
        Application->ProcessMessages();
        if (stopit) break;
        if (success) {
            count++;
            if (maxweave!=0 && count>=maxweave) {
                Application->MessageBox (STRG_DATASENT, APP_TITLE, MB_OK);
                weaving = false;
                return;
            }
            ClearPositionSelection();
            if (weave_position==last_position) {
                if (intrf!=intrf_varpapuu_parallel && 
                    intrf!=intrf_slips && intrf!=intrf_lips)
                {
                    if (!WeaveBackwards->Checked) NextTritt();
                    else PrevTritt();
                } else {
                    if (stat==WEAVE_SUCCESS_NEXT) NextTritt();
                    else PrevTritt();
                }
            }
            firstschuss = false;
            AutoScroll();
            DrawPositionSelection();
            UpdateStatusbar();
            if (!Loop->Checked && !firstschuss && AtBegin()) {
                if (intrf==intrf_arm_patronic_indirect)
                    Application->MessageBox (STRG_DATASENT, APP_TITLE, MB_OK);
                ::MessageBeep(-1);
                ::MessageBeep(-1);
                ::MessageBeep(MB_ICONASTERISK);
                weaving = false;
                return;
            }
        }
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::NextTritt()
{
    weave_position++;
    if (weave_position > klammern[weave_klammer].last) {
        if (weave_repetition<klammern[weave_klammer].repetitions) {
            weave_repetition++;
            weave_position = klammern[weave_klammer].first;
        } else {
            weave_klammer = (weave_klammer+1) % MAXKLAMMERN;
            while (klammern[weave_klammer].repetitions==0)
                weave_klammer = (weave_klammer+1) % MAXKLAMMERN;
            weave_repetition = 1;
            weave_position = klammern[weave_klammer].first;
        }
    }
    current_klammer = weave_klammer;
}
/*-----------------------------------------------------------------*/
void __fastcall TSTRGFRM::PrevTritt()
{
    weave_position--;
    if (weave_position < klammern[weave_klammer].first) {
        if (weave_repetition>1) {
            weave_repetition--;
            weave_position = klammern[weave_klammer].last;
        } else {
            weave_klammer = (weave_klammer+MAXKLAMMERN-1) % MAXKLAMMERN;
            while (klammern[weave_klammer].repetitions==0)
                weave_klammer = (weave_klammer+MAXKLAMMERN-1) % MAXKLAMMERN;
            weave_repetition = klammern[weave_klammer].repetitions;
            weave_position = klammern[weave_klammer].last;
        }
    }
    current_klammer = weave_klammer;
}
/*-----------------------------------------------------------------*/
