/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Serial-loom controllers — ports of the five real protocols from
    legacy steuerung.cpp. All communicate over a SerialPort wrapper
    and share the StWeaveController contract from ui/loom.h so
    TSTRGFRM can drive any of them through a common interface.

    Skipped when DBWEAVE_NO_LOOM is ON; in that configuration
    TSTRGFRM falls back to the built-in dummy controller.         */

#ifndef DBWEAVE_LOOM_SERIAL_CONTROLLERS_H
#define DBWEAVE_LOOM_SERIAL_CONTROLLERS_H

#include <memory>

#include "loom.h"         /* StWeaveController base */
#include "loomsettings.h" /* PORTINIT */

class SerialPort;

/*-----------------------------------------------------------------*/
class StSerialController : public StWeaveController
{
public:
    StSerialController();
    ~StSerialController() override;

    bool Initialize(const INITDATA& _data) override;
    void Terminate() override;

protected:
    SerialPort* serialport = nullptr;
    PORTINIT init_arm_patronic;
    PORTINIT init_arm_designer;
    PORTINIT init_slips;
    PORTINIT init_avlcdiii;
};

/*-----------------------------------------------------------------*/
/*  ARM Patronic direct: "M)D14.54K0<xx>\r" + "M)D14.52K0<xx><xx>\r",
    await \r ack, then 'Q' or 'Z' handshake. */
class StPatronicController : public StSerialController
{
public:
    bool Initialize(const INITDATA& _data) override;
    WEAVE_STATUS WeaveSchuss(std::uint32_t _shafts) override;
    void Terminate() override;

private:
    bool checkSuccess(bool _checkAbort, bool _timeout);
    bool waitForAck();
};

/*-----------------------------------------------------------------*/
/*  ARM Patronic indirect: writes shafts into Patronic memory at
    successive positions, driven by SetSpecialData. */
class StPatronicIndirectController : public StSerialController
{
public:
    StPatronicIndirectController()
    {
        position = 1;
    }
    bool Initialize(const INITDATA& _data) override;
    WEAVE_STATUS WeaveSchuss(std::uint32_t _shafts) override;
    void Terminate() override;
    void SetSpecialData(int _data) override
    {
        position = _data;
    }

private:
    int position = 1;
    bool checkSuccess();
    bool waitForAck();
};

/*-----------------------------------------------------------------*/
/*  ARM Designer: sends STX + 3 shaft bytes, then waits for '1'/'0'
    to pulse through the shed-open / shed-closed cycle twice. */
class StDesignerController : public StSerialController
{
public:
    bool Initialize(const INITDATA& _data) override;
    WEAVE_STATUS WeaveSchuss(std::uint32_t _shafts) override;

private:
    void waitFachGeschlossen();
    void waitFachOffen();
    void sendMuster(std::uint32_t _shafts);
};

/*-----------------------------------------------------------------*/
/*  Generic SLIPS: state machine driven by a gear/lift 2-bit input
    from the loom; the user sets bytes (1..4) to control how wide
    the send is. */
class StSlipsController : public StSerialController
{
public:
    bool Initialize(const INITDATA& _data) override;
    WEAVE_STATUS WeaveSchuss(std::uint32_t _shafts) override;
    void Terminate() override;
    void SetBytes(int _bytes);

private:
    int bytes = 4;
    bool forward = true;
    bool sendData(std::uint32_t _shafts);
    void getData(int& _gear, int& _lift);
};

/*-----------------------------------------------------------------*/
/*  AVL Compu-Dobby III: 8 nibble-tagged bytes + 0x07, wait for
    up-ready (0x61,0x03) then down-ready (0x62,0x03). */
class StAvlCdIIIController : public StSerialController
{
public:
    bool Initialize(const INITDATA& _data) override;
    WEAVE_STATUS WeaveSchuss(std::uint32_t _shafts) override;
    void Terminate() override;

private:
    void matchReply(int _r1, int _r2, bool _timeout);
    void matchReply(int _r, bool _timeout);
    bool waitForUp();
    bool waitForDown();
};

/*-----------------------------------------------------------------*/
/*  Factory: build a controller from a LOOMINTERFACE enum. Falls
    back to the dummy for unsupported types. Returns ownership. */
std::unique_ptr<StWeaveController> makeLoomController(LOOMINTERFACE _intrf);

#endif
