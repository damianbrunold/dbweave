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

#include <vcl\vcl.h>
#pragma hdrstop

#include <stdio.h>
#include "assert.h"
#include "steuerung.h"
#include "comutil.h"
#include "dbw3_strings.h"

#define PATRONIC_INIT "BAUD=4800 PARITY=E DATA=7 STOP=1"
#define DESIGNER_INIT "BAUD=2400 PARITY=N DATA=8 STOP=2"
#define AVLCDIII_INIT "BAUD=9600 PARITY=N DATA=8 STOP=1"

#define TIMEOUTLIMIT 5000

__fastcall StWeaveController::StWeaveController()
{
    aborted = false;
}

__fastcall StWeaveController::~StWeaveController()
{
}

bool __fastcall StWeaveController::Initialize(INITDATA/*_data*/)
{
    aborted = false;
    return true;
}

void __fastcall StWeaveController::Abort()
{
    aborted = true;
}

void __fastcall StWeaveController::CheckAbort()
{
    Application->ProcessMessages();
    if (aborted) throw "aborted";
}

void __fastcall StWeaveController::Terminate()
{
}

__fastcall StSerialController::StSerialController (TComPort* _comport)
: StWeaveController()
{
    init_arm_patronic.parity   = PA_EVEN;
    init_arm_patronic.stopbits = SB_ONE;
    init_arm_patronic.baudrate = BR_4800;
    init_arm_patronic.databits = DB_7;

    init_arm_designer.parity   = PA_NONE;
    init_arm_designer.stopbits = SB_TWO;
    init_arm_designer.baudrate = BR_2400;
    init_arm_designer.databits = DB_8;

    init_slips.parity   = PA_NONE;
    init_slips.stopbits = SB_ONE;
    init_slips.baudrate = BR_9600;
    init_slips.databits = DB_8;

    init_avlcdiii.parity = PA_NONE;
    init_avlcdiii.stopbits = SB_ONE;
    init_avlcdiii.baudrate = BR_9600;
    init_avlcdiii.databits = DB_8;

    try {
        serialport = new SerialPort(_comport);
    } catch (...) {
        serialport = 0;
    }
}

__fastcall StSerialController::~StSerialController()
{
    delete serialport;
}

bool __fastcall StSerialController::Initialize(INITDATA _data)
{
    if (!StWeaveController::Initialize(_data))
        return false;
    return true;
}

void __fastcall StSerialController::Terminate()
{
    dbw3_assert (serialport);
    serialport->Close();
}

bool __fastcall StDummyController::Initialize(INITDATA _data)
{
    if (!StWeaveController::Initialize(_data))
        return false;
    return true;
}

bool __fastcall StDummyController::CheckSuccess (BOOL _checkabort/*=TRUE*/)
{
    return true;
}

bool __fastcall StDummyController::WaitForAck()
{
    // Ein bisschen warten...
    DWORD start = GetTickCount();
    do {
        CheckAbort();
        DWORD now = GetTickCount();
        if (start>now) return true;
        if (now-start > 1000) return true;
        Application->ProcessMessages();
    } while(1);
}

WEAVE_STATUS __fastcall StDummyController::WeaveSchuss (DWORD _schaefte)
{
    return WaitForAck() ? WEAVE_SUCCESS_NEXT : WEAVE_REPEAT;
}

void __fastcall StDummyController::Terminate()
{
    // Nichts tun
}

bool __fastcall StPatronicIndirectController::Initialize (INITDATA _data)
{
    if (!StSerialController::Initialize(_data))
        return false;
    dbw3_assert (serialport);
    return serialport->Open (_data.port, init_arm_patronic);
}

bool __fastcall StPatronicIndirectController::CheckSuccess (BOOL _checkabort/*=TRUE*/)
{
    char ch;
    DWORD start = GetTickCount();
    do {
        if (_checkabort) CheckAbort();
        ch = serialport->GetChar();
        DWORD now = GetTickCount();
        if (now<start || now-start>TIMEOUTLIMIT) {
            serialport->Close();
            throw "timeout";
        }
    } while (ch!='\r');
    ch = serialport->GetChar();
    serialport->PurgeInput();
    return ch=='\r';
}

bool __fastcall StPatronicIndirectController::WaitForAck()
{
    char ch;
    do {
        CheckAbort();
        ch = serialport->GetChar();
    } while (ch!='R');
    serialport->PurgeInput();
    return ch=='R';
}

WEAVE_STATUS __fastcall StPatronicIndirectController::WeaveSchuss (DWORD _schaefte)
{
    if (position>220) return WEAVE_SUCCESS_NEXT;

    char buff[20];
    unsigned char s01_08 = (unsigned char)((_schaefte >> 16) & 0xff);
    unsigned char s09_16 = (unsigned char)((_schaefte >>  8) & 0xff);
    unsigned char s17_24 = (unsigned char)((_schaefte      ) & 0xff);

    char pos1[6];
    char pos2[6];
    int high = position / 32;
    int low = position % 32;
    wsprintf (pos1, "%02d.%02d", high, low*2);
    wsprintf (pos2, "%02d.%02d", high+7, low*2);

    wsprintf (buff, "M)D%sK0%02X%02X\r", pos1, s09_16, s17_24);
    serialport->Send (buff);

    wsprintf (buff, "M)D%sK000%02X\r", pos2, s01_08);
    serialport->Send (buff);
    if (!CheckSuccess()) return WEAVE_REPEAT;

    bool success = WaitForAck();
    if (success) position++;
    return success ? WEAVE_SUCCESS_NEXT : WEAVE_REPEAT;
}

void __fastcall StPatronicIndirectController::Terminate()
{
    dbw3_assert (serialport);
    if (serialport->IsOpen()) {
        serialport->Close();
    }
}

bool __fastcall StPatronicController::Initialize (INITDATA _data)
{
    if (!StSerialController::Initialize(_data))
        return false;
    dbw3_assert (serialport);
    bool opened = serialport->Open (_data.port, init_arm_patronic);

    if (opened) {
        serialport->PurgeInput();
        serialport->Send ("M)D14.50K0AB\r");
        try {
            CheckSuccess(TRUE, TRUE);
        } catch(...) {
            opened = false;
            serialport->Close();
        }
    }

    return opened;
}

bool __fastcall StPatronicController::CheckSuccess (BOOL _checkabort/*=TRUE*/, BOOL _timeout/*=FALSE*/)
{
    if (_timeout) {
        char ch;
        DWORD start = GetTickCount();
        do {
            if (_checkabort) CheckAbort();
            ch = serialport->GetChar();
            DWORD now = GetTickCount();
            if (now<start || now-start>TIMEOUTLIMIT)
                throw "timeout";
        } while (ch!='\r');
        ch = serialport->GetChar();
        serialport->PurgeInput();
        return ch=='\r';
    } else {
        char ch;
        do {
            if (_checkabort) CheckAbort();
            ch = serialport->GetChar();
        } while (ch!='\r');
        ch = serialport->GetChar();
        serialport->PurgeInput();
        return ch=='\r';
    }
}

bool __fastcall StPatronicController::WaitForAck()
{
    char ch;
    do {
        CheckAbort();
        ch = serialport->GetChar();
    } while (ch!='Q' && ch!='Z');
    serialport->PurgeInput();
    return ch=='Q';
}

WEAVE_STATUS __fastcall StPatronicController::WeaveSchuss (DWORD _schaefte)
{
    char buff[20];
    unsigned char s01_08 = (unsigned char)((_schaefte >> 16) & 0xff);
    unsigned char s09_16 = (unsigned char)((_schaefte >>  8) & 0xff);
    unsigned char s17_24 = (unsigned char)((_schaefte      ) & 0xff);

    wsprintf (buff, "M)D14.54K0%02X\r", s01_08);
    serialport->Send (buff);

    wsprintf (buff, "M)D14.52K0%02X%02X\r", s09_16, s17_24);
    serialport->Send (buff);
    if (!CheckSuccess()) return WEAVE_REPEAT;

    return WaitForAck() ? WEAVE_SUCCESS_NEXT : WEAVE_REPEAT;
}

void __fastcall StPatronicController::Terminate()
{
    dbw3_assert (serialport);
    if (serialport->IsOpen()) {
        serialport->Send ("M)D14.50K0\r");
        CheckSuccess(FALSE);
        serialport->Close();
    }
}

bool __fastcall StSlipsController::Initialize (INITDATA _data)
{
    if (!StSerialController::Initialize(_data))
        return false;
    forward = true;
    dbw3_assert (serialport);
    return serialport->Open (_data.port, init_slips);
}

WEAVE_STATUS __fastcall StSlipsController::WeaveSchuss (DWORD _schaefte)
{
    int state = 1;
    enum { H=0, L=1 };
    int gear = H;
    int lift = H;
    bool repeat = false;

    while (state!=0) {
        GetData (gear, lift);
        switch (state) {
            case 1:
                if (gear==H && lift==L) {
                    // Send data
                    SendData (_schaefte);
                    state = 2;
                    break;
                } else if (gear==L && lift==H) {
                    state = 4;
                    break;
                }
                break;

            case 2:
                if (gear==L && lift==H) {
                    state = 3;
                    break;
                } else if (gear==H && lift==H) {
                    state = 0;
                    break;
                }
                break;

            case 3:
                if (gear==L && lift==L) {
                    // Repeat last pick
                    repeat = true;
                    state = 0;
                    break;
                } else if (gear==H && lift==H) {
                    state = 0;
                    break;
                }
                break;

            case 4:
                if (gear==H && lift==H) {
                    // Reverse Direction
                    forward = !forward;
                    state = 0;
                    break;
                } else if (gear==L && lift==L) {
                    // Repeat last pick
                    repeat = true;
                    state = 0;
                    break;
                }
                break;
        }
    }

    if (repeat) {
        if (!forward) return WEAVE_SUCCESS_NEXT;
        else return WEAVE_SUCCESS_PREV;
    } else {
        if (forward) return WEAVE_SUCCESS_NEXT;
        else return WEAVE_SUCCESS_PREV;
    }
}

void __fastcall StSlipsController::GetData (int& _gear, int&_lift)
{
    char ch;
    bool changed;
    int gear, lift;
    do {
        CheckAbort();
        ch = serialport->GetChar();
        gear = int((ch & 2) >> 1);
        lift = int(ch & 1);
        changed = (gear!=_gear || lift!=_lift);
    } while (!changed);
    _gear = gear;
    _lift = lift;
    serialport->PurgeInput();
}

bool __fastcall StSlipsController::SendData (DWORD _schaefte)
{
    char buff[5];
    unsigned char s01_08 = (unsigned char)((_schaefte      ) & 0xff);
    unsigned char s09_16 = (unsigned char)((_schaefte >>  8) & 0xff);
    unsigned char s17_24 = (unsigned char)((_schaefte >> 16) & 0xff);
    unsigned char s25_32 = (unsigned char)((_schaefte >> 24) & 0xff);

    wsprintf (buff, "%c%c%c%c", s01_08, s09_16, s17_24, s25_32);
    serialport->Send (buff, bytes);
    return true;
}

void __fastcall StSlipsController::Terminate()
{
    dbw3_assert (serialport);
    if (serialport->IsOpen()) serialport->Close();
}

bool __fastcall StDesignerController::Initialize (INITDATA _data)
{
    if (!StSerialController::Initialize(_data))
        return false;
    dbw3_assert (serialport);
    return serialport->Open (_data.port, init_arm_designer);
}

WEAVE_STATUS __fastcall StDesignerController::WeaveSchuss (DWORD _schaefte)
{
    try {
        WaitFachGeschlossen();
        SendMuster (_schaefte);
        WaitFachOffen();
        WaitFachGeschlossen();
    } catch (...) {
        return WEAVE_REPEAT;
    }
    return WEAVE_SUCCESS_NEXT;
}

void __fastcall StDesignerController::WaitFachGeschlossen()
{
    dbw3_assert (serialport);
    dbw3_assert (serialport->IsOpen());

    Sleep(50);

    char ch;
    DWORD start = GetTickCount();
    do {
        CheckAbort();
        ch = serialport->GetChar();
        DWORD now = GetTickCount();
        if (now<start || now-start>TIMEOUTLIMIT)
            throw "timeout";
    } while (ch!='1');
}

void __fastcall StDesignerController::WaitFachOffen()
{
    dbw3_assert (serialport);
    dbw3_assert (serialport->IsOpen());

    Sleep(50);

    char ch;
    DWORD start = GetTickCount();
    do {
        CheckAbort();
        ch = serialport->GetChar();
        DWORD now = GetTickCount();
        if (now<start || now-start>TIMEOUTLIMIT)
            throw "timeout";
    } while (ch!='0');
}

void __fastcall StDesignerController::SendMuster (DWORD _schaefte)
{
    dbw3_assert (serialport);
    dbw3_assert (serialport->IsOpen());

    char buffer[4];
    buffer[0] = 2; /*STX*/
    buffer[1] = (char)(_schaefte & 0xff);
    buffer[2] = (char)((_schaefte>>8) & 0xff);
    buffer[3] = (char)((_schaefte>>16) & 0xff);

    serialport->Send (buffer);
}

void __fastcall StAvlCdIIIController::writeLog(const char* msg)
{
    FILE* logfile = fopen("dbweave.log", "a");
    if (logfile != NULL) {
        fputs(msg, logfile);
        fclose(logfile);
    }
}

void __fastcall StAvlCdIIIController::writeLog(const char* msg, int i)
{
    FILE* logfile = fopen("dbweave.log", "a");
    if (logfile != NULL) {
        fprintf(logfile, msg, i);
        fclose(logfile);
    }
}

bool __fastcall StAvlCdIIIController::Initialize (INITDATA _data)
{
    writeLog("initialize...\n");
    if (!StSerialController::Initialize(_data))
        return false;
    dbw3_assert (serialport);
    bool opened = serialport->Open (_data.port, init_avlcdiii);

    if (opened) {
        writeLog("opened serial port: %d\n", _data.port);
        serialport->PurgeInput();
        char buff[3];
        buff[0] = 0x0f;
        buff[1] = 0x07;
        buff[2] = 0;
        serialport->Send(buff);
        writeLog("sent 0f 07\n");
        try {
            MatchReply(0x7f, 0x03, TRUE);
        } catch(...) {
            opened = false;
            serialport->Close();
            writeLog("closed serial port due to failure to match reply\n");
        }
        writeLog("purge input\n");
        serialport->PurgeInput();
    }

    return opened;
}

void __fastcall StAvlCdIIIController::MatchReply(int _reply1, int _reply2, BOOL _timeout)
{
    MatchReply(_reply1, _timeout);
    MatchReply(_reply2, _timeout);
    writeLog("purge input\n");
    serialport->PurgeInput();
}

void __fastcall StAvlCdIIIController::MatchReply(int _reply, BOOL _timeout/*=FALSE*/)
{
    writeLog("match %02x\n", _reply);
    DWORD start = GetTickCount();
    do {
        CheckAbort();
        int r = (unsigned char) serialport->GetChar();
        if (r == _reply)
            break;
        DWORD now = GetTickCount();
        if (_timeout && (now < start || now - start > TIMEOUTLIMIT)) {
            writeLog("timeout in match reply %02x\n", _reply);
            throw "timeout";
        }
    } while (true);
}

bool __fastcall StAvlCdIIIController::WaitForUp()
{
    try {
        writeLog("waiting for up (0x61, 0x03)\n");
        MatchReply(0x61, 0x03, FALSE);
        return true;
    } catch (...) {
        return false;
    }
}

bool __fastcall StAvlCdIIIController::WaitForDown()
{
    try {
        writeLog("waiting for down (0x62, 0x03)\n");
        MatchReply(0x62, 0x03, FALSE);
        return true;
    } catch (...) {
        return false;
    }
}

WEAVE_STATUS __fastcall StAvlCdIIIController::WeaveSchuss (DWORD _schaefte)
{
    writeLog("purge input\n");
    serialport->PurgeInput();

    writeLog("weaving...");
    char buff[20];
    buff[0] = 0x10 | (_schaefte & 0xf);
    buff[1] = 0x20 | ((_schaefte >> 4) & 0xf);
    buff[2] = 0x30 | ((_schaefte >> 8) & 0xf);
    buff[3] = 0x40 | ((_schaefte >> 12) & 0xf);
    buff[4] = 0x50 | ((_schaefte >> 16) & 0xf);
    buff[5] = 0x60 | ((_schaefte >> 20) & 0xf);
    buff[6] = 0x70 | ((_schaefte >> 24) & 0xf);
    buff[7] = 0x80 | ((_schaefte >> 28) & 0xf);
    buff[8] = 0x07;
    buff[9] = 0;

    serialport->Send (buff);

    char logbuff[255];
    wsprintf(logbuff, "sent %02x %02x %02x %02x %02x %02x %02x %02x 07\n",
        (int) buff[0], (int) buff[1], (int) buff[2],
        (int) buff[3], (int) buff[4], (int) buff[5],
        (int) buff[6], (int) buff[7]);
    writeLog(logbuff);

    WaitForUp();
    WaitForDown();

    return WEAVE_SUCCESS_NEXT;
}

void __fastcall StAvlCdIIIController::Terminate()
{
    writeLog("purge input\n");
    serialport->PurgeInput();

    writeLog("terminating...\n");
    dbw3_assert (serialport);
    if (serialport->IsOpen()) {
        char buff[3];
        buff[0] = 0x0f;
        buff[1] = 0x07;
        buff[2] = 0;
        serialport->Send(buff);
        writeLog("sent 0f 07\n");
        try {
            MatchReply(0x7f, 0x03, TRUE);
        } catch(...) {
        }
        serialport->Close();
        writeLog("closed serial port\n");
    }
}


