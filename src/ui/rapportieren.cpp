/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port scope: the six non-click replication utilities from legacy/
    rapportieren.cpp (RapportSchuss, RapportKette, CopyKettfaden,
    CopySchussfaden, ClearKettfaden, ClearSchussfaden). The three
    click handlers (RappRapportierenClick, RappReduzierenClick,
    RappOverrideClick) dispatch through a TRapportForm dialog whose
    port belongs in Phase 7.
*/

/*-----------------------------------------------------------------*/
#include "assert_compat.h"
#include "mainwindow.h"
#include "datamodule.h"
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RapportSchuss (int _ry, bool _withcolors)
{
	int j, j1, j2, sj1, sj2;

	// Schussbereich bestimmen
	CalcRangeSchuesse();
	j1 = schuesse.a;
	j2 = schuesse.b;
	if (j1==-1) return;
	if (j2==-1) return;

	// Schussrapport bestimmen
	CalcRapport();
	sj1 = rapport.sr.a;
	sj2 = rapport.sr.b;
	if (sj1==0 && sj2==-1) {
		sj1 = j1;
		sj2 = j2;
	}

	// Alles ausserhalb Rapport loeschen
	for (j=j1; j<sj1; j++) ClearSchussfaden (j);
	for (j=sj2+1; j<=j2; j++) ClearSchussfaden (j);

	// Rapportieren
	//xxxx momentan rapportiere ich nur nach oben...
	int maxj;
	if (_ry!=-1) maxj = sj1+(sj2-sj1+1)*_ry;
	else maxj = Data->MAXY2;
	if (maxj>=Data->MAXY2) maxj = Data->MAXY2;
	for (j=sj2+1; j<maxj; j++)
		CopySchussfaden (sj1+(j-sj2-1)%(sj2-sj1+1), j, _withcolors);

	// Schussbereich nachfuehren
	//xxx koennte optimaler sein...
	CalcRangeSchuesse();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::RapportKette (int _rx, bool _withcolors)
{
	int i, i1, i2, ki1, ki2;

	// Kettbereich bestimmen
	CalcRangeKette();
	i1 = kette.a;
	i2 = kette.b;
	if (i1==-1) return;
	if (i2==-1) return;

	// Kettrapport bestimmen
	CalcRapport();
	ki1 = rapport.kr.a;
	ki2 = rapport.kr.b;
	if (ki1==0 && ki2==-1) {
		ki1 = i1;
		ki2 = i2;
	}

	// Alles ausserhalb Rapport loeschen
	for (i=i1; i<ki1; i++) ClearKettfaden (i);
	for (i=ki2+1; i<=i2; i++) ClearKettfaden (i);

	// Rapportieren
	//xxxx momentan rapportiere ich nur nach rechts...
	int maxi;
	if (_rx!=-1) maxi = ki1+(ki2-ki1+1)*_rx;
	else maxi = Data->MAXX1;
	if (maxi>=Data->MAXX1) maxi = Data->MAXX1;
	for (i=ki2+1; i<maxi; i++)
		CopyKettfaden (ki1+(i-ki2-1)%(ki2-ki1+1), i, _withcolors);

	// Kettbereich nachfuehren
	//xxx koennte optimaler sein...
	CalcRangeKette();
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::CopyKettfaden (int _von, int _nach, bool _withcolors)
{
	// Blatteinzug kopieren
	//xxxx ?? muss ich das?!
//    blatteinzug.Set (_nach, blatteinzug.Get (_von));

	// Farbe
	if (_withcolors) kettfarben.feld.Set (_nach, kettfarben.feld.Get (_von));

	// Einzug kopieren
	einzug.feld.Set (_nach, einzug.feld.Get (_von));

	// Gewebe kopieren
	if (schuesse.a!=-1 && schuesse.b!=-1) {
		for (int j=schuesse.a; j<=schuesse.b; j++)
			gewebe.feld.Set (_nach, j, gewebe.feld.Get (_von, j));
	}
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::CopySchussfaden (int _von, int _nach, bool _withcolors)
{
	// IsTrittfolgeEmpty kopieren
	trittfolge.isempty.Set (_nach, trittfolge.isempty.Get(_von));

	// Tritt kopieren
	for (int i=0; i<Data->MAXX2; i++)
		trittfolge.feld.Set (i, _nach, trittfolge.feld.Get (i, _von));

	// Farbe kopieren
	if (_withcolors) schussfarben.feld.Set (_nach, schussfarben.feld.Get (_von));

	// Gewebe kopieren
	if (kette.a!=-1 && kette.b!=-1) {
		for (int i=kette.a; i<=kette.b; i++)
			gewebe.feld.Set (i, _nach, gewebe.feld.Get (i, _von));
	}
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ClearKettfaden (int _i)
{
	einzug.feld.Set (_i, 0);
	if (schuesse.b!=-1) for (int j=schuesse.a; j<=schuesse.b; j++) gewebe.feld.Set (_i, j, 0);
}
/*-----------------------------------------------------------------*/
void __fastcall TDBWFRM::ClearSchussfaden (int _j)
{
	for (int i=0; i<Data->MAXX2; i++) trittfolge.feld.Set (i, _j, 0);
	trittfolge.isempty.Set (_j, true);
	if (kette.b!=-1) for (int i=kette.a; i<=kette.b; i++) gewebe.feld.Set (i, _j, 0);
}
/*-----------------------------------------------------------------*/
