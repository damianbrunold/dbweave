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
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#include <vcl\printers.hpp>
#include <mem.h>
#include <math.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "assert.h"
#include "dbw3_form.h"
#include "felddef.h"
#include "datamodule.h"
#include "print.h"
/*-----------------------------------------------------------------*/
// Diese Werte bleiben für alle Seiten gleich und werden
// als Basis für die Berechnung der Seitenaufteilung verwendet.
void __fastcall PrPrinter::CalcBaseDimensions()
{
    // Druckerauflösung bestimmen
    int sx = GetDeviceCaps (Printer()->Handle, LOGPIXELSX);
    int sy = GetDeviceCaps (Printer()->Handle, LOGPIXELSY);

	// Ränder in (Drucker-)Pixel berechnen
	mleft   = mainfrm->borders.range.left*10*sx/254;
	mtop    = (mainfrm->borders.range.top+mainfrm->header.height)*10*sy/254;
    mright  = mainfrm->borders.range.right*10*sx/254;
    mbottom = (mainfrm->borders.range.bottom+mainfrm->footer.height)*10*sy/254;

    // Position von Header und Footer
	y_header = (mainfrm->borders.range.top)*10*sy/254;
	y_footer = Printer()->PageHeight - (mainfrm->borders.range.bottom+mainfrm->footer.height)*10*sy/254;

	// Effektive Seitengrösse und -position
	pwidth  = Printer()->PageWidth - mleft - mright;
	pheight = Printer()->PageHeight - mtop  - mbottom;
    x_page = mleft;
    y_page = mtop;

	// Karogrösse
    int gw  = 20*sx/254; // Gridwidth  = 2.0 mm
    int gh = 20*sy/254; // Gridheight = 2.0 mm

    // Skalieren der Karogrösse
	gwidth  = gw*mainfrm->zoom[mainfrm->currentzoom]/mainfrm->zoom[3];
	gheight = gh*mainfrm->zoom[mainfrm->currentzoom]/mainfrm->zoom[3];

    // Kett/Schussverhältnis
    if (mainfrm->faktor_schuss>mainfrm->faktor_kette) {
        int gridheight = gh*((int)((double)mainfrm->zoom[mainfrm->currentzoom]*mainfrm->faktor_schuss/mainfrm->faktor_kette))/mainfrm->zoom[3];
        einzug.gw = gwidth;
        einzug.gh = gheight;
        aufknuepfung.gw = gwidth;
        aufknuepfung.gh = gheight;
        kettfarben.gw = gwidth;
        kettfarben.gh = gheight;
        blatteinzug.gw = gwidth;
        blatteinzug.gh = gheight;
        trittfolge.gw = gwidth;
        trittfolge.gh = gridheight;
        schussfarben.gw = gwidth;
        schussfarben.gh = gridheight;
        gewebe.gw = gwidth;
        gewebe.gh = gridheight;
    } else {
        int gridwidth = gw*((int)((double)mainfrm->zoom[mainfrm->currentzoom]*mainfrm->faktor_kette/mainfrm->faktor_schuss))/mainfrm->zoom[3];
        einzug.gw = gridwidth;
        einzug.gh = gheight;
        aufknuepfung.gw = gwidth;
        aufknuepfung.gh = gheight;
        kettfarben.gw = gridwidth;
        kettfarben.gh = gheight;
        blatteinzug.gw = gridwidth;
        blatteinzug.gh = gheight;
        trittfolge.gw = gwidth;
        trittfolge.gh = gheight;
        schussfarben.gw = gwidth;
        schussfarben.gh = gheight;
        gewebe.gw = gridwidth;
        gewebe.gh = gheight;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall AddMargins (PrFeld& _feld, int _mleft, int _mtop)
{
    _feld.x0 += _mleft;
    _feld.y0 += _mtop;
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::CalcDimensions()
{
    // Wichtig: vor CalcDimensions sollte unbedingt
    // ClipToPage aufgerufen werden. Denn nur so ist
    // gewährleistet, dass der Druckbereich auf die
    // gewünschte Seite beschränkt ist!

    // Grösse der Felder berechnen
    if (kettfarben.visible) {
        kettfarben.width = kette.count()*kettfarben.gw;
        kettfarben.height = kettfarben.gh;
    }

    if (einzug.visible) {
        einzug.width = kette.count()*einzug.gw;
        einzug.height = schaefte.count()*einzug.gh;
    }

    if (blatteinzug.visible) {
        blatteinzug.width = kette.count()*blatteinzug.gw;
        blatteinzug.height = blatteinzug.gh;
    }

    if (gewebe.visible) {
        gewebe.width = kette.count()*gewebe.gw;
        gewebe.height = schuesse.count()*gewebe.gh;
    }

    if (trittfolge.visible) {
        trittfolge.width = tritte.count()*trittfolge.gw;
        trittfolge.height = schuesse.count()*trittfolge.gh;
    }

    if (aufknuepfung.visible) {
        aufknuepfung.width = tritte.count()*aufknuepfung.gw;
        aufknuepfung.height = schaefte.count()*aufknuepfung.gh;
    }

    if (schussfarben.visible) {
        schussfarben.width = schussfarben.gw;
        schussfarben.height = schuesse.count()*schussfarben.gh;
    }

    // Position der Felder zueinander und Abstand
    // voneinander berechnen
    int xabstand = gwidth;
    int yabstand = gheight;

    // Zuerst die X-Koordinaten
    kettfarben.x0 = 0;
    einzug.x0 = kettfarben.x0;
    blatteinzug.x0 = kettfarben.x0;
    gewebe.x0 = kettfarben.x0;
    if (gewebe.visible) trittfolge.x0 = gewebe.x0 + gewebe.width + xabstand;
	else trittfolge.x0 = einzug.x0 + einzug.width + xabstand;
    aufknuepfung.x0 = trittfolge.x0;
    schussfarben.x0 = trittfolge.x0 + trittfolge.width;
    if (trittfolge.visible) schussfarben.x0 += xabstand;

    // Dann die Y-Koordinate
    if (mainfrm->einzugunten) {
        gewebe.y0 = 0;
        blatteinzug.y0 = gewebe.y0 + gewebe.height;
        if (gewebe.visible) blatteinzug.y0 += yabstand;
        einzug.y0 = blatteinzug.y0 + blatteinzug.height;
        if (blatteinzug.visible) einzug.y0 += yabstand;
        kettfarben.y0 = einzug.y0 + einzug.height;
        if (einzug.visible) kettfarben.y0 += yabstand;

        trittfolge.y0 = gewebe.y0;
        aufknuepfung.y0 = trittfolge.y0 + trittfolge.height;
        if (trittfolge.visible) aufknuepfung.y0 += yabstand;
        if (blatteinzug.visible || (mainfrm->ViewBlatteinzug->Checked && trittfolge.visible))
            aufknuepfung.y0 += blatteinzug.gh + yabstand;

        schussfarben.y0 = gewebe.y0;
    } else {
        kettfarben.y0 = 0;
        einzug.y0 = kettfarben.y0 + kettfarben.height;
        if (kettfarben.visible) einzug.y0 += yabstand;
        blatteinzug.y0 = einzug.y0 + einzug.height;
        if (einzug.visible) blatteinzug.y0 += yabstand;
        gewebe.y0 = blatteinzug.y0 + blatteinzug.height;
        if (blatteinzug.visible) gewebe.y0 += yabstand;

        aufknuepfung.y0 = einzug.y0;
        trittfolge.y0 = aufknuepfung.y0 + aufknuepfung.height;
        if (aufknuepfung.visible) trittfolge.y0 += yabstand;
        if (blatteinzug.visible || (mainfrm->ViewBlatteinzug->Checked && aufknuepfung.visible))
            trittfolge.y0 += blatteinzug.gh + yabstand;

        schussfarben.y0 = gewebe.y0;
    }

    // Seitenränder addieren!
    AddMargins (kettfarben, mleft, mtop);
    AddMargins (einzug, mleft, mtop);
    AddMargins (blatteinzug, mleft, mtop);
    AddMargins (gewebe, mleft, mtop);
    AddMargins (aufknuepfung, mleft, mtop);
    AddMargins (trittfolge, mleft, mtop);
    AddMargins (schussfarben, mleft, mtop);
}
/*-----------------------------------------------------------------*/
void __fastcall PrPrinter::InitVisibility()
{
    // Felder Initialisieren
    kettfarben.Init();
    einzug.Init();
    blatteinzug.Init();
    aufknuepfung.Init();
    trittfolge.Init();
    schussfarben.Init();
    gewebe.Init();

    // Default-Sichtbarkeit Felder bestimmen
    if (mainfrm->ViewFarbe->Checked && kette.count()>0)
        kettfarben.visible = true;

    if (mainfrm->ViewEinzug->Checked && kette.count()>0 && schaefte.count()>0)
        einzug.visible = true;

    if (mainfrm->ViewBlatteinzug->Checked && kette.count()>0)
        blatteinzug.visible = true;

    if (kette.count()>0 && schuesse.count()>0)
        gewebe.visible = true;

    if (mainfrm->ViewTrittfolge->Checked && schuesse.count()>0 && tritte.count()>0)
        trittfolge.visible = true;

    if (mainfrm->ViewEinzug->Checked && mainfrm->ViewTrittfolge->Checked && schaefte.count()>0 && tritte.count()>0)
        aufknuepfung.visible = true;

    if (mainfrm->ViewFarbe->Checked && schuesse.count()>0)
        schussfarben.visible = true;
}
/*-----------------------------------------------------------------*/
bool __fastcall PrPrinter::ClipToPage (int _page)
{
    // Bestimmt die defaultmässig sichtbaren Felder
    InitVisibility();

    // Druckbreite insgesamt
    int gx = kette.count()*gewebe.gw;
    if (trittfolge.visible || aufknuepfung.visible)
        gx += gwidth + tritte.count()*trittfolge.gw;
    if (schussfarben.visible) gx += gwidth + schussfarben.gw;

    // Druckhöhe insgesamt
    int gy = schuesse.count()*gewebe.gh;
    if (blatteinzug.visible) gy += gheight + blatteinzug.gh;
    if (einzug.visible || aufknuepfung.visible)
        gy += gheight + schaefte.count()*einzug.gh;
    if (kettfarben.visible) gy += gheight + kettfarben.gh;

    // Benötigte Seitenzahl um alles zu drucken
    int px = gx/pwidth;
    int py = gy/pheight;
    if ((gx%pwidth)!=0) px++;
    if ((gy%pheight)!=0) py++;
    pages = px*py;

    if (pages<_page) return false;

    // Nun will ich die Seite _page drucken.
    // Die "Koordinaten" dieser Seite sind:
    int x = (_page-1) % px;
    int y = (_page-1) / px;

    // Dementsprechend muss ich von links und rechts her
    // die nicht zu druckenden Teile "entfernen".
    int i1 = x*pwidth; // soviel Pixel müssen links weg
    int i2 = gx - (x+1)*pwidth; // und soviele rechts
    if (i2<0) i2 = 0;

    // links
    if (i1>0) {
        // Gewebe kappen
        if (gewebe.visible) {
            if (i1>kette.count()*gewebe.gw) {
                i1 -= kette.count()*gewebe.gw;
                kette.a = kette.b = 0;
                gewebe.visible = false;
                einzug.visible = false;
                blatteinzug.visible = false;
                kettfarben.visible = false;
                // Tritte kappen
                if (i1>0) {
                    dbw3_assert(tritte.count()*trittfolge.gw>i1);
                    tritte.a += i1/trittfolge.gw;
                }
            } else {
                kette.a += i1/gewebe.gw;
            }
        }
    }
    // rechts
    if (i2>0) {
        // Schussfarben kappen
        if (schussfarben.visible) {
            if (i2>=2*schussfarben.gw) i2-=2*schussfarben.gw;
            else i2 = 0;
            schussfarben.visible = false;
        }
        // Trittfolge kappen
        if (i2>0) {
            if (i2>tritte.count()*trittfolge.gw) {
                i2 -= tritte.count()*trittfolge.gw;
                tritte.a = tritte.b = 0;
                trittfolge.visible = false;
                aufknuepfung.visible = false;
                // Gewebe teilweise kappen
                if (i2>0) {
                    dbw3_assert(kette.count()*gewebe.gw > i2);
                    kette.b -= i2/gewebe.gw;
                }
            } else {
                tritte.b -= i2/trittfolge.gw;
            }
        }
    }

    // Und genauso von oben und unten her.
    int j1 = y*pheight; // soviel Pixel müssen oben weg
    int j2 = gy - (y+1)*pheight; // und soviele unten
    if (j2<0) j2 = 0;

    if (!mainfrm->einzugunten) {
        // oben
        if (j1>0) {
            // Kettfarben kappen
            if (kettfarben.visible) {
                if (j1>2*kettfarben.gh) j1 -= 2*kettfarben.gh;
                else j1 = 0;
                kettfarben.visible = false;
            }
            // Einzug kappen
            if (j1>0) {
                if (j1>schaefte.count()*einzug.gh) {
                    j1 -= (schaefte.count()+1)*einzug.gh;
                    schaefte.a = schaefte.b = 0;
                    einzug.visible = false;
                    aufknuepfung.visible = false;
                    if (j1>0) {
                        // Blatteinzug kappen
                        if (blatteinzug.visible) {
                            if (j1>2*blatteinzug.gh) j1 -= 2*blatteinzug.gh;
                            else j1 = 0;
                            blatteinzug.visible = false;
                        }
                        // Gewebe teilweise kappen
                        if (j1>0) {
                            dbw3_assert(schuesse.count()*gewebe.gh>j1);
                            schuesse.b -= j1/gewebe.gh;
                        }
                    }
                } else {
                    schaefte.b -= j1/einzug.gh;
                }
            }
        }
        // unten
        if (j2>0) {
            // Gewebe kappen
            if (gewebe.visible) {
                if (j2>schuesse.count()*gewebe.gh) {
                    j2 -= (schuesse.count()+1)*gewebe.gh;
                    schuesse.a = schuesse.b = 0;
                    gewebe.visible = false;
                    trittfolge.visible = false;
                    schussfarben.visible = false;
                    // Blatteinzug
                    if (blatteinzug.visible) {
                        if (j2>2*blatteinzug.gh) j2-=2*blatteinzug.gh;
                        else j2=0;
                        blatteinzug.visible = false;
                    }
                    // Einzug kappen
                    if (j2>0) {
                        dbw3_assert(schaefte.count()*einzug.gh > j2);
                        schaefte.a += j2/einzug.gh;
                    }
                } else {
                    schuesse.a += j2/gewebe.gh;
                }
            }
        }
    } else {
        // Einzug unten...

        // oben
        if (j1>0) {
            // Gewebe kappen
            if (j1>0) {
                if (j1>schuesse.count()*gewebe.gh) {
                    j1 -= (schuesse.count()+1)*gewebe.gh;
                    schuesse.a = schuesse.b = 0;
                    gewebe.visible = false;
                    trittfolge.visible = false;
                    schussfarben.visible = false;
                    if (j1>0) {
                        // Blatteinzug kappen
                        if (blatteinzug.visible) {
                            if (j1>2*blatteinzug.gh) j1 -= 2*blatteinzug.gh;
                            else j1 = 0;
                            blatteinzug.visible = false;
                        }
                        // Einzug kappen
                        if (j1>(schaefte.count()+1)*einzug.gh) {
                            j1 -= (schaefte.count()+1)*einzug.gh;
                            einzug.visible = false;
                            aufknuepfung.visible = false;
                            // Kettfarben kappen
                            if (kettfarben.visible) {
                                if (j1>2*kettfarben.gh)
                                	kettfarben.visible = false;
                            }
                        } else {
                            schaefte.b -= j1/einzug.gh;
                        }
                    }
                } else {
                    schuesse.b -= j1/gewebe.gh;
                }
            }
        }
        // unten
        if (j2>0) {
            // Kettfarben kappen
            if (kettfarben.visible) {
                if (j2 > 2*kettfarben.gh) {
                    j2 -= 2*kettfarben.gh;
                    kettfarben.visible = false;
                    // Einzug kappen
                    if (einzug.visible) {
                        if (j2>schaefte.count()*einzug.gh) {
                            j2 -= (schaefte.count()+1)*einzug.gh;
                            schaefte.a = schaefte.b = 0;
                            einzug.visible = false;
                            aufknuepfung.visible = false;
                            // Blatteinzug
                            if (blatteinzug.visible) {
                                if (j2>2*blatteinzug.gh) j2-=2*blatteinzug.gh;
                                else j2=0;
                                blatteinzug.visible = false;
                            }
                            // Gewebe kappen
                            if (j2>0) {
                                dbw3_assert(schuesse.count()*gewebe.gh > j2);
                                schuesse.a += j2/gewebe.gh;
                            }
                        } else {
                            schaefte.a += j2/einzug.gh;
                        }
                    }
                }
            }
        }
    }

    return _page>0 && _page<=pages;
}
/*-----------------------------------------------------------------*/

