/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Foundation types for the pattern editor. PT, SZ, RAPPORT, RANGE are
    pure value types; the FeldBase hierarchy declares weaving-document
    containers whose virtual Clear()/ScrollX()/ScrollY() bodies read
    from the Data and DBWFRM globals (defined in datamodule.h /
    mainwindow.h). For that reason this header lives under src/ui/
    rather than src/domain/ -- the *declarations* are portable, but the
    *definitions* cannot escape the main-window coupling without a
    dependency-injection refactor that the port ground rules defer.
*/

#ifndef DBWEAVE_UI_DBW3_BASE_H
#define DBWEAVE_UI_DBW3_BASE_H

#include "vcl_compat.h"
#include "enums.h"
#include "felddef.h"
#include <cstdlib>   /* abs */

/*-----------------------------------------------------------------*/
const int DEFAULT_MAXX1      = 300;
const int DEFAULT_MAXX2      = 35;
const int DEFAULT_MAXY1      = 35;
const int DEFAULT_MAXY2      = 300;
const int DEFAULT_STRONGLINE = 4;
/*-----------------------------------------------------------------*/
const int DEFAULT_COLORH = 234;
const int DEFAULT_COLORV = 235;
const int DEFAULT_COLOR  = DEFAULT_COLORH;
/*-----------------------------------------------------------------*/
const int AUSHEBUNG = 10;
const int ANBINDUNG = 11;
const int ABBINDUNG = 12;
/*-----------------------------------------------------------------*/
// Klammer fuer Webstuhlsteuerung
struct Klammer
{
	int first;
	int last;
	int repetitions;
	Klammer() { first = 0; last = 1; repetitions = 0; }
	Klammer& operator=(const Klammer& _klammer) {
		if (&_klammer==this) return *this;
		first = _klammer.first;
		last = _klammer.last;
		repetitions = _klammer.repetitions;
		return *this;
	}
};
/*-----------------------------------------------------------------*/
// GRIDPOS verwaltet die Ausmasse und wichtigsten Properties des
// sichtbaren Teils eines Feldes.
struct GRIDPOS
{
	int x0;
	int y0;
	int width;
	int height;
	int strongline_x;
	int strongline_y;
	GRIDPOS() { Init(); }
	void Init() { strongline_x = strongline_y = DEFAULT_STRONGLINE; }
};
/*-----------------------------------------------------------------*/
// GRIDSIZE fuehrt die logische Groesse des sichtbaren Teils eines
// Feldes.
struct GRIDSIZE
{
	int width;  // Breite des sichtbaren Teils des Feldes in Karos
	int height; // Hoehe des sichtbaren Teils des Feldes in Karos

	GRIDSIZE() { width=height=0; }
};
/*-----------------------------------------------------------------*/
// INPUTPOS verwaltet fuer ein Feld den Ort, an dem der Keyboard-Cursor
// ist. Dieser wird pro Feld unabhaengig gefuehrt.
struct INPUTPOS
{
	signed int i; // relative Koordinaten (Karo-Felder)
	signed int j;

	INPUTPOS() { Init(); }
	void Init() { i=j=0; }
};
/*-----------------------------------------------------------------*/
// PT stellt eine Punkt-Koordinate dar.
struct PT
{
	signed int i;
	signed int j;
	PT() { i=j=0; }
	PT (int _i, int _j) { i=(int)_i; j=(int)_j; }
};
/*-----------------------------------------------------------------*/
// SZ (SIZE) stellt eine Von-Bis-Koordinate dar.
struct SZ
{
	signed int a;
	signed int b;
	SZ() { a=b=0; }
	SZ (int _a, int _b) { a=(int)_a; b=(int)_b; }
	int count() const { return (int)((a==b && a==-1) ? 0 : (b-a+1)); }
};
/*-----------------------------------------------------------------*/
inline bool operator==(const SZ& _s1, const SZ& _s2)
{
	return _s1.a==_s2.a && _s1.b==_s2.b;
}
/*-----------------------------------------------------------------*/
inline bool operator!=(const SZ& _s1, const SZ& _s2)
{
	return !(_s1==_s2);
}
/*-----------------------------------------------------------------*/
// RAPPORT verwaltet den Rapport einer Patrone.
struct RAPPORT
{
	bool overridden;
	SZ kr; // Kettrapport
	SZ sr; // Schussrapport
	__fastcall RAPPORT() { overridden = false; }
};
/*-----------------------------------------------------------------*/
// RANGE wird verwendet, um eine Selektion zu verwalten.
struct RANGE
{
	PT begin;
	PT end;
	FELD feld;

	__fastcall RANGE() { Clear(); }
	void __fastcall Clear() { begin = PT(0,0); end = PT(-1,-1); feld = INVALID; }
	bool __fastcall Valid();
	PT   __fastcall LeftDown();
	PT   __fastcall TopRight();
	void __fastcall Normalize();
	int  __fastcall Width() const { return end.i!=-1 ? std::abs(end.i-begin.i)+1 : 1; }
	int  __fastcall Height() const { return end.j!=-1 ? std::abs(end.j-begin.j)+1 : 1; }
};
/*-----------------------------------------------------------------*/
struct FeldBase
{
	INPUTPOS kbd;
	GRIDPOS  pos;
	int gh = 0; // Gridheight -- zero until AllocBuffers / PatternCanvas::resizeEvent
	int gw = 0; // Gridwidth   -- zero until AllocBuffers / PatternCanvas::resizeEvent
	virtual ~FeldBase() = default;
	virtual void __fastcall Clear() = 0;
	virtual int __fastcall SizeX() = 0;
	virtual int __fastcall SizeY() = 0;
	virtual int __fastcall ScrollX() = 0;
	virtual int __fastcall ScrollY() = 0;
};
/*-----------------------------------------------------------------*/
struct FeldBase2 : public FeldBase
{
	GRIDSIZE size;
};
/*-----------------------------------------------------------------*/
struct FeldBase3 : public FeldBase2
{
	DARSTELLUNG darstellung;
};
/*-----------------------------------------------------------------*/
struct FeldBlatteinzug : public FeldBase
{
	FeldVectorChar feld;
	FeldBlatteinzug();
	virtual void __fastcall Clear();
	virtual int __fastcall SizeX() { return feld.Size(); }
	virtual int __fastcall SizeY() { return 1; }
	virtual int __fastcall ScrollX();
	virtual int __fastcall ScrollY();
};
/*-----------------------------------------------------------------*/
struct FeldKettfarben : public FeldBase
{
	FeldVectorChar feld;
	FeldKettfarben();
	virtual void __fastcall Clear();
	virtual int __fastcall SizeX() { return feld.Size(); }
	virtual int __fastcall SizeY() { return 1; }
	virtual int __fastcall ScrollX();
	virtual int __fastcall ScrollY();
};
/*-----------------------------------------------------------------*/
struct FeldSchussfarben : public FeldBase
{
	FeldVectorChar feld;
	FeldSchussfarben();
	virtual void __fastcall Clear();
	virtual int __fastcall SizeX() { return 1; }
	virtual int __fastcall SizeY() { return feld.Size(); }
	virtual int __fastcall ScrollX();
	virtual int __fastcall ScrollY();
};
/*-----------------------------------------------------------------*/
struct FeldEinzug : public FeldBase3
{
	int maxy;
	FeldVectorShort feld;
	FeldEinzug();
	virtual void __fastcall Clear();
	virtual int __fastcall SizeX() { return feld.Size(); }
	virtual int __fastcall SizeY() { return maxy; }
	virtual int __fastcall ScrollX();
	virtual int __fastcall ScrollY();
};
/*-----------------------------------------------------------------*/
struct FeldAufknuepfung : public FeldBase3
{
	FeldGridChar feld;
	FeldAufknuepfung();
	bool pegplanstyle; // Schlagpatronenstil
	virtual void __fastcall Clear();
	virtual int __fastcall SizeX() { return feld.SizeX(); }
	virtual int __fastcall SizeY() { return feld.SizeY(); }
	virtual int __fastcall ScrollX();
	virtual int __fastcall ScrollY();
};
/*-----------------------------------------------------------------*/
struct FeldTrittfolge : public FeldBase3
{
	FeldGridChar feld;
	FeldVectorBool isempty;
	bool einzeltritt;
	FeldTrittfolge();
	virtual void __fastcall Clear();
	virtual int __fastcall SizeX() { return feld.SizeX(); }
	virtual int __fastcall SizeY() { return feld.SizeY(); }
	virtual int __fastcall ScrollX();
	virtual int __fastcall ScrollY();
};
/*-----------------------------------------------------------------*/
struct FeldGewebe : public FeldBase2
{
	FeldGridChar feld;
	FeldGewebe();
	virtual void __fastcall Clear();
	virtual int __fastcall SizeX() { return feld.SizeX(); }
	virtual int __fastcall SizeY() { return feld.SizeY(); }
	virtual int __fastcall ScrollX();
	virtual int __fastcall ScrollY();
};
/*-----------------------------------------------------------------*/
// Die Hilfslinien haben einen Bereich (je einen fuer
// horizontale und einen fuer vertikale Linien), in dem
// sie mit der Maus manipuliert werden koennen.
struct HlineBar : GRIDPOS
{
	int gw;
	int gh;
};
/*-----------------------------------------------------------------*/
#endif
