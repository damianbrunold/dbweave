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

#ifndef steuerungH
#define steuerungH

#include "combase.h"
#include <comport.hpp>
#include <stdio.h>

// Forward-Deklarationen
class SerialPort;

struct INITDATA
{
    // Für Seriell
    PORT port;

    // Für Parallel
    LPT lpt;

    // Für Varpapuu Card
    int port1;
    int port2;

    // Für Varpapuu Parallel
    int delay;
};

enum WEAVE_STATUS {
    WEAVE_REPEAT,
    WEAVE_SUCCESS_NEXT,
    WEAVE_SUCCESS_PREV
};

class StWeaveController
{
protected:
    bool aborted;
public:
    __fastcall StWeaveController();
    virtual __fastcall ~StWeaveController();
    void __fastcall Abort();
    virtual bool __fastcall Initialize(INITDATA _data);
    virtual WEAVE_STATUS __fastcall WeaveSchuss (DWORD _schaefte) = 0;
    virtual void __fastcall SetSpecialData (int _data) = 0;
    virtual void __fastcall Terminate();
    virtual void __fastcall SetBytes (int) {}
    void __fastcall CheckAbort();
    bool __fastcall IsAborted() const { return aborted; }
};

class StSerialController : public StWeaveController
{
protected:
    SerialPort* serialport;
    PORTINIT init_arm_patronic;
    PORTINIT init_arm_designer;
    PORTINIT init_slips;
    PORTINIT init_avlcdiii;

public:
    __fastcall StSerialController (TComPort* _comport);
    virtual __fastcall ~StSerialController();
    virtual bool __fastcall Initialize(INITDATA _data);
    virtual WEAVE_STATUS __fastcall WeaveSchuss (DWORD _schaefte) = 0;
    virtual void __fastcall SetSpecialData (int _data) = 0;
    virtual void __fastcall Terminate();
};

class StDummyController : public StWeaveController
{
public:
    __fastcall StDummyController() : StWeaveController() {}
    virtual bool __fastcall Initialize(INITDATA _data);
    virtual WEAVE_STATUS __fastcall WeaveSchuss (DWORD _schaefte);
    virtual void __fastcall Terminate();
    virtual void __fastcall SetSpecialData (int) {}
protected:
    virtual bool __fastcall CheckSuccess (BOOL _checkabort=TRUE);
    virtual bool __fastcall WaitForAck();
};

class StPatronicIndirectController : public StSerialController
{
public:
    __fastcall StPatronicIndirectController (TComPort* _comport) : StSerialController(_comport) { position=1; }
    virtual bool __fastcall Initialize(INITDATA _data);
    virtual WEAVE_STATUS __fastcall WeaveSchuss (DWORD _schaefte);
    virtual void __fastcall Terminate();
    virtual void __fastcall SetSpecialData (int _data) { position = _data; }
protected:
    int position;
    virtual bool __fastcall CheckSuccess (BOOL _checkabort=TRUE);
    virtual bool __fastcall WaitForAck();
};

class StPatronicController : public StSerialController
{
public:
    __fastcall StPatronicController (TComPort* _comport) : StSerialController(_comport) {}
    virtual bool __fastcall Initialize(INITDATA _data);
    virtual WEAVE_STATUS __fastcall WeaveSchuss (DWORD _schaefte);
    virtual void __fastcall Terminate();
    virtual void __fastcall SetSpecialData (int) {}
protected:
    virtual bool __fastcall CheckSuccess (BOOL _checkabort=TRUE, BOOL _timeout=FALSE);
    virtual bool __fastcall WaitForAck();
};

class StDesignerController : public StSerialController
{
public:
    __fastcall StDesignerController (TComPort* _comport) : StSerialController(_comport) {}
    virtual bool __fastcall Initialize(INITDATA _data);
    virtual WEAVE_STATUS __fastcall WeaveSchuss (DWORD _schaefte);
    virtual void __fastcall SetSpecialData (int) {}
protected:
    void __fastcall WaitFachGeschlossen();
    void __fastcall WaitFachOffen();
    void __fastcall SendMuster (DWORD _schaefte);
};

class StSlipsController : public StSerialController
{
private:
    int bytes;
    bool forward;
public:
    __fastcall StSlipsController (TComPort* _comport) : StSerialController(_comport) {}
    virtual bool __fastcall Initialize(INITDATA _data);
    virtual WEAVE_STATUS __fastcall WeaveSchuss (DWORD _schaefte);
    virtual void __fastcall Terminate();
    virtual void __fastcall SetSpecialData (int) {}
    virtual void __fastcall SetBytes (int _bytes) { dbw3_assert(_bytes>=0 && _bytes<=4); bytes = _bytes; }
protected:
    bool __fastcall SendData (DWORD _schaefte);
    void __fastcall GetData (int& _gear, int&_lift);
};

class StAvlCdIIIController : public StSerialController
{
public:
    __fastcall StAvlCdIIIController (TComPort* _comport) : StSerialController(_comport) {}
    virtual bool __fastcall Initialize(INITDATA _data);
    virtual WEAVE_STATUS __fastcall WeaveSchuss (DWORD _schaefte);
    virtual void __fastcall Terminate();
    virtual void __fastcall SetSpecialData (int) {}
protected:
    virtual void __fastcall MatchReply(int _reply1, int _reply2, BOOL _timeout);
    virtual void __fastcall MatchReply(int _reply, BOOL _timeout=FALSE);
    virtual bool __fastcall WaitForUp();
    virtual bool __fastcall WaitForDown();

private:
    virtual void __fastcall writeLog(const char* msg);
    virtual void __fastcall writeLog(const char* msg, int i);
};

#endif

