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

#include "assert.h"
#include "string.h"
#include "comutil.h"

__fastcall SerialPort::SerialPort (TComPort* _comport)
{
    comport = _comport;
    comport->Active = false;
}

__fastcall SerialPort::~SerialPort()
{
    Close();
//    delete comport;
}

bool __fastcall SerialPort::Open (PORT _port, const PORTINIT& _init)
{
    dbw3_assert(comport);
    dbw3_assert(!comport->Active);
    try {
        switch (_init.baudrate) {
            case BR_1200:  comport->BaudRate = br1200; break;
            case BR_2400:  comport->BaudRate = br2400; break;
            case BR_4800:  comport->BaudRate = br4800; break;
            case BR_9600:  comport->BaudRate = br9600; break;
            case BR_14400: comport->BaudRate = br14400; break;
        }
        switch (_init.databits) {
            case DB_7: comport->DataBits = db7; break;
            case DB_8: comport->DataBits = db8; break;
        }
        switch (_init.stopbits) {
            case SB_ONE: comport->StopBits = sb1; break;
            case SB_ONEFIVE: comport->StopBits = sb1_5; break;
            case SB_TWO: comport->StopBits = sb2; break;
        }
        switch (_init.parity) {
            case PA_NONE: comport->Parity = paNone; break;
            case PA_EVEN: comport->Parity = paEven; break;
            case PA_ODD:  comport->Parity = paOdd; break;
        }
        switch (_port) {
            case P_COM1: comport->DeviceName = "COM1"; break;
            case P_COM2: comport->DeviceName = "COM2"; break;
            case P_COM3: comport->DeviceName = "COM3"; break;
            case P_COM4: comport->DeviceName = "COM4"; break;
            case P_COM5: comport->DeviceName = "COM5"; break;
            case P_COM6: comport->DeviceName = "COM6"; break;
            case P_COM7: comport->DeviceName = "COM7"; break;
            case P_COM8: comport->DeviceName = "COM8"; break;
        }
        comport->Active = true;
    } catch (...) {
        return false;
    }
    return true;
}

bool __fastcall SerialPort::IsOpen() const
{
    dbw3_assert(comport);
    return comport->Active;
}

void __fastcall SerialPort::Close()
{
    dbw3_assert(comport);
    if (comport->Active) {
        comport->Active = false;
    }
}

bool __fastcall SerialPort::Send (char* _buffer)
{
    dbw3_assert(comport);
    dbw3_assert(_buffer);
    try {
        while (*_buffer!='\0') {
            comport->TransmitChar (*_buffer);
            _buffer++;
            Sleep(10);
        }
    } catch (...) {
        return false;
    }
    return true;
}

bool __fastcall SerialPort::Send (char* _buffer, int _length)
{
    dbw3_assert(comport);
    dbw3_assert(_buffer);
    try {
        for (int i=0; i<_length; i++) {
            comport->TransmitChar (*_buffer);
            _buffer++;
            Sleep(10);
        }
    } catch (...) {
        return false;
    }
    return true;
}

bool __fastcall SerialPort::Receive (char* _buffer, int _length)
{
    dbw3_assert(comport);
    try {
        memset (_buffer, 0, _length);
        AnsiString data = comport->ReadString();
        lstrcpyn (_buffer, data.c_str(), _length);
    } catch (...) {
        return false;
    }
    return true;
}

char __fastcall SerialPort::GetChar()
{
    dbw3_assert(comport);
    return comport->InputCount()>0 ? comport->ReadChar() : '\0';
}

void __fastcall SerialPort::PurgeInput()
{
    dbw3_assert(comport);
    comport->PurgeInput();
}

