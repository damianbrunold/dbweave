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

#ifndef comutilH
#define comutilH

#include <comport.hpp>
#include "combase.h"

class SerialPort
{
private:
    TComPort* comport;

public:
    __fastcall SerialPort (TComPort* _comport);
    virtual __fastcall ~SerialPort();
    bool __fastcall Open (PORT _port, const PORTINIT& _init);
    bool __fastcall IsOpen() const;
    void __fastcall Close();
    bool __fastcall Send (char* _buffer);
    bool __fastcall Send (char* _buffer, int _length);
    bool __fastcall Receive (char* _buffer, int _length);
    char __fastcall GetChar();
    void __fastcall PurgeInput();
};

#endif

