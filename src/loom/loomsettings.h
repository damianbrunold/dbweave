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
    Lives in the loom module (built only when DBWEAVE_BUILD_LOOM is
    ON) so the rest of the code doesn't pull Qt6SerialPort
    headers. */

#ifndef DBWEAVE_LOOM_LOOMSETTINGS_H
#define DBWEAVE_LOOM_LOOMSETTINGS_H

enum PORT     { P_NONE=0, P_COM1=1, P_COM2, P_COM3, P_COM4, P_COM5, P_COM6, P_COM7, P_COM8 };
enum LPT      { LP_NONE=0, LP_LPT1=1, LP_LPT2, LP_LPT3 };
enum PARITY   { PA_NONE=0, PA_ODD=1, PA_EVEN=2 };
enum STOPBITS { SB_ONE=0, SB_ONEFIVE=1, SB_TWO=2 };
enum BAUDRATE { BR_1200=0, BR_2400=1, BR_4800=2, BR_9600=3, BR_14400=4 };
enum DATABITS { DB_7=0, DB_8=1 };

struct PORTINIT
{
	PARITY   parity   = PA_NONE;
	STOPBITS stopbits = SB_ONE;
	BAUDRATE baudrate = BR_9600;
	DATABITS databits = DB_8;
};

#endif
