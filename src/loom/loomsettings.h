/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Serial-port settings enums — verbatim port of legacy combase.h.
    Lives in the loom module (skipped when DBWEAVE_NO_LOOM is ON)
    so the rest of the code doesn't pull Qt6SerialPort headers. */

#ifndef DBWEAVE_LOOM_LOOMSETTINGS_H
#define DBWEAVE_LOOM_LOOMSETTINGS_H

/*  Legacy combase.h also declared PORT (P_COM1..P_COM8) and LPT
    enums. The port is now identified by its platform-native device
    name instead -- a fixed COM1..COM8 list no longer reaches the
    USB-serial adapters these looms hang off -- and the parallel-port
    interfaces were never implemented, so both enums are gone. */
enum PARITY { PA_NONE = 0, PA_ODD = 1, PA_EVEN = 2 };
enum STOPBITS { SB_ONE = 0, SB_ONEFIVE = 1, SB_TWO = 2 };
enum BAUDRATE { BR_1200 = 0, BR_2400 = 1, BR_4800 = 2, BR_9600 = 3, BR_14400 = 4 };
enum DATABITS { DB_7 = 0, DB_8 = 1 };

struct PORTINIT {
    PARITY parity = PA_NONE;
    STOPBITS stopbits = SB_ONE;
    BAUDRATE baudrate = BR_9600;
    DATABITS databits = DB_8;
};

#endif
