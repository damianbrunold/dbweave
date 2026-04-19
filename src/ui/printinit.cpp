/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Page-layout calculations — verbatim port of legacy
    printinit.cpp. All arithmetic is in device pixels (printer
    coordinates); the margins struct on TDBWFRM stores mm-tenths
    (1 unit == 0.1 mm). */

#include "print.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "assert_compat.h"

/*-----------------------------------------------------------------*/
void PrPrinter::CalcBaseDimensions ()
{
	const int sx = dpiX();
	const int sy = dpiY();

	/*  mm-tenths → device pixels:  px = mmTenths * 10 * dpi / 254.
	    (254 = 25.4 mm/inch × 10 mm-tenths/mm.) */
	mleft   = mainfrm->borders.range.left   * 10 * sx / 254;
	mtop    = (mainfrm->borders.range.top    + mainfrm->header.height) * 10 * sy / 254;
	mright  = mainfrm->borders.range.right  * 10 * sx / 254;
	mbottom = (mainfrm->borders.range.bottom + mainfrm->footer.height) * 10 * sy / 254;

	y_header = mainfrm->borders.range.top * 10 * sy / 254;
	y_footer = pageHeight() - (mainfrm->borders.range.bottom + mainfrm->footer.height) * 10 * sy / 254;

	pwidth  = pageWidth()  - mleft - mright;
	pheight = pageHeight() - mtop  - mbottom;
	x_page  = mleft;
	y_page  = mtop;

	const int gw = 20 * sx / 254;   /* 2.0 mm */
	const int gh = 20 * sy / 254;

	/*  Zoom-derived scale preserves legacy behaviour (current zoom
	    mapped against zoom[3] baseline). Legacy array values kept
	    here locally; the port doesn't expose them on TDBWFRM. */
	static constexpr int zoomTable[10] = { 5, 7, 9, 11, 13, 15, 17, 19, 21, 23 };
	const int zoomCur  = zoomTable[mainfrm->currentzoom];
	const int zoomBase = zoomTable[3];
	gwidth  = gw * zoomCur / zoomBase;
	gheight = gh * zoomCur / zoomBase;

	if (mainfrm->faktor_schuss > mainfrm->faktor_kette) {
		const int gridheight = gh * int(double(zoomCur) * mainfrm->faktor_schuss / mainfrm->faktor_kette) / zoomBase;
		einzug.gw       = gwidth;  einzug.gh       = gheight;
		aufknuepfung.gw = gwidth;  aufknuepfung.gh = gheight;
		kettfarben.gw   = gwidth;  kettfarben.gh   = gheight;
		blatteinzug.gw  = gwidth;  blatteinzug.gh  = gheight;
		trittfolge.gw   = gwidth;  trittfolge.gh   = gridheight;
		schussfarben.gw = gwidth;  schussfarben.gh = gridheight;
		gewebe.gw       = gwidth;  gewebe.gh       = gridheight;
	} else {
		const int gridwidth = gw * int(double(zoomCur) * mainfrm->faktor_kette / mainfrm->faktor_schuss) / zoomBase;
		einzug.gw       = gridwidth; einzug.gh       = gheight;
		aufknuepfung.gw = gwidth;    aufknuepfung.gh = gheight;
		kettfarben.gw   = gridwidth; kettfarben.gh   = gheight;
		blatteinzug.gw  = gridwidth; blatteinzug.gh  = gheight;
		trittfolge.gw   = gwidth;    trittfolge.gh   = gheight;
		schussfarben.gw = gwidth;    schussfarben.gh = gheight;
		gewebe.gw       = gridwidth; gewebe.gh       = gheight;
	}
}

/*-----------------------------------------------------------------*/
static void addMargins (PrFeld& _f, int _mleft, int _mtop)
{
	_f.x0 += _mleft;
	_f.y0 += _mtop;
}

void PrPrinter::CalcDimensions ()
{
	if (kettfarben.visible) {
		kettfarben.width  = kette.count() * kettfarben.gw;
		kettfarben.height = kettfarben.gh;
	}
	if (einzug.visible) {
		einzug.width  = kette.count()    * einzug.gw;
		einzug.height = schaefte.count() * einzug.gh;
	}
	if (blatteinzug.visible) {
		blatteinzug.width  = kette.count() * blatteinzug.gw;
		blatteinzug.height = blatteinzug.gh;
	}
	if (gewebe.visible) {
		gewebe.width  = kette.count()    * gewebe.gw;
		gewebe.height = schuesse.count() * gewebe.gh;
	}
	if (trittfolge.visible) {
		trittfolge.width  = tritte.count()   * trittfolge.gw;
		trittfolge.height = schuesse.count() * trittfolge.gh;
	}
	if (aufknuepfung.visible) {
		aufknuepfung.width  = tritte.count()   * aufknuepfung.gw;
		aufknuepfung.height = schaefte.count() * aufknuepfung.gh;
	}
	if (schussfarben.visible) {
		schussfarben.width  = schussfarben.gw;
		schussfarben.height = schuesse.count() * schussfarben.gh;
	}

	const int xabstand = gwidth;
	const int yabstand = gheight;

	kettfarben.x0   = 0;
	einzug.x0       = kettfarben.x0;
	blatteinzug.x0  = kettfarben.x0;
	gewebe.x0       = kettfarben.x0;
	if (gewebe.visible) trittfolge.x0 = gewebe.x0 + gewebe.width + xabstand;
	else                trittfolge.x0 = einzug.x0 + einzug.width + xabstand;
	aufknuepfung.x0 = trittfolge.x0;
	schussfarben.x0 = trittfolge.x0 + trittfolge.width;
	if (trittfolge.visible) schussfarben.x0 += xabstand;

	if (mainfrm->einzugunten) {
		gewebe.y0      = 0;
		blatteinzug.y0 = gewebe.y0 + gewebe.height;
		if (gewebe.visible) blatteinzug.y0 += yabstand;
		einzug.y0      = blatteinzug.y0 + blatteinzug.height;
		if (blatteinzug.visible) einzug.y0 += yabstand;
		kettfarben.y0  = einzug.y0 + einzug.height;
		if (einzug.visible) kettfarben.y0 += yabstand;

		trittfolge.y0   = gewebe.y0;
		aufknuepfung.y0 = trittfolge.y0 + trittfolge.height;
		if (trittfolge.visible) aufknuepfung.y0 += yabstand;
		if (blatteinzug.visible || (mainfrm->ViewBlatteinzug && mainfrm->ViewBlatteinzug->isChecked() && trittfolge.visible))
			aufknuepfung.y0 += blatteinzug.gh + yabstand;

		schussfarben.y0 = gewebe.y0;
	} else {
		kettfarben.y0  = 0;
		einzug.y0      = kettfarben.y0 + kettfarben.height;
		if (kettfarben.visible) einzug.y0 += yabstand;
		blatteinzug.y0 = einzug.y0 + einzug.height;
		if (einzug.visible) blatteinzug.y0 += yabstand;
		gewebe.y0      = blatteinzug.y0 + blatteinzug.height;
		if (blatteinzug.visible) gewebe.y0 += yabstand;

		aufknuepfung.y0 = einzug.y0;
		trittfolge.y0   = aufknuepfung.y0 + aufknuepfung.height;
		if (aufknuepfung.visible) trittfolge.y0 += yabstand;
		if (blatteinzug.visible || (mainfrm->ViewBlatteinzug && mainfrm->ViewBlatteinzug->isChecked() && aufknuepfung.visible))
			trittfolge.y0 += blatteinzug.gh + yabstand;

		schussfarben.y0 = gewebe.y0;
	}

	addMargins(kettfarben,   mleft, mtop);
	addMargins(einzug,       mleft, mtop);
	addMargins(blatteinzug,  mleft, mtop);
	addMargins(gewebe,       mleft, mtop);
	addMargins(aufknuepfung, mleft, mtop);
	addMargins(trittfolge,   mleft, mtop);
	addMargins(schussfarben, mleft, mtop);
}

/*-----------------------------------------------------------------*/
void PrPrinter::InitVisibility ()
{
	kettfarben.Init();
	einzug.Init();
	blatteinzug.Init();
	aufknuepfung.Init();
	trittfolge.Init();
	schussfarben.Init();
	gewebe.Init();

	const bool viewFarbe  = mainfrm->ViewFarbe        && mainfrm->ViewFarbe->isChecked();
	const bool viewEinzug = mainfrm->ViewEinzug       && mainfrm->ViewEinzug->isChecked();
	const bool viewBlatt  = mainfrm->ViewBlatteinzug  && mainfrm->ViewBlatteinzug->isChecked();
	const bool viewTritt  = mainfrm->ViewTrittfolge   && mainfrm->ViewTrittfolge->isChecked();

	if (viewFarbe  && kette.count() > 0)                           kettfarben.visible   = true;
	if (viewEinzug && kette.count() > 0 && schaefte.count() > 0)   einzug.visible       = true;
	if (viewBlatt  && kette.count() > 0)                           blatteinzug.visible  = true;
	if (kette.count() > 0 && schuesse.count() > 0)                 gewebe.visible       = true;
	if (viewTritt  && schuesse.count() > 0 && tritte.count() > 0)  trittfolge.visible   = true;
	if (viewEinzug && viewTritt && schaefte.count() > 0 && tritte.count() > 0) aufknuepfung.visible = true;
	if (viewFarbe  && schuesse.count() > 0)                        schussfarben.visible = true;
}

/*-----------------------------------------------------------------*/
bool PrPrinter::ClipToPage (int _page)
{
	InitVisibility();

	int gx = kette.count() * gewebe.gw;
	if (trittfolge.visible || aufknuepfung.visible)
		gx += gwidth + tritte.count() * trittfolge.gw;
	if (schussfarben.visible) gx += gwidth + schussfarben.gw;

	int gy = schuesse.count() * gewebe.gh;
	if (blatteinzug.visible) gy += gheight + blatteinzug.gh;
	if (einzug.visible || aufknuepfung.visible)
		gy += gheight + schaefte.count() * einzug.gh;
	if (kettfarben.visible) gy += gheight + kettfarben.gh;

	int px = gx / pwidth;
	int py = gy / pheight;
	if ((gx % pwidth)  != 0) px++;
	if ((gy % pheight) != 0) py++;
	if (px < 1) px = 1;
	if (py < 1) py = 1;
	pages = px * py;

	if (pages < _page) return false;

	const int x = (_page - 1) % px;
	const int y = (_page - 1) / px;

	int i1 = x * pwidth;
	int i2 = gx - (x + 1) * pwidth;
	if (i2 < 0) i2 = 0;

	/*  Clip from the left. */
	if (i1 > 0 && gewebe.visible) {
		if (i1 > kette.count() * gewebe.gw) {
			i1 -= kette.count() * gewebe.gw;
			kette.a = kette.b = 0;
			gewebe.visible      = false;
			einzug.visible      = false;
			blatteinzug.visible = false;
			kettfarben.visible  = false;
			if (i1 > 0) {
				dbw3_assert(tritte.count() * trittfolge.gw > i1);
				tritte.a += i1 / trittfolge.gw;
			}
		} else {
			kette.a += i1 / gewebe.gw;
		}
	}
	/*  Clip from the right. */
	if (i2 > 0) {
		if (schussfarben.visible) {
			if (i2 >= 2 * schussfarben.gw) i2 -= 2 * schussfarben.gw;
			else i2 = 0;
			schussfarben.visible = false;
		}
		if (i2 > 0) {
			if (i2 > tritte.count() * trittfolge.gw) {
				i2 -= tritte.count() * trittfolge.gw;
				tritte.a = tritte.b = 0;
				trittfolge.visible   = false;
				aufknuepfung.visible = false;
				if (i2 > 0) {
					dbw3_assert(kette.count() * gewebe.gw > i2);
					kette.b -= i2 / gewebe.gw;
				}
			} else {
				tritte.b -= i2 / trittfolge.gw;
			}
		}
	}

	int j1 = y * pheight;
	int j2 = gy - (y + 1) * pheight;
	if (j2 < 0) j2 = 0;

	if (!mainfrm->einzugunten) {
		/*  Standard layout. */
		if (j1 > 0) {
			if (kettfarben.visible) {
				if (j1 > 2 * kettfarben.gh) j1 -= 2 * kettfarben.gh;
				else j1 = 0;
				kettfarben.visible = false;
			}
			if (j1 > 0) {
				if (j1 > schaefte.count() * einzug.gh) {
					j1 -= (schaefte.count() + 1) * einzug.gh;
					schaefte.a = schaefte.b = 0;
					einzug.visible = false;
					aufknuepfung.visible = false;
					if (j1 > 0 && blatteinzug.visible) {
						if (j1 > 2 * blatteinzug.gh) j1 -= 2 * blatteinzug.gh;
						else j1 = 0;
						blatteinzug.visible = false;
					}
					if (j1 > 0) {
						dbw3_assert(schuesse.count() * gewebe.gh > j1);
						schuesse.b -= j1 / gewebe.gh;
					}
				} else {
					schaefte.b -= j1 / einzug.gh;
				}
			}
		}
		if (j2 > 0 && gewebe.visible) {
			if (j2 > schuesse.count() * gewebe.gh) {
				j2 -= (schuesse.count() + 1) * gewebe.gh;
				schuesse.a = schuesse.b = 0;
				gewebe.visible       = false;
				trittfolge.visible   = false;
				schussfarben.visible = false;
				if (blatteinzug.visible) {
					if (j2 > 2 * blatteinzug.gh) j2 -= 2 * blatteinzug.gh;
					else j2 = 0;
					blatteinzug.visible = false;
				}
				if (j2 > 0) {
					dbw3_assert(schaefte.count() * einzug.gh > j2);
					schaefte.a += j2 / einzug.gh;
				}
			} else {
				schuesse.a += j2 / gewebe.gh;
			}
		}
	} else {
		/*  Einzug-unten layout (legacy einzugunten). */
		if (j1 > 0) {
			if (j1 > schuesse.count() * gewebe.gh) {
				j1 -= (schuesse.count() + 1) * gewebe.gh;
				schuesse.a = schuesse.b = 0;
				gewebe.visible       = false;
				trittfolge.visible   = false;
				schussfarben.visible = false;
				if (j1 > 0 && blatteinzug.visible) {
					if (j1 > 2 * blatteinzug.gh) j1 -= 2 * blatteinzug.gh;
					else j1 = 0;
					blatteinzug.visible = false;
				}
				if (j1 > (schaefte.count() + 1) * einzug.gh) {
					j1 -= (schaefte.count() + 1) * einzug.gh;
					einzug.visible       = false;
					aufknuepfung.visible = false;
					if (kettfarben.visible && j1 > 2 * kettfarben.gh)
						kettfarben.visible = false;
				} else {
					schaefte.b -= j1 / einzug.gh;
				}
			} else {
				schuesse.b -= j1 / gewebe.gh;
			}
		}
		if (j2 > 0 && kettfarben.visible && j2 > 2 * kettfarben.gh) {
			j2 -= 2 * kettfarben.gh;
			kettfarben.visible = false;
			if (einzug.visible) {
				if (j2 > schaefte.count() * einzug.gh) {
					j2 -= (schaefte.count() + 1) * einzug.gh;
					schaefte.a = schaefte.b = 0;
					einzug.visible       = false;
					aufknuepfung.visible = false;
					if (blatteinzug.visible) {
						if (j2 > 2 * blatteinzug.gh) j2 -= 2 * blatteinzug.gh;
						else j2 = 0;
						blatteinzug.visible = false;
					}
					if (j2 > 0) {
						dbw3_assert(schuesse.count() * gewebe.gh > j2);
						schuesse.a += j2 / gewebe.gh;
					}
				} else {
					schaefte.a += j2 / einzug.gh;
				}
			}
		}
	}

	return _page > 0 && _page <= pages;
}
