/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*-----------------------------------------------------------------*/
#include "dbw3_base.h"
#include "datamodule.h"
/*-----------------------------------------------------------------*/
void __fastcall FeldBlatteinzug::Clear()
{
	for (int i=0; i<Data->MAXX1; i++)
		feld.Set(i, (((i+1)%4)==0||((i+2)%4)==0) ? (char)1 : (char)0);
}
/*-----------------------------------------------------------------*/
void __fastcall FeldKettfarben::Clear()
{
	feld.Init (Data->defcolorh);
}
/*-----------------------------------------------------------------*/
void __fastcall FeldSchussfarben::Clear()
{
	feld.Init (Data->defcolorv);
}
/*-----------------------------------------------------------------*/
void __fastcall FeldEinzug::Clear()
{
	feld.Init (0);
}
/*-----------------------------------------------------------------*/
void __fastcall FeldAufknuepfung::Clear()
{
	feld.Init (0);
}
/*-----------------------------------------------------------------*/
void __fastcall FeldTrittfolge::Clear()
{
	feld.Init (0);
	isempty.Init (true);
}
/*-----------------------------------------------------------------*/
void __fastcall FeldGewebe::Clear()
{
	feld.Init (0);
}
/*-----------------------------------------------------------------*/
