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

#ifndef combaseH
#define combaseH

enum PORT { P_NONE=0, P_COM1=1, P_COM2, P_COM3, P_COM4, P_COM5, P_COM6, P_COM7, P_COM8 };
enum LPT  { LP_NONE=0, LP_LPT1=1, LP_LPT2, LP_LPT3 };
enum PARITY { PA_NONE=0, PA_ODD=1, PA_EVEN=2 };
enum STOPBITS { SB_ONE=0, SB_ONEFIVE=1, SB_TWO=2 };
enum BAUDRATE { BR_1200=0, BR_2400=1, BR_4800=2, BR_9600=3, BR_14400=4 };
enum DATABITS { DB_7=0, DB_8=1 };

struct PORTINIT
{
    PARITY parity;
    STOPBITS stopbits;
    BAUDRATE baudrate;
    DATABITS databits;
};

#endif
